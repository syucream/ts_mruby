class Request
  attr_accessor :path, :keys, :template
end

FactoryGirl.define do
  factory :request do
    keyhash = {
      'ATS::Request#hostname' => 'hostname',
      'ATS::Request#url' => 'url'
    }

    path '/ats_request'
    keys keyhash
    template "{\n" +
             (keyhash.map do |k, v| "  '#{k}': %{#{v}}" end).join("\n") +
             "\n}"
  end
end
