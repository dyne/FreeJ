#!/usr/bin/ruby
#for now update the load path to include the location of 
#the freej module and freej_extensions [as we haven't installed it yet]
$: << '../../bindings/ruby/.libs'
$: << '../../bindings/ruby/'
##import the Freej module
require 'Freej'
#initializes Freej creating a Contex
cx = Freej::Context.new
#refresh the filter list
cx.plugger.refresh(cx)
#we collect because this creates a ruby array out of the filter linked list
#then we sort by name [case insensitive], then we print the name and description
#of each of the sorted filters
cx.filters.collect{|f| f}.sort {|x,y| x.name.downcase <=> y.name.downcase }.each do |f|
  puts f.name
  puts "\t#{f.description}"
end
