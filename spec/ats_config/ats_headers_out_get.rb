require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
class Handler
  def on_read_response_hdr
    hout = ATS::Headers_out.new
    f = ATS::Filter.new
    f.body = "#{get(:headers_out_get).key}: " + hout["#{get(:headers_out_get).key}"]
  end
end
es = ATS::EventSystem.new
es.register Handler
EOS
