---
id: 53
title: The measured spectral reflectances of the walls
type: optics
status: backlog
milestone: v0.4
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: scene
effort: m
---

The red wall and the green wall are not `#ff0000` and `#00ff00`, and this is
the first place the no-RGB rule pays for itself visibly.

They are measured reflectance spectra of real paint, they are broad and
gentle rather than saturated, and the colour bleeding onto the white wall —
which is the single most famous thing about this image — is the product of
that spectrum with the light's spectrum, computed per wavelength.

Rendered from RGB, the bleed is a plausible tint. Rendered spectrally, it is
the answer.

- [ ] The wall spectra bypass the RGB upsampling path entirely
- [ ] `./cornell spectrum red-wall` plots the measured curve
