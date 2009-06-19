#!/usr/bin/ruby
##import the Freej module
#if not require 'Freej'
require '../../bindings/ruby/.libs/Freej'
#end

# check that we have an argument
if ARGV.length < 1
  puts "[!] this script needs an argument: file to play"
  exit
end

module Freej
  class Context
    def start
      #create a ruby thread and call cafudda in a loop so that we render
      @thread = Thread.new { loop { self.cafudda(1); sleep(0.04) } }
      self
    end
    def stop
      #if the thread exists terminate it
      if @thread
        @thread.terminate
      end
      self
    end
  end
  class BaseLinklist
    def [](item)
      if item.is_a?(String)
        res = self.search(item)
        if res == 0
          return nil
        else
          return res[0]
        end
      else
        return self.pick(item)
      end
    end
  end
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

#loop until the user kills the script
loop{ sleep(1) }

