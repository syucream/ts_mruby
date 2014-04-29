if server_name == "NGINX"
  Server = Nginx
elsif server_name == "Apache"
  Server = Apache
elsif server_name == "ApacheTrafficServer"
  Server = ATS
end

Server::rputs "Hello #{Server::module_name}/#{Server::module_version} world!"
# mod_mruby => "Hello mod_mruby/0.9.3 world!"
# ngx_mruby => "Hello ngx_mruby/0.0.1 world!"
# ts_mruby  => "Hello ts_mruby/0.0.1 world!"
