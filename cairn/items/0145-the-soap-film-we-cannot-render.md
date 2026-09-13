---
id: 145
title: The soap film we cannot render
type: spike
status: backlog
milestone: later
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: bsdf
effort: l
---

## Question

Thin-film interference: the colours in a soap bubble, an oil slick, a beetle's
shell, the anti-reflective coating on every lens in v1.5.

## Why it is here

It is the clearest single example of what geometric optics cannot do. Light
reflecting off the front and back of a film thinner than a wavelength
interferes with itself, and the result depends on the phase, which this
renderer does not carry at all.

## What it would cost

Amplitude and phase instead of intensity, at least at the interface. It is
tractable — production renderers do it — as a special-case BSDF that computes
the interference analytically rather than by making the whole renderer a wave
simulator.

## What would settle it

Whether the special case can be added without the rest of the project
pretending it is more general than it is. An honest thin-film BSDF that says
"this is the only place phase exists" would be acceptable; one that implies
the renderer understands waves would not.
