---
id: 99
title: An environment light, and sampling it by importance
type: optics
status: backlog
milestone: v0.8
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: light
effort: l
---

A sphere of incident radiance at infinity: the sky, a studio, a photographed
room. Sampling it well means building a 2D distribution over the image and
inverting it, which is the same inversion machinery the spectral upsampling
used and should share it.

- [ ] The pdf includes the sin(theta) from the spherical parameterisation, and
      the comment says why a naive 2D sample is wrong at the poles
- [ ] chi2 passes over a real environment image, not a synthetic one
