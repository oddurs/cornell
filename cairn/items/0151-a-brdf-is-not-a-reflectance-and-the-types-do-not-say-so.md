---
id: 151
title: A BRDF is not a reflectance, and the types do not say so
type: chore
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-15
priority: p2
area: spectrum
effort: l
---

`bsdf.hpp` returned the BRDF value as a `Reflectance`, and a BRDF is not a
reflectance. A reflectance is dimensionless and at most 1. A BRDF has units of
inverse steradians and is unbounded — a smooth mirror's is a delta function,
and a rough conductor's is routinely far above 1 near the specular lobe.

The estimator `f · cos(theta) / pdf` is dimensionless because the sr⁻¹ in `f`
cancels the sr⁻¹ in `pdf`. None of that cancellation was visible to the
compiler, which made this the one place this project's own argument — that
`Radiance` and `Irradiance` are distinct types because they differ by a
steradian — was not being enforced.

## What it needed, and what it got

The item said `Sampled` would have to carry a unit, "an exponent on
steradians, at least". An exponent turned out to be the wrong shape: it only
pays for itself if something *computes* with it, and every legal operation in
`spectrum.hpp` is spelled out by hand precisely so that the illegal ones have
no generic rule to fall through. What the cancellation actually needs is for
the divisor to have a type.

So there are two new ones:

- `Brdf`, a `Sampled` with its own tag. It cannot scale a `Radiance`, because
  `f · L` is not a radiance.
- `SolidAngleDensity` in `density.hpp`, a density with respect to solid angle.
  It exists because there are two densities in this program — the other is
  `Wavelengths::pdf()`, which is per metre of wavelength — both spelled `pdf`,
  differing by sr·m⁻¹, and kept apart so far only by the accident that no
  expression has yet needed both.

And exactly two operations connect them to everything else. `per_steradian`
spreads a `Reflectance` over a solid angle to make a `Brdf`, which is the one
place an sr⁻¹ is born — `lambert.hpp` dividing its albedo by the projected
hemisphere. `operator/(Brdf, SolidAngleDensity)` is the one place it dies,
and it is the estimator.

## What the compiler now refuses

Eight expressions, checked with `-fsyntax-only`:

      REJECTED   a BRDF scaling a radiance
      REJECTED   a BRDF read as a reflectance
      REJECTED   a density read as a number
      REJECTED   f times a cosine, read as a reflectance
      compiles   f times a cosine, still a BRDF
      compiles   a reflectance scaling a radiance
      compiles   the estimator, f cos / pdf
      REJECTED   a reflectance divided by a density

The last one is the pleasing one. Dividing a reflectance by a density is not a
typo a person would think to guard against, and it is meaningless, and it now
fails to compile for the same reason the estimator succeeds.

## The image is unchanged, and the arithmetic is not

The estimator was `f * (cos / pdf)` and is now `f * cos / pdf`, because that
is what the types compose into. Floating-point multiplication is not
associative, so this is a different computation:

      albedo table entries            228
        rho*(1/pi) != rho/pi           32
      estimator evaluations     152000000
        f*(cos/pdf) != (f*cos)/pdf   53924616   (35.5%)

A third of the estimator evaluations land on a different double. And
`cornell.pfm` — 200 px, 32 spp, one thread — is byte for byte what it was
before, because a one-ulp difference in a double survives rounding to a float
about once in five hundred million and there are eighty thousand of them.
Worth stating rather than claiming the change was free: it was free at the
precision anything downstream can see.

## Acceptance criteria

- [x] `Brdf` and `Reflectance` are distinct types
- [x] `f * cos / pdf` yields a `Reflectance` without a cast
- [ ] The furnace test in v0.5 is expressed in those types

The third cannot be ticked here and is not being ticked: `apps/furnace` does
not exist. Item 0062 now carries it. The point of the furnace is that a BRDF
integrated against the cosine over the hemisphere is at most 1 — that the
*integral* is a reflectance while the integrand is not — and that sentence is
only expressible now.
