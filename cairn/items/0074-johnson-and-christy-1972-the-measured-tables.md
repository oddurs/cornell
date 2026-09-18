---
id: 74
title: 'Johnson and Christy, 1972: the measured tables'
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

- [x] Tables are transcribed from the paper with the wavelengths as published
- [x] A comment gives the citation in full
- [ ] `./cornell spectrum gold --imaginary` plots k and it looks like the
      figure in the paper

The plot is not built. `./cornell swatch` prints what the tables *do* rather
than what they look like, and the check that matters is not the shape of a
curve by eye — it is where the reflectance edge falls, which is on the
inspection sheet against the published band-structure energy.

## The grid, which is why `Tabulated` exists

Johnson and Christy measured at 49 evenly spaced **photon energies** from 0.64
to 6.6 eV, because that is how a spectrometer of that period was driven. Energy
is inversely proportional to wavelength, so their grid runs from 1.9 nm apart
at the blue end to 320 nm apart at the infrared one.

`Measured<N>` takes a first wavelength and a step, and fits Cornell's walls at
4 nm and the observer at 5 nm. Resampling the metals onto an even wavelength
grid would throw away resolution where the measurement is dense and invent it
where it is sparse — the first quiet lie in the chain this milestone exists to
keep honest. So `spectrum.hpp` grew `Tabulated`, where the wavelengths travel
with the values, which is what that file's opening said would have to happen.

## Provenance, which is not the same as citation

Transcribed from refractiveindex.info, retrieved 17 September 2026, which
mirrors the paper's table. Both facts are in `metals.hpp` and they are
different claims: the citation is who made the measurement, the retrieval is
the path the digits took.

One thing the mirror lost. The wavelengths are stored rounded to 0.1 nm, so
converting back gives energy steps of 0.1177 to 0.1314 eV against a constant
0.124 — the exact energies are not recoverable. The wavelengths are
transcribed as the mirror gives them, which is the honest thing to hold.

## What the asserts caught

`gold_n[48] == 13.78`, written by reading the bottom of a CSV without noticing
it was the bottom of the **k** block. Gold's refractive index at 1937 nm is
0.92; 13.78 is its extinction coefficient.

The assert *passed*, because nothing included `metals.hpp` yet, so none of the
file was compiled. A check in a file nobody includes is not a check — which is
what deleting `mesh.hpp` in v0.4 was about, arriving again from the other
direction.
