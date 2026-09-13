---
id: 117
title: 'README: the final argument, with every figure re-checked'
type: prose
status: backlog
milestone: v1.0
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: prose
effort: l
---

## What it has to explain

The whole thing, for somebody who has never rendered anything: what the box
is, what the two claims are, what was done to check them, and what came out.

House rule 6 applies hardest here. Every figure in it is a copy of something
the program printed, the program has changed at every milestone, and copies
rot. `windsor`'s README drifted three times and always the same way — an edit
anchored on a string that had been reformatted, applied without checking that
the anchor existed, silently doing nothing.

## Acceptance criteria

- [ ] Every number carries the command that produced it
- [ ] Re-run `verify`, `furnace`, `chi2`, `converge` and reconcile each one
- [ ] Any programmatic edit asserts its anchor exists first
