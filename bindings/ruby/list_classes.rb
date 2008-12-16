#!/usr/bin/ruby1.9
require '.libs/Freej'

puts "Freej class methods:"
Freej.methods(false).sort.each do |m|
  puts "\t#{m.to_s}"
end

defines = Freej.constants.select{|x| Freej.const_get(x).kind_of? Fixnum}.sort
if defines.size > 0
  puts "Freej Defines:"
  defines.each do |c|
    puts "\t #{c.to_s}"
  end
end

puts "Freej Classes:"
Freej.constants.select{|x| Freej.const_get(x).kind_of? Class}.sort.each do |c|
  sc = Freej.const_get(c)
  puts "\t #{sc.to_s}"
  m = sc.methods(false)
  if m.size > 0 
    puts "\t\t class methods:"
    m.sort.each do |im|
      puts "\t\t\t #{im.to_s}"
    end
  end
  m = sc.instance_methods(false)
  if m.size > 0 
    puts "\t\t instance methods:"
    m.sort.each do |im|
      puts "\t\t\t #{im.to_s}"
    end
  end
end
