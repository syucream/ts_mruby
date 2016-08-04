require 'factory_girl'

# load fixtures
FactoryGirl.definition_file_paths = %w(../factories/)
FactoryGirl.find_definitions
include FactoryGirl::Syntax::Methods

# variables
origin = build(:origin)

# generate template
template = <<EOS
ATS::rputs "#{origin.body}"
EOS

puts template
