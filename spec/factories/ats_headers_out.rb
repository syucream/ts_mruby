class HeadersOutGet
  attr_accessor :path, :key
end

FactoryGirl.define do
  factory :headers_out_get do
    path '/ats_headers_out_get'
    key 'Server'
  end
end
