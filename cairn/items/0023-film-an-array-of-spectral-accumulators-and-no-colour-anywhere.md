---
id: 23
title: 'Film: an array of spectral accumulators, and no colour anywhere'
type: optics
status: done
milestone: v0.1
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: camera
effort: s
---

## What it is

Where samples land. It accumulates spectral radiance and a sample count, and
it knows nothing about pixels, gamma, or RGB — the conversion happens once, at
the moment of writing a file, in the one place allowed to do it.

## Acceptance criteria

- [x] The film can be written out before `cie.hpp` exists, as a false-colour
      or single-wavelength image, and the project still produces a picture

## 2026-09-13

Written and verified, but left open: the acceptance criterion is that the film can be written out as a picture before cie.hpp exists, and there is no writer and no dispatcher yet. Closed by 0025, where the first image appears. Measured here: 47 bins of 10.0000 nm spanning 470 nm exactly; 47000 stratified paths deposit 188000 wavelength samples and every bin receives exactly 4000 of them, so the quarter-range rotation in Wavelengths::sample stratifies without residue. A flat 2.5 W/m2/sr/m comes back out of all 47 bins with zero error. 564 bytes per pixel: 67.7 MB at 400x300, 147.8 MB at 512x512, 443.5 MB at 1024x768, against 24 bytes per pixel for XYZ.

## 2026-09-13

Closed here rather than in its own commit: the criterion was that the film be writable as a picture before cie.hpp exists, and that is what 0025 does. The single-wavelength image at 555 nm is the film's bin 19.
