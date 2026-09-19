---
id: 91
title: 'veach.hpp: the balance heuristic, derived'
type: optics
status: backlog
milestone: v0.8
labels:
- derivation
created: 2026-09-13
updated: 2026-09-19
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

## 2026-09-19

From 0160, which decided how a stochastic BSDF fits the three-method contract: veach.hpp owns a rule with a silent failure mode, and the check for it belongs here rather than where it was discovered.

The rule: every strategy must weight with the SAME density. Veach's condition for an unbiased combination is that the weights sum to one, not that they are correct - so a proxy density used consistently is unbiased and merely worse, while a proxy used on one side only is biased and looks like a slightly dark material.

Measured, 2^25 samples over 1024 batches on a 1-D integral with two strategies: balance heuristic with true pdfs -1.0 sigma from truth; a proxy wrong by a factor of two used by both strategies +0.2 sigma, costing 1.1x the standard deviation; a hopeless proxy used by both +0.8 sigma, costing 1.8x the standard deviation, so 3.4x the samples; the same proxy used by one side only -814.7 sigma.

This matters here because v0.7's multiple-scattering walk has no closed-form density and will hand MIS a proxy. A check shaped like the table above - a deliberately wrong proxy, applied consistently and then inconsistently, with the second one caught - is what stops that rule from being a comment.
