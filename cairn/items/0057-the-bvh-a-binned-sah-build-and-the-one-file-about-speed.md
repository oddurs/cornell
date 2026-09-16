---
id: 57
title: 'The BVH: a binned SAH build, and the one file about speed'
type: optics
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-15
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

- [x] Flat array of 32-byte nodes, `left = node + 1`, no pointer tree — and
      32 bytes means `float` bounds in a `double` renderer, rounded outward so
      the float box always contains the double box
- [x] Build cost and traversal speedup both measured and quoted — 1.59x on
      the Cornell box, 59x at twenty thousand primitives, and 190 to 222 ns
      per primitive to build
- [x] The file's opening admits what it is

Corrected: this item said a median split is "two to four times slower". It is
about twenty per cent slower at scale and no slower at all on a small scene.
The comparison earned its place anyway — the first run had the median split
seventeen times *faster*, which was a bug in the heuristic's bounds
arithmetic rather than a result.

## 2026-09-15

Measured against exhaustive intersection, which scene.hpp keeps for the purpose: 1.59x on the Cornell box's 38 triangles, 2.58x at 238, 10.60x at 2038 and 59.00x at 20038. Build is 190-222 ns per primitive, 4.5 ms for twenty thousand. The item's claim that a median split is two to four times slower is overstated - measured fairly it is about 20 per cent slower at scale and no slower on a small scene. That comparison found a real bug though: the first run had median beating the heuristic by seventeen times, which is a bug report rather than a tuning difference. Bounds::grow(const Bounds&) was growing by empty boxes, whose low is +inf and high is -inf, so one empty bin poisoned the running bounds for every later candidate, every split costed infinity, none was chosen, and nodes became leaves of arbitrary size - 757 nodes for twenty thousand primitives instead of 13485. It produced correct images throughout. It simply was not accelerating anything.

## 2026-09-15

Two real bugs in the BVH, both found by a code review and both reproduced before fixing. (1) The traversal stack was a fixed uint32[64] with no depth cap in the builder: exponentially spaced centroids make every split peel off one primitive, so 998 of them built a tree of depth 107 and overflowed the stack on every worker thread at once, producing correct-looking images. The build caps depth at 60 now and a static_assert ties that to the stack size. (2) count was a uint16 and make_leaf was reachable with arbitrary count - the zero-width, no-candidate and degenerate-partition paths all fell through to it - so 70008 coincident primitives built a tree reaching 4472 and losing exactly 65536, silently. count is 30 bits packed with a 2-bit axis now, still 32 bytes, checked by static_assert at 70000 and at the stated maximum; and when the heuristic declines on a set too large to leaf, an arbitrary split by index is taken instead. Also: ./cornell verify could have been vacuous, because Scene::intersect falls back to the exhaustive path for a single-node tree, so a scene that declined to split would have compared the exhaustive search against itself and printed agree. It now refuses to run in that case. And the new deep-tree check found that comparing surface pointers is too strict for coincident geometry - the distances are identical and the two routes are entitled to break a tie differently - so ties are counted and reported separately from disagreements.
