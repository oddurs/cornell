---
id: 49
title: 'Verify: D65 lands on its published chromaticity'
type: verify
status: backlog
milestone: v0.3
created: 2026-09-13
updated: 2026-09-13
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

- [ ] Also checked: equal-energy white lands at 1/3, 1/3, exactly
- [ ] Also checked: a Macbeth chart under D65 against published Lab values
