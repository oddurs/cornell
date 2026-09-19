---
id: 85
title: 'Verify: the rough conductor vanishes in the furnace'
type: verify
status: done
milestone: v0.7
assignee: Oddur Sigurdsson
labels:
- thesis
created: 2026-09-13
updated: 2026-09-19
priority: p0
area: verification
effort: s
---

## The claim

A rough metal with reflectance 1, in a uniform environment, returns radiance 1
in every direction and disappears.

## How it is checked

`./cornell furnace --bsdf conductor --alpha 0.0 .. 1.0`, residual per
roughness, tabulated.

## What failure looks like

It will fail. That is expected, it is the next item, and this check is what
makes the failure visible rather than aesthetic. The object will be there —
darker than the background, increasingly so with roughness — and the residual
will be a number rather than an impression.

The specific signature matters: uniformly too dark means energy lost to
multiple scattering; a dark rim only at grazing angles means the masking term
is wrong; too bright means the normalisation is.

## 2026-09-19

Measured. Mean radiance off a reflectance-1 rough conductor in the furnace: 0.999998 at alpha 0.001, 0.9913 at 0.05, 0.9152 at 0.2, 0.6318 at 0.6, 0.4111 at 1.0. It vanishes at the smooth end and loses three fifths of the light at the rough end.

On the signature the item predicted in advance, which is worth correcting rather than quietly passing. The item said a dark rim at grazing means the masking term is wrong. Measured, at low roughness the deficit IS almost entirely at grazing, and the masking term is right - 0083 checked it against the covering requirement it was derived from. The heuristic is too crude. What actually explains every entry in the table is the estimator identity from 0084: one draw weighs G2/G1 = (1+Lambda_o)/(1+Lambda_o+Lambda_i), so the deficit is one minus the mean of that. At low roughness both Lambdas vanish except near grazing, so the loss appears at the rim first. At high roughness Lambda_o diverges at grazing and the ratio tends to one, so the loss is LARGEST head-on and smallest at the rim - the opposite of the predicted rim, and not a bug. A rough conductor in a furnace is dark in the middle and bright at the edge.

Residual gained a mean. It reported the darkest single sample, which for any BSDF that can return a black sample is 0.0 whatever the model does - a microfacet surface draws facets reflecting into the ground and those paths correctly carry nothing. Lambert never does, so the minimum had been the mean and the maximum all at once. The furnace asks how much light comes back, which is an average.

An inversion worth noting: the quadrature column, which furnace.hpp calls the strong form because it touches neither sample nor pdf, is the one that fails at low roughness - 0.005 instead of 1 at alpha 0.001, because a 512x512 grid cannot see a lobe two parts in a million of mu. Same wall chi2 hit. The estimator has no such problem because it draws from the lobe. Two instruments, each blind where the other sees, and their agreement from alpha 0.2 up (3.4e-04) is what licenses believing either.

What this asserts today, since the energy loss is 0086's: nothing brighter than the light put in at any roughness or angle, the smooth end losing nothing, and eval/sample/pdf agreeing where all three can see. CI runs it. README carries the table.
