---
id: 36
title: Russian roulette, with the survival probability written out
type: optics
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
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

- [x] Starts only after a few bounces, and the reason (variance at low depth)
      is stated
- [ ] `converge` in v0.5 will confirm the slope is unchanged by enabling it
      — moved to item 0064, which is the instrument that measures slopes.
      Unbiasedness is checked here: 12 batches, mean z of −0.066 and +0.235
      against an expected ±0.289

## 2026-09-13

Measured against the closed cavity. Roulette on vs disabled, nothing else changed, 1e5 paths: rho 0.5 goes from 3.27s to 0.05s (65x) and rho 0.9 from 3.28s to 0.15s (22x); the price is a sample sd rising from ~0 to 0.586 and 8.97 respectively. Unbiasedness checked properly rather than assumed: 12 independent batches scored in standard errors give mean z of -0.066 and +0.235 against an expected +-0.289. The first two runs both landed about 1.7 se low, which looked like a bias and was not.
