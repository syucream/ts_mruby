require 'factory_girl'

# load fixtures
FactoryGirl.definition_file_paths = %w(../factories/)
FactoryGirl.find_definitions
include FactoryGirl::Syntax::Methods

# variables
records_set = build(:records_set)

# generate template
template = <<EOS
records = ATS::Records.new
records.set ATS::Records::#{records_set.key}, #{records_set.value}
EOS

puts template
