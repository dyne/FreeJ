#!/usr/bin/env python3
"""Exercise the pinned action bundle against a private Git history."""

import argparse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
from pathlib import Path
import subprocess
import tempfile
import threading
import unittest
from urllib.parse import parse_qs, unquote, urlsplit

import yaml


SOURCE = Path(__file__).resolve().parents[1]
BASELINE = "2afb631cb3a429fdf312627ef904ec32b51e812d"
BREAKING_MESSAGE = """refactor(release)!: adopt the FreeJ 2.0 native build profiles

BREAKING CHANGE: The static distribution embeds its selected effects.
Install the native compatibility build to load plugins from disk."""


def git(directory, *args):
    return subprocess.check_output(["git", *args], cwd=directory, text=True).strip()


def load_yaml(path):
    return yaml.load(path.read_text(), Loader=yaml.BaseLoader)


class VersionWorkflow(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        workflow = load_yaml(SOURCE / ".github/workflows/version.yml")
        cls.job = workflow["jobs"]["version"]
        cls.step = next(step for step in cls.job["steps"] if step.get("id") == "semver")
        pin = cls.step["uses"].removeprefix("ietf-tools/semver-action@")
        assert git(cls.action_directory, "rev-parse", "HEAD") == pin
        assert git(cls.action_directory, "status", "--porcelain") == ""
        meta = load_yaml(cls.action_directory / "action.yml")
        assert subprocess.check_output(["node", "--version"], text=True).split(".")[0] == \
            meta["runs"]["using"].replace("node", "v")
        cls.entrypoint = cls.action_directory / meta["runs"]["main"]
        assert git(cls.action_directory, "hash-object", cls.entrypoint) == \
            git(cls.action_directory, "rev-parse", "HEAD:" + meta["runs"]["main"])
        cls.defaults = {
            "INPUT_" + name.upper(): details.get("default", "")
            for name, details in meta["inputs"].items()
        }
        temporary = tempfile.TemporaryDirectory(prefix="freej-version-")
        cls.addClassCleanup(temporary.cleanup)
        cls.work = Path(temporary.name)
        cls.repo = cls.work / "repo"
        git(SOURCE, "clone", "--quiet", "--single-branch", "--no-tags", "--no-local",
            str(SOURCE), str(cls.repo))
        release = git(cls.repo, "log", "-1", "--first-parent", "--format=%H",
                      "--fixed-strings", f"--grep={BREAKING_MESSAGE.splitlines()[0]}",
                      f"{BASELINE}..HEAD")
        cls.pre_release = git(cls.repo, "rev-parse", f"{release}^" if release else "HEAD")
        git(cls.repo, "config", "user.name", "FreeJ Version Fixture")
        git(cls.repo, "config", "user.email", "fixture@example.invalid")
        git(cls.repo, "config", "commit.gpgsign", "false")

    def setUp(self):
        git(self.repo, "checkout", "--quiet", "--detach", BASELINE)
        for tag in git(self.repo, "tag", "--list").splitlines():
            git(self.repo, "tag", "--delete", tag)
        git(self.repo, "tag", "v1.0.0", BASELINE)

    def commit(self, message):
        git(self.repo, "commit", "--quiet", "--allow-empty", "-m", message)
        return git(self.repo, "rev-parse", "HEAD")

    def compare_range(self, head):
        return subprocess.run(
            ["bash", SOURCE / ".github/scripts/version-range.sh", head],
            cwd=self.repo, text=True, capture_output=True, check=False,
        )

    def assert_rejected(self, head, diagnostic):
        result = self.compare_range(head)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, "")
        self.assertIn(diagnostic, result.stderr)

    def action_outputs(self, head, tag, additional=""):
        """Supply GitHub's read endpoints from Git; execute the unmodified bundle."""
        repo = self.repo
        requests = []

        class GitHubFixture(BaseHTTPRequestHandler):
            def reply(self, payload, status=200):
                body = json.dumps(payload).encode()
                self.send_response(status)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)

            def do_POST(self):
                body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
                variables = body.get("variables", {})
                requested = variables.get("tag", "")
                requests.append(("GraphQL", requested))
                if (
                    self.path != "/graphql"
                    or "query singleTag" not in body.get("query", "")
                    or variables != {"owner": "local", "repo": "freej", "tag": f"refs/tags/{tag}"}
                ):
                    self.reply({"message": "Expected the exact tag query"}, 400)
                    return
                oid = git(repo, "rev-parse", requested)
                self.reply({"data": {"repository": {"ref": {"name": tag, "target": {"oid": oid}}}}})

            def do_GET(self):
                url = urlsplit(self.path)
                requested = unquote(url.path)
                page = int(parse_qs(url.query).get("page", ["0"])[0])
                requests.append(("REST", requested, page))
                if requested != f"/repos/local/freej/compare/{tag}...{head}" or page < 1:
                    self.reply({"message": "Expected the tag and the exact head SHA"}, 400)
                    return
                shas = git(repo, "rev-list", "--reverse", f"refs/tags/{tag}..{head}").splitlines()
                commits = [
                    {"sha": sha, "commit": {"message": git(repo, "show", "-s", "--format=%B", sha)}}
                    for sha in shas[(page - 1) * 100:page * 100]
                ]
                self.reply({"total_commits": len(shas), "commits": commits})

            def log_message(self, *args):
                pass

        with ThreadingHTTPServer(("127.0.0.1", 0), GitHubFixture) as server:
            worker = threading.Thread(target=server.serve_forever, daemon=True)
            worker.start()
            output = self.work / "output"
            environment = self.work / "env"
            output.write_text("")
            environment.write_text("")
            env = {
                "PATH": os.environ["PATH"],
                "GITHUB_REPOSITORY": "local/freej",
                "GITHUB_SHA": head,
                "GITHUB_API_URL": f"http://127.0.0.1:{server.server_port}",
                "GITHUB_OUTPUT": str(output),
                "GITHUB_ENV": str(environment),
                **self.defaults,
            }
            for key, value in self.step["with"].items():
                env["INPUT_" + key.upper()] = {
                    "token": "local-fixture-token",
                    "branch": head,
                    "fromTag": tag,
                }.get(key, value)
            env["INPUT_ADDITIONALCOMMITS"] = additional
            try:
                action = subprocess.run(
                    ["node", self.entrypoint], cwd=repo, env=env,
                    text=True, capture_output=True, timeout=30, check=False,
                )
            finally:
                server.shutdown()
                worker.join()
        self.assertEqual(action.returncode, 0, action.stdout + action.stderr)
        self.assertEqual(requests, [
            ("GraphQL", f"refs/tags/{tag}"), ("REST", f"/repos/local/freej/compare/{tag}...{head}", 1),
        ])
        outputs = {}
        lines = output.read_text().splitlines()
        self.assertEqual(len(lines) % 3, 0)
        for offset in range(0, len(lines), 3):
            name, value, end = lines[offset:offset + 3]
            key, delimiter = name.split("<<", 1)
            self.assertEqual(end, delimiter)
            outputs[key] = value
        return outputs

    def assert_version(self, head, version, bump, tag="v1.0.0", additional=""):
        comparison = self.compare_range(head)
        self.assertEqual(comparison.returncode, 0, comparison.stderr)
        self.assertEqual(comparison.stdout.splitlines(), [f"head={head}", f"tag={tag}"])
        result = self.action_outputs(head, tag, additional)
        current, next_strict, actual_bump = result["current"], result["nextStrict"], result.get("bump")
        self.assertEqual((current, next_strict, actual_bump), (tag, version, bump))
        print(f"{head} current={current} nextStrict={next_strict} bump={actual_bump or '(unset)'}", flush=True)
        return result

    def test_ordinary_patch(self):
        self.assert_version(self.commit("fix(movie): preserve the decoded frame"), "1.0.1", "patch")

    def test_ordinary_minor(self):
        self.assert_version(self.commit("feat(effects): add a portable effect"), "1.1.0", "minor")

    def test_breaking_message_on_actual_history(self):
        git(self.repo, "checkout", "--quiet", "--detach", self.pre_release)
        self.assert_version(self.pre_release, "1.1.0", "minor")
        head = self.commit(BREAKING_MESSAGE)
        self.assertEqual(git(self.repo, "log", "-1", "--format=%B"), BREAKING_MESSAGE)
        self.assert_version(head, "2.0.0", "major")

    def test_additional_breaking_message_on_actual_history(self):
        git(self.repo, "checkout", "--quiet", "--detach", self.pre_release)
        self.assert_version(self.pre_release, "2.0.0", "major", additional=BREAKING_MESSAGE)

    def test_singular_breaking_footer(self):
        self.assert_version(self.commit(BREAKING_MESSAGE.replace(")!", ")")), "2.0.0", "major")

    def test_empty_range(self):
        self.assert_version(BASELINE, "1.0.0", None)

    def test_no_bump_with_commits(self):
        self.assert_version(self.commit("docs: explain the configuration"), "1.0.0", "none")

    def test_exact_head_repeatable_after_branch_moves(self):
        head = self.commit("fix(movie): preserve the decoded frame")
        git(self.repo, "update-ref", "refs/heads/freej-revamp", head)
        previous = self.assert_version(head, "1.0.1", "patch")
        later = self.commit(BREAKING_MESSAGE)
        git(self.repo, "update-ref", "refs/heads/freej-revamp", later)
        git(self.repo, "checkout", "--quiet", "--detach", head)
        self.assertEqual(self.assert_version(head, "1.0.1", "patch"), previous)

    def test_later_annotated_release_excludes_unrelated_tags(self):
        older = self.commit("feat(effects): adopt a profile")
        git(self.repo, "tag", "v1.9.0", older)
        released = self.commit("feat(effects): add a portable effect")
        git(self.repo, "tag", "-a", "v1.10.0", "-m", "fixture release", released)
        git(self.repo, "tag", "v88.0.0", BASELINE)
        git(self.repo, "tag", "v99.0.0", f"{BASELINE}^")
        head = self.commit("fix(movie): preserve the decoded frame")
        git(self.repo, "tag", "v4.0.0-rc.1", head)
        git(self.repo, "tag", "debian-freej-99.99.99-1", head)
        git(self.repo, "checkout", "--quiet", "--detach", BASELINE)
        git(self.repo, "tag", "v3.0.0", self.commit(BREAKING_MESSAGE))
        git(self.repo, "checkout", "--quiet", "--detach", head)
        self.assert_version(head, "1.10.1", "patch", tag="v1.10.0")

    def test_missing_baseline_with_only_debian_tag(self):
        git(self.repo, "tag", "--delete", "v1.0.0")
        git(self.repo, "tag", "debian-freej-1.0.0-1")
        self.assert_rejected(BASELINE, "v1.0.0 must be provisioned")

    def test_wrong_baseline_tag(self):
        head = self.commit("fix(movie): preserve the decoded frame")
        git(self.repo, "tag", "--force", "v1.0.0", head)
        self.assert_rejected(head, f"expected {BASELINE}")

    def test_unreachable_baseline(self):
        git(self.repo, "checkout", "--quiet", "--detach", f"{BASELINE}^")
        self.assert_rejected(git(self.repo, "rev-parse", "HEAD"), "must be reachable")

    def test_checkout_must_match_comparison_head(self):
        self.assert_rejected(self.pre_release, "Expected checkout")

    def test_shallow_checkout_is_rejected(self):
        shallow = self.work / "shallow"
        git(SOURCE, "clone", "--quiet", "--depth=1", "--no-tags", SOURCE.as_uri(), str(shallow))
        self.repo = shallow
        self.assert_rejected(git(shallow, "rev-parse", "HEAD"), "needs full history and tags")

    def test_workflow_wiring_and_independent_pr_builds(self):
        workflow = load_yaml(SOURCE / ".github/workflows/version.yml")
        self.assertEqual(set(workflow["on"]), {"push", "workflow_dispatch"})
        self.assertEqual(workflow["on"]["push"]["branches"], ["freej-revamp"])
        self.assertEqual(workflow["permissions"], {"contents": "read"})
        self.assertEqual(self.job["if"], "github.ref_type == 'branch'")
        checkout = self.job["steps"][0]["with"]
        self.assertEqual(checkout["ref"], "${{ github.sha }}")
        self.assertEqual(checkout["fetch-depth"], "0")
        self.assertEqual(checkout["persist-credentials"], "false")
        self.assertEqual(self.step["with"]["branch"], "${{ steps.range.outputs.head }}")
        self.assertEqual(self.step["with"]["fromTag"], "${{ steps.range.outputs.tag }}")
        self.assertEqual(self.job["outputs"], {
            "current": "${{ steps.semver.outputs.current }}",
            "nextStrict": "${{ steps.semver.outputs.nextStrict }}",
            "bump": "${{ steps.semver.outputs.bump || 'none' }}",
        })
        native = load_yaml(SOURCE / ".github/workflows/build.yml")
        self.assertIn("pull_request", native["on"])
        self.assertNotIn("needs", native["jobs"]["native"])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action_directory", type=Path, help="clean checkout of the pinned action")
    VersionWorkflow.action_directory = parser.parse_args().action_directory.resolve()
    unittest.main(argv=[__file__, "-v"])
