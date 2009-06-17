include("../lib/json2.js");

echo("JSON library included");

files = scandir(".");

echo("files found: " + files.length);

// stringify the array
json_enc = JSON.stringify(files);

echo("json size: " + json_enc.length );

// and parse it back
json_dec = JSON.parse(json_enc);

// then go through the array and load the scripts we like
var c;
for(c=0; c<json_dec.length; c++) {
    echo(json_dec[c]);
}

