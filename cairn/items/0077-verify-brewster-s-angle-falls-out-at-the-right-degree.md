---
id: 77
title: 'Verify: Brewster''s angle falls out at the right degree'
type: verify
status: backlog
milestone: v0.6
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: verification
effort: s
---

## The claim

At one specific angle, light polarised in the plane of incidence is not
reflected at all. For an air-glass boundary at n = 1.5 that angle is
arctan(1.5), about 56.3 degrees, and it is the reason polarising sunglasses
work on a wet road.

## How it is checked

Evaluate the parallel-polarisation Fresnel term across incidence angle, find
the minimum numerically, and compare with arctan(n2/n1).

## What failure looks like

Nothing, in any image, because the renderer averages the polarisations. This
check exists because the *equations* are the thing being verified, not the
image — and a project that only tests what shows up in the picture is testing
its own tolerance for error.
