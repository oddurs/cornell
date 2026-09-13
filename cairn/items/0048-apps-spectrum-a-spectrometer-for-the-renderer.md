---
id: 48
title: 'apps/spectrum: a spectrometer for the renderer'
type: instrument
status: backlog
milestone: v0.3
created: 2026-09-13
updated: 2026-09-13
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
