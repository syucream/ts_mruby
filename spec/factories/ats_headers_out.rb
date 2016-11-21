class HeadersOutGet
  attr_accessor :path, :key
end
class HeadersOutSet
  attr_accessor :path, :key, :value
end

FactoryGirl.define do
  factory :headers_out_get do
    path '/ats_headers_out_get'
    key 'Server'
  end

  factory :headers_out_set do
    path '/ats_headers_out_set'
    key 'x-ts_mruby'
    value 'dummy'
  end
end
