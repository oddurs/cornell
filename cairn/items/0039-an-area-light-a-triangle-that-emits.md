---
id: 39
title: 'An area light: a triangle that emits'
type: optics
status: backlog
milestone: v0.2
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: light
effort: s
---

The simplest emitter that produces a soft shadow, and the point at which the
project's second thesis becomes checkable: nobody writes a soft shadow, the
penumbra is the light's solid angle varying across the floor.

At v0.2 it is found only by paths that happen to hit it. Sampling it directly
is v0.8, and the difference between the two images at equal sample count is
the best advertisement multiple importance sampling has.

- [ ] One-sided by default, and the comment says why two-sided emitters are a
      modelling convenience rather than a physical object
