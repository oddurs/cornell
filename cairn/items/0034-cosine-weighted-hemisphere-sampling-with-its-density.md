---
id: 34
title: Cosine-weighted hemisphere sampling, with its density
type: optics
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
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

- [x] χ² test in v0.5 will exercise this first
- [x] The Jacobian appears in the comment

## 2026-09-13

The item's premise is wrong and the header says so: polar disc sampling with r = sqrt(u) preserves area exactly - chi2 over 16 equal-area annuli, mean 17.1 across 12 streams against 15 dof - and what it destroys is shape. Measured on a 32x32 stratified grid by nearest-neighbour spacing: concentric 0.04419 to 0.04908, ratio 1.11; polar 0.01588 to 0.05121, ratio 3.23. Density checks: the pdf integrates to 1.000122 over the hemisphere, and the samples follow it with chi2 17.5 over 20 cos bins on 19 dof.
