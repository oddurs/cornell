---
id: 71
title: 'Prose: why the instruments are built before the physics'
type: prose
status: done
milestone: v0.5
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-16
priority: p2
area: prose
effort: s
---

## What it has to explain

That a check written after the thing it checks is a check written to pass.

The ordering of this roadmap is its most deliberate feature: the furnace, the
chi-squared test and the convergence plot are built at v0.5, and the microfacet
model that will fail all three arrives at v0.7. That failure is the point. It
is the project catching itself, in public, with an instrument it built before
it knew what the answer would be.

## What it must not do

Claim more rigour than the instruments deliver. They test what they test.

## How that got written down

Not as a caution, which is what a sentence like the one above usually becomes.
Each v0.5 check already carried a deliberately wrong model written to be
caught by it; this item ran **every liar through every check** and printed the
misses beside the catches.

      deliberately wrong model            furnace     chi2  density  swapped
      claims a flat density                     -   caught        -        -
      claims one 2% too steep                   -   caught        -        -
      a density that is half of one             -   caught   caught        -
      weights only the incoming ray        caught        -        -   caught

Four models, each wrong in one way, none caught by every column. A density can
be two percent wrong and conserve energy exactly — it sails through the furnace
and is invisible in an image. A BRDF that weights only the incoming direction
samples honestly and has a valid density. No column is sufficient.

It is in `./cornell verify`, not in prose, so it cannot quietly stop being
true when an instrument changes.

## And it immediately corrected something

The density column is the honest exception: nothing here is caught by it
alone. Item 0067 had closed on the argument that a chi-squared compares shapes
and is therefore blind to a density that is uniformly half of one. That is
true of a chi-squared which renormalises its expectations to the observed
total, and this project's does not — it measures the half-density at chi2/dof
515 and p = 0.

So the check that was justified by a blindness that does not exist is now
justified by two narrower things that do: it is exact rather than statistical,
and it needs no sampler, which is the only form available in v0.8 where a
density is evaluated on directions another strategy produced. The correction
is in `chi2.hpp`, in `verify.hpp`, in item 0067 and in the README.

That is the strongest form the argument for building instruments early can
take: the matrix was built to say what the instruments cannot see, and the
first thing it found was a place where the *project* had claimed the wrong
blindness.
