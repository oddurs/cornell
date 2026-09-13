---
id: 47
title: What to do about negative RGB
type: spike
status: backlog
milestone: v0.3
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: colour
effort: s
---

## Question

A saturated spectral colour converts to sRGB values outside [0,1], often
negative. Clipping changes the hue. What should happen?

## Why it has to be answered first

It determines whether the v0.9 prism image is honest or quietly clipped, and
the prism is one of the project's two showpieces.

## Options

Clip per channel; scale toward the white point preserving hue; compress the
gamut smoothly; or output a wider-gamut file and say so.

## What would settle it

Render the prism. Whichever option lets a reader see that the spread is
continuous and spectral, rather than three bands, is the right one.

## Answer

<!-- Filled in when this closes. -->
