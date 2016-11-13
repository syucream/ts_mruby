require './spec_helper'

describe 'ATS::Filter class', :js => false do
  let (:register) { get(:eventsystem_register) }

  describe 'register' do
    it 'sets eventhandlers' do
      visit register.path

      hdr = page.response_headers
      send_request = register.send_request_hdr
      read_response = register.read_response_hdr
      send_response = register.send_response_hdr
      expect(hdr[send_request['key']]).to eq send_request['value']
      expect(hdr[read_response['key']]).to eq read_response['value']
      expect(hdr[send_response['key']]).to eq send_response['value']
    end
  end

end
