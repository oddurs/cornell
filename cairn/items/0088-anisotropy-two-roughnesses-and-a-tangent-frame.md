---
id: 88
title: 'Anisotropy: two roughnesses and a tangent frame'
type: optics
status: backlog
milestone: v0.7
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: bsdf
effort: m
---

Brushed metal, hair, vinyl, the bottom of a saucepan. The facets are stretched
in one direction, so the distribution takes two roughness parameters and a
tangent frame to orient them.

It falls out of the same derivation as the isotropic case if the derivation
was done properly, which is a good test of whether it was.

- [ ] The isotropic case is the anisotropic one with equal parameters, not a
      separate code path
- [ ] chi2 passes for anisotropic sampling at extreme aspect ratios
