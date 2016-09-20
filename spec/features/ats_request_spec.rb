require './spec_helper'

describe 'ATS::Request class', :js => false do
  let (:request) { get(:request) }

  describe 'getter' do
    it 'gets request info' do
      visit request.path

      keys = request.keys
      keys.each do |k, v|
        expect(page.body).to match(/'#{k}': .+\n/)
      end
    end
  end
end
