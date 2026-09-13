---
id: 90
title: 'Prose: what a statistical surface is'
type: prose
status: backlog
milestone: v0.7
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: prose
effort: m
---

## What it has to explain

That a microfacet BRDF is not a description of a surface — it is a description
of the *distribution* of a surface, and everything that follows is a question
about statistics rather than geometry.

Which is why the model can lose energy without being wrong about any
individual facet, why the masking term is not a choice, and why a rough
surface looks the way it does at grazing angles.

## What it must not do

Use the phrase "microfacet theory" without saying what is being assumed:
that the facets are much larger than a wavelength and much smaller than a
pixel. Both ends of that sandwich are places the model breaks, and a reader
should know where they are.
