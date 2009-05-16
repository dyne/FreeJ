include("../lib/json2.js");

echo("JSON library included");

files = scandir(".");

echo("files found: " + files.length);

// stringify the array
json_enc = JSON.stringify(files);

// and parse it back
json_dec = JSON.parse(json_enc);

// then go through the array and load the scripts we like
var c;
for(c=0; c<json_dec.length; c++) {
    if(json_dec[c] == "./star.js") {
	echo("loading star.js");
	include(json_dec[c]);
    }
    if(json_dec[c] == "./test_rand.js") {
	echo("loading test_rand.js");
	include(json_dec[c]);
    }
    if(json_dec[c] == "./cynosure.js") {
	echo("loading cynoruse.js");
	include(json_dec[c]);
    }
}

