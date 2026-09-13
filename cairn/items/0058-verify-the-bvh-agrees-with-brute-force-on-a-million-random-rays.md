---
id: 58
title: 'Verify: the BVH agrees with brute force on a million random rays'
type: verify
status: backlog
milestone: v0.4
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: verification
effort: s
---

## The claim

The acceleration structure changes only the speed of the answer, never the
answer.

## How it is checked

Fire a million rays with random origins and directions at the box, intersect
each both ways, and compare the hit distance and primitive id exactly.

## What failure looks like

A BVH bug is the worst class of bug this project can have, because it produces
an image that is plausible everywhere and wrong in a thin band nobody notices
until a validation figure is off by two percent and three weeks have passed.

## Acceptance criteria

- [ ] Rays that start inside geometry are included
- [ ] Rays exactly along an axis are included
- [ ] Degenerate and zero-area triangles are included
