---
id: 30
title: Derive Lambert's 1/pi in the file that uses it
type: optics
status: backlog
milestone: v0.2
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: bsdf
effort: s
---

## What it is

`lambert.hpp`. Photometria, Augsburg, 1760 — the oldest result in the project
and the one most often typed in as `0.31831`.

## What it must derive

The normalisation. A surface that reflects all incident light and scatters it
equally in every direction has a BRDF of `rho/pi`, and the pi is not a
convention: it is the integral of the cosine over the hemisphere,

    ∫ cos(theta) dw = pi

which is the projected solid angle of a hemisphere. Do the integral in the
comment. Two lines, and it turns a magic number into a fact.

## What is not modelled

That no real matte surface is Lambertian. Paint, paper and plaster all retro-
reflect at grazing angles, which Oren and Nayar modelled in 1994 and which is
deliberately absent — the Cornell walls are close enough that the error is
below the measurement, and that claim gets checked in v1.0 rather than
asserted here.

## Acceptance criteria

- [ ] The constant `1/pi` appears nowhere as a literal
- [ ] The hemisphere integral is written out in the comment
