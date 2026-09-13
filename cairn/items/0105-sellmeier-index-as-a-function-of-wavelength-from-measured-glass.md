---
id: 105
title: 'Sellmeier: index as a function of wavelength, from measured glass'
type: optics
status: backlog
milestone: v0.9
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: spectrum
effort: m
---

## What it is

The payoff of five milestones of spectral discipline.

The refractive index of glass is not a number, it is a curve, and the
Sellmeier equation fits it with three resonance terms whose coefficients are
published by every glass manufacturer for every glass they sell. BK7, SF11,
fused silica: real materials with real datasheets.

## What it must derive

Dispersion. Blue light bends more than red because the index is higher there,
and that is the entire content of the phenomenon. There is no dispersion
parameter, no spread control, and no prism code anywhere in the project.

Also the Abbe number, which is the optical industry's one-number summary of
how dispersive a glass is, computed from the fit at three specific spectral
lines — and which can therefore be checked against the manufacturer's
datasheet.

## What is not modelled

Absorption bands and the behaviour outside the visible range, where the
Sellmeier fit is not valid and the file says so.

## Acceptance criteria

- [ ] Coefficients are quoted from datasheets, with the glass named
- [ ] The Abbe number comes out right, and that is a verify item
