---
id: 106
title: Hero wavelength sampling, so a prism does not band
type: optics
status: backlog
milestone: v0.9
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: sampling
effort: l
---

Once a path refracts dispersively, the wavelengths separate and a path can no
longer carry them together. The naive fix is one wavelength per path, which
works and is extremely noisy.

Wilkie et al., 2014: carry four, sample the first, derive the others by
rotation through the visible range, and combine them with — inevitably — the
multiple importance sampling machinery from v0.8, which is why that milestone
comes first.

- [ ] Falls back to four correlated wavelengths when nothing disperses
- [ ] The variance improvement over single-wavelength is measured
- [ ] chi2 covers the wavelength sampler like any other
