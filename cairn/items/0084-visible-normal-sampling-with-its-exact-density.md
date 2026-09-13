---
id: 84
title: Visible-normal sampling, with its exact density
type: optics
status: backlog
milestone: v0.7
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: sampling
effort: l
---

Sampling the normal distribution directly wastes most of its samples on facets
that face away from the viewer. Sampling the *visible* normals — the
distribution weighted by how much of each facet the viewer can see — is
strictly better, and Heitz's 2018 formulation does it by sampling a hemisphere
stretched into an ellipsoid, in about fifteen lines.

The pdf is exact, which matters more than the speed: an approximate pdf here
would pass casual inspection and fail the chi2 instrument, which is the whole
reason that instrument was built two milestones ago.

- [ ] chi2 passes at roughness 0.001 through 1.0
- [ ] The stretch-and-unstretch geometry is explained, not just performed
