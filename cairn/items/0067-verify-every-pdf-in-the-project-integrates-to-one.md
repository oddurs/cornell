---
id: 67
title: 'Verify: every pdf in the project integrates to one'
type: verify
status: backlog
milestone: v0.5
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: verification
effort: m
---

## The claim

Every density function the project defines is a density function.

## How it is checked

Numerically integrate each `pdf()` over its domain — the sphere, the
hemisphere, the surface of a light — by a quadrature independent of the
sampling routine it belongs to.

## What failure looks like

An unnormalised pdf produces an image that is uniformly too bright or too dark
by a constant factor, which is exactly the kind of error that gets
compensated for by adjusting the light's intensity until it looks right, and
then never found.
