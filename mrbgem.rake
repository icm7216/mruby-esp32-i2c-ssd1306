MRuby::Gem::Specification.new('mruby-esp32-i2c-ssd1306') do |spec|
  spec.license = 'MIT'
  spec.authors = 'icm7216'

  spec.cc.include_paths << "#{build.root}/src"

  spec.add_dependency('mruby-esp32-i2c', :github => "mruby-esp32/mruby-esp32-i2c")
  spec.add_dependency "mruby-toplevel-ext"
end
