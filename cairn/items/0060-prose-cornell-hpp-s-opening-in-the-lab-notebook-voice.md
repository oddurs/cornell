---
id: 60
title: 'Prose: cornell.hpp''s opening, in the lab-notebook voice'
type: prose
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-15
priority: p2
area: prose
effort: m
---

## What it has to explain

That this is a real object. Who built it, when, why, what they measured it
with, and what they were arguing about at the time — which was whether a
radiosity solution could be believed, and the answer they gave was to
photograph the thing and put the photograph next to the render.

That comparison is the project's own final milestone, forty years later, with
a different algorithm, and the header should say so.

## What it must not do

Describe the geometry. The geometry is in the code beneath it.

## 2026-09-15

The opening explains who built it and what they were arguing about - that radiosity produced pictures nobody had reason to believe, and that the form factor was a geometric abstraction with no obvious way to check it, so they built the thing and printed the photograph beside the render. It names the debt this project owes: the same comparison, forty years later, by a method that did not exist in usable form until Kajiya two years afterwards. It is precise about which photograph, since 0051 found there are two eras. And it lists in advance what could be wrong if v1.0 fails - the four-point lamp spectrum, the unpublished absolute scale, the Lambertian assumption, geometric optics - because a prediction made before a measurement is worth more than an explanation after it. No geometry in the opening; that is in the code beneath.
