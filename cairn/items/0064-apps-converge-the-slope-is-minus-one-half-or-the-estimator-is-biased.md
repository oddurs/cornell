---
id: 64
title: 'apps/converge: the slope is minus one half or the estimator is biased'
type: instrument
status: done
milestone: v0.5
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-16
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

- [x] The measured slope appears in the README, refreshed whenever the
      integrator changes
- [x] Russian roulette on and off both give the same slope
- [x] Confirm that enabling Russian roulette does not change the N^-1/2
      slope, which is item 0036's last criterion: the roulette was built to
      be unbiased and there was no instrument yet to measure a slope with

-0.5049 +/- 0.0114 with the roulette and -0.5109 +/- 0.0123 without, in the
Cornell box. Item 0036's last criterion is discharged.

## What the reference turned out to be

The item said "compute RMSE against a very long reference" and added that a
reference from the same code path is not allowed to be the whole story. Both
halves of that were right and the second was more expensive than it sounds,
because the obvious method is wrong twice and both ways were measured rather
than reasoned about.

**A reference shares draws with the thing being measured.** The sampler is
addressed by `(pixel, sample)` — the property that makes the thread count
irrelevant to the answer — which means a 1024-sample render *contains* a
512-sample render, draw for draw. The errors partly cancel, and cancel more
as N approaches the reference. Measured slope: **-0.5663**. Too steep, which
is precisely the signature this instrument exists to raise the alarm about,
manufactured here by the measurement.

**A reference has noise of its own.** Put it on disjoint samples and the
measured error becomes `sqrt(sigma^2/N + sigma^2/R)`, where the second term is
a floor that flattens the line exactly where the estimate was getting good.
Measured slope: **-0.4625**.

So there is no reference. Two renders at the same sample count on disjoint
halves of the sampler differ by the sum of two independent errors, so their
RMSE is sqrt(2) times either one's — and a constant factor is the one thing a
slope does not see. Both problems gone, no reference rendered, and the
cavity's closed form is what covers the "converging to the right answer"
half that no self-comparison can.

## The error bar is measured

At first the slope came out -0.33 between 512 and 2048 samples, which looked
like a serious bias. It was one firefly. Measured on that pair: kurtosis 290
against a normal distribution's 3, a worst pixel 27 times the RMSE, and an
RMSE a quarter too high; the same pair at 8192 samples measured kurtosis 20
and landed back on the line.

Which makes RMSE-over-1536-pixels a badly estimated statistic in a scene with
fireflies in it, and a hand-picked tolerance a tolerance picked after seeing
the answer. So the instrument pools four independent pairs, prints the
kurtosis of the errors it actually saw, and derives the standard error of the
slope from it. The pass condition is three of those.

## What the roulette needed

A scene with noise in it either way, which the cavity is not: with the
roulette off every path there returns the same number, so there is nothing to
converge and the only error left is the depth limit's truncation. That is
printed rather than asserted, because a column of zeros in a convergence table
looks like a bug and is the correct answer — and it comes with the one figure
worth having from it, `rho^256 / (1 - rho)` predicted as 1.932e-11 against
1.933e-11 measured.

`transport.hpp`'s `radiance()` gained a `roulette_start` parameter, beside the
depth limit it already had, because a claim about a switch needs a switch. A
depth past the limit is a roulette that never starts, which is what off means.
