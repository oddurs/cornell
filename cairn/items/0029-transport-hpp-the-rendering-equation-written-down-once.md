---
id: 29
title: 'transport.hpp: the rendering equation, written down once'
type: optics
status: backlog
milestone: v0.2
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: transport
effort: m
---

## What it is

Kajiya, 1986. The crank-slider of this project — one equation that everything
else is decoration on top of:

    L(x, wo) = Le(x, wo) + ∫ f(x, wi, wo) L(x, wi) (n·wi) dwi

Every feature this renderer will ever have is a consequence of solving that
integral honestly, and the file's opening prose says so, because it is the
claim the whole repository is making.

## What it must derive

That the list of things with no code — soft shadows, colour bleeding,
caustics, ambient occlusion, glossy reflection — is a consequence rather than
a boast. The file should name them and say that none of them appears anywhere
in the project, because a reader will not believe it otherwise.

## What is not modelled

The equation as written assumes light travels unchanged between surfaces
(no participating media, deferred to v1.1), arrives and leaves at the same
point (no subsurface transport, v1.3), at the same time (no fluorescence,
never), and at the same wavelength.

## Acceptance criteria

- [ ] The integral appears in the header, in the notation the 1986 paper used
- [ ] The file states which term each subsequent header is responsible for
- [ ] `grep -ri "soft_shadow\|ambient_occlusion\|fake" include/` is empty, and
      stays empty
