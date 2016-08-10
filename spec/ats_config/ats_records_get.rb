require 'factory_girl'

# load fixtures
FactoryGirl.definition_file_paths = %w(../factories/)
FactoryGirl.find_definitions
include FactoryGirl::Syntax::Methods

# variables
records_get = build(:records_get)

# generate template
template = <<EOS
records = ATS::Records.new
value = records.get ATS::Records::#{records_get.key}
ATS::echo value
EOS

puts template
