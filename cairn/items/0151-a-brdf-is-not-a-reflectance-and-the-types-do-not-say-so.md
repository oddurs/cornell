---
id: 151
title: A BRDF is not a reflectance, and the types do not say so
type: chore
status: backlog
milestone: v0.5
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: spectrum
effort: l
---

`bsdf.hpp` returns the BRDF value as a `Reflectance`, and a BRDF is not a
reflectance. A reflectance is dimensionless and at most 1. A BRDF has units
of inverse steradians and is unbounded — a smooth mirror's is a delta
function, and a rough conductor's is routinely far above 1 near the
specular lobe.

The estimator `f · cos(theta) / pdf` is dimensionless because the sr⁻¹ in
`f` cancels the sr⁻¹ in `pdf`. None of that cancellation is visible to the
compiler, which is the one place this project's own argument — that
`Radiance` and `Irradiance` are distinct types because they differ by a
steradian — is currently not enforced.

It belongs in v0.5 rather than earlier because that is the milestone where
the claims get checked: the furnace test asserts that a BRDF integrates to
at most 1 over the hemisphere *against the cosine*, and getting that
integral's units wrong is exactly the kind of error the furnace exists to
catch and the type system should have prevented.

## What it needs

`Sampled` carries a tag and nothing else. It would need to carry a unit —
an exponent on steradians, at least — so that multiplying a `Brdf` by a
cosine and dividing by a density produces a `Reflectance` by construction
rather than by the author having checked. That is a change to
`spectrum.hpp` and to every expression downstream of it, which is why it is
an item rather than a paragraph.

## Acceptance criteria

- [ ] `Brdf` and `Reflectance` are distinct types
- [ ] `f * cos / pdf` yields a `Reflectance` without a cast
- [ ] The furnace test in v0.5 is expressed in those types
