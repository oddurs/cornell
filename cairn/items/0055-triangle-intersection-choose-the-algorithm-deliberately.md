---
id: 55
title: 'Triangle intersection: choose the algorithm deliberately'
type: spike
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-15
priority: p2
area: geometry
effort: s
---

## Question

Moeller-Trumbore, or Woop's watertight algorithm?

## Why it has to be answered first

Moeller-Trumbore is shorter, faster, universally used, and leaks light through
shared edges at glancing angles. Woop's is watertight by construction, longer,
and needs a precomputed ray transformation.

The Cornell box is made of large triangles meeting at right angles. Leaks
along the edges of the box would be a visible, embarrassing, and entirely
avoidable artefact in the project's signature image.

## What would settle it

A test that fires ten million rays at a shared edge between two triangles and
counts the misses. If Moeller-Trumbore leaks at the box's own dimensions, the
question answers itself.

## Answer

**Möller–Trumbore**, and the question did answer itself: it does not leak at
the box's own dimensions, in double precision, for any ray that is not aimed
exactly at an edge.

### The test this item asked for

A quad at the back wall's size — 0.5592 m — split into two triangles sharing
the diagonal, which is how every wall of the box gets built.

    10,000,000 rays aimed exactly at the shared edge     26.63 % missed
    10,000,000 rays aimed anywhere on the quad            0     missed

So the leak is real and it is confined to the measure-zero set. A ray that
lands exactly on the edge misses about a quarter of the time; a ray that lands
anywhere else never does, across ten million attempts.

### The test that actually settles it

The published box, sealed with a front wall it does not really have, so that
from inside it every ray must hit something and any escape is a leak:

    1,442,401 rays on an exact-fraction direction grid    0 escaped
    2,000,000 rays on sampled directions                  0 escaped

The first of those is deliberately adversarial. The directions come from
exact fractions `i/1200`, so they land on round numbers and meet this
axis-aligned geometry at exactly the coincidences that would expose a
watertightness failure. Nothing escaped.

    1,000,000 rays aimed exactly along the floor/right-wall corner
                                                          9.48 % missed

Aim at an edge on purpose and it leaks. Render a picture and it does not.

### Why this differs from the received answer

Woop, Benthin and Wald's watertight algorithm was published for `float`
renderers, and the leak rates that motivate it are `float` leak rates. This
project is in `double` throughout — a decision made in `si.hpp` for
unrelated reasons — and 52 bits of mantissa move the failure from
"occasionally visible" to "only when aimed at deliberately".

The other half of watertightness shows up too, and is harmless here: of the
rays aimed exactly at the shared edge, 23.4 % were claimed by *both*
triangles. For a closest-hit query that is two identical distances and either
answer is correct; for an occlusion query it is two identical yeses.

### When to revisit

If this project ever moves to `float`, or acquires meshes dense enough that a
pixel spans several shared edges, the answer changes and this test is written
down so it can be re-run rather than re-argued. `triangle.hpp` already names
Woop in its "what is not modelled" and now points here.
