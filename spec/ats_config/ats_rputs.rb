require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
ATS::rputs "#{get(:rputs).body}"
EOS
