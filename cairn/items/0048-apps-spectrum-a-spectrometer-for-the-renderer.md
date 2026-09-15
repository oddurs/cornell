---
id: 48
title: 'apps/spectrum: a spectrometer for the renderer'
type: instrument
status: done
milestone: v0.3
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-14
priority: p2
area: instrument
effort: s
---

## What it witnesses

Any spectrum in the project — an illuminant, a reflectance, a measured index
of refraction — printed as a plot in the terminal with its chromaticity
coordinates and correlated colour temperature beside it.

## What it prints

    $ ./cornell spectrum d65
    $ ./cornell spectrum gold --imaginary

The `windsor` equivalent is the shop-manual page: a part, quoted.

## What it may not do

Compute anything `cie.hpp` should compute.

## 2026-09-14

Two independent checks fell out of building it. D65's correlated colour temperature, via McCamy's fit applied to the chromaticity this project computes, comes out 6504 K - which is the number in D65's own definition, arrived at from the other end. And D65 maps to linear sRGB (1.0000, 1.0000, 1.0000) exactly, which is srgb.hpp's derived matrix checking itself against the illuminant it was derived from. Equal-energy E gives 5460 K against a published ~5455; the gap is McCamy's fit, which is accurate near the Planckian locus and E sits slightly off it. E also renders warm on an sRGB display - +1.0000, +0.7869, +0.7544 - which is correct and a useful thing to be able to see. Gold with its imaginary part waits for v0.6's tables; the instrument grows a case rather than changing shape.
