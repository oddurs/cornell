---
id: 144
title: 'Fluorescence: light that changes wavelength'
type: spike
status: backlog
milestone: later
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: spectrum
effort: l
---

## Question

Should the renderer model re-emission at a different wavelength?

## Why it matters

Almost all white paper contains optical brighteners that absorb ultraviolet
and re-emit blue, which is why it looks brighter than white under daylight and
odd under tungsten. Highlighter pens, safety vests, and white shirts in a
nightclub are all fluorescence.

## What it would cost

The reflectance stops being a function of one wavelength and becomes a matrix
mapping incoming wavelengths to outgoing ones — a Donaldson matrix — and the
hero wavelength machinery from v0.9 has to handle paths whose wavelength
changes mid-flight.

## What would settle it

A scene that needs it. Given that the project has a spectral pipeline
already, this is the cheapest of the four `later` items and the most likely to
happen.
