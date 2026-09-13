---
id: 113
title: Render the box under the measured conditions
type: verify
status: backlog
milestone: v1.0
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: verification
effort: l
---

## The claim

Given the published geometry, the measured wall reflectances and the measured
light, this renderer predicts what was measured in the room.

## How it is checked

Render at the measured camera position with the measured emission, in absolute
radiometric units, with no exposure fitting of any kind — which is the part
that makes it a prediction rather than a match.

## What failure looks like

An error of a few percent is a result worth quoting. An error that can be
removed by scaling is an exposure bug and should be found and named. An error
that varies across the image is a transport bug and is the interesting case.

## Acceptance criteria

- [ ] No free parameter is fitted to improve the agreement, and the README
      says so in those words
- [ ] The comparison is in absolute units, not normalised
