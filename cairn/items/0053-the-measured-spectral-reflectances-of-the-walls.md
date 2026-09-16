---
id: 53
title: The measured spectral reflectances of the walls
type: optics
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
labels:
- thesis
created: 2026-09-13
updated: 2026-09-15
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

- [x] The wall spectra bypass the RGB upsampling path entirely — they are
      `Measured<76>` tables read straight from Cornell's figures, and
      `jakob_hanika.hpp` is never involved
- [x] `./cornell spectrum red-wall` plots the measured curve, and
      `green-wall` and `white-wall` beside it

## 2026-09-15

228 measured numbers, 400 to 700 nm at 4 nm, checked at compile time against the endpoints and extremes a reader can verify on the page. The red wall peaks at 0.657 and bottoms at 0.040 - a broad gentle curve, not a primary - and under D65 it comes out at linear sRGB (1.0000, 0.0991, 0.1038) with a luminous reflectance of 0.1435. Green peaks at 0.481 around 528 nm. Holding the endpoints outside 400-700 nm was measured rather than hand-waved: the observer has 0.0796%/0.1296% of x-bar, 0.0023%/0.0468% of y-bar and 0.3760%/0.0000% of z-bar outside that range, so reporting zero instead would darken every surface systematically and unevenly across channels, which is a tint. Also fixed: the spectrometer was printing a correlated colour temperature for a reflectance - 1632 K for the red wall - which is McCamy's cubic evaluated far off the locus it was fitted near and means nothing. It prints luminous Y instead for anything that is not a light.
