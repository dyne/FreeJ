#!/usr/bin/ruby
#for now update the load path to include the location of 
#the freej module and freej_extensions [as we haven't installed it yet]
$: << '../../bindings/ruby/.libs'
$: << '../../bindings/ruby/'
##import the Freej module
require 'Freej'

# check that we have an argument
if ARGV.length < 1
  puts "[!] this script needs an argument: file to play"
  exit
end

#initializes Freej creating a Contex
cx = Freej::Context.new

#creates a screen of given size
scr = Freej::SdlScreen.new(400, 300)

#adds the screen
cx.add_screen(scr)

#refreshes the list of available filter effects
cx.plugger.refresh(cx)

#opens the file given on commandline as a layer
lay = cx.open(ARGV[0])

#gets the vertigo filter effect
filt = cx.filters["vertigo"]

#adds the filter to the layer if the filter was found
lay.add_filter( filt ) if filt

#start the layer thread
lay.start

#adds the layer to the Freej context
cx.add_layer(lay)

#fit the video layer to the screen
lay.fit

#start the context
cx.start
