require './spec_helper'

describe 'ATS class', :js => false do
  describe 'rputs' do
    it 'responds passed message' do
      rputs = build(:rputs)

      visit rputs.path
      expect(page).to have_text rputs.body
    end
  end
end
