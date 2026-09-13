---
id: 93
title: Sampling a triangle's solid angle, not its area
type: optics
status: backlog
milestone: v0.8
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: light
effort: l
---

## What it is

Arvo, 1995. Choosing a point uniformly on a light's surface is easy and wrong:
it puts most samples where the shading point can barely see them, and the
conversion from area to solid angle then divides by a squared distance that
varies enormously across the light. The variance shows up as noise in exactly
the region a soft shadow occupies.

Sampling uniformly in solid angle — projecting the triangle onto the sphere
around the shading point and sampling the spherical triangle — removes that
entirely.

## What it must derive

The spherical excess, and the pdf, which is its reciprocal. Girard's theorem:
the area of a spherical triangle is the sum of its angles minus pi.

## What is not modelled

Partial visibility. Sampling assumes the whole light is visible from the
shading point; where it is not, the samples are wasted rather than wrong.

## Acceptance criteria

- [ ] chi2 passes for the spherical triangle sampler
- [ ] Noise in the penumbra is measured before and after, and quoted
