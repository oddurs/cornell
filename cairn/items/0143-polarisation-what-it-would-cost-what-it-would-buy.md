---
id: 143
title: 'Polarisation: what it would cost, what it would buy'
type: spike
status: backlog
milestone: later
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: transport
effort: xl
---

## Question

Should this renderer carry polarisation?

## What it would cost

Four Stokes parameters per wavelength instead of one scalar, a 4x4 Mueller
matrix at every interaction instead of a scalar reflectance, a rotation into
and out of a reference frame at every bounce, and roughly four times the state
on every path. Fresnel already computes the two polarisations separately and
throws the distinction away, so the physics is half-present already.

## What it would buy

Correct glare off water and glass at steep angles. The polarisation pattern of
the sky, which bees navigate by. Stress birefringence in plastic, which is the
only way to render the coloured fringes in a clear plastic ruler between two
filters.

## What would settle it

Whether any image the project wants to make requires it. Currently: no. Kept
as an open question rather than a silence, per house rule 7.
