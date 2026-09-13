---
id: 141
title: Vignetting, which falls out of the barrel
type: optics
status: backlog
milestone: v1.5
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: camera
effort: s
---

The corners of a photograph are darker for three separate reasons, and only
one of them is a post-process in anybody's software: the cos^4 falloff from
geometry, mechanical occlusion by the lens barrel, and pupil aberration.

Trace through a real barrel and the first two arrive unbidden.

- [ ] The cos^4 law is derived in the comment
- [ ] Measured falloff compared with the analytic prediction
