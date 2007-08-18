// 
// freej joystick controller settings
// basic template by jaromil
//

joy = new JoystickController();
register_controller( joy );


joy.axismotion = function(which, axis, value) { 
    echo("joystick " + which +
         " axis "    + axis  +
         " value "   + value);
}


joy.button = function (which, button, state) {
    echo("joystick " + which +
         " button "  + button  +
         " state "   + state);
}
