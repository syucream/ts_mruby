req = ATS::Request.new
path = req.uri + req.args

if path =~ /^\/(ogre.*)\/bad/
  ATS::return ATS::HTTP_NOT_FOUND
elsif path =~ /^\/oldurl\/(.*)$/
  ATS::redirect 'http://news.example.com/new/' + $1, ATS::HTTP_MOVED_TEMPORARILY
end
