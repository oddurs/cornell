---
id: 38
title: 'Shadow ray offset: the Waechter construction, not an epsilon'
type: optics
status: backlog
milestone: v0.2
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: geometry
effort: s
---

## What it is

The fix for shadow acne, done properly the first time.

A fixed epsilon is wrong at both ends: too small and surfaces self-shadow,
too large and contact shadows detach and objects float. Both failures are
scale-dependent, so any epsilon that works for the Cornell box is wrong for a
scene in millimetres.

## What it must derive

The offset from the magnitude of the intersection point itself. The integer-
based construction from *Ray Tracing Gems* chapter 6 is about ten lines,
offsets along the geometric normal by an amount proportional to the floating-
point spacing at that coordinate, and is correct at every scale.

## What is not modelled

Nothing, but the comment should be explicit that this is a floating-point
problem and not an optics problem — it is the one place in the project where
the machine intrudes on the physics, and pretending otherwise would be
dishonest.

## Acceptance criteria

- [ ] A scene at 1000x scale and 0.001x scale render identically
- [ ] No `EPSILON` constant anywhere in the project
