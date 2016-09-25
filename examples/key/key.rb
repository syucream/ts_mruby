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
    field_name = key_item[0...fname_index]

    # 5-4)
    field_value = headers[field_name]

    # 5-5)
    parameters = key_item[fname_index+1..key_item.length]

    # 5-6)
    pi = 0; ci = 0
    param_list = []
    while ci = parameters.index(';', ci); !ci.nil?
      # excepting ";" characters within quoted strings
      if ci < parameters.length && parameters[ci-1] == '\"' && parameters[ci+1] = '\"'
        next
      end
      param_list.push(parameters[pi..ci])
      pi = ci + 1
    end
    param_list.push(parameters[pi..parameters.length-1])

    # 5-7)
    param_list.map do |parameter|
      pair = parameter.split('=')
      next if pair.length != 2
      param_name = pair[0].gsub(' ', '').downcase
      param_value = pair[1].gsub(' ', '')

      # If the first and last characters of "param_value" are both DQUOTE:
      if param_value[0] == '"' && param_value[param_value.length-1] == '"'
        param_value[0] = '\\'
        param_value.chop!
      end

      calculate_each_(param_name, param_value, field_value)
    end.join('')
  end
  
  # Append a separator character (e.g., NULL)
  seckey.join("\000")
end

# 2.3. Key Parameters
def calculate_each_(param_name, param_value, field_value)
  key_tmp = ''

  # TODO 2.3.1. div
  # TODO 2.3.2. partition
  # TODO 2.3.3. match

  # 2.3.4. substr
  if param_name
    if !field_value.empty?
      key_tmp = field_value.include?(param_value) ? '1' : '0'
    else
      key_tmp = 'none'
    end
  end

  # TODO 2.3.5. param

  key_tmp
end

# Get request headers converted to lower case
def get_headers
  hin = ATS::Headers_in.new
  hin.all.reduce({}) do |memo, pair| 
    memo[pair.first.downcase] = pair[1]
    memo
  end
end

# TODO Enable to get full URL
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
  # TODO These need transaction!
  sec_genid = redis.hget url, seckey
  if sec_genid.nil?
    max_genid = redis.hincrby url, 'max-genid', 1
    redis.hset url, seckey, max_genid.to_s

    sec_genid = max_genid.to_s
  end
  p sec_genid

  # Set secondary cache key by using cache generation
  records = ATS::Records.new
  records.set ATS::Records::TS_CONFIG_HTTP_CACHE_GENERATION, sec_genid.to_i
end

#
# Register 'Key' header field value on read_response_hdr hook
# TODO I should replace hset with hsetnx ...
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
      redis.hset url, 'key', key unless redis.hexists? url, 'key'
    end
  end

  def on_send_response_hdr; end
end
es = ATS::EventSystem.new
es.register KeyHeaderHandler
