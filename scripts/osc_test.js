// testing OSC messaging

// auxiliary keyboard

kbd = new KeyboardController();
register_controller( kbd );
kbd.released_q = function() { quit(); }


osc = new OscController("9696"); // open an OSC listener on port 9696

 // define the function that will be called
osc.test_callback =  function test_callback(i,f,s) {
     echo("OSC remote call to my test function");
     echo("int argument is: " + i);
     echo("float argument is: " + f);
     echo("string argument is: " + s);
     return true;
};

// create a /my_new_method call on OSC, taking one integer as argument, that
// will call the remote_called_function() defined in javascript:
osc.add_method("/test","ifs","test_callback");


// starts the osc listener thread
osc.start();
// to stop it during execution use osc.stop();

// and don't forget to register the controller
register_controller(osc);
