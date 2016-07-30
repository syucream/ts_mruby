class Rputs
  attr_accessor :path, :body
end
class Echo
  attr_accessor :path, :body
end

FactoryGirl.define do
  factory :rputs do
    path '/ats_rputs'
    body 'ATS::rputs test'
  end

  factory :echo do
    path '/ats_echo'
    body 'ATS::echo test'
  end
end
