// The historical r-key handler used reset(filename) to reload the set.
// Call that same API in an isolated process. It resets the engine and
// ignores the filename on the Linux reference.
include("scene.js");
loadScene();
echo("replay: reset start");
reset("reset-target.js");
echo("replay: reset returned");
quit();
