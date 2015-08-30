if server_name == "NGINX"
  Server = Nginx
elsif server_name == "Apache"
  Server = Apache
elsif server_name == "ApacheTrafficServer"
  Server = ATS
end

# rewrite request headers
r = Server::Request.new
conn = Server::Connection.new
r.headers_in["X-ATS-Plugin"] = "ts_mruby"
r.headers_in["X-Forwarded-For"] = conn.remote_ip
r.headers_in.delete("Cookie")
