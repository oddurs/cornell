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
- [x] `land` will not treat "no checks configured" as "checks passed"
- [x] The loop is written down where an agent will read it, in `AGENTS.md`
