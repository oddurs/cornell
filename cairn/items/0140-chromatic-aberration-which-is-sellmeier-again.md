---
id: 140
title: Chromatic aberration, which is Sellmeier again
type: optics
status: backlog
milestone: v1.5
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: camera
effort: m
---

The purple fringing on a high-contrast edge is the lens focusing blue and red
at different distances, because the glass has a different index at each — the
same coefficients that made the prism work in v0.9, now in a lens.

Longitudinal and lateral, both, and both for free. An achromatic doublet
cancels most of it by pairing a crown and a flint, and that cancellation
should happen in this renderer because the two glasses are real and their
dispersions really do oppose.

- [ ] A doublet demonstrably reduces the fringing, with no correction code
