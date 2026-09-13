---
id: 39
title: 'An area light: a triangle that emits'
type: optics
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: light
effort: s
---

The simplest emitter that produces a soft shadow, and the point at which the
project's second thesis becomes checkable: nobody writes a soft shadow, the
penumbra is the light's solid angle varying across the floor.

At v0.2 it is found only by paths that happen to hit it. Sampling it directly
is v0.8, and the difference between the two images at equal sample count is
the best advertisement multiple importance sampling has.

- [ ] One-sided by default, and the comment says why two-sided emitters are a
      modelling convenience rather than a physical object

## 2026-09-13

One-sided, verified both ways: a camera in front of the +z face sees 2.0 and one behind it sees 0.0. The first version of that test had two spheres between the camera and the emitter and was measuring the spheres. Triangle is Moller-Trumbore with no epsilon - the parallel test is det == 0.0 exactly, because a small determinant is a grazing hit and a tolerance would throw it away and crack the shared edge of two triangles. Grazing rays at 1e-6 down to 1e-15 radians all hit. Variant dispatch is bit-identical to calling the model directly.
