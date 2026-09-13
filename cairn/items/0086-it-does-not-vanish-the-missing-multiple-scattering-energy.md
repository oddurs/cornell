---
id: 86
title: 'It does not vanish: the missing multiple-scattering energy'
type: bug
status: backlog
milestone: v0.7
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: bsdf
effort: l
---

## What the image does

A rough conductor with reflectance 1 renders visibly darker than the furnace
around it, and the deficit grows with roughness — a few percent at alpha 0.2,
a large fraction by alpha 0.8.

## What the physics says it should do

Vanish. Reflectance 1 means no energy is absorbed.

## Why

The single-scattering microfacet model accounts for light that hits one facet
and leaves. Light that hits a facet, bounces to a second facet, and leaves is
counted by the masking term as *lost* — it is shadowed, so it is dropped. At
low roughness there is almost none of it. At high roughness it is most of the
energy.

This is not a bug in the implementation. It is a bug in the model, it is
present in almost every renderer ever shipped, and finding it with an
instrument built before the model is the single best thing this project can
demonstrate.

## Reproduction

1. `./cornell furnace --bsdf conductor --alpha 0.6`
2. Observe a grey square on a white background
3. `./cornell furnace --table` for the deficit against roughness
