MRuby::Build.new do |conf|
  toolchain :gcc

  conf.gembox 'default'
  # conf.gem :mgem => 'mruby-redis'
  conf.gem :github => 'matsumoto-r/mruby-redis'
end
