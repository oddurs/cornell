---
id: 38
title: 'Shadow ray offset: the Waechter construction, not an epsilon'
type: optics
status: doing
milestone: v0.2
assignee: Oddur Sigurdsson
claimed: 2026-09-13
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

- [x] A scene at 1000x scale and 0.001x scale render identically — done in
      item 0040, once there was a renderer: the same room at 1×, 1000× and
      0.001× produces bit-for-bit identical images, every pixel the same
      double, mean radiance 0.410179 in all three
- [x] No `EPSILON` constant anywhere in the project — `ray_epsilon` is gone,
      `t_min` defaults to zero, and the only occurrences of the word left are
      the two files explaining why there is no such constant

## 2026-09-13

Geometric scale-invariance measured over nine orders of magnitude. Spawning a bounce straight up the normal from a raw hit point self-intersects on over half of 200000 attempts at every scale from 1e-3 to 1e6 m - 103292, 106005, 107940, 108727 - every one at exactly t = 0, now visible because t_min is zero rather than an epsilon hiding it. From the offset point: zero at every scale. The offset moves the point by 2.78e-14 to 2.98e-14 of the scene scale across six decades, i.e. the same relative step with nothing in the code that knows how big the scene is. The published construction is float and three of its constants do not port: 2^-16 is 15 micrometres, which in a two-metre box is enormous. The ulp count carries (it is a claim about roundings, not about the type); the near-zero fallback was rethought rather than rescaled, and uses ulps of the point's largest coordinate, because the error in a coordinate that is nearly zero comes from the other two. shadow_ray was deleted rather than ported: it was built out of two epsilon subtractions, was never called, and v0.8 needs a both-ends version that is a different function.

## 2026-09-13

Closed properly in 0040. The same room rendered at 1x, 1000x and 0.001x is bit-for-bit identical - every pixel the same double, mean radiance 0.410179 in all three. Radiance is invariant under a uniform scaling, so any length hidden in the spawn logic would have shown up as a six-order-of-magnitude difference in the contact shadows.
