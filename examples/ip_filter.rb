if server_name == "NGINX"
  Server = Nginx
elsif server_name == "Apache"
  Server = Apache
elsif server_name == "ApacheTrafficServer"
  Server = ATS
end

whitelist = [
  "127.0.0.1"
]

# deny if client IP is listed in whitelist
conn = Server::Connection.new
unless whitelist.include?(conn.remote_ip)
  Server::echo "Your access is not allowed ..."
  Server::return Server::HTTP_FORBIDDEN
end
