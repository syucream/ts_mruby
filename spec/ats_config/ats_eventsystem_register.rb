require '../support/factory_girl_helper'
include FactoryGirlHelper

fixtures = get(:eventsystem_register)

# generate template
puts <<EOS
class Handler
  def on_send_request_hdr
    hout = ATS::Headers_out.new
    hout["#{fixtures.send_request_hdr['key']}"] = "#{fixtures.send_request_hdr['value']}"
  end

  def on_read_response_hdr
    hout = ATS::Headers_out.new
    hout["#{fixtures.read_response_hdr['key']}"] = "#{fixtures.read_response_hdr['value']}"
  end

  def on_send_response_hdr
    hout = ATS::Headers_out.new
    hout["#{fixtures.send_response_hdr['key']}"] = "#{fixtures.send_response_hdr['value']}"
  end
end

es = ATS::EventSystem.new
es.register Handler
EOS
