#!/usr/bin/env python3
"""Exercise directory discovery with isolated loadable C modules and a native FreeJ host."""
import argparse
from collections import Counter
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
INCLUDE = HERE.parents[1] / "src/include"


def build(source, module, *defines):
    compiler = shlex.split(os.environ.get("CC", "cc"))
    obj = module.with_suffix(".o")
    subprocess.run([*compiler, "-fPIC", "-freg-struct-return", "-I", str(INCLUDE),
                    *defines, "-c", str(HERE / source), "-o", str(obj)], check=True)
    subprocess.run([*compiler, "-shared", str(obj), "-o", str(module)], check=True)


def script_automation(root, cli, env):
    script = root / "automation.js"
    script.write_text("""var layer = new GeneratorLayer(64, 48, "Clock source");
add_layer(layer);
var filter = new Filter("Clock filter");
layer.add_filter(filter);
for (var i = 0; i < 32; ++i) {
    filter.set_parameter("toggle", i % 2);
    filter.set_parameter("amount", 0.25);
    filter.set_parameter("position", 0.25, 0.75);
}
echo("effect automation complete");
quit();
""")
    trace = root / "script-trace"
    env = dict(env, FREEJ_EFFECT_TRACE=str(trace), FREEJ_EFFECT_TRACE_SETS="1")
    process = subprocess.run([str(cli.resolve()), "-c", "-D", "1", "-s", "64x48", "-j", script.name],
                             cwd=root, env=env, timeout=15, capture_output=True, text=True)
    # The established CLI status for a script calling quit() is 1.
    assert process.returncode == 1 and "effect automation complete" in process.stdout.splitlines(), (
        process.returncode, process.stderr[-2000:])
    counts = Counter(trace.read_text().splitlines())
    assert [counts["filter " + s] for s in ("set-true", "set-false", "quarter", "position")] == [16, 16, 32, 32]
    print("scripted quit with live source and filter: 96 parameter changes")


def run(root, args):
    user = root / "home/.freej/plugins"
    other = root / "other"
    user.mkdir(parents=True)
    other.mkdir()
    for directory, ident, title, kind, model, extra in [
        (user, "filter", "Clock filter", 0, 1, []),
        (other, "alternative", "Clock filter", 0, 1, []),
        (user, "source", "Clock source", 1, 0, []),
        (user, "mixer2", "Clock mixer", 2, 2, []),
        (user, "mixer3", "Clock mixer", 3, 2, []),
        (user, "invalid", "Packed source", 1, 2, []),
        (user, "failed", "Failed", 0, 1, ["-DFAIL_INIT"]),
        (user, "missing", "Missing", 0, 1, ["-DMISSING_DEINIT"]),
    ]:
        build("frei0r-fixture.c", directory / (ident + ".so"), f'-DID="{ident}"',
              f'-DTITLE="{title}"', f"-DKIND={kind}", f"-DMODEL={model}", *extra)
    build("freeframe-fixture.c", user / "ff.so")
    command = [str(args.driver.resolve())]
    if args.real:
        for module in args.real:
            shutil.copyfile(module, user / module.name)
        command.append("--real")
    trace = root / "trace"
    env = dict(os.environ, HOME=str(root / "home"), SDL_VIDEODRIVER="dummy", FREEJ_EFFECT_TRACE=str(trace))
    subprocess.run(command, cwd=root, env=env, timeout=30, check=True)
    events = [s.split() for s in trace.read_text().splitlines()]
    for ident, constructions in [("filter", 2), ("source", 2), ("alternative", 0),
                                 ("mixer2", 0), ("mixer3", 0), ("invalid", 0)]:
        seq = [event for name, event in events if name == ident]
        counts = Counter(seq)
        assert seq[0] == "init" and seq[-1] == "deinit", (ident, seq)
        assert counts["init"] == counts["deinit"] >= 1, (ident, seq)
        assert counts["construct"] == counts["destruct"] == constructions, (ident, seq)
        if ident in ("filter", "source"):
            assert counts["init"] == 1 and counts["construct-failed"] == 1, (ident, seq)
        print(ident, dict(counts))
    assert not any(name == "missing" for name, event in events), events
    failed = [event for name, event in events if name == "failed"]
    assert failed and set(failed) == {"init-failed"}, failed
    if args.cli:
        script_automation(root, args.cli, env)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--cli", type=Path, default=os.environ.get("FREEJ_EFFECT_CLI"))
    parser.add_argument("--driver", type=Path, default=Path("frei0r-host-test"))
    parser.add_argument("--real", nargs=2, type=Path, metavar=("FILTER", "SOURCE"))
    parser.add_argument("--work", type=Path)
    args = parser.parse_args()
    if args.work:
        args.work.mkdir()
        run(args.work.resolve(), args)
    else:
        with tempfile.TemporaryDirectory() as scratch:
            run(Path(scratch), args)
