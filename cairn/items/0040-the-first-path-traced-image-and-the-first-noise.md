---
id: 40
title: The first path-traced image, and the first noise
type: instrument
status: backlog
milestone: v0.2
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: instrument
effort: s
---

## What it witnesses

That the integrator works. A box, a light, matte walls, and an image that is
correct and extremely noisy.

## What it prints

`./cornell render --spp 16` and `--spp 4096`, side by side, so that the
project's relationship with noise is established early: it is the variance of
an estimator, it falls as the inverse square root of the sample count, and
v0.5 will measure that rather than assert it.
