---
id: 57
title: 'The BVH: a binned SAH build, and the one file about speed'
type: optics
status: backlog
milestone: v0.4
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: accel
effort: l
---

## What it is

The acceleration structure, and the file that is honest about being a
performance optimisation rather than a piece of physics. Every other file in
the project earns its place by explaining something; this one earns its place
by making the others runnable, and it says so in its first paragraph.

## What it must derive

The split planes, from the surface area heuristic: the expected cost of a
split is the probability of entering each child — which for a convex child
inside a convex parent is the ratio of their surface areas — times the cost of
traversing it. Sixteen bins, evaluated in a sweep.

A median split is two lines shorter and two to four times slower, and the
comment should say so with the measured figure rather than the claim.

## What is not modelled

SIMD packet traversal, ray reordering, and every other technique that would
make this two to five times faster and unreadable. Stated as a decision: this
project trades throughput for legibility, and this is the file where that
trade is most costly and most deliberate.

## Acceptance criteria

- [ ] Flat array of 32-byte nodes, `left = node + 1`, no pointer tree
- [ ] Build cost and traversal speedup both measured and quoted
- [ ] The file's opening admits what it is
