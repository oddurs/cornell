---
id: 32
title: 'The estimator is visible: never cancel the pdf inside the sampler'
type: optics
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
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

- [x] The path loop contains a visible division by a pdf
- [x] A comment states the cancellation, shows it, and says why it is not
      taken
- [x] Measured: the un-cancelled version is not detectably slower — 3.757 s
      against 3.774 s over 10.24 million paths, which is the written-out
      version being half a percent *faster*, i.e. inside the noise. Both
      produce a mean radiance of 0.410850, so they are the same computation

## 2026-09-13

Measured rather than asserted. A 200x200 render at 256 spp, best of three: the written-out estimator takes 3.757 s (2.73 Mpaths/s) and the collapsed 'throughput *= rho' version - with sample no longer forming f or its density at all - takes 3.774 s (2.71 Mpaths/s). The written-out one is half a percent faster, which is to say there is no difference to measure: nine multiplies and a division per bounce disappear beside a ray-scene intersection. Both give mean radiance 0.410850, identical, which confirms the algebra. The reason not to collapse was never speed: MIS in v0.8 needs f and pdf separately, and a renderer that collapsed them has lost the quantities from the design rather than from the code.
