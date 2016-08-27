MRuby::Build.new do |conf|

  toolchain :gcc

  conf.gembox 'default'

  conf.gem :github => 'mattn/mruby-mysql'
  conf.gem :github => 'happysiro/mruby-maxminddb'

  conf.cc do |cc|
    cc.flags = [ENV['CFLAGS'] || %w(-fPIC)]
  end

  conf.linker.libraries << 'mysqlclient'
  conf.linker.libraries << 'maxminddb'

end
