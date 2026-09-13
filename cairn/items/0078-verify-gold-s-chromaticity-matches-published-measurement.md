---
id: 78
title: 'Verify: gold''s chromaticity matches published measurement'
type: verify
status: backlog
milestone: v0.6
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
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

- [ ] Copper and silver checked the same way
- [ ] The illuminant used for the comparison is stated with the figure
- [ ] The three numbers go in the README, refreshed from `./cornell verify`
