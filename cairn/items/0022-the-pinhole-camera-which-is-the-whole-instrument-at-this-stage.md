---
id: 22
title: The pinhole camera, which is the whole instrument at this stage
type: optics
status: done
milestone: v0.1
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: camera
effort: s
---

## What it is

A dark room with a hole in it. Generate a ray per pixel through an aperture
of zero size, which is the only camera that has no aberrations because it is
the only one with no glass in it.

## What is not modelled

Everything a real lens does. v1.5 is entirely about that, and this file should
say so, so that a reader who wants depth of field knows it is coming rather
than missing.

## Acceptance criteria

- [x] Field of view is derived from the film size and focal length, never
      given directly

## 2026-09-13

Field of view is three derived accessors and no stored field. Checked against itself: the ray through the image corner makes 23.396502 deg with the viewing axis, and half the derived diagonal fov is 23.396502 deg, agreeing to better than 1e-12 rad. The distance is called film_distance, not focal length - there is no lens, so there is no focus.
