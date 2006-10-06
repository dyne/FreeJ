

kbd = new KeyboardController();
register_controller( kbd );

kbd.pressed_a = function() {
    echo("porcodio");
}

run(10.0);

quit();

