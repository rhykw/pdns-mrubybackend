MRuby::Build.new do |conf|

  toolchain :gcc

  conf.gembox 'default'

  # conf.gem :github => 'matsumoto-r/mruby-vedis'
  # conf.gem :github => 'mattn/mruby-mysql'
  # conf.gem :github => 'matsumoto-r/mruby-geoip'

  conf.cc do |cc|
    cc.flags = [ENV['CFLAGS'] || %w(-fPIC)]
  end

end
