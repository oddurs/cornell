---
id: 91
title: 'veach.hpp: the balance heuristic, derived'
type: optics
status: backlog
milestone: v0.8
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: transport
effort: l
---

## What it is

Eric Veach and Leonidas Guibas, 1995: *Optimally Combining Sampling Techniques
for Monte Carlo Rendering*. The largest reduction in noise per line of code
available anywhere in rendering.

Two strategies, each catastrophic exactly where the other is excellent.
Sampling the light works beautifully for a small bright source and terribly
for a large dim one. Sampling the BSDF works beautifully for a sharp highlight
and terribly for a small source it will almost never hit by chance. Neither
can be dropped and choosing between them per-scene is what renderers did
before this paper.

## What it must derive

The balance heuristic weight, from the requirement that the combined estimator
be unbiased: the weight for strategy i is its density over the sum of all
densities that could have produced the same sample. That requirement is three
lines of algebra and it makes the weight a consequence rather than a formula.

## What is not modelled

Veach's optimal weights, which are provably better and require knowing the
variance of each strategy in advance.

## Acceptance criteria

- [ ] The unbiasedness argument is in the comment
- [ ] Every strategy's pdf is the same `pdf()` the chi2 instrument tests
