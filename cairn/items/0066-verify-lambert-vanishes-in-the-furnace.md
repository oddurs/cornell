---
id: 66
title: 'Verify: Lambert vanishes in the furnace'
type: verify
status: done
milestone: v0.5
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-16
priority: p0
area: verification
effort: s
---

## The claim

A Lambertian surface with reflectance 1, in a uniform environment of radiance
1, returns exactly radiance 1 in every direction.

## How it is checked

`./cornell furnace --bsdf lambert`, residual below 1e-6.

## What failure looks like

Too dark by a factor of pi: the normalisation was dropped. Too dark by a small
amount: the cosine is being applied twice, or once in the sampler and once in
the estimator. Too bright: the pdf is wrong.

Every one of those is invisible in a rendered image and obvious here.

## What it actually checks

The residual asked for was "below 1e-6". It is **exactly zero**, so the check
is `== 0.0` and not a tolerance. A tolerance would be a place for an error to
hide that this scene cannot produce: the estimator divides the same cosine by
itself and `rho/pi` against `1/pi` cancels exactly at rho = 1, so the correct
answer is a bit pattern rather than a small number.

Three albedos, not one. At reflectance 1 the sphere must be gone; at 1/2 and 0
it must be *there*, by exactly `1 - rho`. Without those rows the check would
also pass on a scene that had lost the sphere, which is the way this test
fails silently — and the pixel count in the row is the second guard against
the same thing.

And one claim an image cannot make: the BRDF integrated against the cosine
over the hemisphere is a reflectance, and for this one it is the albedo. Taken
through `eval` alone at four incident angles, so it is blind to the sampling
routine, and worst 2.398e-14 over a 256 x 256 grid — which is summation of
65,536 doubles rather than quadrature error, because the integrand is constant
and the midpoint rule is analytically exact. That is the distinction item 0151
made a type for: the integrand is a `Brdf` and the integral is a
`Reflectance`, and the sentence was not expressible before.

The instrument itself is `./cornell furnace`, which prints the argument, all
three ways of asking, and the picture. What is on the inspection sheet is the
sentence and its verdict.
