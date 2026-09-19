---
id: 161
title: The microsurface as a place a ray travels through
type: optics
status: done
milestone: v0.7
assignee: Oddur Sigurdsson
created: 2026-09-19
updated: 2026-09-19
priority: p0
area: bsdf
effort: xl
---

## What it is

The repair 0087 chose. Heitz, Hanika, d'Eon and Dachsbacher, "Multiple-
Scattering Microfacet BSDFs with the Smith Model", SIGGRAPH 2016.

`smith.hpp` treats the microsurface as a statistical abstraction: a
distribution of facets with a masking function that says what fraction of them
you can see. This makes it a *place*. A ray enters, hits a facet, reflects,
and if it is heading back into the surface it hits another one, and the walk
continues until it escapes. The light that the single-scattering model drops
is followed instead.

## What it must derive

That the walk's stationary behaviour is the Smith model it is built on —
the distribution of the first facet a ray meets is `visible_normals.hpp`'s,
and the probability that it escapes without another hit is `G₁`. The walk is
not a new model bolted on beside the old one; it is the old one with the
second bounce no longer thrown away, and the file has to show that rather
than assert it.

## What 0086 already knows about it

The two channels the walk has to follow, at roughness 1 and normal incidence:
0.193 masked on the way out, and 0.500 reflected into the surface. The second
is the larger, and it is not the one the usual telling of this failure blames.

A prediction to check against: if the escape probability were the same at
every bounce, the mean number of scattering events at roughness 1 would be
about three. It should come out below that, because a ray that has already
bounced is better oriented to leave.

## What is not modelled

Correlation between the heights a ray meets on successive bounces, which the
Smith model does not represent and which a real height field has.

## Acceptance criteria

- [x] The interface follows 0160's answer rather than inventing one
- [x] The walk reduces to single scattering when it is told to stop at one
      bounce, and that is checked against the current model exactly
- [x] `./cornell chi2` covers whatever part of it can be covered

## 2026-09-19

Built and measured. Albedo 1.000000000000000 at every roughness and angle - exact rather than converged, because with reflectance 1 the weight never changes and every walk escapes, so the estimator is the constant 1.

Two sign errors, both caught by checking a case whose answer was already known rather than by looking at output. The reflection formula 2(w.m)m - w is for a direction pointing AWAY from the surface; a propagation direction needs the reverse, and the escaping ray's direction of travel IS wi with no negation. With those wrong the walk never escaped upward and the albedo was 0.

The third and worst: smith.hpp's Lambda had no sign. It is derived for directions you can see the surface from, and the walk asks about rays travelling INTO it, where the projection has the other sign and straight down gives exactly -1. Without it the walk still conserved energy and still matched the single-scattering model at normal incidence, and was out by 0.216 at alpha 1 and 60 degrees - plausible everywhere, correct only where the sign cannot matter. Same shape as the 0084 frame bug, found the same way. Lambda is now signed; verify's figures are byte-identical, so it is a capability and not a behaviour change.

An interface problem 0160 missed: sample() gets exactly two variates and a walk needs an unbounded number. Resolved by spending the two on SEEDING a stream, which preserves specular.hpp's stated invariant - a path's draw count must not depend on what it hits, or two paths at the same address diverge. The recovery u * 0x1p32 is exact because next() is next_u32() * 2^-32.

furnace's directional_albedo_by_sampling formed f cos / pdf unconditionally and returned NaN for every row, which was luckier than it sounds: a plausible number would have been believed. It now carries the same branch transport.hpp has.

0087's bounce prediction was WRONG and should be recorded as such. I predicted 'near three, and below it, because a ray that has already bounced is better oriented to leave'. Measured 4.245 at alpha 1 head on, and uncapped the longest of 2^21 walks was 102. The reasoning was backwards - a ray that failed to escape is deeper in, not better aimed.

The contract change from 0160 (specular bool -> three-way Kind) is inert: every instrument printed the same figures before the walk existed.

Cost: furnace --bsdf walk is 11 s against the conductor's 5 s, which is the extra bounces and is the price 0087 said would be paid.
