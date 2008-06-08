require("freej")
print("hello")
cx = freej.Context()
print(cx)
print("\n")
print("\n")
cx:init(400,300,false,false)
cx:config_check("keyboard.js")
cx.plugger:refresh(cx)

-- add a layer
lay = freej.create_layer(cx,"/home/caedes/dvgrab-001.avi")
print(lay)
print("\n")
lay:start()
var = 1
filter = cx.filters:search("vertigo")
print(filter)
print("\n")
lay:add_filter(filter)
lay.active = true
cx:add_layer(lay)

-- main loop
while 1 do
  i = 10000
  cx:cafudda(0.0)
  while i < 10 do
    i=i+1
  end
end
Sleep(10)
