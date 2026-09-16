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

## Why this is not already covered by the chi-squared

It looks as though it should be. `./cornell chi2` compares a sampler against
its own density over the whole sphere and would surely notice a density that
was half of one.

It would not, and the reason is what Pearson's statistic is. The expected
count in a cell is the density's share of the total times the number of draws,
so halving every density halves every expectation, and dividing through by the
new total gives back exactly the expectations it started with. The statistic
compares *shapes*. A density that is uniformly half of one has the right shape,
agrees with its own sampler perfectly, conserves energy, and renders an image
that is uniformly wrong.

So there is a third deliberate liar beside the two in `chi2.hpp` — same
cosine-weighted shape, half the magnitude — and this section is the one that
catches it. Measured: -5.000e-01, against every other check on the sheet
passing it.

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
