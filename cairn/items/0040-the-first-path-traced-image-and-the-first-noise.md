---
id: 40
title: The first path-traced image, and the first noise
type: instrument
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: instrument
effort: s
---

## What it witnesses

That the integrator works. A box, a light, matte walls, and an image that is
correct and extremely noisy.

## What it prints

`./cornell render --spp 16` and `--spp 4096`, side by side, so that the
project's relationship with noise is established early: it is the variance of
an estimator, it falls as the inverse square root of the sample count, and
v0.5 will measure that rather than assert it.

## 2026-09-13

The image shows a soft-edged shadow, corners darker than wall centres, and a lit ceiling, none of which has any code. Noise measured rather than asserted: RMSE against an 8192-spp reference at 16/64/256/1024 spp is 1.19850, 0.67538, 0.29972, 0.14068, ratios 1.775, 2.253, 2.130 against an expected 2, and a least-squares slope of -0.522 against -0.5. Some of the excess is the reference itself not being converged; v0.5's converge does it properly. Throughput 2.5 million paths per second single-threaded. The exposure is stated: 1 W/m2/sr/m maps to white, which puts the walls near mid-grey and clips the lamp 12x over, as a photograph exposed for the walls does.
