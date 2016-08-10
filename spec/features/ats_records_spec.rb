require './spec_helper'

describe 'ATS::Records class', :js => false do
  describe 'get' do
    it 'returns config values set to current transaction' do
      records_get = build(:records_get)

      visit records_get.path
      expect(page).to have_text records_get.value
    end
  end

  # NOTE: Expect 'proxy.config.http.insert_response_via_str' is 0 by default
  describe 'set' do
    it 'set config values to current transaction' do
      records_set = build(:records_set)

      visit records_set.path
      expect(page.response_headers['Via']).not_to be_nil
    end
  end
end
