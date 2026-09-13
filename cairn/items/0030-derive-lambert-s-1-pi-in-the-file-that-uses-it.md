---
id: 30
title: Derive Lambert's 1/pi in the file that uses it
type: optics
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
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

- [x] The constant `1/pi` appears nowhere as a literal
- [x] The hemisphere integral is written out in the comment

## 2026-09-13

1/pi is derived from the hemisphere cosine integral and the constant is spelled projected_hemisphere rather than pi, because what the code divides by is the integral. Quadrature over 1e6 Simpson panels gives 3.14159265358982 against pi's 3.14159265358979 - thirteen places, the difference being the quadrature. Integrating f*cos for albedo 1 gives 1.0000000000000173. The estimator f*cos/pdf over 1e7 draws returns mean 1, min 1, max 1, sample sd exactly 0: every sample is bit-exactly 1.0, which survives because 1.0/si::pi and si::inv_pi are the same double (checked). Also fixed the last of the latent sign conversions in spectrum.hpp - its private loops now go through operator[], which is the one place the int/size_t boundary is crossed.
