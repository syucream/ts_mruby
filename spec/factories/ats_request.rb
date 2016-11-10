class Request
  attr_accessor :path, :keys, :template
end

FactoryGirl.define do
  factory :request do
    keyhash = {
      'ATS::Request#args' => 'args',
      'ATS::Request#hostname' => 'hostname',
      'ATS::Request#protocol' => 'protocol',
      'ATS::Request#scheme' => 'scheme',
      'ATS::Request#unparsed_uri' => 'unparsed_uri',
      'ATS::Request#url' => 'url'
    }

    path '/ats_request'
    keys keyhash
    template "{\n" +
             (keyhash.map do |k, v| "  '#{k}': %{#{v}}" end).join("\n") +
             "\n}"
  end
end
