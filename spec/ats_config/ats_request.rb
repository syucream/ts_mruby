require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
template = "#{get(:request).template}"
req = ATS::Request.new
ATS::echo template % {hostname: req.hostname, url: req.url}
EOS
