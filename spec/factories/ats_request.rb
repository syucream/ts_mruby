class RequestGetters
  attr_accessor :path, :keys, :template
end
class RequestSetters
  attr_accessor :path, :values, :template
end

FactoryGirl.define do
  factory :request_getters do
    keyhash = {
      'ATS::Request#args'         => 'args',
      'ATS::Request#hostname'     => 'hostname',
      'ATS::Request#protocol'     => 'protocol',
      'ATS::Request#scheme'       => 'scheme',
      'ATS::Request#unparsed_uri' => 'unparsed_uri',
      'ATS::Request#url'          => 'url'
    }

    path '/ats_request_getters'
    keys keyhash
    template "{\n" +
             (keyhash.map do |k, v| "  '#{k}': %{#{v}}" end).join("\n") +
             "\n}"
  end

  factory :request_setters do
    keyhash = {
      'ATS::Request#args'     => 'args',
      'ATS::Request#hostname' => 'hostname',
      'ATS::Request#protocol' => 'protocol',
      'ATS::Request#scheme'   => 'scheme',
    }
    valuehash = {
      'ATS::Request#args'     => 'field1=value11',
      'ATS::Request#hostname' => 'example.com',
      'ATS::Request#protocol' => 'http/1.0',
      'ATS::Request#scheme'   => 'http',
    }

    path '/ats_request_setters'
    values valuehash
    template "{\n" +
             (keyhash.map do |k, v| "  '#{k}': %{#{v}}" end).join("\n") +
             "\n}"
  end
end
