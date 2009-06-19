#import the freej module
#if not require 'freej'
  require '../../bindings/ruby/.libs/Freej'
#end

# initializes FreeJ creating a Contex
cx = Freej::Context.new
# creates a screen of given size
scr = Freej::SdlScreen.new( 400, 300 )
# adds the screen
cx.add_screen(scr)
# create an instance of a TextLayer
txt = Freej::TextLayer.new
# initializes the new layer with the freej context
txt.init(cx)
# writes the hello world text inside the layer
txt.write("Hello World!")
# start the layer
txt.start
# add the layer to the screen
cx.add_layer(txt)
# starts Freej
th = Thread.new { loop{cx.cafudda(1)} }
#XXX ruby doesn't have native threads, so we cannot call cx.start
#th = Thread.new { cx.start }
