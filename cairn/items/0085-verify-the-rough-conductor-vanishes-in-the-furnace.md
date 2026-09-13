---
id: 85
title: 'Verify: the rough conductor vanishes in the furnace'
type: verify
status: backlog
milestone: v0.7
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
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
