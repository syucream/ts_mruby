require './spec_helper'

describe 'ATS::Headers_out class', :js => false do
  let (:getter) { get(:headers_out_get) }
  let (:setter) { get(:headers_out_set) }

  describe '[]' do
    it 'gets specified response header field value' do
      visit getter.path
      expect(page.body).to match(/^#{getter.key}: .+$/)
    end
  end

  describe '[]=' do
    it 'set a value to specified response header field' do
      visit setter.path
      expect(page.response_headers[setter.key]).to eq setter.value
    end
  end
end
