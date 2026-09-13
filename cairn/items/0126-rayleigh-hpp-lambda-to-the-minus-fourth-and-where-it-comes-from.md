---
id: 126
title: 'rayleigh.hpp: lambda to the minus fourth, and where it comes from'
type: optics
status: backlog
milestone: v1.2
labels:
- thesis
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: media
effort: m
---

## What it is

The best argument in optics that is visible from a garden.

Lord Rayleigh, 1871. A particle much smaller than a wavelength scatters light
in proportion to the inverse fourth power of that wavelength. Violet at 400 nm
scatters about nine times more strongly than red at 700, the sky is therefore
blue, and that single exponent is the whole explanation.

## What it must derive

The blue. Not a gradient, not a sky colour, not a preset: a scattering
coefficient with a wavelength dependence, in a medium with a density profile,
integrated by the same volumetric machinery as smoke.

And then, from precisely the same code with no addition: the sunset. Lower the
sun, lengthen the path through the atmosphere, and the blue is scattered out
before it arrives, leaving red. **Nobody writes a sunset.** That sentence is
the reason this milestone exists.

## What is not modelled

Ozone absorption, which is a real and visible contribution to the colour of
twilight, and which should be added or explicitly refused rather than
forgotten.

## Acceptance criteria

- [ ] The lambda^-4 dependence is the only colour input
- [ ] `grep -ri "sunset\|sky_colour\|horizon" include/` finds nothing
