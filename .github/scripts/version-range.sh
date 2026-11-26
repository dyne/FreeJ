#!/usr/bin/env bash
set -euo pipefail

fail() {
    printf '%s\n' "$*" >&2
    exit 1
}

[[ $# == 1 && $1 =~ ^[0-9a-f]{40}$ ]] || fail 'Expected the full release candidate SHA.'
head=$(git rev-parse --verify HEAD)
[[ $head == "$1" ]] || fail "Expected checkout $1, found $head."
[[ $(git rev-parse --is-shallow-repository) == false ]] || fail 'Release versioning needs full history and tags.'

baseline=2afb631cb3a429fdf312627ef904ec32b51e812d
baseline_config=$(git show "$baseline:configure.ac") || fail "Could not read the FreeJ 1.0 baseline $baseline."
grep -Fqx 'AC_INIT([FreeJ],[1.0],[freej@lists.dyne.org],[freej])' <<< "$baseline_config" ||
    fail "The approved baseline $baseline must declare FreeJ 1.0."
git merge-base --is-ancestor "$baseline" "$head" ||
    fail "The FreeJ 1.0 baseline $baseline must be reachable from $head."

baseline_tag=$(git rev-parse --verify --quiet 'refs/tags/v1.0.0^{commit}') ||
    fail "Release prerequisite: v1.0.0 must be provisioned on $baseline."
[[ $baseline_tag == "$baseline" ]] || fail "v1.0.0 points to $baseline_tag; expected $baseline."

# Select a stable FreeJ release tag on this history. The action's default
# tag query orders all repository tags by commit date, regardless of ancestry.
tags=$(git tag --merged "$head" --sort=-version:refname --list 'v[0-9]*')
stable_version='^v(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)$'
while IFS= read -r tag; do
    [[ $tag =~ $stable_version ]] || continue
    target=$(git rev-parse --verify "refs/tags/$tag^{commit}")
    if [[ $tag == v1.0.0 ]] ||
        { [[ $target != "$baseline" ]] && git merge-base --is-ancestor "$baseline" "$target"; }; then
        printf 'head=%s\ntag=%s\n' "$head" "$tag"
        exit 0
    fi
done <<< "$tags"
fail "No stable release tag reaches $head from the approved baseline."
