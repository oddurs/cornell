---
id: 80
title: 'apps/swatch: the metals, from their tables alone'
type: instrument
status: done
milestone: v0.6
assignee: Oddur Sigurdsson
labels:
- thesis
created: 2026-09-13
updated: 2026-09-17
priority: p2
area: instrument
effort: s
---

## What it witnesses

The thesis, in one image.

Five spheres — gold, copper, silver, aluminium, and a dielectric — lit by a
stated illuminant, rendered from nothing but measured optical constants. No
colour appears in the scene description, because the scene description is a
list of citations.

## What it prints

A PNG-shaped argument. Underneath it, the chromaticity of each, printed, so
the image is accompanied by numbers rather than being the claim itself.

## What it may not do

Have a fallback. If a metal has no table, it is not in this picture.

That is structural rather than a rule somebody remembers: a conductor is a
`Conductor<Optical<N>>`, and the only way to get one is to hand it a table.
There is no default index and no "approximately gold".

## What it prints

Five spheres — gold, copper, silver, aluminium and a dielectric at n = 1.5 —
in the enclosure `furnace.hpp` uses, lit by D65, exposed so that white is the
illuminant. And underneath, the chromaticity, the sRGB, and the reflectance
edge of each, because the image is not the claim: a picture of a yellow sphere
proves nothing, since every renderer ever written can produce one and most of
them do it by typing 0.766.

The glass sphere comes out nearly black, which is correct and worth saying.
`specular.hpp` models reflection and not transmission, so the 96% that enters
the glass does not come back. v0.9 gives the dielectric its other half.

## Acceptance criteria

- [x] No colour appears in the scene description
- [x] The chromaticity of each is printed under the image
- [x] No fallback: a metal without a table is not in the picture
