---
id: 102
title: The dielectric BSDF, and the sign bug everybody writes
type: optics
status: backlog
milestone: v0.9
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: bsdf
effort: m
---

## What it is

Glass, water, a diamond. Fresnel decides the split between reflection and
refraction, and it is already written.

## What it must derive

Nothing new. This item exists mostly as a warning: the sign of the cosine at
the boundary, and the decision about which way round the indices go, is the
single most commonly written bug in ray tracing. It produces glass that is
*almost* right — right at normal incidence, wrong at the silhouette, and
entirely plausible to a reader who is not looking for it.

The test is the next item and it must be written before this one is believed.

## What is not modelled

Absorption inside the glass, which is Beer's law and is separate. A thick
piece of "clear" glass is green, and that green comes from the medium, not the
surface.

## Acceptance criteria

- [ ] Entering and leaving are one code path, not two
- [ ] The hollow sphere test passes
