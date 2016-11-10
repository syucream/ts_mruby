require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
template = "#{get(:request_setters).template}"

req = ATS::Request.new
req.args     = "#{get(:request_setters).values['ATS::Request#args']}"
req.hostname = "#{get(:request_setters).values['ATS::Request#hostname']}"
req.protocol = "#{get(:request_setters).values['ATS::Request#protocol']}"
req.scheme   = "#{get(:request_setters).values['ATS::Request#scheme']}"

ATS::echo template % {
  args: req.args,
  hostname: req.hostname,
  protocol: req.protocol,
  scheme: req.scheme,
}

EOS
