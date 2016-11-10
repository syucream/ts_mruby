require './spec_helper'

describe 'ATS::Request class', :js => false do
  let (:req_g) { get(:request_getters) }
  let (:req_s) { get(:request_setters) }

  describe 'getter' do
    it 'gets request info' do
      visit req_g.path + '?field1=value1&field2=value2'

      keys = req_g.keys
      keys.each do |k, v|
        expect(page.body).to match(/'#{k}': .+\n/)
      end
    end
  end

  describe 'setter' do
    it 'sets request info' do
      visit req_s.path + '?field1=value1&field2=value2'

      values = req_s.values
      values.each do |k, v|
        expect(page.body).to match(/'#{k}': #{v}\n/)
      end
    end
  end
end
