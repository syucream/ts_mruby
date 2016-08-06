class TransformBang
  attr_accessor :path, :txt
end

FactoryGirl.define do
  factory :transform_bang do
    path '/ats_filter_transform_bang'
    txt ' overwritten by ts_mruby :D'
  end
end
