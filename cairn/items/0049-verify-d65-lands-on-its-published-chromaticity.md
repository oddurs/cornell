---
id: 49
title: 'Verify: D65 lands on its published chromaticity'
type: verify
status: done
milestone: v0.3
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-14
priority: p0
area: verification
effort: s
---

## The claim

The illuminant spectrum, integrated against the 1931 observer by this
project's own code, produces the chromaticity coordinates CIE published for
D65.

## How it is checked

Integrate and compare, to four decimal places, in `./cornell verify`.

## What failure looks like

The normalisation constant is wrong, or the interpolation between the 5 nm
table entries is wrong, or the tables were transcribed with an error. All
three are silent everywhere else and would poison every colour in the project.

## Acceptance criteria

- [x] The claim itself: D65 integrated against the 1931 observer by this
      project's own arithmetic gives x = 0.31271, y = 0.32901 against the
      published 0.31272, 0.32903 — four decimal places, as a `static_assert`
      rather than a runtime check, because the tables are `constexpr` and a
      check the compiler runs cannot be skipped
- [x] Also checked: equal-energy white lands at 1/3, 1/3 — but *not*
      exactly, and this criterion was wrong to say so. It lands at 0.333314,
      0.333287, off by 2e-5, and the residual is the 5 nm quadrature: the
      tables sample continuous functions whose integrals are equal by
      definition, and summing 95 samples is not integrating
- [ ] Also checked: a Macbeth chart under D65 against published Lab values —
      moved to item 0065, the inspection sheet, which is where a check that
      needs a data set of its own and a Lab implementation belongs
