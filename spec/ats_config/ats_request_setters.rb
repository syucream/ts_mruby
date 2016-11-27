require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
template = "#{get(:request_setters).template}"

req = ATS::Request.new
req.args   = "#{get(:request_setters).values['ATS::Request#args']}"
req.uri    = "#{get(:request_setters).values['ATS::Request#uri']}"
req.method = "#{get(:request_setters).values['ATS::Request#method']}"

ATS::echo template % {
  args: req.args,
  uri: req.uri,
  method: req.method,
}

EOS
