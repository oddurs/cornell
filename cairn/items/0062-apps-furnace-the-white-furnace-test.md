---
id: 62
title: 'apps/furnace: the white furnace test'
type: instrument
status: done
milestone: v0.5
assignee: Oddur Sigurdsson
labels:
- thesis
created: 2026-09-13
updated: 2026-09-16
priority: p0
area: instrument
effort: m
---

## What it witnesses

Energy conservation, visibly, as a pass/fail a human can see.

Put an object in an environment of uniform radiance 1 and give it a
reflectance of 1. If the BSDF conserves energy and the sampling is unbiased,
every direction returns exactly 1 and **the object disappears** — it renders
the same white as the background it sits in, and the image is a blank square.

This is the best test in rendering. It requires no reference image, no
tolerance chosen after the fact, and no judgement: either the object is there,
or it is not.

## What it prints

A blank white square, and the residual as a number:

    $ ./cornell furnace --bsdf lambert
    lambert          rho=1.0    residual 0.0000000   vanished

## What it is expressed in

Item 0151 made `Brdf` a type distinct from `Reflectance`, and left this item
the criterion it could not tick itself. The furnace's claim is precisely that
the *integral* of a BRDF against the cosine over the hemisphere is a
reflectance — dimensionless, at most 1 — while the integrand is not, and it
must be written in those two types rather than in one type used for both.

## What it may not do

Special-case anything. The furnace is the ordinary integrator pointed at an
ordinary scene; if it needed its own code path it would be testing that code
path.

## Acceptance criteria

- [x] Built before the BSDFs it will judge — v0.7 is where it starts failing
- [x] Residual quoted in the README for every BSDF in the project

There is one BSDF, and its residual is exactly zero. The README says so, and
says why that is the calibration rather than the result.

## What it found on the way

Three things, none of them about energy.

**The residual is exactly zero, not nearly.** `furnace.ppm` at rho = 1 holds a
single distinct byte value across 65,536 pixels. Every path returns the same
double the walls emit, because the estimator divides the same cosine by itself
and `rho/pi` against `1/pi` cancels when rho is 1.

**Which is why the asserted albedos are binary fractions.** At rho = 0.9 the
sphere comes back two ulp low — the two roundings of pi no longer agree — and
`worst |L - 1|` is then sixteen ulp high, because subtracting from 1 keeps the
absolute error and divides the value by nine. The same error, counted against
two different scales. It is reported under the table rather than asserted, so
that the choice of 1, 1/2, 1/4 and 0 does not look like a convenient one.

**The quadrature's residual is summation, not quadrature.** The integrand is
constant, so the midpoint rule is analytically exact, and what is left grows
with the number of terms: -3.3e-16 at 64 x 64 up to +1.2e-13 at 1024 x 1024.
Naive addition of N doubles. Nine orders of magnitude below the factor of pi
the table exists to catch, and worth printing rather than leaving as an
unexplained trailing 945.

## What the README had wrong

Its copy of the instrument list was four milestones stale: no `spec`, `verify`
filed under v0.5, and every `built` flag missing. Re-copied from what the
program prints. The item count said 135 against 157.
