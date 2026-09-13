---
id: 17
title: 'si.hpp: the dictionary of what this program will accept'
type: optics
status: planned
milestone: v0.1
labels:
- foundation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: units
effort: m
---

## What it is

The unit convention, and the first file written, because every other file
depends on it. Metres, watts, steradians, kelvin, nanometres, radians —
`consteval` literal operators in, named conversions out, and nothing in
between.

    Wavelength peak {550.0_nm};
    auto sun = 5778.0_K;
    auto d   = 1.0_sr;

## What it must derive

Nothing. This is the one file in the project that is allowed to be a list.

## What is not modelled

Photometric units — lumens, lux, candela. They are the radiometric ones
weighted by the photopic curve, which is a `cie.hpp` concern, and putting them
here would put an observer inside the dictionary.

## Notes

Most of the literals will never be called. That is deliberate, and it is
`windsor`'s argument repeated: this file is not a call graph, it is a
statement of what the program will and will not accept, and a statement with
the inconvenient half left out is not one. `consteval` means an unused one
generates nothing at all.

## Acceptance criteria

- [ ] Every literal has an inverse in `namespace as`
- [ ] `101325.0_Pa` and `101325.0` compile to the same constant
- [ ] No file outside `si.hpp`, `cie.hpp` and `srgb.hpp` divides by a
      conversion factor
