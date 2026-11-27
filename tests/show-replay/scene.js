// Adapted from filter_image.js. The fixture uses a stateless frei0r filter
// so the expected pixels do not depend on the worker's wall clock.
function image(file, name) {
    var layer = new ImageLayer();
    layer.open(file);
    layer.name = name;
    if (!add_layer(layer)) throw "could not add " + name;
    return layer;
}

function loadScene() {
    var base = image("base.png", "base");
    base.start();
    var overlay = image("overlay.png", "overlay");
    overlay.set_position(16, 12);
    var tint = new Filter("Brightness");
    overlay.add_filter(tint);
    tint.set_parameter("Brightness", 0.5);
    overlay.start();
    return {base: base, overlay: overlay, tint: tint};
}
