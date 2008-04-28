/*
user settings
*/
script_search_path = new Array("/usr/lib/freej/scripts", "~/.freej/scripts", ".", "./scripts", "./Scripts");
// TODO: font path, "media" path, lets see.

/*
	Compatiblity with old freej behaviour
*/
this._add_layer = add_layer;
this.add_layer = function (l) {
	debug("kicking " + l);
	this._add_layer(l);
	l.activate(true); // layer_set_fps: argument 0 is not a number
	l.start();
	l.set_fps();
}


/*
	include file, look in script_search_path
*/
this.include_search = function(file) {
	for (path_idx in script_search_path) {
		var filename = script_search_path[path_idx] + "/" + file;
		try {
			include(filename);
			debug("included " + filename);
			return;
		} catch (e) { 
			// FIXME TODO: status/exception code?
			// file not found -> continue,
			// otherwise not (syntax error in included file)
			debug("include("+filename+") failed:" + e);
		}
	}
}

/*
	some goodies
*/
Math.int = function(i) {
    if (i>0) 
        return this.floor(i)
    else
        return this.ceil(i)
}
Number.prototype.int = function() {
    return Math.int(this);
}
// round a number of d digits
Number.prototype.round = function(d) {
    if (d == 0)
        return Math.round(this);
    d = Math.pow(10,d);
    return Math.round(this*d)/d;
}

echo("freej std js library loaded.");



