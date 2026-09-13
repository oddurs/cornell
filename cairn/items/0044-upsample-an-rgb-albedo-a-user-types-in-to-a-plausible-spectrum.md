---
id: 44
title: Upsample an RGB albedo a user types in to a plausible spectrum
type: optics
status: backlog
milestone: v0.3
created: 2026-09-13
updated: 2026-09-13
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

- [ ] Round-trip error is measured and quoted
- [ ] Reflectances stay within [0,1] at every wavelength, which naive fits do
      not
- [ ] The Cornell walls do NOT go through this path — they are measured
