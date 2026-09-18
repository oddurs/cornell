---
id: 73
title: Conductor Fresnel with a complex index of refraction
type: optics
status: done
milestone: v0.6
assignee: Oddur Sigurdsson
labels:
- thesis
created: 2026-09-13
updated: 2026-09-17
priority: p0
area: bsdf
effort: m
---

## What it is

The moment the project's central claim becomes true or false.

A metal has a complex refractive index, n + ik. The real part refracts and the
imaginary part absorbs, both depend strongly on wavelength, and the reflectance
that comes out of Fresnel with a complex index is therefore different at every
wavelength — which is what the colour of a metal *is*.

## What it must derive

The colour of gold. Not approximately, not as a tint: the spectral reflectance
curve, from n and k, evaluated per wavelength, integrated against the CIE
observer at the end of the path like everything else.

## What is not modelled

That n and k vary with temperature and with surface oxidation, and that a real
gold surface has a thin layer of something else on it.

## Acceptance criteria

- [x] No metal in the project has an RGB value anywhere, in any file
- [ ] `grep -r "0.766\|0.336" include/` returns nothing, now and forever

The second criterion cannot hold as written, and the reason is worth more than
the criterion was. **0.336200000 is the CIE 1931 observer's x-bar at 450 nm** —
measured by seventeen people in London and with nothing whatever to do with
gold. A guard that fires on real data is a guard somebody switches off.

So CI greps for `0.766`, which has no other business in this repository, and
allows it in exactly one place under exactly one name: the `typed` array the
instruments compare their derived answer against. That is the constant being
held up, not used.

## What came out

      gold        x 0.38177  y 0.38870    sRGB 1.0000 0.7020 0.3514
      copper      x 0.35575  y 0.34559    sRGB 1.0000 0.6683 0.5606
      silver      x 0.31350  y 0.32967    sRGB 1.0000 0.9945 0.9883
      aluminium   x 0.31161  y 0.32825    sRGB 0.9837 0.9930 1.0000

Gold is **(1.0000, 0.7020, 0.3514)**, from two columns of measured n and k
through Fresnel through the 1931 observer, with nothing typed. The constant
every renderer types is (1.0, 0.766, 0.336). They agree to 0.064, and they
have never met: one is a measurement, the other has been copied between
renderers for thirty years without a citation.

Swap the table for copper's and the same code returns (1.0000, 0.6683,
0.5606).
