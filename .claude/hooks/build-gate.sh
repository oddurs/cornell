#!/bin/sh
# build-gate.sh — the one rule in CLAUDE.md that a person cannot be trusted
# with: never commit a state that does not build.
#
# It is not a pre-commit hook, because a pre-commit hook lives in .git/hooks,
# which is not tracked, is not copied into a worktree, and therefore protects
# exactly the clone it was installed in. This runs in front of the tool call
# instead — Claude Code hands every Bash invocation to it on stdin, and a
# non-zero exit stops the command and hands the reason back to the agent.
#
# It is the first of two gates and the cheap one. The second is CI, which runs
# the same build on a machine that is not this one and with a compiler that is
# not this one, and which the pull request cannot be merged without. This gate
# exists so that the cycle is seconds rather than minutes; it is not the
# authority.
#
# The cost is that it fires on every shell command in the session, so the
# first thing it does is decide it has nothing to do.
#
# Escape hatch: --no-verify, spelled the way git spells it.

set -eu

command=$(python3 -c 'import json,sys; print(json.load(sys.stdin).get("tool_input",{}).get("command",""))' 2>/dev/null || true)

case "$command" in
    *"git commit"*) ;;
    *) exit 0 ;;
esac

case "$command" in
    *--no-verify*) exit 0 ;;
esac

root=$(git rev-parse --show-toplevel 2>/dev/null) || exit 0
cd "$root"

# Before apps/main.cpp exists there is nothing to link, and `make` says so at
# some length. The gate asserts that the tree builds, not that it is finished.
set -- apps/*.cpp
if [ -e "$1" ] && ! out=$(make 2>&1); then
    printf 'Refusing the commit: the tree does not build.\n\n%s\n' "$out" >&2
    exit 2
fi

if ! out=$(cairn check 2>&1); then
    printf 'Refusing the commit: cairn check fails.\n\n%s\n' "$out" >&2
    exit 2
fi

exit 0
