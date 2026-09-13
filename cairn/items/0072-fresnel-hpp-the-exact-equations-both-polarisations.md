---
id: 72
title: 'fresnel.hpp: the exact equations, both polarisations'
type: optics
status: backlog
milestone: v0.6
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: bsdf
effort: m
---

## What it is

The thesis file. Augustin-Jean Fresnel, 1823 — how much light reflects from a
boundary, as a function of angle, index, and polarisation.

The opening comment has the best footnote available to this project: Fresnel
derived these from elastic-ether theory, a physical model in which light is a
mechanical vibration of an invisible solid filling the universe, and which is
entirely wrong. He got the right answer forty years before Maxwell explained
why it is the right answer. Being wrong about the mechanism and right about
the boundary conditions is a thing that happens, and it is worth a reader
knowing it.

## What it must derive

Everything visible. Specifically, all of the following must fall out of these
equations and appear nowhere as a parameter:

- that every material becomes a mirror at grazing incidence
- the sheen on a page held up to a window
- why a wet road is darker and shinier than a dry one
- Brewster's angle, at which reflected light is fully polarised
- the colour of every metal in the project

## What is not modelled

Polarisation, which is the irony of the file: Fresnel's equations are stated
per polarisation and this renderer averages the two because it carries scalar
radiance. That is correct for unpolarised illumination and wrong for light
that has already reflected once at a steep angle. Named, quantified if
possible, and left out.

## Acceptance criteria

- [ ] The dielectric and conductor cases are the same function, differing only
      in whether the index is complex
- [ ] Total internal reflection falls out rather than being branched on
- [ ] The comment derives Brewster's angle from the equations above it
