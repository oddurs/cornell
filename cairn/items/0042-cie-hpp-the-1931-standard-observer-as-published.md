---
id: 42
title: 'cie.hpp: the 1931 standard observer, as published'
type: optics
status: backlog
milestone: v0.3
labels:
- foundation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: colour
effort: m
---

## What it is

The bridge from a spectrum to three numbers, and one of exactly two files in
this project permitted to produce tristimulus values.

The colour matching functions come from Wright and Guild, who between them
measured seventeen observers in London around 1930, asking each to match a
monochromatic light by mixing three primaries. The entire colour science of
every screen you have ever looked at rests on those seventeen people.

## What it must derive

The normalisation constant. The Y integral is weighted so that a perfect
diffuse reflector under the chosen illuminant gives Y = 1, and that constant
is an integral of the illuminant against y-bar, not a number to be typed.

## What is not modelled

Individual variation, which is large; the 10-degree observer, which is a
different table for a different viewing condition; and anything about
adaptation, appearance or surround. This file converts a spectrum to
tristimulus and makes no claim to model perception.

## Acceptance criteria

- [ ] The tables are the published ones, cited, at 5 nm and interpolated
- [ ] The file's opening says whose eyes these were
- [ ] `constexpr` — the observer is known at compile time
