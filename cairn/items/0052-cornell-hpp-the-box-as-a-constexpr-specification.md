---
id: 52
title: 'cornell.hpp: the box as a constexpr specification'
type: optics
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
labels:
- thesis
created: 2026-09-13
updated: 2026-09-15
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

- [x] Every figure carries its source in a comment
- [x] Every figure that was chosen rather than measured says CALIBRATED, in
      the `windsor` manner — and the file uses four markings rather than two,
      because the data needed them: MEASURED, INFERRED (the units, which are
      not stated), ASSUMED (Lambertian, which Cornell assumed) and CALIBRATED
- [x] The file reads as a specification, not as data — and `./cornell spec`
      prints the consequences, every one computed from the vertices

## 2026-09-15

The published box renders: red wall left, green right, both blocks, the lamp filling its hole in the ceiling, colour bleeding onto the white surfaces from measured spectra rather than a tint. 38 triangles. Cornell's own camera - 278 273 -800 mm, 35 mm lens on 25 mm square film - which camera.hpp turns into a 39.3 degree field of view without being told one. ./cornell spec prints the derived quantities and checks itself where it can: the lamp's solid angle from the floor beneath it is 0.044803 sr by closed form and 0.044803 sr by numerical integration, differing by 1.26e-10. The two side walls differ in area by 1.56e-05 m2, which is the measurement rather than an error. apps/box.hpp is deleted: the test rig existed so the integrator had corners before there was a real scene, and there is one now.
