---
id: 58
title: 'Verify: the BVH agrees with brute force on a million random rays'
type: verify
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-15
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

- [x] Rays that start inside geometry are included — origins are drawn over
      a volume twice the scene's size, so many start inside the blocks and
      about half outside the box entirely, and one section starts every ray
      exactly on the floor
- [x] Rays exactly along an axis are included — all six, on a 201 x 201 grid
      of origins, which is where a slab test divides by zero and where an
      axis-aligned scene is met exactly
- [x] Degenerate and zero-area triangles are included — a point, a line and
      three collinear vertices, which also exercises a bounding box that is a
      point or a line

## 2026-09-15

1.64 million rays across four categories, every one intersected both ways and compared exactly rather than to a tolerance: 1000000 random, 242406 exactly axis-aligned on a grid of origins, 200000 starting exactly on the floor, and 200000 against a scene salted with a point, a line and three collinear vertices. Exact agreement everywhere. The instrument is apps/verify.hpp, which is item 0065's file arriving early and nearly empty - a check needing a million rays cannot be a static_assert, and a check with nowhere to live does not get run. CI runs it on every pull request, so a BVH that starts disagreeing cannot be merged.
