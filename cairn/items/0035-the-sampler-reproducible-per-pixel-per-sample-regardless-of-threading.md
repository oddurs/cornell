---
id: 35
title: 'The sampler: reproducible per pixel, per sample, regardless of threading'
type: optics
status: backlog
milestone: v0.2
labels:
- foundation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: sampling
effort: m
---

## What it is

PCG32, seeded by a hash of `(pixel_index, sample_index)` rather than drawn
from a shared stream.

## What it must derive

Determinism. A renderer whose output depends on thread scheduling has made
every bug a heisenbug, and debugging a Monte Carlo image is hard enough with
the noise being reproducible. This is not a performance decision, it is the
decision that makes the project debuggable at all.

## What is not modelled

Low-discrepancy sequences — Sobol, Halton, blue-noise masks. They converge
faster and they complicate the χ² machinery, and the honest order is: get it
right with plain random numbers, prove it with the instruments in v0.5, then
make it faster and prove it again.

## Acceptance criteria

- [ ] Two renders at different thread counts are bit-identical
- [ ] A test asserts it
