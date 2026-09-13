---
id: 35
title: 'The sampler: reproducible per pixel, per sample, regardless of threading'
type: optics
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
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

- [ ] Two renders at different thread counts are bit-identical — moved to
      item 0059, which is where the threads arrive. There were none to
      count here
- [x] A test asserts the property that criterion is really about: a
      thousand pixels visited forwards and backwards give bit-identical
      draws, because a stream is addressed rather than dispensed

## 2026-09-13

Order-independence is tested directly rather than via threads: a thousand pixels visited forwards and backwards give bit-identical first draws, because a stream is addressed by (pixel, sample) rather than dispensed from a shared queue. The thread-count form of the criterion went to 0059 in v0.4, where the threads are. Measured: chi2 mean 63.1 against 63 dof over 20 streams of 2e6 draws, spread 50.2-85.6 against an expected sd of 11.2; mean 0.500144, variance 0.083376 vs 1/12. The correlation between the first draws of adjacent pixels is -0.000236 against a noise floor of 0.001; the same measurement on a bare LCG seeded pixel*1000 is +0.996906, which is the artefact the SplitMix64 mixing step exists to prevent.
