---
id: 23
title: 'Film: an array of spectral accumulators, and no colour anywhere'
type: optics
status: planned
milestone: v0.1
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: camera
effort: s
---

## What it is

Where samples land. It accumulates spectral radiance and a sample count, and
it knows nothing about pixels, gamma, or RGB — the conversion happens once, at
the moment of writing a file, in the one place allowed to do it.

## Acceptance criteria

- [ ] The film can be written out before `cie.hpp` exists, as a false-colour
      or single-wavelength image, and the project still produces a picture
