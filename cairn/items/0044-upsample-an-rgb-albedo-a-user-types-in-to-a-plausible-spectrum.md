---
id: 44
title: Upsample an RGB albedo a user types in to a plausible spectrum
type: optics
status: done
milestone: v0.3
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-15
priority: p2
area: colour
effort: m
---

## What it is

The inverse problem, and the ugliest honest corner of the project. A user
types `#c04040` for a wall. That is three numbers and a reflectance is a
function, so the mapping is one-to-many and something must choose.

Jakob and Hanika (2019) fit a smooth low-parameter model per RGB triple.
Smits (1999) uses a small basis. Both are defensible; both are *choices*, and
the file must say so where a reader will see it.

## What it must derive

That the result round-trips: upsampling an RGB and integrating back against
the observer must return the RGB you started with, to within a tolerance the
file states.

## What is not modelled

Metamerism as an artistic control. Two reflectances that match under D65 and
differ under a sodium lamp are physically interesting and there is no way to
ask for one here.

## Acceptance criteria

- [x] Round-trip error is measured and quoted — 1.43e-07 worst over 729
      colours, and that worst case is pure white where the sigmoid asymptotes
- [x] Reflectances stay within [0,1] at every wavelength, which naive fits do
      not — [0.000000136, 0.999999922] against [-8.860240, 9.860240] for the
      same fit with the sigmoid removed
- [x] The Cornell walls do NOT go through this path — they are measured, and
      `cornell.hpp` in v0.4 reads the published numbers. The file says so at
      the top rather than in a footnote

## 2026-09-15

Jakob and Hanika's parameterisation, not their table: a sigmoid of a quadratic, three coefficients, fitted per colour by Gauss-Newton when asked rather than looked up from a precomputed 64^3 grid. That is the right trade for a scene with a handful of albedos and the wrong one for a renderer with textures, and the file says which. Measured over 729 colours on a 9x9x9 grid: worst round-trip error 1.43e-07, nothing missed by more than 1e-3, and every reflectance at every one of 95 wavelengths inside [0.000000136, 0.999999922]. The same fit with the sigmoid removed ranges over [-8.86, 9.86] - at pure green it asks for a wall reflecting 986 per cent of the light at some wavelengths and a negative amount at others, which is an energy source rather than a material. The Cornell walls do not come through here; they are measured, and comparing a made-up wall against a photograph proves nothing.
