---
id: 83
title: 'smith.hpp: derive the masking function from the distribution'
type: optics
status: done
milestone: v0.7
assignee: Oddur Sigurdsson
labels:
- thesis
- derivation
created: 2026-09-13
updated: 2026-09-18
priority: p0
area: bsdf
effort: l
---

## What it is

The best derivation in the project, and the purest expression of house rule 2.

Facets hide each other. The masking-shadowing function G says what fraction of
them are visible from a given direction, and every renderer written for
pleasure picks one from a menu — Smith, Cook-Torrance, implicit, Kelemen — as
though it were a style.

It is not a choice. Heitz showed in 2014 (*Understanding the Masking-Shadowing
Function in Microfacet-Based BRDFs*) that G follows from D by a single
constraint: the microsurface, projected onto the macrosurface direction, must
have the same area as the macrosurface. That one requirement determines
Smith's Lambda uniquely for any given distribution.

## What it must derive

Lambda for Trowbridge-Reitz, from that constraint, in the comment. It is about
a page of algebra and it converts the most arbitrary-looking term in the whole
model into a consequence.

## What is not modelled

Correlation between masking and shadowing. The separable form assumes the two
are independent, which is false at grazing angles; the height-correlated form
is better and is what should be implemented, with the separable one named as
the thing it improves on.

## Acceptance criteria

- [x] The projection constraint is stated and the derivation follows it
- [x] No menu of G functions exists in the codebase
- [x] The white furnace test is what confirms the derivation is right

## 2026-09-18

Lambda = (sqrt(1 + alpha^2 tan^2 theta) - 1)/2, derived in slope space from the covering requirement cos(theta_v) = integral G1 <v.m> D(m) dm. Every step checked numerically before it was written as prose: the marginal slope density alpha^2/(2(alpha^2+x^2)^{3/2}) against a 2D quadrature to 1e-9, and the closed form against A-/cos(theta) to 3e-8.

On criterion 3, precisely: what is on the sheet is Heitz's WEAK white furnace test (masking but no shadowing, every Fresnel factor 1), which is exactly what confirms G1 and which the derivation must satisfy. The full white furnace on a rough conductor needs the BRDF from 0084 and is item 0085; it will fail, and 0086 is why. The weak test integrates over the WHOLE SPHERE of wi, not the hemisphere - the facets reflecting below the horizon still block light, and over the hemisphere alone it comes to 0.50 at alpha 1, which cost an hour before the domain was the answer.

It is the only Monte Carlo row on the sheet, deliberately: the integrand jumps where the half vector crosses the horizon, so a tensor-product midpoint rule is first order there and at 85 degrees its residual sticks at 1.2e-3 from 256 cells through 1024 because the same grid lines straddle the cliff every time.

Criterion 2: the only masking function in include/render is this one. Beckmann's Lambda exists in verify.hpp as a liar, alongside HalfADensity and MisMeasured, and being caught is its whole job - it is the best demonstration in the project that G follows from D, because it is a correct function paired with the wrong distribution and looks like nothing at all.
