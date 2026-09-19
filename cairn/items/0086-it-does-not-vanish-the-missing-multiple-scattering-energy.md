---
id: 86
title: 'It does not vanish: the missing multiple-scattering energy'
type: bug
status: done
milestone: v0.7
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-19
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
and leaves. Anything that hits a second facet is dropped, because the model
has nothing to say about what happens next. At low roughness there is almost
none of it. At high roughness it is most of the energy.

**Refined by measuring it.** This item was written saying the masking term is
what drops the light. `./cornell furnace --table` accounts for every draw in
three exhaustive states, and the masking term is the *smaller* channel at
every roughness except at grazing. At normal incidence and roughness 1, a
fifth of the light is masked on the way out and half of it never points
outward at all — the facet drawn from the visible distribution is tilted far
enough that its mirror direction goes into the ground. Both are the ray
meeting the microsurface again; the second is the bigger one, and it is not
the term usually blamed.

The three channels sum to one to 7.1e-14, which is what makes this a bug in
the model rather than in the program: no light is missing, it is all in
states a single-scattering model refuses to follow.

This is not a bug in the implementation. It is a bug in the model, it is
present in almost every renderer ever shipped, and finding it with an
instrument built before the model is the single best thing this project can
demonstrate.

## Reproduction

1. `./cornell furnace --bsdf conductor --alpha 0.6`
2. Observe a grey square on a white background
3. `./cornell furnace --table` for the deficit against roughness

## 2026-09-19

Diagnosed by accounting rather than by argument. ./cornell furnace --table splits every draw off a reflectance-1 rough conductor into three exhaustive states - escaped, masked on the way out, reflected into the surface - and they sum to one to 7.1e-14. That closure is the whole point: no light is missing, so the deficit is entirely rays that met the microsurface a second time, which is a bug in the model and not in the program. The escaped column agrees to 4.0e-04 with the directional albedo measured by a routine that shares no arithmetic with it.

The item's stated mechanism was wrong and the item body has been corrected. It said the masking term drops the light, which is the usual telling. Measured, masking is the SMALLER channel everywhere except at grazing: at normal incidence and roughness 1, 0.193 is masked on the way out and 0.500 never points outward at all. A rough surface loses light mostly because it is rough enough to reflect into itself.

I also had to correct a claim I wrote one item ago. The 0085 comment said a rough conductor's furnace image is darkest in the middle and brightest at the rim. That holds at roughness 1 - annulus-averaged from the render, 0.308 at the centre rising to 0.589 in the outer tenth, against an analytic 0.307 and 0.88 - and it is false at 0.4, where the profile is nearly flat with a shallow minimum of 0.757 near sixty degrees against 0.786 head on. The angular trend reverses around alpha 0.5, for the same Lambda_o-divergence reason as everything else here. A single row of pixels at 16 spp is mostly noise and cannot tell the two apart; annuli can.

The default --alpha is now 0.6, which is what this item's reproduction asks for and is above the crossover.

CI runs --table. If the closure ever stops holding, the deficit has stopped being multiple scattering and started being a bug.
