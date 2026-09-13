---
id: 18
title: Decide the spectral representation before anything depends on it
type: spike
status: done
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

## 2026-09-13

## Answer

**Four wavelengths per path, hero-sampled, stratified by rotation.** Wilkie et
al., 2014.

Fixed bins were rejected because the banding they produce in a prism is in the
representation rather than in the noise, so no sample count removes it — and
the prism in v0.9 is one of the project's two showpieces. One wavelength per
path was rejected as needlessly noisy: it throws away three quarters of the
colour information a path could carry for almost no saving, since the cost of
a path is the intersection, not the arithmetic.

Four is carried in `Wavelengths`, drawn from a single uniform number, with the
other three placed at exact quarters of the visible range and wrapped. That
makes them stratified by construction rather than by luck, and it means a
caller who stratifies `u` across paths gets the whole visible range covered
evenly for free — which will matter in v0.8.

The range is 360 to 830 nm, which is the domain of the CIE 1931 tables. It is
wider than useful and the edges are chosen by the table rather than by
somebody's judgement, which is the reason to prefer it.

A `separated` flag is already on the type and nothing sets it. v0.9 does, when
a path refracts dispersively and the four stop travelling together.
