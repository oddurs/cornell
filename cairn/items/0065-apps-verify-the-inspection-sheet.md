---
id: 65
title: 'apps/verify: the inspection sheet'
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

- [x] Every number in the README traces to a line of this output
- [x] Runs in under a minute, or grows a `--quick` that says what it skipped
- [ ] A Macbeth chart under D65, against published Lab values — item 0049's
      last criterion. D65 itself and equal-energy white are checked as
      static_asserts in `illuminant.hpp`; the chart needs a spectral data set
      and a Lab implementation, which is an instrument's job rather than a
      header's

The third is not ticked and is not dropped. It is **item 0158**, with the
blocker named: the check needs the ColorChecker's spectral reflectances from a
citable source, and this repository does not have them. `lab.hpp` is the easy
half. Twenty-four spectra produced from memory would be a check that passes
because both sides came from the same place, which is the failure this file
opens by refusing.

## What the README audit found

The sheet runs in **1.7 seconds**, so there is no `--quick` and nothing to
explain — and the figure is printed at the bottom of the run rather than
claimed here, because the day somebody adds a check that renders at 4096
samples is the day this criterion silently stops being true.

The README was the other half, and it was worse than expected. Its "Where it
has got to" section was still at **v0.2**, three milestones behind, quoting:

- `spectral radiance at 555 nm, W/m2/sr/m: mean 0.4090` — a line the program
  had stopped printing entirely
- `exposed against 1.0 W/m2/sr/m and gamma 2.2` — an exposure and a transfer
  function the program had stopped using
- "five grey walls", about a box that has had Cornell's measured red and green
  paint on it since v0.4
- "a 0.6 m panel in a 2 m room", about a 0.130 x 0.105 m lamp in a 0.55 m room
- "the same room renders bit for bit identically at 1× and at 1000×" —
  which item 0156 had already found to be false, in this repository, four
  commits earlier: the powers of two are exact and 1000× is a rounding

The section immediately below all of that is the one explaining that every
figure in the file is a copy, that copies rot, and that this had already
happened once and been caught by a code review. It had happened again,
underneath the paragraph about it happening.

That is the argument for this item existing rather than for trusting the rule.
The rule is house rule 6 and it is correct; what it lacks is a mechanism, and
a criterion that says *every number traces to a line of output* is one,
because it makes the audit a thing that can be done rather than a habit that
can be skipped.

Every numeric block in the README now carries the command that printed it,
including the blindness matrix, which had none.
