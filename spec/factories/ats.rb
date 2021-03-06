class Rputs
  attr_accessor :path, :body
end
class Echo
  attr_accessor :path, :body
end
class Return
  attr_accessor :path, :rc_class, :rc_num
end
class Redirect
  attr_accessor :path, :host, :rc_class, :rc_num
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

  factory :return do
    path '/ats_return'
    rc_class 'ATS::HTTP_OK'
    rc_num 200
  end
  
  factory :redirect do
    path '/ats_redirect'
    host 'http://www.google.com:80/'
    rc_class 'ATS::HTTP_MOVED_PERMANENTLY'
    rc_num 301
  end
end
