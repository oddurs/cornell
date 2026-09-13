---
id: 83
title: 'smith.hpp: derive the masking function from the distribution'
type: optics
status: backlog
milestone: v0.7
labels:
- thesis
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: bsdf
effort: l
---

## What it is

The best derivation in the project, and the purest expression of house rule 2.

Facets hide each other. The masking-shadowing function G says what fraction of
them are visible from a given direction, and every renderer written for
pleasure picks one from a menu — Smith, Cook-Torrance, implicit, Kelemen — as
though it were a style.

It is not a choice. Heitz showed in 2014 (*Understanding the Masking-Shadowing
Function in Microfacet-Based BRDFs*) that G follows from D by a single
constraint: the microsurface, projected onto the macrosurface direction, must
have the same area as the macrosurface. That one requirement determines
Smith's Lambda uniquely for any given distribution.

## What it must derive

Lambda for Trowbridge-Reitz, from that constraint, in the comment. It is about
a page of algebra and it converts the most arbitrary-looking term in the whole
model into a consequence.

## What is not modelled

Correlation between masking and shadowing. The separable form assumes the two
are independent, which is false at grazing angles; the height-correlated form
is better and is what should be implemented, with the separable one named as
the thing it improves on.

## Acceptance criteria

- [ ] The projection constraint is stated and the derivation follows it
- [ ] No menu of G functions exists in the codebase
- [ ] The white furnace test is what confirms the derivation is right
