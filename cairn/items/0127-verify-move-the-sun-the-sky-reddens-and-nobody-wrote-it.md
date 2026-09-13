---
id: 127
title: 'Verify: move the sun, the sky reddens, and nobody wrote it'
type: verify
status: backlog
milestone: v1.2
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: verification
effort: m
---

## The claim

The colour of the sky at every solar elevation is a consequence of one
exponent and a density profile.

## How it is checked

Render the zenith and the horizon across solar elevations from 90 to -6
degrees, print the chromaticity of each, and compare the progression with
published daylight measurements.

## What failure looks like

If the sunset needs help — a tint, a gradient, a horizon colour — the model
has stopped being true, in exactly the way the metals thesis says it must
never. This is that check, for the atmosphere.

## Acceptance criteria

- [ ] A contact sheet across elevations goes in the README
- [ ] The zenith chromaticity at noon is compared with measurement
