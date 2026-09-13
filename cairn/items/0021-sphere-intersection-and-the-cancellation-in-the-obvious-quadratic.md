---
id: 21
title: Sphere intersection, and the cancellation in the obvious quadratic
type: optics
status: planned
milestone: v0.1
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: geometry
effort: s
---

## What it is

The first thing a ray hits. Also the first place where the algebraically
correct formula and the numerically correct formula differ, which is worth
the half page it takes to explain.

## What it must derive

The stable root. Solving `at² + bt + c = 0` with the quadratic formula loses
most of its significant figures when `b² >> 4ac` — a ray grazing a large
sphere from far away, which is the common case, not the exotic one. The fix
is to compute the well-conditioned root and get the other from `t1·t2 = c/a`.

## Acceptance criteria

- [ ] A test that fails with the naive formula and passes with this one
- [ ] The comment says what is lost, not what is computed
