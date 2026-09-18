---
id: 78
title: 'Verify: gold''s chromaticity matches published measurement'
type: verify
status: done
milestone: v0.6
assignee: Oddur Sigurdsson
labels:
- thesis
created: 2026-09-13
updated: 2026-09-17
priority: p0
area: verification
effort: m
---

## The claim

The colour this project computes for gold, from the Johnson and Christy table
through Fresnel through the CIE observer, agrees with the published
chromaticity of a gold mirror at normal incidence.

## How it is checked

Compute the normal-incidence spectral reflectance, integrate against the
observer under a stated illuminant, quote the xy coordinate, and compare.

## What failure looks like

If this fails, the thesis fails. Either the tables are wrong, the complex
arithmetic is wrong, or the observer integration is wrong — and there is no
way to tell which from looking at a picture of a gold sphere, which is exactly
why the check exists.

## Acceptance criteria

- [x] Copper and silver checked the same way
- [x] The illuminant used for the comparison is stated with the figure
- [x] The three numbers go in the README, refreshed from `./cornell verify`

D65, stated in the heading of every table that quotes a chromaticity, because
a chromaticity without an illuminant is not a number.

## What it is compared against, which took rethinking

The item said "compare with the published chromaticity of a gold mirror". The
trouble with that is circularity: the chromaticity is a consequence of the
table, so comparing it against a figure derived from the same table proves
only that the arithmetic ran.

The **reflectance edge** is not circular. Where a metal's reflectance falls
off a cliff is an interband transition — an electron promoted from a filled d
band to the Fermi surface — and its energy is a property of the metal's band
structure, quoted in the solid-state literature independently of anybody's
optical measurement. Measured here as the steepest fall of R against photon
energy:

      metal       measured    literature
      gold         2.380 eV   ~2.4 eV, d-band threshold
      copper       2.129 eV   ~2.1 eV
      silver       3.740 eV   ~3.8 eV, plasma edge
      aluminium    1.342 eV   ~1.5 eV, parallel-band absorption

Four metals, four energies, none of them typed into this program and none
derivable from the tables without the equations in between. Silver's edge is
*above* the visible, which is exactly why silver is neutral and gold is not —
the ordering is a prediction rather than an observation.

And the comparison the project opens with: derived gold is (1.0000, 0.7020,
0.3514) against the typed (1.0, 0.766, 0.336), worst component 0.064, asserted
on the sheet.
