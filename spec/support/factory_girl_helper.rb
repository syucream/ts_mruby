require 'factory_girl'

#
# Helper for common use of FactoryGirl
#
module FactoryGirlHelper
  include FactoryGirl::Syntax::Methods

  FactoryGirl.definition_file_paths = %w(../factories/)
  FactoryGirl.find_definitions

  def get(target)
    build(target)
  end
end
