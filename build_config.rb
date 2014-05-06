
############################
# Start of your build_config

MRuby::Build.new do |conf|
  toolchain :gcc

  conf.bins = %w(mrbc)

  # mruby's Core GEMs
  conf.gem 'mrbgems/mruby-bin-mirb'
  conf.gem 'mrbgems/mruby-bin-mruby'
  conf.gem 'mrbgems/mruby-array-ext'
  conf.gem 'mrbgems/mruby-enum-ext'
  conf.gem 'mrbgems/mruby-eval'
  conf.gem 'mrbgems/mruby-exit'
  conf.gem 'mrbgems/mruby-fiber'
  conf.gem 'mrbgems/mruby-hash-ext'
  conf.gem 'mrbgems/mruby-math'
  conf.gem 'mrbgems/mruby-numeric-ext'
  conf.gem 'mrbgems/mruby-object-ext'
  conf.gem 'mrbgems/mruby-objectspace'
  conf.gem 'mrbgems/mruby-print'
  conf.gem 'mrbgems/mruby-proc-ext'
  conf.gem 'mrbgems/mruby-random'
  conf.gem 'mrbgems/mruby-range-ext'
  conf.gem 'mrbgems/mruby-sprintf'
  conf.gem 'mrbgems/mruby-string-ext'
  conf.gem 'mrbgems/mruby-string-utf8'
  conf.gem 'mrbgems/mruby-struct'
  conf.gem 'mrbgems/mruby-symbol-ext'
  conf.gem 'mrbgems/mruby-time'
  conf.gem 'mrbgems/mruby-toplevel-ext'

  # user-defined GEMs
  # No GEMs activated
end

# End of your build_config
############################
