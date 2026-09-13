---
id: 131
title: Random-walk subsurface scattering
type: optics
status: backlog
milestone: v1.3
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: media
effort: l
---

The honest version: light enters the surface, scatters hundreds of times
inside a dense medium, and leaves somewhere else. It is the volumetric
machinery from v1.1 with a refractive boundary, and it needs nothing new
except patience.

Everything else in this area — dipole diffusion, separable approximations,
screen-space blurring — is an approximation to this, and having the honest
version first means each of them can be measured against it rather than
argued about.

- [ ] Converges to the same answer as a dipole where the dipole is valid
- [ ] The failure regions of the dipole are shown, not described
