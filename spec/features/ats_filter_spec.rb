require './spec_helper'

describe 'ATS::Filter class', :js => false do
  let (:body_eq) { get(:body_eq) }
  let (:transform_bang) { get(:transform_bang) }
  let (:origin) { get(:origin) }

  describe 'transform!' do
    it 'returns transformed response body' do
      visit transform_bang.path
      expect(page).to have_text origin.body + transform_bang.txt
    end
  end

  describe 'body=' do
    it 'returns rewritten response body' do
      visit body_eq.path
      expect(page).to have_text body_eq.body
      expect(page).not_to have_text origin.body
    end
  end

  describe 'output=' do
    it 'returns rewritten response body' do
      visit body_eq.alias_path
      expect(page).to have_text body_eq.body
      expect(page).not_to have_text origin.body
    end
  end
end
