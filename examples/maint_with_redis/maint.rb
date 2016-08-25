redis = Redis.new '127.0.0.1', 6789

if redis['ts_maint']
  body = <<'EOS'
<html>
  <head>
    <title>Message from your proxy ...</title>
  </head>
  <body>
    <p>Sorry, maintainance mode now ...</p>
  </body>
</html>
EOS
  ATS::echo body
end 
