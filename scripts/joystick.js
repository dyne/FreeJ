// 
// freej joystick controller settings
// basic template by jaromil
//

joy = new JoystickController();
register_controller( joy );


joy.axis = function(which, axis, value) { echo("joystick " + which +
					       " axis "    + axis  +
					       " value " + value); }

run(10);

quit();
