---
id: 55
title: 'Triangle intersection: choose the algorithm deliberately'
type: spike
status: backlog
milestone: v0.4
created: 2026-09-13
updated: 2026-09-13
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

<!-- Filled in when this closes. -->
