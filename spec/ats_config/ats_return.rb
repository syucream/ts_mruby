require 'factory_girl'

# load fixtures
FactoryGirl.definition_file_paths = %w(../factories/)
FactoryGirl.find_definitions
include FactoryGirl::Syntax::Methods

# variables
ret = build(:return)

# generate template
template = <<EOS
ATS::echo "ATS::return test"
ATS::return #{ret.rc_class}
EOS

puts template
