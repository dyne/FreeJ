# Linux native CI

The build workflow runs the compatibility and static profiles on the Debian 13
amd64 image shown in [build.yml](../.github/workflows/build.yml). It runs on pull
requests, pushes and manual dispatch. Both jobs verify the checked-out event SHA
before building. Pull request runs normally check the synthetic merge revision.
Use a topic-branch push run when the check must identify the topic commit itself.
Manual dispatch becomes available after the workflow reaches the default branch.

The job installs Debian build tools and runtime packages, records the exact
package versions, and builds the SDKs using the same checksummed source inputs
listed in [Native profiles](native-profiles.md). The hosted cache stores SDK
compiler objects under a Debian version, architecture, compiler, profile and
lock/recipe key. Each job starts a separate empty compiler cache for the FreeJ
cold build, then repeats the build with that cache populated. Both passes build
in a clean tree, run `make check`, stage FreeJ and Qfreej, audit the binaries,
and run the profile's show. The comparison checks capability selection and
executed checks, plus canonical frame hashes in the compatibility profile.
The warm run must record compiler cache hits. The archive and each staged
binary have individual SHA-256 digests.

Compatibility exercises the SDL dummy video path with the pinned Brightness
effect. The deliberately reversed fixture must exit 1 with the reordered cue's
RGB mismatch. The static profile runs its installed image show, Qt/XCB startup
and installed factory with Xvfb and a separate JACK dummy daemon. The static
profile's built-in effects arrive with the frei0r integration. See the
[capability ledger](capabilities.md) for historical backend observations.

Each profile uploads its own artifact named with the checkout SHA, run ID and
attempt. It contains the warm `stage.tar.gz`, SDK build logs, cold and warm
test logs, capabilities, link audits and identities. Run `sha256sum -c SHA256SUMS`
from the artifact's `evidence` directory. These are CI build artifacts. Use the
recorded prefix and SDK inputs when reproducing the compatibility loader
paths or using installed build metadata.

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

The static run needs a JACK 1 daemon with a dummy driver, a writable daemon
socket directory, Xvfb, and Linux Landlock and ptrace support for the runtime
file audit. `FREEJ_CI_JACKD` can point to a separate JACK installation. The
build recipes default to four make jobs. Set `FREEJ_CI_JOBS` to adjust this.
