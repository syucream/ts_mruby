class EventsystemRegister
  attr_accessor :path, :send_request_hdr, :read_response_hdr, :send_response_hdr
end

FactoryGirl.define do
  factory :eventsystem_register do
    path '/ats_eventsystem_register'

    send_request_hdr_hash = {
      'key' => 'x-on_send_request_hdr',
      'value' => '1'
    }
    read_response_hdr_hash = {
      'key' => 'x-on_read_response_hdr',
      'value' => '2'
    }
    send_response_hdr_hash = {
      'key' => 'x-on_send_response_hdr',
      'value' => '3'
    }

    send_request_hdr send_request_hdr_hash
    read_response_hdr read_response_hdr_hash
    send_response_hdr send_response_hdr_hash
  end
end
