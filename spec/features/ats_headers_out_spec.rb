require './spec_helper'

describe 'ATS::Headers_out class', :js => false do
  let (:getter) { get(:headers_out_get) }
  let (:setter) { get(:headers_out_set) }
  let (:deleter) { get(:headers_out_delete) }
  let (:all) { get(:headers_out_all) }

  describe '[]' do
    it 'gets specified response header field value' do
      visit getter.path
      expect(page.body).to match(/^#{getter.key}: .+$/)
    end
  end

  describe '[]=' do
    it 'sets a value to specified response header field' do
      visit setter.path
      expect(page.response_headers[setter.key]).to eq setter.value
    end
  end

  describe 'delete' do
    it 'deletes specified response header field' do
      visit deleter.path
      expect(page.response_headers[deleter.key]).to be_nil
    end
  end

  describe 'all' do
    it 'gets hash-ed header fields' do
      visit all.path
      expect(page.body).to match(/^#{all.key}: .+$/)
    end
  end

end
