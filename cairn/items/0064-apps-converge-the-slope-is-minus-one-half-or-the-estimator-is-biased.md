---
id: 64
title: 'apps/converge: the slope is minus one half or the estimator is biased'
type: instrument
status: backlog
milestone: v0.5
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: instrument
effort: m
---

## What it witnesses

That the renderer is unbiased.

Render the same image at increasing sample counts, compute RMSE against a very
long reference, and fit the slope on a log-log plot. Monte Carlo error falls
as N to the minus one half. That is not a rule of thumb, it is the central
limit theorem, and any renderer whose error falls faster is not clever — it is
biased, or it is measuring against a reference produced by the same bias.

## What it prints

    $ ./cornell converge
    spp      rmse
    16       0.08412
    ...
    slope   -0.4997     unbiased

## What it may not do

Use a reference rendered by the same code path as the thing being measured
when the point is to detect a systematic error. Where possible the reference
comes from an independent estimator.

## Acceptance criteria

- [ ] The measured slope appears in the README, refreshed whenever the
      integrator changes
- [ ] Russian roulette on and off both give the same slope
- [ ] Confirm that enabling Russian roulette does not change the N^-1/2
      slope, which is item 0036's last criterion: the roulette was built to
      be unbiased and there was no instrument yet to measure a slope with
