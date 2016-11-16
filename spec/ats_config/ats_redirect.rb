require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
ATS::redirect "#{get(:redirect).host}", #{get(:redirect).rc_class}
EOS
