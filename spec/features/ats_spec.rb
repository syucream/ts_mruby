require './spec_helper'

describe 'ATS class', :js => false do
  describe 'rputs' do
    it 'create response text' do
      rputs = build(:rputs)

      visit rputs.path
      expect(page).to have_text rputs.body
    end
  end

  describe 'echo' do
    it 'create response text which is terminated with a newline' do
      echo = build(:echo)

      visit echo.path
      expect(page).to have_text echo.body + "\n"
    end
  end

  describe 'return' do
    it 'return ATS status code' do
      ret = build(:return)

      visit ret.path
      expect(page.status_code).to be(ret.rc_num)
    end
  end
end
