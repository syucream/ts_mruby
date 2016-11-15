require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
template = "#{get(:request).template}"
req = ATS::Request.new
ATS::echo template % {
  args: req.args,
  hostname: req.hostname,
  protocol: req.protocol,
  scheme: req.scheme,
  unparsed_uri: req.unparsed_uri,
  url: req.url,
}
EOS
