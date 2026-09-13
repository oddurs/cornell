---
id: 114
title: Quote the difference as a number, in the README
type: prose
status: backlog
milestone: v1.0
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: prose
effort: m
---

## What it has to explain

What was compared, under what conditions, with what result, and what that
result does and does not establish.

A single number, with its method beside it. Not a pair of images captioned
"close enough".

## What it must not do

Round in its own favour, choose the metric after seeing the results, or omit
the regions where the agreement is worst. If the corners are wrong, the
corners are in the README.

## Acceptance criteria

- [ ] The metric is chosen and written down before the comparison is run
- [ ] The figure is refreshed by `./cornell verify` and re-checked after every
      model change, per house rule 6
