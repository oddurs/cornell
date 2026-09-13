---
id: 43
title: 'srgb.hpp: the transfer function exactly, not 2.2'
type: optics
status: backlog
milestone: v0.3
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: colour
effort: s
---

The sRGB transfer function is a linear segment near black spliced to a 2.4
power curve, and it is not a 2.2 gamma. The approximation is close enough to
look fine and wrong enough to matter when a number is being quoted.

Also: the primaries and white point are a matrix derived from published
chromaticities, not a matrix copied from somewhere.

- [ ] The matrix is computed from the chromaticities at compile time
- [ ] Round-trip encode/decode is exact to within a bit
- [ ] The comment states that this file and `cie.hpp` are the only two
      exceptions to the no-RGB rule
