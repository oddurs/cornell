---
id: 98
title: 'Verify: the variance actually drops, quoted as a number'
type: verify
status: backlog
milestone: v0.8
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: verification
effort: s
---

## The claim

MIS is worth the complexity it costs.

## How it is checked

`./cornell converge` with each strategy, and quote the sample count each needs
to reach the same RMSE. The claim in the README is then a ratio, measured,
rather than "greatly reduces noise".

## What failure looks like

If the ratio is small, the scene is one where one strategy already dominated,
and the honest thing is to find a scene where it is not — a large dim emitter
and a glossy floor — and quote both.
