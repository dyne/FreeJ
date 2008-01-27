/*  FreeJ example scripts
 *  (c) Copyright 2008 Christoph Rudorff aka MrGoil <goil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* a nice helper to set script values with controler values
   automagicly check ranges and sets values by name in the caller context
*/

function Param(obj, name, in_min, in_max, out_min, out_max, out_start_value) {
    if (arguments.length != 7) 
        throw "Wrong no of Args.";
    if ( (out_max-out_min == 0) || (in_max-in_min == 0))
        throw "Argument value error";
    this.frac = 0; // default output Integer
    this.obj = obj;
    this.name = name;
    this.in_min = in_min;
    this.in_max = in_max;
    this.out_min = out_min;
    this.out_max = out_max;

    this.out_value = Math.round(out_start_value);
    obj[name] = out_start_value;

    this.mult = (out_max-out_min)/(in_max-in_min);
    this.in_value = Math.int((this.out_value -this.out_min)/this.mult + this.in_min);
}

Param.prototype.setValue = function(in_value) {
    in_value = this.range(Math.int(in_value));
    var out_value = (this.mult*(in_value-this.in_min) + this.out_min).round(this.frac);
    this.obj[this.name] = out_value;
    this.in_value = in_value;
    this.out_value= out_value;
    echo(this.name + "in: " + in_value + " out: " + this.out_value);
}

Param.prototype.step = function(n) {
    // if this.mult<1 ... 
    var in_value = this.range(this.in_value + n);
    this.setValue(in_value); 
}

Param.prototype.range = function(v) {
    return Math.min(this.in_max, Math.max(this.in_min, v));
}

Param.prototype.print = function(textlayer) {
    if (textlayer instanceof TextLayer) {
        var msg = this.name + ": " + this.out_value;
        textlayer.print(msg);
    } else {
        throw("I want a TextLayer!");
    }
}


// goodies ... return js_int(!)

Math.int = function(i) {
    if (i>0) 
        return this.floor(i)
    else
        return this.ceil(i)
}
Number.prototype.int = function() {
    return Math.int(this);
}
Number.prototype.round = function(d) {
    if (d == 0)
        return Math.round(this);
    d = Math.pow(10,d);
    return Math.round(this*d)/d;
}


