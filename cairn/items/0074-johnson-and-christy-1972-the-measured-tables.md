---
id: 74
title: 'Johnson and Christy, 1972: the measured tables'
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

*Optical Constants of the Noble Metals*, Physical Review B, 1972. n and k for
copper, silver and gold, measured across the visible spectrum on evaporated
films, by two people with a spectrometer.

This table is the project's `windsor.hpp` spec sheet: real figures, from the
original source, in a file that exists to say where they came from.

## What it must derive

Three colours, from two columns of numbers each, and nothing else.

## What is not modelled

Sample dependence. Johnson and Christy measured evaporated films; a rolled
sheet, an electroplated surface and a cast ingot differ, and the differences
are larger than the measurement error. The file says so.

## Acceptance criteria

- [ ] Tables are transcribed from the paper with the wavelengths as published
- [ ] A comment gives the citation in full
- [ ] `./cornell spectrum gold --imaginary` plots k and it looks like the
      figure in the paper
