include("timing-config.js");
include("scene.js");
set_clear_all();
set_fps(25);
var show = workload === "idle" ? null : loadScene();
var osc;
var audio;
var encoder;
if (workload === "osc") {
    osc = new OscController(serverPort);
    for (var id = 0; id < 4; ++id) {
        var name = "timing" + id;
        osc[name] = function() { return true; };
        osc.add_method("/timing/" + id, "", name);
    }
    osc.start();
    register_controller(osc);
    osc.send_to("127.0.0.1", peerPort);
    osc.send("/timing/ready", "");
} else if (workload === "capture") {
    audio = new AudioJack("replay_pcm:pulse", 2048, 48000);
    encoder = new VideoEncoder(50, 64000, 0, 0);
    register_encoder(encoder);
    encoder.start_filesave("timing.ogg");
}
var nonzeroWindows = 0;
for (var frame = 0; frame < frameCount; ++frame) {
    if (show) {
        show.overlay.set_position(16 + frame % 8, 12);
        if (frame % 12 === 0) show.tint.set_parameter("Brightness", frame % 24 ? 0.75 : 0.5);
    }
    if (audio) {
        audio.fft();
        for (var band = 1; band <= 16; ++band) {
            if (audio.get_harmonic(band) !== 0) {
                ++nonzeroWindows;
                break;
            }
        }
    }
    run(0);
}
if (encoder) {
    encoder.stop_filesave();
    echo("timing: nonzero FFT windows " + nonzeroWindows);
}
if (osc) osc.stop();
echo("timing: workload complete");
quit();
