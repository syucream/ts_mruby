require 'factory_girl'

# load fixtures
FactoryGirl.definition_file_paths = %w(../factories/)
FactoryGirl.find_definitions
include FactoryGirl::Syntax::Methods

# variables
transform_bang = build(:transform_bang)

# generate template
template = <<EOS
f = ATS::Filter.new
f.transform! do |body|
  body + "#{transform_bang.txt}"
end
EOS

puts template
