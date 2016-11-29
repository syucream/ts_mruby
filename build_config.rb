MRuby::Build.new do |conf|
  toolchain :gcc

  conf.cc.flags << '-fPIC'

  conf.gembox 'default'

  # ts_mruby extensions
  conf.gem './mrbgems/ts_mruby_mrblib/'

  # user-defined GEMs
  # ...
end
