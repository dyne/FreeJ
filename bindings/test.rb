require 'Freej'
print "hello"
cx = Freej::Context.new
print cx
print "\n"
print cx.instance_of? Freej::Entry
print "\n"
cx.init(400,300,0,0)
cx.config_check("keyboard.js")
cx.plugger.refresh(cx)

# add a layer
lay = Freej::create_layer(cx,"/home/caedes/dvgrab-001.avi")
print lay
print "\n"
lay.start()
filter = cx.filters.search("vertigo")[0]
print filter
print "\n"
lay.add_filter(filter)
lay.active = 1
cx.add_layer(lay)

# main loop
while 1
  cx.cafudda(0.0)
  sleep 0.04
end
sleep 10
