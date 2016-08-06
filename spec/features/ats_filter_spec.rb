require './spec_helper'

describe 'ATS::Filter class', :js => false do
  describe 'transform!' do
    it 'returns transformed response body' do
      transform_bang = build(:transform_bang)
      origin = build(:origin)

      visit transform_bang.path
      expect(page).to have_text origin.body + transform_bang.txt
    end
  end
end
