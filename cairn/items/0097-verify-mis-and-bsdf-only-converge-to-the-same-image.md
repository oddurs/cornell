---
id: 97
title: 'Verify: MIS and BSDF-only converge to the same image'
type: verify
status: backlog
milestone: v0.8
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: verification
effort: m
---

## The claim

Multiple importance sampling changes the variance of the estimator and not its
expectation. The image it converges to is the same image.

## How it is checked

Render the box to very high sample count with BSDF sampling alone, with light
sampling alone, and with MIS. Compare the three, pixel by pixel, within the
noise floor each has at that count.

## What failure looks like

A MIS weight that does not sum to one over the strategies produces an image
that is wrong by a few percent in exactly the regions where both strategies
contribute — which is the soft shadow, which is the most looked-at part of the
picture and the least likely place anyone will notice a small error.
