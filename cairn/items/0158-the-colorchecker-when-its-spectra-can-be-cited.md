---
id: 158
title: The ColorChecker, when its spectra can be cited
type: verify
status: backlog
milestone: v1.0
created: 2026-09-16
updated: 2026-09-16
priority: p2
area: verification
effort: m
---

Item 0065 carried this as a criterion — "a Macbeth chart under D65, against
published Lab values" — inherited from item 0049, and it cannot be met the way
the rest of this project meets things.

## Why it is its own item rather than a tick

The check needs two things. One is CIE 1976 L\*a\*b\*, which is derivable from
XYZ and a white point and is a short file; that is not the problem. The other
is the **spectral reflectance of twenty-four painted patches**, measured by
somebody, on a grid, with a citation — and this repository does not have it.

Cornell's reflectances are in `cornell.hpp` because they were transcribed from
a published table, 228 numbers, with static asserts pinning both ends of each
one against the page. That is the standard. A ColorChecker table produced from
memory would be 24 spectra of about 36 samples each with no page behind them,
and house rule 5 is that a figure gets checked before it is quoted. Inventing
plausible numbers and comparing the renderer against them would be a check
that passes because both sides came from the same place, which is precisely
the failure `verify.hpp` opens by refusing.

So the criterion is not ticked, it is not quietly dropped, and it is not met by
something easier wearing its name.

## What it needs

The ColorChecker's spectral reflectances from a citable source, transcribed
the way Cornell's were: the numbers, the grid they are on, who measured them,
and asserts on the values a reader can check against the page. With those, the
rest is small — `lab.hpp`, a delta-E, and a row per patch.

## What it is worth

The colour pipeline in this project — `cie.hpp`, `srgb.hpp`, `bradford.hpp` —
is currently checked against D65's published chromaticity and against equal-
energy white, which are checks of the *illuminants*. Nothing checks a
*reflectance* through the whole chain against a value somebody else published,
and that is the gap. It belongs beside v1.0's comparison against Cornell's
photograph, which is the same kind of claim about the same kind of evidence.

## Acceptance criteria

- [ ] The spectra are transcribed from a named source, with asserts pinning
      values a reader can check against it
- [ ] `lab.hpp`, derived rather than declared, with the white point it is
      relative to in the type or in the call
- [ ] Every patch's delta-E against the published Lab, on the inspection sheet
