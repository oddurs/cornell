---
id: 77
title: 'Verify: Brewster''s angle falls out at the right degree'
type: verify
status: done
milestone: v0.6
assignee: Oddur Sigurdsson
labels:
- derivation
created: 2026-09-13
updated: 2026-09-16
priority: p0
area: verification
effort: s
---

## The claim

At one specific angle, light polarised in the plane of incidence is not
reflected at all. For an air-glass boundary at n = 1.5 that angle is
arctan(1.5), about 56.3 degrees, and it is the reason polarising sunglasses
work on a wet road.

## How it is checked

Evaluate the parallel-polarisation Fresnel term across incidence angle, find
the minimum numerically, and compare with arctan(n2/n1).

## What failure looks like

Nothing, in any image, because the renderer averages the polarisations. This
check exists because the *equations* are the thing being verified, not the
image — and a project that only tests what shows up in the picture is testing
its own tolerance for error.

## Measured

The minimum of `r_p` is *searched* for, a ten-thousandth of a degree at a
time, and compared against the formula `fresnel.hpp` derives. The two are
different questions with the same answer, which is the only way a derivation
can be checked against anything:

      water,    n = 1.330    searched 53.0612    arctan 53.0612
      glass,    n = 1.500    searched 56.3099    arctan 56.3099
      diamond,  n = 2.417    searched 67.5234    arctan 67.5234

The reflectance at the minimum is below 1e-9 in each — it is a zero, not a
dip.

Five more claims went in beside it, because they are the same file's and the
sheet is where its claims live now:

- Past the critical angle, glass to air, R is 1 over 48,140 angles: half of
  them exactly, none above, worst 1.5 ulp short.
- At normal incidence the general function meets the closed form
  `((n-1)^2 + k^2)/((n+1)^2 + k^2)` to 1.1e-16, over six indices including
  three complex ones. Two routes to one number.
- No reflectance exceeds 1, over 810,009 pairs — arriving through anything
  clear, and out of it again for the transparent ones, where total internal
  reflection is the thing being checked.

  That qualifier was put there by CI. The first version swept both directions
  for all six indices, passed under clang, and under g++-14 returned a
  conductor-to-air case at 90 degrees a few ulp above 1. Both compilers were
  right and the check was wrong: `|r|^2` is a ratio of *amplitudes*, and it is
  a ratio of energy flux only when the medium the light arrives through does
  not absorb. A ray inside a metal is not something geometric optics has, so
  the configuration never arises — but asking for it gets an answer above 1,
  which is the equation being asked a question rather than the equation being
  wrong.

  The same run turned up a second compiler-specific figure this project had
  nearly quoted as universal: past the critical angle, 50% of the reflectances
  are exactly 1.0 under libc++ and 100% under libstdc++, because the two
  implement complex multiplication differently. The invariant — never above 1,
  worst 1.5 ulp short — is what the check asserts; the count is printed and
  not relied on.
- From 80 to 90 degrees both polarisations rise monotonically to 0.99989.
  Everything becomes a mirror at grazing incidence, and nobody wrote that.
- And the one the complex case gives away free: a conductor has **no**
  Brewster angle. Its `r_p` bottoms at 70.02 degrees with a value of 0.86497
  rather than reaching zero. Nothing was written for that either — the same
  equation simply stops having a root.
