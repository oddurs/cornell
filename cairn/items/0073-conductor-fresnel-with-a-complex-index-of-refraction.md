---
id: 73
title: Conductor Fresnel with a complex index of refraction
type: optics
status: backlog
milestone: v0.6
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
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

- [ ] No metal in the project has an RGB value anywhere, in any file
- [ ] `grep -r "0.766\|0.336" include/` returns nothing, now and forever
