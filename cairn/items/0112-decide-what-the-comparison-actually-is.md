---
id: 112
title: Decide what the comparison actually is
type: spike
status: backlog
milestone: v1.0
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: verification
effort: m
---

## Question

Against exactly what measurement is this renderer compared, and what does
agreement mean?

## Why it has to be answered first

"Compared against the photograph" is the name of the milestone and the claim
of the project. If it degenerates into "they look similar", the whole thing
collapses into the category of work it was written to be unlike.

## Options

A photograph is the weakest option: it has been through a lens, a sensor, a
white balance and a JPEG encoder, and matching one proves very little.
Radiometric measurements at specified points in the box are much stronger.
Comparison against an independently written renderer is stronger still on
correctness and says nothing about reality.

## What would settle it

Do all three, rank them by what each proves, and state in the README which
claim rests on which. A number against a radiometer beats a picture that looks
right, and saying so plainly is the point.

## Answer

<!-- Filled in when this closes. -->
