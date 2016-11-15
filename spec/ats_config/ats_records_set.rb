require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
records = ATS::Records.new
records.set ATS::Records::#{get(:records_set).key}, #{get(:records_set).value}
EOS
