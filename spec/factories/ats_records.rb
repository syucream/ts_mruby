class RecordsGet
  attr_accessor :path, :key, :value
end
class RecordsSet
  attr_accessor :path, :key, :value
end

# NOTE: Expect 'proxy.config.http.insert_response_via_str' is 0 by default
FactoryGirl.define do
  factory :records_get do
    path '/ats_records_get'
    key 'TS_CONFIG_HTTP_INSERT_RESPONSE_VIA_STR'
    value 0
  end

  factory :records_set do
    path '/ats_records_set'
    key 'TS_CONFIG_HTTP_INSERT_RESPONSE_VIA_STR'
    value 2
  end
end
