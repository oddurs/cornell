---
id: 66
title: 'Verify: Lambert vanishes in the furnace'
type: verify
status: backlog
milestone: v0.5
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: verification
effort: s
---

## The claim

A Lambertian surface with reflectance 1, in a uniform environment of radiance
1, returns exactly radiance 1 in every direction.

## How it is checked

`./cornell furnace --bsdf lambert`, residual below 1e-6.

## What failure looks like

Too dark by a factor of pi: the normalisation was dropped. Too dark by a small
amount: the cosine is being applied twice, or once in the sampler and once in
the estimator. Too bright: the pdf is wrong.

Every one of those is invisible in a rendered image and obvious here.
