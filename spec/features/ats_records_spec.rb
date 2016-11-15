require './spec_helper'

describe 'ATS::Records class', :js => false do
  let(:records_get) { get(:records_get) }
  let(:records_set) { get(:records_set) }

  describe 'get' do
    it 'returns config values set to current transaction' do
      visit records_get.path
      expect(page).to have_text records_get.value
    end
  end

  # NOTE: Expect 'proxy.config.http.insert_response_via_str' is 0 by default
  describe 'set' do
    it 'set config values to current transaction' do
      visit records_set.path
      expect(page.response_headers['Via']).not_to be_nil
    end
  end
end
