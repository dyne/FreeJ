// 
// freej keyboard controller settings
// basic template by jaromil
//

kbd = new KeyboardController();
register_controller( kbd );

__kbd_sel = selected_layer();

kbd.pressed_1 = function() { __kbd_sel.set_blit("RGB");   }
kbd.pressed_2 = function() { __kbd_sel.set_blit("RED");   }
kbd.pressed_3 = function() { __kbd_sel.set_blit("GREEN"); }
kbd.pressed_4 = function() { __kbd_sel.set_blit("BLUE");  }
kbd.pressed_5 = function() { __kbd_sel.set_blit("ADD"); }
kbd.pressed_6 = function() { __kbd_sel.set_blit("SUB"); }
kbd.pressed_7 = function() { __kbd_sel.set_blit("ABSDIFF"); }
kbd.pressed_8 = function() { __kbd_sel.set_blit("AND"); }
kbd.pressed_9 = function() { __kbd_sel.set_blit("OR"); }
kbd.pressed_0 = function() { __kbd_sel.set_blit("NEG"); }


kbd.pressed_a = function() { }
kbd.pressed_b = function() { }
kbd.pressed_c = function() { }
kbd.pressed_d = function() { }
kbd.pressed_e = function() { }
kbd.pressed_f = function() { }
kbd.pressed_g = function() { }
kbd.pressed_h = function() { }
kbd.pressed_i = function() { }
kbd.pressed_j = function() { }
kbd.pressed_k = function() { }
kbd.pressed_l = function() { }
kbd.pressed_m = function() { }
kbd.pressed_n = function() { }
kbd.pressed_o = function() { }
kbd.pressed_p = function() { }
kbd.pressed_q = function() { }
kbd.pressed_r = function() { }
kbd.pressed_s = function() { }
kbd.pressed_t = function() { }
kbd.pressed_u = function() { }
kbd.pressed_v = function() { }
kbd.pressed_w = function() { }
kbd.pressed_x = function() { }
kbd.pressed_y = function() { }
kbd.pressed_z = function() { }

// more letter keys are available
// in combination with control, shift or alt keys
// define them as:
kbd.pressed_ctrl_a  = function() { }
kbd.pressed_shift_b = function() { }
kbd.pressed_alt_c   = function() { }
// .. and so on with other letters
// you can also combine ctrl+shift+alt for example:
kbd.pressed_ctrl_shift_alt_a = function() { }

// symbol keys:
kbd.pressed_up       = function() { }
kbd.pressed_down     = function() { }
kbd.pressed_insert   = function() { }
kbd.pressed_home     = function() { }
kbd.pressed_end      = function() { }
kbd.pressed_pageup   = function() { }
kbd.pressed_pagedown = function() { }

// left/right are to change the selected layer
kbd.pressed_left = function() {
    if(!__kbd_sel) __kbd_sel = selected_layer();
    __kbd_sel = __kbd_sel.prev(); __kbd_sel.select();
}
kbd.pressed_right = function() {
    if(!__kbd_sel) __kbd_sel = selected_layer();
    __kbd_sel = __kbd_sel.next(); __kbd_sel.select();
}
// plus/minus are to move layer up and down
kbd.pressed_plus = function() {
    if(!__kbd_sel) __kbd_sel = selected_layer();
    __kbd_sel.up();
}
kbd.pressed_minus = function() {
    if(!__kbd_sel) __kbd_sel = selected_layer();    
    __kbd_sel.down();
}

kbd.pressed_backspace = function() { }
kbd.pressed_tab       = function() { }
kbd.pressed_return    = function() { }
kbd.pressed_space     = function() { }
kbd.pressed_less      = function() { }
kbd.pressed_greater   = function() { }
kbd.pressed_equals    = function() { }

// numeric keypad keys:
kbd.pressed_num_1 = function() { }
kbd.pressed_num_2 = function() { }
kbd.pressed_num_3 = function() { }
kbd.pressed_num_4 = function() { }
kbd.pressed_num_5 = function() { }
kbd.pressed_num_6 = function() { }
kbd.pressed_num_7 = function() { }
kbd.pressed_num_8 = function() { }
kbd.pressed_num_9 = function() { }
kbd.pressed_num_0 = function() { }

kbd.pressed_num_period   = function() { }
kbd.pressed_num_divide   = function() { }
kbd.pressed_num_multiply = function() { }
kbd.pressed_num_minus    = function() { }
kbd.pressed_num_plus     = function() { }
kbd.pressed_num_enter    = function() { }
kbd.pressed_num_equals   = function() { }

// to quit we have default keys:
kbd.pressed_ctrl_q = function() { quit(); }
kbd.pressed_ctrl_c = function() { quit(); }
kbd.pressed_esc    = function() { quit(); }




//////////////////////////////////////////
/// actions at key release

kbd.released_1 = function() { }
kbd.released_2 = function() { }
kbd.released_3 = function() { }
kbd.released_4 = function() { }
kbd.released_5 = function() { }
kbd.released_6 = function() { }
kbd.released_7 = function() { }
kbd.released_8 = function() { }
kbd.released_9 = function() { }
kbd.released_0 = function() { }

kbd.released_a = function() { }
kbd.released_b = function() { }
kbd.released_c = function() { }
kbd.released_d = function() { }
kbd.released_e = function() { }
kbd.released_f = function() { }
kbd.released_g = function() { }
kbd.released_h = function() { }
kbd.released_i = function() { }
kbd.released_j = function() { }
kbd.released_k = function() { }
kbd.released_l = function() { }
kbd.released_m = function() { }
kbd.released_n = function() { }
kbd.released_o = function() { }
kbd.released_p = function() { }
kbd.released_q = function() { }
kbd.released_r = function() { }
kbd.released_s = function() { }
kbd.released_t = function() { }
kbd.released_u = function() { }
kbd.released_v = function() { }
kbd.released_w = function() { }
kbd.released_x = function() { }
kbd.released_y = function() { }
kbd.released_z = function() { }

// more letter keys are available
// in combination with control, shift or alt keys
// define them as:
kbd.released_ctrl_a  = function() { }
kbd.released_shift_b = function() { }
kbd.released_alt_c   = function() { }
// .. and so on with other letters
// you can also combine ctrl+shift+alt for example:
kbd.released_ctrl_shift_alt_a = function() { }

// symbol keys:
kbd.released_up       = function() { }
kbd.released_down     = function() { }
kbd.released_left     = function() { }
kbd.released_right    = function() { }
kbd.released_insert   = function() { }
kbd.released_home     = function() { }
kbd.released_end      = function() { }
kbd.released_pageup   = function() { }
kbd.released_pagedown = function() { }
kbd.released_esc      = function() { }

kbd.released_backspace = function() { }
kbd.released_tab       = function() { }
kbd.released_return    = function() { }
kbd.released_space     = function() { }
kbd.released_plus      = function() { }
kbd.released_minus     = function() { }
kbd.released_less      = function() { }
kbd.released_greater   = function() { }
kbd.released_equals    = function() { }

// numeric keypad keys:
kbd.released_num_1 = function() { }
kbd.released_num_2 = function() { }
kbd.released_num_3 = function() { }
kbd.released_num_4 = function() { }
kbd.released_num_5 = function() { }
kbd.released_num_6 = function() { }
kbd.released_num_7 = function() { }
kbd.released_num_8 = function() { }
kbd.released_num_9 = function() { }
kbd.released_num_0 = function() { }

kbd.released_num_period   = function() { }
kbd.released_num_divide   = function() { }
kbd.released_num_multiply = function() { }
kbd.released_num_minus    = function() { }
kbd.released_num_plus     = function() { }
kbd.released_num_enter    = function() { }
kbd.released_num_equals   = function() { }



