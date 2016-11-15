require "bundler/setup"
require 'capybara/mechanize'
require 'capybara/rspec'
require 'factory_girl'
require './support/factory_girl_helper'
require 'rspec'

RSpec.configure do |config|
  config.color = true
  config.formatter = :documentation

  config.include Capybara::DSL

  config.include FactoryGirlHelper
  config.before(:suite) do
    FactoryGirl.definition_file_paths = %w(./factories/)
    FactoryGirl.find_definitions
  end
end

# Capybara config
Capybara.app = 'ts_mruby'
Capybara.default_driver = :mechanize
Capybara.app_host = 'http://127.0.0.1:8080'
Capybara.run_server = false
