// 
// freej joystick controller settings
// basic template by jaromil
//

joy = new JoystickController();
register_controller( joy );


joy.axis_1 = function() { echo("axis 1"); }
joy.axis_2 = function() { echo("axis 2"); }

joy.button_1_down = function() { echo("button 1 down"); }
joy.button_1_up   = function() { echo("button 1 up"); }

joy.button_2_down = function() { echo("button 2 down"); }
joy.button_2_up   = function() { echo("button 2 up"); }



run(10);

quit();
