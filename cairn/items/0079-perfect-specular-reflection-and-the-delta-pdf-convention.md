---
id: 79
title: Perfect specular reflection, and the delta pdf convention
type: optics
status: backlog
milestone: v0.6
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: bsdf
effort: m
---

## What it is

A mirror, and the first BSDF whose distribution is not a function.

A perfect specular surface scatters in exactly one direction, so its pdf is a
Dirac delta. That breaks the three-method contract in a specific way — `eval`
and `pdf` both return zero for every pair of directions, because the
probability of hitting a measure-zero set is zero — and every renderer handles
this with a convention.

## What it must derive

Nothing. But the convention must be written down once, in `transport.hpp`, and
obeyed everywhere: a sampled delta lobe returns its weight with the pdf
already divided out and a flag saying so; `eval` and `pdf` return zero; and
multiple importance sampling in v0.8 must skip delta lobes rather than weight
them.

Getting this wrong produces a mirror that is too dark by the pdf, which looks
like a plausible artistic choice.

## Acceptance criteria

- [ ] The convention is stated in one place and referenced, not restated
- [ ] The chi2 instrument knows to skip delta lobes and says so in its output
