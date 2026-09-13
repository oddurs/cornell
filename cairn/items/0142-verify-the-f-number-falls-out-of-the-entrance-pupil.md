---
id: 142
title: 'Verify: the f-number falls out of the entrance pupil'
type: verify
status: backlog
milestone: v1.5
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: verification
effort: s
---

## The claim

The f-number of a modelled lens is not a setting. It is the focal length over
the diameter of the entrance pupil, both of which are consequences of the
prescription.

## How it is checked

Trace paraxial rays to find the focal length and the pupil, compute the ratio,
and compare with the value stamped on the lens in the patent.

## What failure looks like

If they disagree, the prescription was transcribed wrongly or the sign
convention on a radius is inverted — which is the lens equivalent of the
dielectric sign bug and just as easy to make.
