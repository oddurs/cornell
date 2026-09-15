---
id: 42
title: 'cie.hpp: the 1931 standard observer, as published'
type: optics
status: done
milestone: v0.3
assignee: Oddur Sigurdsson
labels:
- foundation
created: 2026-09-13
updated: 2026-09-14
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

- [x] The tables are the published ones, cited, at 5 nm and interpolated —
      cvrl.org's `ciexyz31.csv`, generated into the header rather than
      transcribed, because 285 numbers typed by hand is 285 chances
- [x] The file's opening says whose eyes these were — Wright's ten and
      Guild's seven, and what is wrong with the result as well as what is
      right
- [x] `constexpr` — the observer is known at compile time, and so are the
      checks on it: y-bar is exactly 1 at 555 nm, z-bar peaks at 445, and
      equal-energy white lands on the achromatic point, all as static_asserts

## 2026-09-14

Tables are cvrl.org's ciexyz31.csv, 95 rows at 5 nm from 360 to 830, generated into the header by script rather than transcribed. Verified at compile time against what is published about them: y-bar is exactly 1.0 at 555 nm and is the maximum, z-bar peaks at 445 nm, and equal-energy white lands at x 0.333314, y 0.333287. That last is not exactly 1/3 - the residual is the 5 nm quadrature, since the tables are sampled versions of continuous functions whose integrals are equal by definition - so the assert carries a 2e-5 tolerance and the comment says why rather than hiding it. xyz_estimate is the first caller of Wavelengths::pdf(), which spectrum.hpp kept through two milestones on the grounds that a sample and its density are one object.
