---
id: 34
title: Cosine-weighted hemisphere sampling, with its density
type: optics
status: backlog
milestone: v0.2
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: sampling
effort: s
---

## What it must derive

The warp and its pdf, from the inversion method, in the comment. Concentric
disc mapping (Shirley and Chiu) projected up to the hemisphere — and the
reason for concentric rather than polar is that polar mapping distorts area
badly near the centre and shows up as a visible artefact in low-sample images.

The pdf is `cos(theta)/pi` and it must be derived from the Jacobian, not
quoted.

## Acceptance criteria

- [ ] χ² test in v0.5 will exercise this first
- [ ] The Jacobian appears in the comment
