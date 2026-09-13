---
id: 96
title: 'Light selection: uniform, then by power, then by a tree'
type: optics
status: backlog
milestone: v0.8
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: light
effort: m
---

With one light, picking a light is free. With a thousand, it is the whole
problem, and the pdf of the selection has to enter the MIS weight correctly at
every stage.

Build it in order — uniform, then power-weighted, then a light BVH — because
each step is a strictly better estimator of the same integral, and the
convergence instrument should show each one improving without changing the
converged image. That is a good test of whether the pdfs were threaded through
correctly.

- [ ] The converged image is identical under all three strategies
- [ ] Only the variance differs, and by how much is measured
