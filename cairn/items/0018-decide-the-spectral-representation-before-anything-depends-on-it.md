---
id: 18
title: Decide the spectral representation before anything depends on it
type: spike
status: planned
milestone: v0.1
labels:
- foundation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: spectrum
effort: m
---

## Question

How is a spectrum stored? This decision is load-bearing for the whole project
and is very expensive to change after v0.6.

## Why it has to be answered first

`Spectrum` appears in the signature of every BSDF, every light, every film
pixel. Changing it later is changing every file.

## Options

**Fixed uniform bins.** N samples from 360 to 830 nm. Simple, obvious, and
wrong in a specific way: dispersion through a prism produces banding at any
tractable N, and the memory is N floats per ray.

**Hero wavelength sampling (Wilkie et al. 2014).** Carry four wavelengths per
path, sample one and derive the rest by rotation, MIS across them. Costs a
paragraph of explanation and buys correct dispersion with four floats.

**Basis functions.** Fourier or Gaussian mixtures. Compact, and the reader
now has to understand the basis before they can understand a reflectance.

## What would settle it

v0.9 renders a prism. If the representation cannot produce a clean spectrum
from white light without a dispersion hack, it has failed the thesis.

## Answer

<!-- Filled in when this closes. -->
