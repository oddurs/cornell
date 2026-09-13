---
id: 51
title: Find out exactly what Cornell published, and what units it is in
type: spike
status: backlog
milestone: v0.4
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: scene
effort: m
---

## Question

What measured data actually exists for the Cornell box, in what units, and
what was measured versus what has been passed down by repetition?

## Why it has to be answered first

The entire premise of naming the project `cornell` is that the box is a real
object with published measurements. If the geometry in circulation is a
reconstruction and the reflectances are somebody's guess, the project needs to
know that before it builds a validation milestone on top of it.

Note against temptation: the figure "2.7 metres on a side" was floated during
the naming of this repository and is an invention. It is recorded here so that
nobody finds it in the history and believes it.

## Options

The Cornell Program of Computer Graphics published geometry, spectral
reflectances for the walls, and the emission spectrum of the light, along with
photographs and radiometric measurements. Establish which of those are
primary, and read the 1984 paper — Goral, Torrance, Greenberg and Battaile,
*Modeling the Interaction of Light Between Diffuse Surfaces* — rather than a
summary of it.

## What would settle it

A written note in this item recording, for each figure the project will use:
the source, the units, and whether it was measured or chosen. That note
becomes the opening comment of `cornell.hpp`.

## Answer

<!-- Filled in when this closes. Nothing in cornell.hpp may be written until
     it is. -->
