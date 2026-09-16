---
id: 67
title: 'Verify: every pdf in the project integrates to one'
type: verify
status: done
milestone: v0.5
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-16
priority: p0
area: verification
effort: m
---

## The claim

Every density function the project defines is a density function.

## How it is checked

Numerically integrate each `pdf()` over its domain — the sphere, the
hemisphere, the surface of a light — by a quadrature independent of the
sampling routine it belongs to.

## What failure looks like

An unnormalised pdf produces an image that is uniformly too bright or too dark
by a constant factor, which is exactly the kind of error that gets
compensated for by adjusting the light's intensity until it looks right, and
then never found.

## Why this is not covered by the chi-squared — corrected

This item was written and closed on the argument that `./cornell chi2` is
blind to a normalisation error, because Pearson's statistic compares *shapes*:
halve every density, renormalise the expectations to the draw count, and they
come back unchanged.

**That is true of a chi-squared that renormalises, and this project's does
not.** Item 0071's blindness matrix — every deliberate liar through every
check, rather than each one through its own — measured the half-density at
chi2/dof 515 and p = 0. The expected count here is the density integrated over
the bin times the number of draws, full stop, so a missing factor of two shows
up as one.

Which makes the chi-squared stronger than the textbook version rather than
weaker, and leaves this section standing for two reasons that survive the
correction:

- It is **exact rather than statistical**: -5.000e-01, not a p-value.
- It needs **no sampler**. That is the form v0.8 requires, where multiple
  importance sampling evaluates one strategy's density on directions another
  strategy produced, and a chi-squared has nothing to compare against.

The honest version of the original claim is narrower and still worth making: a
density that is unnormalised *and* whose sampler agrees with it cannot exist,
because the missing half has to go somewhere. The half-density liar is a
sampler drawing from a normalised distribution while reporting an unnormalised
one, and that is a disagreement — which is exactly what the chi-squared is
for.

## What was integrated

Every density the project has, over its own domain, by a midpoint rule that
does not call the sampling routine it belongs to:

      lambert, all three alternatives, over the sphere     +2.887e-14
      warp.hpp's cosine hemisphere, over the hemisphere    +1.243e-13
      the hero wavelength draw, over 360-830 nm            +1.732e-14

The domains are not the same domain, which is the point `density.hpp` exists
to make: the first two are per steradian and the third is per metre of
wavelength, they are both spelled `pdf`, and nothing in this program may
divide one into the other.

The residuals are summation and not quadrature — every integrand here is
constant or exactly integrated by the midpoint rule, so what is left is a
million doubles added end to end.

There is no row for the surface of a lamp, because there is no light-sampling
density yet. It arrives with v0.8's multiple importance sampling, and so does
the row.
