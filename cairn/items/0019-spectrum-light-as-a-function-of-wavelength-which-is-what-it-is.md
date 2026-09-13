---
id: 19
title: 'Spectrum: light as a function of wavelength, which is what it is'
type: optics
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

## What it is

The type that replaces `vec3 colour` everywhere, and the reason this project
can make the claims it makes. Arithmetic, and a strict refusal to be indexed
as if it had three components.

## What it must derive

Nothing yet. But it must make `Spectrum * Spectrum` mean what it means for
light meeting a reflectance, and it must make the compiler reject a spectrum
used where a tristimulus value is wanted.

## What is not modelled

Polarisation. A full description is four Stokes parameters per wavelength, and
we carry one. Stated at the top of the file rather than discovered later.

## Acceptance criteria

- [ ] `Spectrum` has no `.r`, `.g`, `.b`, and no `operator[]` taking 0..2
- [ ] `Radiance` and `Reflectance` are distinct types that multiply to
      `Radiance` and will not add to each other
