require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
ATS::echo "ATS::return test"
ATS::return #{get(:return).rc_class}
EOS
