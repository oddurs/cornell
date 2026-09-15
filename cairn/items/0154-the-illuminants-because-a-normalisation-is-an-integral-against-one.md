---
id: 154
title: The illuminants, because a normalisation is an integral against one
type: optics
status: done
milestone: v0.3
assignee: Oddur Sigurdsson
created: 2026-09-14
updated: 2026-09-14
priority: p0
area: colour
effort: s
---

`cie.hpp` derives its normalisation as an integral of an illuminant against
y-bar rather than as a typed constant, which means there has to be an
illuminant for it to integrate against. The backlog had no item for one.

Two, defined in interestingly different ways, which is the reason this is an
optics item rather than a chore.

D65 is a table and could not be anything else: daylight is not a blackbody,
and the D series was derived by Judd, MacAdam and Wyszecki in 1964 from 622
measured daylight spectra reduced to a mean and two basis vectors. There is
no formula, and the table *is* the definition.

E is one line. Equal energy, no favourites, no citation, and it does not
exist.

## Acceptance criteria

- [x] D65 is the published table, cited, on the observer's grid, generated
      rather than transcribed
- [x] Its chromaticity is checked at compile time against CIE 15:2004
- [x] The file says which of the two is a measurement and which is a
      definition
