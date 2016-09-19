# 2.2. Calculating a Secondary Cache Key
# Its parsing part.
def parse_key(key)
  # 4)
  key_values = key.split(',')

  # 5)
  parsed = {}
  key_values.map do |key_value|
    params = key_value.split(';')
    next if params.length <= 1

    field_name = params[0].gsub(' ', '')

    parameters = []
    params[1..params.length].each do |param|
      if param =~ / *(.+) *= *(.+) */
        parameters.push($1.downcase => $2)
      end
    end

    parsed[field_name.downcase] = parameters
  end

  parsed
end

# 2.2. Calculating a Secondary Cache Key
# Its calculating part.
def calculate_key(headers, parsed_list)
  secondary_key = ''

  parsed_list.each do |field_name, parameters|
    value = headers[field_name]
    value = '' if value.nil?

    key_tmp = ''
    parameters.each do |param|
      key_tmp += calculate_each_(value, param)
    end
    secondary_key += key_tmp
  end

  secondary_key
end

def calculate_each_(value, param)
  key_tmp = ''

  # TODO 2.3.1. div
  # TODO 2.3.2. partition
  # TODO 2.3.3. match

  # 2.3.4. substr
  if param.has_key?('substr')
    if !value.empty?
      key_tmp = value.include?(param['substr']) ? '1' : '0'
    else
      key_tmp = 'none'
    end
  end

  # TODO 2.3.5. param

  # Append a separator character (e.g., NULL)
  key_tmp + "\000"
end


hin = ATS::Headers_in.new
headers_tmp = hin.all
headers = headers_tmp.reduce({}) do |memo, pair| 
  memo[pair.first.downcase] = pair[1]
  memo
end

# k1 = 'user-agent;substr=MSIE'
# k1_params = parse_key(k1)
# p calculate_key(headers, k1_params)
# k2 = 'user-agent;substr=MSIE;Substr="mobile", Cookie;param="ID"'
# k2_params = parse_key(k2)
# p calculate_key(headers, k2_params)

# TODO Enable to get full URL
# req = ATS::Request.new
# path = req.uri + req.args
url = 'http://127.0.0.1:8080/'

redis = Redis.new '127.0.0.1', 6789
key = redis.hget url, 'key'
k_params = parse_key(key)
sec_key = calculate_key(headers, k_params)

sec_genid = redis.hget url, sec_key
if sec_genid.nil?
  # TODO implement to mruby-redis
  # max_genid = redis.hincrby url, 'max-genid', 1
  sec_genid = '1'
  redis.hset url, sec_key, sec_genid
end
p sec_genid

# Set secondary cache key by using cache generation
records = ATS::Records.new
records.set TS_CONFIG_HTTP_CACHE_GENERATION, sec_genid.to_i
