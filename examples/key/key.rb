#
# Process 'Key' header field value and set secondary cache key
#
class KeyProcessor
  def initialize(host, port)
    @redis = Redis.new host, port

    req = ATS::Request.new
    @url = "#{req.scheme}://#{req.hostname}#{req.uri}#{req.args}"
  end

  #
  # Calculate and set secondary cache key
  #
  def process
    key_value = get_key_value_
    puts "Key header field value: #{key_value}"

    if !key_value.nil?
      begin
        seckey = calculate_key_(key_value, get_headers_)
        puts "Secondary cache key: #{seckey}"

        genid = generate_generation_id_(seckey)
        puts "Genid: #{genid}"

        set_generation_id_(genid)
      rescue FailingParameterProcessing
        # 2.2.2. Failing Parameter Processing
        # behave as if the Key header was not present ...
        return
      end
    end
  end

  private

  # Get 'key' field value from Redis
  def get_key_value_
    @redis.hget @url, 'key'
  end

  # Get request headers converted to lower case
  def get_headers_
    hin = ATS::Headers_in.new
    hin.all.reduce({}) do |memo, pair| 
      memo[pair.first.downcase] = pair[1]; memo
    end
  end

  # 2.2. Calculating a Secondary Cache Key
  def calculate_key_(key_value, headers)
    # 4)
    key_list = key_value.split(',')

    # 5)
    key_list.map do |key_item|
      # 5-2)
      fname_index = key_item.index(';')
      raise FailingParameterProcessing if fname_index.nil?

      # 5-3)
      field_name = key_item[0...fname_index].gsub(/^\s*(.+)\s*$/){$1}

      # 5-4)
      # XXX Alternatively call below gsub to avoid ATS bug.
      field_value = headers[field_name].gsub(/^(.*)\"(.+)$/){$1 + '"' + $2 + '"'}
      field_value = '' if field_value.nil?

      # 5-5)
      parameters = key_item[fname_index+1..key_item.length].gsub(/^\s*(.+)\s*$/){$1}

      # 5-6)
      param_list = parameters.split(/(?!");(?!")/)

      # 5-7)
      param_list.map do |parameter|
        pair = parameter.gsub(/^\s*(.+)\s*$/){$1}.split('=')
        next if pair.length != 2
        param_name = pair[0].downcase
        # param_value = pair[1].gsub(/^\"(.+)\"$/){$1}
        # XXX Alternatively call below gsub to avoid ATS bug.
        param_value = pair[1].gsub(/^\"(.+)$/){$1}

        calculate_keyparam_(param_name, param_value, field_value)
      end.join("\000") # Append a separator character (e.g., NULL)
    end.join('')
  end

  # 2.3. Key Parameters
  def calculate_keyparam_(param_name, param_value, field_value)
    case param_name
    when 'div'
      return 'none' if field_value.empty?
      raise FailingParameterProcessing if param_value == '0'
      if field_value =~ (/^\s*(\d+)\s*,?.*$/)
        return ($1.to_i / param_value.to_i).to_i.to_s
      else
        raise FailingParameterProcessing
      end

    when 'partition'
      return 'none' if field_value.empty?
      if field_value =~ (/^\s*(\d+)(\.\d+)?\s*,?.*$/)
        header_value = $1 + ($2.nil? ? '' : $2)
        segment_id = 0
        param_value.split(':').each do |segment_value|
          next if header_value.to_f < segment_value.to_f
          segment_id += 1
        end
        return segment_id.to_s
      else
        raise FailingParameterProcessing
      end

    when 'match'
      return 'none' if field_value.empty?
      exist = field_value.split(',').any? do |header_item|
        header_item.gsub(/^\s(.+)\s$/){$1} === param_value
      end
      return exist ? "1" : "0"

    when 'substr'
      return 'none' if field_value.empty?
      exist = field_value.split(',').any? do |header_item|
        header_item.gsub(/^\s(.+)\s$/){$1}.include?(param_value)
      end
      return exist ? "1" : "0"

    when 'param'
      header_list = []
      field_value.split(',').each do |header_item_tmp1|
        header_item_tmp1.split(';').each do |header_item_tmp2|
          header_list.push(header_item_tmp2.gsub(/^\s*(.+)\s*$/){$1})
        end
      end
      header_list.each do |header_item|
        if header_item =~ /^(.+)=(.+)$/
          return $2 if param_value === $1
        else
          next
        end
      end
      return ''

    else
      raise FailingParameterProcessing
    end
  end

  # Get genid related to the secondary cache key, from Redis
  def generate_generation_id_(seckey)
    # NOTE: Do hsetnx with '-1' to detect weather the seckey is already set
    if @redis.hsetnx @url, seckey, '-1'
      max_genid = @redis.hincrby @url, 'max-genid', 1
      @redis.hset @url, seckey, max_genid.to_s
      return max_genid.to_s
    else
      return @redis.hget @url, seckey
    end
  end

  # Set genid
  def set_generation_id_(genid)
    records = ATS::Records.new
    records.set ATS::Records::TS_CONFIG_HTTP_CACHE_GENERATION, genid.to_i
  end

  # 2.2.2. Failing Parameter Processing
  class FailingParameterProcessing < StandardError; end
end
processor = KeyProcessor.new '127.0.0.1', 6789
processor.process


#
# Register 'Key' header field value on read_response_hdr hook
#
class KeyHeaderHandler
  def on_send_request_hdr; end

  def on_read_response_hdr
    hout = ATS::Headers_out.new
    key = hout['key']

    if key
      req = ATS::Request.new
      url = "#{req.scheme}://#{req.hostname}#{req.uri}#{req.args}"

      redis = Redis.new '127.0.0.1', 6789
      redis.hsetnx url, 'key', key
    end
  end

  def on_send_response_hdr; end
end
es = ATS::EventSystem.new
es.register KeyHeaderHandler
