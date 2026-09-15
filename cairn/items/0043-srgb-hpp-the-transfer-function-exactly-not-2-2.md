---
id: 43
title: 'srgb.hpp: the transfer function exactly, not 2.2'
type: optics
status: done
milestone: v0.3
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-14
priority: p0
area: colour
effort: s
---

The sRGB transfer function is a linear segment near black spliced to a 2.4
power curve, and it is not a 2.2 gamma. The approximation is close enough to
look fine and wrong enough to matter when a number is being quoted.

Also: the primaries and white point are a matrix derived from published
chromaticities, not a matrix copied from somewhere.

- [x] The matrix is computed from the chromaticities at compile time — and
      doing so turned up that the standard's printed matrix is not consistent
      with the standard's printed white point
- [x] Round-trip encode/decode is exact to within a bit — 4.44e-16 over 10^6
      values, which is two ulps at 1.0
- [x] The comment states that this file and `cie.hpp` are the only two
      exceptions to the no-RGB rule

## 2026-09-14

Deriving the matrix rather than pasting it turned up a discrepancy in the standard. sRGB states a white point of 0.3127, 0.3290 and prints a matrix; a matrix derived from that white point misses the printed one by 2.28e-4. Applying the printed matrix to (1,1,1) implies a white point of 0.312727, 0.329023, and deriving from that reproduces it to 1.36e-7. So the printed matrix was computed from a more precise D65 than the document quotes beside it. This file uses a third value - 0.312712, 0.329008, this project's own D65 integrated from the CIE tables - because a white reflector under the illuminant the scene is lit by has to come out RGB (1,1,1) or white in a render is not white. Cost: 1.40e-4 worst element against the pasted matrix, 0.036 of one 8-bit code. Round trip 4.44e-16, two ulps. The standard is also very slightly discontinuous at its splice, by 2.85e-8, because it rounded the crossover; noted rather than corrected, since a smoother version disagrees with every other decoder. image.hpp now calls encode() and its gamma parameter is gone, which is the promise that file made in v0.1.
