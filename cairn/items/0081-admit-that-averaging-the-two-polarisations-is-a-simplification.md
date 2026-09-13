---
id: 81
title: Admit that averaging the two polarisations is a simplification
type: prose
status: backlog
milestone: v0.6
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: prose
effort: s
---

Fresnel's equations give two reflectances, one per polarisation state. This
renderer carries scalar radiance, so it averages them, which is exactly right
for unpolarised light arriving at a surface for the first time and
progressively less right for light that has already reflected.

What it costs: glare off water and glass at steep angles is wrong in a way
that a polarising filter would reveal; the sky's polarisation pattern, which
bees navigate by, cannot be represented at all.

What it would cost to fix: four Stokes parameters per wavelength, a Mueller
matrix at every interaction, and roughly four times the state on every path.

- [ ] Stated in fresnel.hpp, with both costs named
- [ ] An item exists under `later` for doing it properly
