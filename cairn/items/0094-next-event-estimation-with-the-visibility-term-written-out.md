---
id: 94
title: Next event estimation, with the visibility term written out
type: optics
status: backlog
milestone: v0.8
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: transport
effort: m
---

At every non-specular vertex, connect directly to a light and test visibility.

The comment must write the estimator out in full — the BSDF, the two cosines,
the visibility indicator, the squared distance, the light's pdf — because this
is the single expression in which the most terms cancel and the most renderers
therefore lose one. House rule 3 exists for this line.

- [ ] The geometry term appears as a named quantity, not inlined
- [ ] Delta lobes are skipped, per the convention set in v0.6
- [ ] The image at 16 spp before and after goes in the README
