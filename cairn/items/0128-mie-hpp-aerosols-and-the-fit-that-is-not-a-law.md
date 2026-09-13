---
id: 128
title: 'mie.hpp: aerosols, and the fit that is not a law'
type: optics
status: backlog
milestone: v1.2
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: media
effort: l
---

Rayleigh covers particles much smaller than a wavelength. Haze, dust and water
droplets are not, and Mie's exact solution is an infinite series that nobody
evaluates at render time.

So this file is a fit, it is named for the person whose exact solution it
approximates rather than the person who fitted it, and that tension should be
stated: the honest name is somewhere between `mie.hpp` and the surname of
whoever produced the approximation actually used.

- [ ] The file says which it is: the exact series or a named approximation
- [ ] The forward-scattering lobe that makes a hazy day white is visible
