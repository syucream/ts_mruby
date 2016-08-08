class BodyEq
  attr_accessor :path, :alias_path, :body
end
class TransformBang
  attr_accessor :path, :txt
end

FactoryGirl.define do
  factory :body_eq do
    path '/ats_filter_body_eq'
    alias_path '/ats_filter_output_eq'
    body 'entire body was overwritten by ts_mruby :D'
  end

  factory :transform_bang do
    path '/ats_filter_transform_bang'
    txt ' overwritten by ts_mruby :D'
  end
end
