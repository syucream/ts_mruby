require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
f = ATS::Filter.new
f.body = "#{get(:body_eq).body}"
EOS
