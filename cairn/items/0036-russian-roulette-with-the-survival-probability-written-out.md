---
id: 36
title: Russian roulette, with the survival probability written out
type: optics
status: backlog
milestone: v0.2
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: transport
effort: s
---

Terminating a path early biases the result unless the survivors are scaled by
the reciprocal of their survival probability. Derive the unbiasedness in the
comment — it is three lines of expectation algebra and it is the difference
between a reader trusting the technique and a reader assuming it is a hack.

- [ ] Starts only after a few bounces, and the reason (variance at low depth)
      is stated
- [ ] `converge` in v0.5 will confirm the slope is unchanged by enabling it
