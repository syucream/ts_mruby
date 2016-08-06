require './spec_helper'

describe 'ATS::Upstream class', :js => false do
  describe 'server=' do
    it 'non-overried origin server works' do
      origin = build(:origin)

      visit origin.path
      expect(page).to have_text origin.body
    end

    it 'override host of an origin server' do
      visit_exception = nil
      begin
        # It may raise exception if origin fetch is failed
        server_eq = build(:server_eq)
        visit server_eq.path
      rescue => e
        visit_exception = e
      end

      if (visit_exception)
        expect(page.driver.browser.errored_remote_response).not_to be_nil
      else
        origin = build(:origin)
        expect(page).not_to have_text origin.body
      end
    end
  end
end
