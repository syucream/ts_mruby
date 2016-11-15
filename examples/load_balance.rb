backends = [
  "127.0.0.1:8001",
  "127.0.0.1:8002",
  "127.0.0.1:8003",
]
upstream = ATS::Upstream.new
upstream.server = backends[rand(backends.length)]
