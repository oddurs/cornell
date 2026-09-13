---
id: 87
title: What to do about the missing energy
type: spike
status: backlog
milestone: v0.7
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: bsdf
effort: m
---

## Question

Given that the single-scattering model loses energy, what does this project do
about it?

## Why it has to be answered first

Every answer is a compromise and the project's credibility depends on saying
which one it took.

## Options

**Kulla-Conty**: add a compensation lobe fitted to the measured deficit. Cheap,
used in production, and it is a fit — house rule 8 says it would have to be
named for whoever fitted it and marked as a curve.

**Heitz's stochastic multiple scattering**: random-walk the microsurface.
Correct by construction, unbiased, slower, and beautiful — it makes the
microsurface a real place that a ray actually travels through rather than a
statistical abstraction.

**Leave it, and quote the deficit.** Honest, and leaves the furnace test
failing forever, which erodes the instrument.

## What would settle it

The second option is the one that fits this project: it is a derivation rather
than a fit, the furnace test then passes for the right reason, and the cost is
performance, which this project has already said it will spend.

## Answer

<!-- Filled in when this closes. -->
