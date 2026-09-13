---
id: 52
title: 'cornell.hpp: the box as a constexpr specification'
type: optics
status: backlog
milestone: v0.4
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: scene
effort: l
---

## What it is

This project's `windsor.hpp`, and the reason the repository has this name. A
real object, real figures, from the original source, with every chosen number
marked as chosen.

Built at compile time from the published dimensions and reflectances, so that
the scene is a specification a reader can check against a paper rather than a
data file they must take on trust.

## What it must derive

Everything that is a consequence: wall areas, the solid angle the light
subtends from the floor, the total flux entering the box, the theoretical
first-bounce irradiance at the centre of the floor. Those are all computable
from the specification, they all appear in `./cornell spec`, and they are all
things a reader can check by hand.

## What is not modelled

The real box had a camera in front of it, a room around it, and plywood that
is not perfectly Lambertian. Each of those is an item of its own.

## Acceptance criteria

- [ ] Every figure carries its source in a comment
- [ ] Every figure that was chosen rather than measured says CALIBRATED, in
      the `windsor` manner
- [ ] The file reads as a specification, not as data
