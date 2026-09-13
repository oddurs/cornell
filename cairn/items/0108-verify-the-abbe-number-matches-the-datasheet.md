---
id: 108
title: 'Verify: the Abbe number matches the datasheet'
type: verify
status: backlog
milestone: v0.9
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: verification
effort: s
---

## The claim

The dispersion this project computes for a named glass is the dispersion its
manufacturer measured.

## How it is checked

Evaluate the Sellmeier fit at the d, F and C spectral lines, form the Abbe
number, and compare with the published value for that glass.

## What failure looks like

A transcription error in a coefficient, or a units error — Sellmeier
coefficients are conventionally quoted with wavelength in micrometres, and
this project is in metres, so there is exactly one conversion and it is
exactly the kind that produces a plausible wrong answer.
