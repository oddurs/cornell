---
id: 153
title: transport.hpp is three files in a trenchcoat
type: chore
status: backlog
milestone: v0.5
created: 2026-09-14
updated: 2026-09-14
priority: p2
area: prose
effort: m
---

621 lines, against 253 for the next largest file in the project. Rule 8 says
one idea per file and the layout section says each is small enough to be read
in one sitting. This one holds, in order: the geometric-optics admission and
its four named omissions; the rendering equation in two notations with the
change of variables between them; which file owns which term; the list of
features with no code; the recursion-to-iteration argument; the estimator and
why it is not collapsed; the depth limit; Russian roulette and what it trades;
the vacuum admission; and three tables of measurements.

That is several ideas, and 570 of the 621 lines are prose.

## Why the obvious split is wrong

Move the admissions out. Except rule 7 says, in as many words, that the
geometric-optics limit "is stated at the top of `transport.hpp`" — so
`CLAUDE.md` mandates part of what makes this file long, and a split that
contradicts the rules to satisfy the rules is not a fix.

## What is actually separable

Russian roulette. It has its own item, its own argument — a bias-for-variance
trade — and its own measurements, and it is a technique for terminating an
estimator rather than a part of the rendering equation. Roughly 120 lines of
prose and ten of code.

The counter-argument is that it lives *inside* the path loop, and a file that
holds an estimator's termination rule separately from the estimator may be
worse to read than a long file.

## Why v0.5 rather than now

Because the answer probably depends on what v0.5 does to this file. The
convergence instrument and the furnace will both want to reach into the
transport, and if the loop needs to be parameterised for them, the seam will
be obvious rather than argued about.

## Acceptance criteria

- [ ] Either it is split, or the file says why 600 lines is the right size
      for it and rule 8 is amended to match
