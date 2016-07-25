class Rputs
  attr_accessor :path, :body
end

FactoryGirl.define do
  factory :rputs do
    path '/ats_rputs'
    body 'ATS::rputs test'
  end
end
