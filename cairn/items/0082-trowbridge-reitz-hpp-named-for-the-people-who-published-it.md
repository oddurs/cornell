---
id: 82
title: trowbridge_reitz.hpp, named for the people who published it
type: optics
status: backlog
milestone: v0.7
created: 2026-09-13
updated: 2026-09-13
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

- [ ] The normalisation integral is in the comment
- [ ] Both names appear; the file has one
