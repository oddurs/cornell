---
id: 61
title: Admit that the plywood is not Lambertian
type: prose
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
labels:
- admission
created: 2026-09-13
updated: 2026-09-15
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

- [x] The admission names the model that is missing and the paper it is in —
      Oren and Nayar, "Generalization of Lambert's Reflectance Model", 1994 —
      and says which direction the error goes and where it is largest
- [ ] v1.0 either confirms the claim with a number or this becomes an item —
      handed to item 0115, which is where every place the model was let off
      gets stated against a number. This is the first entry on that list

## 2026-09-15

Cornell makes the assumption first and says so: 'Surfaces are assumed to be Lambertian' appears on the data page before a single number. So the admission belongs in cornell.hpp, where the paint is, rather than in lambert.hpp. The missing model is named with its paper - Oren and Nayar, Generalization of Lambert's Reflectance Model, 1994 - along with the mechanism and the direction of the error: matte paint retroreflects, brightest seen from the direction the light comes from and brighter still at grazing angles, because a rough surface's facets mask each other and looking along the illumination shows only the lit ones. What makes it a claim rather than a shrug is that the departure is largest at grazing angles and in corners, which is where the colour bleeding is and what v1.0 measures. The second criterion went to 0115, where every place the model was let off gets a number.
