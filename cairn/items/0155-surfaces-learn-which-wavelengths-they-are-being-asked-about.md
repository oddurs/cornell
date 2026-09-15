---
id: 155
title: Surfaces learn which wavelengths they are being asked about
type: optics
status: done
milestone: v0.3
assignee: Oddur Sigurdsson
created: 2026-09-14
updated: 2026-09-14
priority: p0
area: bsdf
effort: l
---

## What it is

The change the backlog did not have an item for, and without which v0.3
cannot deliver what its milestone promises.

A reflectance is a function of wavelength. `Reflectance` has been able to
hold one since v0.1 — it is four numbers — but the four wavelengths differ
from path to path, so a surface cannot say what it reflects without being
told which wavelengths it is being asked about. For two milestones nothing
needed to ask: every material was grey, every emitter flat, and all four
components of every `Reflectance` were the same number.

So all three methods of the contract in `bsdf.hpp` take a `Wavelengths`, and
emission does too. `box.hpp` predicted exactly this when it explained why its
walls were grey, and that is why the refusal to type a plausible red cost
nothing — the walls were always going to arrive as spectra.

## What it must derive

The normalisation, which `cie.hpp` defines as an integral of the illuminant
against y-bar and this is the first caller of. Absolute tristimulus for this
room is around 1e-7 W·m⁻²·sr⁻¹, which is correct and useless; Y is meant to
be relative luminance, and the constant that makes it so is computed from the
scene's own lamp at compile time rather than typed.

## What is not modelled

Dispersion. `pdf` does not take the wavelengths, because a direction's
density does not depend on one — this project samples a direction and carries
four wavelengths along it. v0.9's prism splits the path instead, which is
what `Wavelengths::separated()` has been waiting for since v0.1.

## Acceptance criteria

- [x] All three BSDF methods and emission take the path's wavelengths
- [x] The lamp emits tabulated D65 rather than a flat placeholder
- [x] The film accumulates tristimulus alongside its spectral bins, and keeps
      both, as `film.hpp` said in v0.1 that it would
- [x] The normalisation is derived from the lamp's spectrum, not typed
- [x] A grey room under D65 renders neutral: R, G and B channel means of
      0.8549, 0.8561, 0.8557, agreeing to 0.0012

## 2026-09-14

End-to-end check of the whole colour path: a grey room lit by tabulated D65 renders with linear sRGB channel means of 0.8549, 0.8561 and 0.8557 - neutral to 0.0012. That is the observer tables, the D65 table, the derived normalisation and the derived matrix all agreeing at once, and any one of them being wrong would show as a tint. Absolute tristimulus for this room is about 1e-7 W/m2/sr, so the normalisation matters: derived from the lamp at compile time, it puts the lamp itself at Y = 1.04. Out-of-gamut pixels fall from 638 of 40000 at 128 spp to 214 of 90000 at 256 - it is chromaticity noise from estimating three numbers out of four wavelengths per path, not a scene property, which is worth knowing before item 0047 decides what to do about negative RGB.
