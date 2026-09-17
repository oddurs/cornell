---
id: 79
title: Perfect specular reflection, and the delta pdf convention
type: optics
status: done
milestone: v0.6
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-16
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

- [x] The convention is stated in one place and referenced, not restated
- [x] The chi2 instrument knows to skip delta lobes and says so in its output

The convention is in `bsdf.hpp`, on `BsdfSample`, with the list of everywhere
that has to obey it — `transport.hpp`, `chi2.hpp`, and v0.8's weighting, which
does not exist yet and is named anyway. `specular.hpp` and `transport.hpp`
point at it rather than restating it.

## The weight, derived rather than asserted

The item said the convention must be written down, and writing down *what the
weight is* turned out to be the part worth doing:

      f(wo, wi)  =  R(theta) delta(wi - mirror(wo)) / cos(theta)
      pdf(wi)    =            delta(wi - mirror(wo))

so `f · cos / pdf` is exactly `R(theta)` — the delta cancels and the cosine
cancels the one the BRDF carries. That `1/cos` is not a fudge inserted to make
the arithmetic work: it is what a delta BRDF has to carry for the rendering
equation's cosine to come back out, and writing it down is the difference
between a convention and a coincidence.

`weight` is therefore a `Reflectance` where `f` is a `Brdf`, and item 0151's
types make the two impossible to confuse.

## What checks it

A mirror of reflectance 1 has to vanish in the furnace exactly as a Lambertian
of reflectance 1 does, and it does — |L−1| of 0.000e+00, with 1/2 and 1/4
visible by exactly 1 − R. Each of the three ways to get the convention wrong
(dividing by the delta's zero density, not dividing at all, taking `f` instead
of `weight`) makes the mirror too dark or too bright by a factor that looks
like a decision somebody made about how shiny things should be.

Two other sections of the sheet had to learn about it, and both now say so
rather than quietly excluding it:

- The chi-squared **declines** on the first draw rather than running and
  reporting p = 0 for a perfectly correct surface. A test that is right about
  there being a problem and wrong about what it is, is worse than no test.
- The normalisation check reports a mirror integrating to **exactly zero**
  over 65,536 cells, which is the correct answer to the wrong question. Its
  heading claims every density integrates to one; the one that is not a
  function is named underneath rather than left out of the loop.
