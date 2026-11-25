// Adapted from freej_equalizer.js. The replay_pcm client supplies one known
// mono pulse per period. Report the 16 values before any visual mapping.
var audio = new AudioJack("replay_pcm:pulse", 2048, 48000);
set_fps(25);
for (var frame = 0; frame < 25; ++frame) {
    audio.fft();
    var bands = [];
    for (var band = 1; band <= 16; ++band) bands.push(audio.get_harmonic(band));
    echo("replay: bands " + bands.join(","));
    run(0);
}
echo("replay: audio quit");
quit();
