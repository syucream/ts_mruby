require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
ATS::echo "#{get(:echo).body}"
EOS
