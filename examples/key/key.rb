# 2.2.2. Failing Parameter Processing
class FailingParameterProcessing < StandardError; end

# 2.2. Calculating a Secondary Cache Key
def calculate_key(key_value, headers)
  # 4)
  key_list = key_value.split(',')

  # 5)
  seckey = key_list.map do |key_item|
    # 5-2)
    fname_index = key_item.index(';')
    raise FailingParameterProcessing if fname_index.nil?

    # 5-3)
    field_name = key_item[0...fname_index].gsub(/^\s*(.+)\s*$/){$1}

    # 5-4)
    field_value = headers[field_name]

    # 5-5)
    parameters = key_item[fname_index+1..key_item.length].gsub(/^\s*(.+)\s*$/){$1}

    # 5-6)
    param_list = parameters.split(/(?!");(?!")/)

    # 5-7)
    param_list.map do |parameter|
      pair = parameter.gsub(/^\s*(.+)\s*$/){$1}.split('=')
      next if pair.length != 2
      param_name = pair[0].downcase
      # param_value = pair[1].gsub(/^\"(.+)\"$/){ $1 }
      # XXX Alternatively process to avoid ATS bug.
      param_value = pair[1].gsub(/^\"(.+)$/){ $1 }

      calculate_keyparam_(param_name, param_value, field_value)
    end.join('')
  end
  
  # Append a separator character (e.g., NULL)
  seckey.join("\000")
end

#
# 2.3. Key Parameters
#
# return a part of secondary cache key from given pair of parameter and field_value
#
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
        header_list.push(header_item_tmp2.gsub(/^\s(.+)\s$/){$1})
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

# Get request headers converted to lower case
def get_headers
  hin = ATS::Headers_in.new
  hin.all.reduce({}) do |memo, pair| 
    memo[pair.first.downcase] = pair[1]; memo
  end
end

req = ATS::Request.new
url = "#{req.scheme}://#{req.hostname}#{req.uri}#{req.args}"
redis = Redis.new '127.0.0.1', 6789
key_value = redis.hget url, 'key'
p key_value

if !key_value.nil?
  seckey = ''
  begin
    seckey = calculate_key(key_value, get_headers())
  rescue FailingParameterProcessing
    # 2.2.2. Failing Parameter Processing
    # behave as if the Key header was not present ...
    return
  end
  p seckey

  # 1. get genid related to the secondary cache key
  # NOTE: Do hsetnx with '-1' to detect weather the seckey is already set
  genid = ''
  if redis.hsetnx url, seckey, '-1'
    max_genid = redis.hincrby url, 'max-genid', 1
    redis.hset url, seckey, max_genid.to_s
    genid = max_genid.to_s
  else
    genid = redis.hget url, seckey
  end
  p genid

  # Set secondary cache key by using cache generation
  records = ATS::Records.new
  records.set ATS::Records::TS_CONFIG_HTTP_CACHE_GENERATION, genid.to_i
end

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
