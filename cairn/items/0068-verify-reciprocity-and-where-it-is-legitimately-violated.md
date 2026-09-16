---
id: 68
title: 'Verify: reciprocity, and where it is legitimately violated'
type: verify
status: done
milestone: v0.5
assignee: Oddur Sigurdsson
labels:
- admission
created: 2026-09-13
updated: 2026-09-16
priority: p2
area: verification
effort: s
---

## The claim

`f(wi, wo) == f(wo, wi)` for every BSDF in the project, except where physics
says otherwise.

## How it is checked

Random pairs of directions, every BSDF, every roughness.

## What failure looks like

Non-reciprocal BSDFs break bidirectional path tracing in v1.4 in ways that are
extremely hard to diagnose from an image.

## The exception, which must be stated rather than excused

Refraction through an interface between media of different index is *not*
reciprocal in radiance — it carries a factor of the squared index ratio, for
the good reason that radiance itself is not conserved across such a boundary.
That is real physics, it arrives in v0.9, and this test must know about it
rather than be relaxed when it starts failing. What stays exactly true across
such a boundary is

      f(wo, wi) / n_o^2  ==  f(wi, wo) / n_i^2

and a reflector is the case where the two indices are the same one. That
sentence is in `verify.hpp` now, before the file that needs it exists, so that
the day it starts failing there is something to read other than a tolerance
somebody widened.

## What it measured

2^20 direction pairs per BSDF, drawn over the whole **sphere** rather than the
hemisphere, so that 524,901 of them land on opposite sides of the surface.
Those are not skipped: a reflector must return zero for both orderings, and
zero on both sides is a claim worth making — a hemisphere test written with
the wrong comparison is how light leaks through a wall. The check requires
both that some pairs cross the surface and that not all of them do, which is
what stops it passing on a BSDF that returns zero for everything.

Compared exactly, with no tolerance. A Lambertian's BRDF does not depend on
either direction once both are on the same side, so the two calls return the
same double rather than nearly the same one, and a tolerance here would be
room for an error that cannot exist yet to hide alongside one that can.

And the calibration: a BRDF that weights only `wi` — the shape of every model
somebody has invented by multiplying a cosine into the wrong place. It samples
honestly, it has a valid density, and it fails here at a worst difference of
1.588e-01.
