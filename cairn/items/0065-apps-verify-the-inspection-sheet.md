---
id: 65
title: 'apps/verify: the inspection sheet'
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

Every physical claim the project makes, in one run, with a contents page.

`windsor` has this and it is the file that makes its voice credible: the
shop-manual tone is only earned because a single command re-derives every
figure quoted anywhere. This is the same instrument.

## What it prints

Sections, each with a claim, a method, a computed value, an expected value,
and a verdict. Exit code nonzero if anything fails.

## What it may not do

Reach into the physics. Every check must go through the same public surface a
reader would use.

## Acceptance criteria

- [ ] Every number in the README traces to a line of this output
- [ ] Runs in under a minute, or grows a `--quick` that says what it skipped
