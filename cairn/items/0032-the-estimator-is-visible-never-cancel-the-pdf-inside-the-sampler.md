---
id: 32
title: 'The estimator is visible: never cancel the pdf inside the sampler'
type: optics
status: backlog
milestone: v0.2
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: transport
effort: s
---

## What it is

House rule 3, made concrete in the one place it is always violated.

Sampling the cosine-weighted hemisphere gives `pdf = cos(theta)/pi`. Evaluating
Lambert gives `f = rho/pi`. The estimator is `f * cos(theta) / pdf`, and the
whole thing collapses to `rho`. Every tutorial renderer therefore returns
`rho` and deletes the other three quantities.

It is algebraically correct and it is the single most expensive shortcut
available, because when v0.8 needs to weight this sample against a light
sample, the density it needs has been optimised away — not from the code, from
the *design*.

## What it must derive

Nothing. It must refuse to simplify.

## Acceptance criteria

- [ ] The path loop contains a visible division by a pdf
- [ ] A comment states the cancellation, shows it, and says why it is not
      taken
- [ ] Measured: the un-cancelled version is not detectably slower
