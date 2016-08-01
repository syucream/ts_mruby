class Origin
  attr_accessor :path, :body
end

FactoryGirl.define do
  factory :origin do
    path '/origin'
    body 'Dummy response from self-hosted origin server'
  end
end
