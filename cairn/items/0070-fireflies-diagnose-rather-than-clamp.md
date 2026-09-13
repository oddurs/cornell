---
id: 70
title: 'Fireflies: diagnose rather than clamp'
type: spike
status: backlog
milestone: v0.5
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: transport
effort: m
---

## Question

What does this project do about fireflies — single pixels left enormously
bright by a rare high-energy path?

## Why it has to be answered first

The universal answer is to clamp indirect radiance, which works, looks fine,
and is *biased*: it silently removes energy, it breaks the convergence slope
that v0.5 measures, and it is exactly the kind of quiet dishonesty this
project exists to avoid.

## Options

Clamp and admit it, breaking the unbiasedness claim. Sample better, so the
paths that cause them are found more often — which is what multiple importance
sampling in v0.8 is for, and which fixes the cause. Or report them: a
diagnostic that says which path produced the outlier.

## What would settle it

If the fireflies that remain after v0.8 come from a single identifiable
mechanism, fix the mechanism. If clamping is still wanted afterwards it goes
in the tone mapping pass, where non-physical things are already admitted to
live, and never in the integrator.

## Answer

<!-- Filled in when this closes. -->
