---
id: 68
title: 'Verify: reciprocity, and where it is legitimately violated'
type: verify
status: backlog
milestone: v0.5
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
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
rather than be relaxed when it starts failing.
