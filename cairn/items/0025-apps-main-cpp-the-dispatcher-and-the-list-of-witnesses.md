---
id: 25
title: 'apps/main.cpp: the dispatcher, and the list of witnesses'
type: instrument
status: done
milestone: v0.1
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: instrument
effort: s
---

## What it witnesses

The shape of the project. `./cornell` with no arguments prints what the
instruments are, which is the fastest way for a reader to understand what has
been built.

## What it may not do

Contain any physics. It is an argument parser.

## 2026-09-13

Two bugs found by looking at the first image rather than by reasoning about it. (1) A missed ray deposited nothing instead of zero, so the film's count never saw it and an edge pixel averaged only the samples that hit: the image had exactly two values, black and white, with no antialiasing anywhere. A path that found no emitter carries zero radiance, which is a measurement. (2) 400x300 pixels over 3:2 film gave non-square pixels and the sphere rendered as an ellipse 287 wide by 323 tall, clipped by a frame too short for it. The height is now derived from the width and the film's shape, and the disc measures 225 x 226 against a predicted 225.4.
