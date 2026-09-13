---
id: 62
title: 'apps/furnace: the white furnace test'
type: instrument
status: backlog
milestone: v0.5
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: instrument
effort: m
---

## What it witnesses

Energy conservation, visibly, as a pass/fail a human can see.

Put an object in an environment of uniform radiance 1 and give it a
reflectance of 1. If the BSDF conserves energy and the sampling is unbiased,
every direction returns exactly 1 and **the object disappears** — it renders
the same white as the background it sits in, and the image is a blank square.

This is the best test in rendering. It requires no reference image, no
tolerance chosen after the fact, and no judgement: either the object is there,
or it is not.

## What it prints

A blank white square, and the residual as a number:

    $ ./cornell furnace --bsdf lambert
    lambert          rho=1.0    residual 0.0000000   vanished

## What it may not do

Special-case anything. The furnace is the ordinary integrator pointed at an
ordinary scene; if it needed its own code path it would be testing that code
path.

## Acceptance criteria

- [ ] Built before the BSDFs it will judge — v0.7 is where it starts failing
- [ ] Residual quoted in the README for every BSDF in the project
