---
id: 76
title: 'schlick.hpp: the 1994 fit, in the file next door'
type: optics
status: backlog
milestone: v0.6
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: bsdf
effort: s
---

## What it is

Christophe Schlick, *An Inexpensive BRDF Model for Physically-based Rendering*,
Eurographics 1994. A one-line approximation to the file next to it:

    R(theta) = R0 + (1 - R0)(1 - cos theta)^5

It is in every real-time renderer in the world, it is a fit, and it sits
beside the exact version deliberately so that a reader can see the trade
rather than inherit it. This is house rule 8's clearest case: a law and a
curve-fit must never be spelled the same way.

## What it must derive

Its own error. The file quotes the maximum and mean deviation from exact
Fresnel, measured by this project, for a dielectric and for gold — and the
metal figure is the interesting one, because the approximation assumes a real
index and a metal's is not.

## Acceptance criteria

- [ ] Not used by default anywhere; it exists to be compared
- [ ] The error figures in the comment come from `./cornell verify`
