require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
upstream = ATS::Upstream.new
upstream.server = "#{get(:server_eq).server}"
EOS
