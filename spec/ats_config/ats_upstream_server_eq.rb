require 'factory_girl'

# load fixtures
FactoryGirl.definition_file_paths = %w(../factories/)
FactoryGirl.find_definitions
include FactoryGirl::Syntax::Methods

# variables
server_eq = build(:server_eq)

# generate template
template = <<EOS
upstream = ATS::Upstream.new
upstream.server = "#{server_eq.server}"
EOS

puts template
