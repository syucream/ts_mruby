class MyHandler 
  def on_send_request_hdr
    puts 'on_send_request_hdr'
    c = ATS::Connection.new
    puts c.remote_ip
  end

  def on_read_response_hdr
    puts 'on_read_response_hdr'
    c = ATS::Connection.new
    puts c.remote_ip
  end

  def on_send_response_hdr
    puts 'on_send_response_hdr'
    c = ATS::Connection.new
    puts c.remote_ip
  end
end

es = ATS::EventSystem.new
es.register MyHandler
