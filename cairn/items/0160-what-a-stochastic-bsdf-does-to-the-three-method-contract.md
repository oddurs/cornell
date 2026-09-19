---
id: 160
title: What a stochastic BSDF does to the three-method contract
type: spike
status: done
milestone: v0.7
assignee: Oddur Sigurdsson
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

**The contract keeps all three methods. `pdf` changes what it means, and the
change is written down rather than absorbed.**

### This is the second instance, not the first

The project already has a lobe whose density is not a function. A mirror's is
a Dirac delta, and `bsdf.hpp` settled the convention in v0.1: `sample` returns
the estimator already formed as `weight`, a flag says so, `eval` and `pdf`
return zero, and `transport.hpp` branches at the point of use so that the
place where the division did not happen stays visible.

A stochastic lobe borrows most of that. The walk's weight arrives pre-formed
for the same reason the mirror's does — there is no ratio to write, because
the density was never evaluated — so the estimator side of the contract needs
nothing new.

What it cannot borrow is the rest of the convention. A delta lobe is skipped
by multiple importance sampling because a delta cannot be weighted against a
finite density. A stochastic lobe is finite, scatters in every direction, and
carries most of the light at high roughness. Skipping it would mean no BSDF
strategy for a rough metal at all, which is the strategy that matters most
when the lamp is small.

So the question is precisely: what does MIS weight it with?

### What `pdf` is actually for, which turns out not to be what it says

`pdf` has been doing two jobs that have never needed separating, because for
every material so far they are the same number:

1. the density the estimator divides by, and
2. the density multiple importance sampling weights with.

For a walk, the first is never evaluated — it arrives inside the weight. Only
the second is needed, and the second has a property the first does not:

> **It does not have to be correct. It has to be the same one every strategy
> uses.**

Veach's condition for an unbiased combination is that the weights sum to one
wherever the integrand is non-zero. It says nothing about the weights being
*good*. Substitute any consistent proxy into the balance heuristic and the
weights still sum to one; the estimator stays unbiased and gets worse.

### Measured, because that is the whole decision

A one-dimensional integral with two strategies, 2^25 samples in 1024 batches.
Strategy 1 stands for the walk: its estimator arrives already divided, so its
true density is never used, and only the weights see a proxy.

    variant                        mean - truth    sigma off    sd of one
    balance heuristic, true pdfs      -4.47e-05         -1.0     1.50e-03
    wrong proxy, used by BOTH         +8.43e-06         +0.2     1.65e-03
    hopeless proxy, used by BOTH      +7.06e-05         +0.8     2.75e-03
    wrong proxy, used by ONE          -3.13e-02       -814.7     1.23e-03

A proxy wrong by a factor of two costs a tenth of a standard deviation. A
proxy so bad it gives the strategy almost no weight anywhere costs 1.8x the
standard deviation — 3.4x the samples for the same error — and is still
unbiased to within a fifth of a standard error.

Using it on one side only is wrong by 815 standard errors. That is the failure
mode, it is silent, it looks like a slightly dark material, and it is what
happens if somebody wires the proxy into the BSDF's own weight and leaves the
light sampler weighting against the true density.

### The decision

- `pdf(wo, wi)` **stays a function returning a deterministic number**. It is
  not stochastic, so the criterion below is met by construction.
- Its meaning becomes *the density to weight this direction with when
  strategies are combined* — which for every closed-form material is still the
  density `sample` drew from, and for the walk is the single-scattering
  density, declared as a proxy.
- A `BsdfSample` must be able to say which of three kinds it is: closed-form
  (divide by `pdf`), delta (use `weight`, skip in MIS), or stochastic (use
  `weight`, weight in MIS with the proxy). The `specular` flag becomes a
  three-way answer. Spelling is 0161's; the three cases are this item's.
- **Every strategy weights with the same proxy.** This is the load-bearing
  rule and the one with a silent failure, so `veach.hpp` in v0.8 owns it and
  the table above belongs next to it as a check rather than in this item.

### What it costs, stated rather than discovered

`./cornell chi2` loses the walk. Its argument is that `sample` and `pdf` are
two independent statements about one distribution that a machine can check
against each other, and for a stochastic lobe they are deliberately statements
about *different* distributions. The instrument is not weakened — it keeps
testing everything it tests today, including the single-scattering lobe that
the proxy comes from — but it cannot reach the new material, and pretending
otherwise by comparing a sampler against its own proxy would be a test written
to pass.

What replaces it is the furnace, which never needed a density: 0162. An
energy-conservation check does not care how the estimator was formed, which is
exactly why `furnace.hpp` opens by calling it the best test in rendering.

## Criterion, restated and met

No method returns a number that looks deterministic and is not. `pdf` returns
a real density; it is simply not always the one `sample` used, and the sample
says so.

## 2026-09-19

Answered. The contract keeps all three methods; pdf changes meaning and the change is written into bsdf.hpp where a reader meets it.

The key realisation is that this is the SECOND instance, not the first. A mirror's density is already not a function, and v0.1 settled the convention: weight pre-formed, flagged, eval and pdf zero, transport branching at the point of use. A walk borrows the estimator half of that unchanged. What it cannot borrow is being skipped by MIS - a delta must be, a stochastic lobe must not, because it carries most of the light off a rough metal.

So the question reduces to what MIS weights it with, and pdf turns out to have been doing two jobs that never needed separating: the density the estimator divides by, and the density MIS weights with. A walk needs only the second, and the second does not have to be correct - it has to be the same one every strategy uses. Veach's condition is that the weights sum to one, not that they are good.

Measured rather than asserted, 2^25 samples in 1024 batches: a proxy wrong by 2x costs 1.1x the sd and 0.2 sigma of bias; a hopeless proxy costs 1.8x the sd and 0.8 sigma; a proxy used on ONE side only is off by 814.7 sigma. The last is the failure mode and it is silent.

Decision: pdf stays deterministic - never an estimate, which is the line this project will not cross - and means 'the density to weight with when strategies are combined'. specular stops being a bool when the walk arrives, because there are three kinds of sample and two values can name two of them. Spelling is 0161's; the three cases are this item's.

Cost stated rather than discovered: chi2 loses the walk, because sample and pdf then describe different distributions BY DESIGN and comparing a sampler against its own proxy would be a test written to pass. The furnace does not need a density and is what checks it instead - 0162.

The consistent-proxy rule and its check are noted on 0091, where veach.hpp will live.
