require 'factory_girl'

# load fixtures
FactoryGirl.definition_file_paths = %w(./factories/)
FactoryGirl.find_definitions
include FactoryGirl::Syntax::Methods

# variables
etc_dir = ENV['ATS_ETC_DIR']
rputs = build(:rputs)

# generate template
template = <<EOS
# suppose ATS listens 127.0.0.1:8080, and origin server does 127.0.0.1:80

# ATS::
map #{rputs.path} http://127.0.0.1:80/ @plugin=ts_mruby.so @pparam=#{etc_dir}scripts/ats_rputs.rb
EOS

puts template
