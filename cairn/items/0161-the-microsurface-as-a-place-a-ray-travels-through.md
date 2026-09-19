---
id: 161
title: The microsurface as a place a ray travels through
type: optics
status: backlog
milestone: v0.7
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

- [ ] The interface follows 0160's answer rather than inventing one
- [ ] The walk reduces to single scattering when it is told to stop at one
      bounce, and that is checked against the current model exactly
- [ ] `./cornell chi2` covers whatever part of it can be covered
