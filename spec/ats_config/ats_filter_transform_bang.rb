require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
f = ATS::Filter.new
f.transform! do |body|
  body + "#{get(:transform_bang).txt}"
end
EOS
