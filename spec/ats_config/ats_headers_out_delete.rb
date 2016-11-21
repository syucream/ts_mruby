require '../support/factory_girl_helper'
include FactoryGirlHelper

# generate template
puts <<EOS
class Handler
  def on_read_response_hdr
    hout = ATS::Headers_out.new
    hout["#{get(:headers_out_delete).key}"] = "dummy"
    hout.delete("#{get(:headers_out_delete).key}")
  end
end
es = ATS::EventSystem.new
es.register Handler
EOS
