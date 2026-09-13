---
id: 61
title: Admit that the plywood is not Lambertian
type: prose
status: backlog
milestone: v0.4
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: prose
effort: s
---

Matte paint on plywood retro-reflects. Oren and Nayar modelled it in 1994 with
a roughness parameter over a Gaussian distribution of Lambertian microfacets,
and this project does not implement it.

The claim being made instead is that the error is smaller than the measurement
uncertainty in the comparison. That claim is checkable, it is checked in v1.0,
and until then it is an assumption written down where it can be found.

- [ ] The admission names the model that is missing and the paper it is in
- [ ] v1.0 either confirms the claim with a number or this becomes an item
