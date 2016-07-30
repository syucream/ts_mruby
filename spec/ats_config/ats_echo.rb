require 'factory_girl'

# load fixtures
FactoryGirl.definition_file_paths = %w(../factories/)
FactoryGirl.find_definitions
include FactoryGirl::Syntax::Methods

# variables
echo = build(:echo)

# generate template
template = <<EOS
ATS::echo "#{echo.body}"
EOS

puts template
