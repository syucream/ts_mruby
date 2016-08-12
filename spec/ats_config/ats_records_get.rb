require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
records = ATS::Records.new
value = records.get ATS::Records::#{get(:records_get).key}
ATS::echo value
EOS
