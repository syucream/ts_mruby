class ServerEq
  attr_accessor :path, :server
end

FactoryGirl.define do
  factory :server_eq do
    path '/ats_upstream_server_eq'
    server 'www.google.com'
  end
end
