---
id: 82
title: trowbridge_reitz.hpp, named for the people who published it
type: optics
status: done
milestone: v0.7
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-18
priority: p0
area: bsdf
effort: m
---

## What it is

The normal distribution function: how the microscopic facets of a rough
surface are oriented, as a probability density over the sphere.

House rule 9 in its clearest case. Trowbridge and Reitz published this in
1975, in *Average Irregularity Representation of a Rough Surface for Ray
Reflection*. Walter, Marschner, Li and Torrance rediscovered it in 2007, named
it GGX, and the field has used the second name ever since. Both names go in
the comment; the file takes the first.

## What it must derive

The normalisation. A normal distribution must integrate to one over the
projected hemisphere — `∫ D(m) (n·m) dm = 1` — and that requirement is what
fixes the constant. Derive it; do not quote it.

## What is not modelled

Beckmann, which is the physically derived Gaussian alternative and which
almost nobody uses because its tails are too short to match measured
materials. Mentioned, because the fact that the field chose the empirically
better fit over the theoretically motivated one is exactly the kind of thing
this project should tell a reader.

## Acceptance criteria

- [x] The normalisation integral is in the comment
- [x] Both names appear; the file has one

## 2026-09-18

The derivation gives k = alpha^2/pi from the covering condition alone; verify integrates D(m)(n.m) on a 512x512 midpoint grid and halves the spacing, and the residual divides by 4.00 at every alpha, which is the rule's second order rather than the model. The integral without the projection is 1.0095 at alpha=0.05 and exactly 2 at alpha=1 — the solid-angle mistake is invisible on a polished surface and a factor of two on a rough one. Blinn had this distribution in 1977 and the field used the other two he compared. Beckmann is mentioned, not written; the tail evidence is Walter 2007's ground glass, not Ngan 2005, which did not test this distribution.
