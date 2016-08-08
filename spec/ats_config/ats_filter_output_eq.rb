require 'factory_girl'

# load fixtures
FactoryGirl.definition_file_paths = %w(../factories/)
FactoryGirl.find_definitions
include FactoryGirl::Syntax::Methods

# variables
body_eq = build(:body_eq)

# generate template
template = <<EOS
f = ATS::Filter.new
f.output = "#{body_eq.body}"
EOS

puts template
