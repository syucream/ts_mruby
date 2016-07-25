require 'factory_girl'

# load fixtures
FactoryGirl.definition_file_paths = %w(./factories/)
FactoryGirl.find_definitions
include FactoryGirl::Syntax::Methods

# variables
rputs = build(:rputs)

# generate template
template = <<EOS
ATS::rputs "#{rputs.body}"
EOS

puts template
