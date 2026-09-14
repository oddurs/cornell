---
id: 149
title: The loop the work happens in, and the gate in front of it
type: chore
status: done
milestone: v0.1
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: build
effort: m
---

Several agents working one backlog in one checkout will overwrite each
other's edits and interleave each other's commits, and the log here is meant
to be read. So: one item, one worktree, one branch, one commit, one pull
request, and `main` a single line of argument with no merge commits in it.

Every change arrives as a pull request, including the one-line ones and
including this item. Not for review — this is a project of one — but because
a pull request is the only place a change is checked by something that is
not the machine that wrote it.

Two gates, and they are not the same gate. The hook is fast and local and
proves the tree builds *here*. CI is slow and clean and proves it builds on
Linux, under gcc, from a fresh checkout, which is three assumptions the
local one quietly makes. The second is the authority; the first exists so
the cycle is seconds.

## Acceptance criteria

- [x] The build rule from CLAUDE.md is enforced rather than remembered, and
      not by a `.git/hooks/pre-commit`, which is untracked and is not copied
      into a worktree
- [x] CI builds every pull request under gcc as well as clang, which is the
      only portability test this project has
- [x] `land` will not treat "no checks configured" as "checks passed", and
      will not mistake "not queued yet" for either of them
- [x] The loop is written down where an agent will read it, in `AGENTS.md`

## 2026-09-13

The criterion about 'no checks configured' was ticked and was false. land asked gh for the checks immediately after the push, was told there were none because none had been queued yet, said so and merged; the pull request went green about forty seconds later. It now waits up to 60s for a check to appear and treats a genuine absence as a failure rather than a pass. Separately, gh pr merge --delete-branch tries to check out the base branch locally to clean up, which cannot work when main is held by the main worktree - which is the arrangement the whole loop is built on.

## 2026-09-13

CI told us something on its first green run on main: actions/checkout@v4 targets Node 20, which the runners now force onto Node 24 and will eventually stop forcing. Bumped to v5. This is the kind of rot house rule 4's fifteen-year claim is about - it just lives in the CI rather than in the program, and CI is the only thing that was ever going to notice.

## 2026-09-13

Three tooling faults found by the first code review. (1) land's wait loop broke on a zero exit from 'gh pr checks', which exits non-zero both while checks are pending and when one has failed - so it never broke early, burnt its full 60s on every pull request, and then reported 'no checks appeared' both for slow CI and for a genuinely failed gcc build. It said exactly that about the 0041 branch, which had failed. It now waits on the presence of output rather than on an exit code. (2) The CI smoke test ran './cornell render 120 4', and main.cpp treats every bare number as the width, so 4 overwrote 120 and it rendered a 4x4 image - about a thousand paths instead of a million. (3) The build gate refused every commit with 'cairn check fails' when cairn is merely absent, which is the state of any fresh clone and of CI itself.
