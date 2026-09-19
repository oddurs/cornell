---
id: 160
title: What a stochastic BSDF does to the three-method contract
type: spike
status: backlog
milestone: v0.7
created: 2026-09-19
updated: 2026-09-19
priority: p0
area: bsdf
effort: m
---

## Question

Heitz's multiple-scattering model defines the BSDF as an expectation over
random walks. There is no closed form for `f(wo, wi)` and none for
`pdf(wo, wi)`; both become unbiased estimators. `bsdf.hpp` requires all three
methods and means them as functions. What gives?

## Why it has to be answered first

Because the alternative is that it gets answered by accident. Whoever writes
the walk will hit this on their first afternoon, will make `eval` return a
random number or will quietly return the single-scattering value, and the
project will have taken a position on one of its oldest design claims without
noticing that it did.

0087 chose this model knowing this was the real cost. This is where the cost
gets paid explicitly.

## What is at stake

Three things already depend on `pdf` being a function:

- **`bsdf.hpp`'s argument for the third method.** It is there so `sample` and
  `pdf` are two independent statements about one distribution, written in
  different code, that a machine can check against each other. An estimator
  cannot testify against itself.
- **`./cornell chi2`**, which integrates the density over histogram cells by
  quadrature. A density that can only be sampled cannot be integrated that
  way. Item 0084 already had to reach for a stretched frame to keep this
  instrument honest at low roughness; this is a harder version of the same
  problem.
- **v0.8's multiple importance sampling**, which evaluates one strategy's
  density on the other strategy's direction. The balance heuristic needs a
  number, not a draw.

## Options

**Keep the single-scattering lobe's closed form for `eval` and `pdf`, and let
the multiple-scattering energy ride in `sample`'s weight.** What most
implementations do. `chi2` then tests the single-scattering part and is silent
about the rest; MIS weights on a density that is not quite the one being
sampled, which is a bias unless the weights still sum to one across
strategies, and that has to be checked rather than assumed.

**Make `eval` and `pdf` stochastic and say so in the type.** Honest, and it
means `chi2` needs a different instrument — comparing two estimators rather
than an estimator against a quadrature.

**Split the material into two lobes with a selection probability**, so that
each lobe keeps a closed form and the stochastic part is confined.

## What would settle it

Whichever option is taken, the criterion is the same: no method may return a
number that looks deterministic and is not. If `eval` becomes an estimate, the
type says so.

## Answer

<!-- Filled in when this closes. -->
