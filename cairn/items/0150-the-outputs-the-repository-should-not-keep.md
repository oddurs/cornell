---
id: 150
title: The outputs the repository should not keep
type: chore
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: build
effort: s
---

An image written by this program is a *result*, not a source, and every one
of them is reproducible from a command in one line. None of them belong in
the history.

`.gitignore` covered `cornell`, `*.ppm`, `*.pfm` and `*.exr` and stopped
there, which was right until the moment somebody wanted to look at the
picture. macOS cannot open a PPM, so looking at it means converting it, and
the conversion lands a `.png` in the repository root that `git status` then
reports forever.

The cost is not the untracked file. It is that `.claude/propose` refuses on
a dirty tree, so the next time the loop stops it will be for a reason that
has nothing to do with the work in front of you.

## Acceptance criteria

- [x] Looking at the output cannot make the tree dirty
- [x] The list covers the formats a person converts *to*, not only the ones
      the program writes
