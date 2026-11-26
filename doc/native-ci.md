# Linux native CI

The build workflow runs the compatibility and static profiles on the Debian 13
amd64 image shown in [build.yml](../.github/workflows/build.yml). It runs on pull
requests, pushes and manual dispatch. Both jobs verify the checked-out event SHA
before building. Pull request runs normally check the synthetic merge revision.
Use a topic-branch push run when the check must identify the topic commit itself.
Manual dispatch becomes available after the workflow reaches the default branch.

## Calculate the release version

The [version workflow](../.github/workflows/version.yml) runs on pushes to
`freej-revamp` and supports manual dispatch on branches once the workflow is
available on the default branch. Its read-only job checks out the event SHA,
reads full history and tags, and reports `current`, `nextStrict`, and `bump`.
Pull requests run the native build workflow independently.

Before the first authorized release-branch push, a maintainer with explicit
tag authority must provision `v1.0.0` in the release repository, pointing at
`2afb631cb3a429fdf312627ef904ec32b51e812d`. That commit declares package
version `1.0` in `configure.ac` and must be an ancestor of the checkout. Treat
published release tags as immutable. A missing or misplaced baseline fails
the version job. Later runs select the highest stable `vMAJOR.MINOR.PATCH` tag
reachable from the checkout and descended from the approved baseline.
Historical Debian packaging tags and prerelease tags do not enter this range.

The pinned action compares the selected tag to the event SHA. An empty range
or a range without a version-changing commit reports the current version as
`nextStrict` and `none` as the job's bump. The job calculates package SemVer.
The libtool `FREEJ_VERSION_INFO` triplet in `configure.ac` tracks the library
interface separately.

The local regression fixture runs the pinned Node 24 bundle against a private
Git history and local GitHub API responses. With Node 24 and Python 3 with
PyYAML available, run from the FreeJ checkout:

```bash
scratch=$(mktemp -d)
git clone --no-checkout https://github.com/ietf-tools/semver-action.git "$scratch/semver-action"
git -C "$scratch/semver-action" checkout --detach 46feaa27e7375da0e3d602721e9fb9573598b880
python3 tests/version-workflow.py "$scratch/semver-action"
```

## Native build jobs

The job installs Debian build tools and runtime packages, records the exact
package versions, and builds the SDKs using the same checksummed source inputs
listed in [Native profiles](native-profiles.md). The hosted cache stores SDK
compiler objects under a Debian version, architecture, compiler, profile and
lock/recipe key. Each job starts a separate empty compiler cache for the FreeJ
cold build, then repeats the build with that cache populated. Both passes build
in a clean tree, run `make check`, stage FreeJ and Qfreej, audit the binaries,
and run the profile's show. The comparison checks capability selection,
executed checks, and frei0r frame bytes. Compatibility also compares the
canonical replay frames.
The warm run must record compiler cache hits. The archive and each staged
binary have individual SHA-256 digests.

Compatibility replays the pinned Brightness show through SDL dummy and through
X11 on Xvfb. The deliberately reversed fixture must exit 1 with the reordered
cue's RGB mismatch. The OSC fixture observes packets on loopback through argument
parsing and dispatch. The synthetic PCM fixture observes the known impulse at
FreeJ's JACK input, then checks the historical 25 windows of 16 zero-valued FFT
bands. JS callbacks still fail and the capture-to-FFT path remains silent.
These checks leave OSC-to-visible and audio-to-visible response unmeasured.

The static profile renders an image and a built-in effects show in the installed
CLI, runs the effects show through the installed Qt/XCB frontend on Xvfb, and
connects an installed factory consumer to a separate JACK dummy daemon. ELF and
file-access checks cover the isolated stage. The parity job compares the two
profiles' digests for the same 24 frames, using the pinned native modules and
built-in archive. The [capability ledger](capabilities.md) records the historical
reference.

Each profile uploads its own artifact named with the checkout SHA, run ID and
attempt. It contains the warm `stage.tar.gz`, SDK build logs, cold and warm
test logs, capabilities, link audits and identities. Run `sha256sum -c SHA256SUMS`
from the artifact's `evidence` directory. These are CI build artifacts. Use the
recorded prefix and SDK inputs when reproducing the compatibility loader
paths or using installed build metadata.

The job summary reports whether the CLI and Qt executables built, staged and
passed the link audits, then lists the completed runtime checks. The coverage
report uses the staged package's selected capabilities to identify disabled
backends. Physical checks are named separately. Runtime logs and `checks.json`
retain the executed fixture identities and outcomes.

## Run the same recipes on Debian 13 amd64

Provision the packages listed in the workflow. From a FreeJ checkout, choose
a new scratch directory outside the checkout. Both invocations require distinct
empty work and evidence paths. For each of `compatibility` and `static`, run:

```bash
profile=compatibility
scratch=$(mktemp -d)
export CCACHE_DIR="$scratch/sdk-ccache"
tests/native-ci-prepare.sh "$profile" "$scratch/sdk"
tests/native-ci-run.sh "$profile" "$scratch/sdk" "$scratch/evidence"
```

Both runs need a JACK 1 daemon with a dummy driver, a writable daemon socket
directory, and Xvfb. The static runtime file audit also needs Linux Landlock
and ptrace support. `FREEJ_CI_JACKD` can point to a separate JACK installation.
The static Qt runtime check uses `libXtst.so.6` to drive the File menu on Xvfb.
The build recipes default to four make jobs. Set `FREEJ_CI_JOBS` to adjust this.
After both runs, compare their `evidence/warm/frei0r-frames.bin` files with `cmp`.

## Collect evidence on a trusted Linux rig

Use a checkout you have reviewed and a workstation with the intended display,
audio interface and controllers. Run the same software recipes locally first.
Save their evidence directories alongside the rig data and keep the device
checks on the workstation. The hosted job runs with Xvfb and a dummy JACK
server and needs only software fixtures.

Record the exact checkout, staged executable hash and runtime setup in the rig
log. Record the display mode and refresh rate, audio interface, sample rate
and buffer size, camera or sensor rate, device nodes and controller mappings.
Run the established show from a clean directory with the staged executable.
Check fullscreen and SDL input, physical JACK loopback and recording sync,
V4L2 camera and DV capture, and any ALSA MIDI, joystick, Wii or ViMo devices
used by the show. Give each check a pass, failure or specific reason it could
not run. The [capability ledger](capabilities.md) identifies the entry points
and dependencies for each device.

Follow the [stage measurement procedure](../tests/show-replay/timing-budget.md#outstanding-stage-measurements)
for present-to-photon latency and physical audio loopback. Keep the raw
capture, common timing reference and the software trace from at least five
warmed runs per workload. Record only observed measurements for the current
revision and keep the physical evidence local to the trusted rig.
