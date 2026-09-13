---
id: 123
title: 'Verify: an optically thin medium converges to no medium'
type: verify
status: backlog
milestone: v1.1
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: verification
effort: s
---

## The claim

As the extinction coefficient goes to zero, the volumetric integrator
reproduces the surface-only integrator exactly.

## How it is checked

Render the Cornell box filled with a medium of vanishing density and compare
with the v1.0 image.

## What failure looks like

A constant offset means transmittance is being computed with a bias. A
difference that grows with path length means the medium's pdf is not
normalised.
