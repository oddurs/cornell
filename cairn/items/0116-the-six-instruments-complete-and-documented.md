---
id: 116
title: The six instruments, complete and documented
type: instrument
status: backlog
milestone: v1.0
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: instrument
effort: m
---

## What it witnesses

The project, finished enough to be read.

`render`, `furnace`, `chi2`, `converge`, `spectrum`, `swatch`, `spec` and
`verify` — a camera, a furnace, a statistician, a convergence plot, a
spectrometer, a colour chart, a specification sheet, and an inspection sheet.

## What it may not do

Reach into the physics. It is worth re-checking at this milestone that none of
them has quietly grown a private code path to make a figure come out, because
that is the failure mode that would make every number in the README worthless.

## Acceptance criteria

- [ ] `./cornell` with no arguments explains all of them in one screen
- [ ] A grep for physics in `apps/` finds nothing that should be in `include/`
