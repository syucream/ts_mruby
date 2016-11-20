require './spec_helper'

describe 'ATS class', :js => false do
  let (:getter) { get(:headers_out_get) }

  describe 'get' do
    it 'gets specified response header field value' do
      visit getter.path
      expect(page.body).to match(/^#{getter.key}: .+$/)
    end
  end

end
