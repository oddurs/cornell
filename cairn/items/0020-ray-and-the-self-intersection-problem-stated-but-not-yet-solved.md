---
id: 20
title: Ray, and the self-intersection problem stated but not yet solved
type: optics
status: done
milestone: v0.1
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: geometry
effort: s
---

## What it is

An origin, a direction, and an interval. Three fields and a paragraph
explaining that the interval is the entire reason this type exists rather
than being two loose vectors.

## What is not modelled

Ray differentials. They are how a renderer knows what texture mip level to
use, they are genuinely useful, and there are no textures until much later.
Noted here so that adding them later is a decision rather than a surprise.

## Acceptance criteria

- [x] The self-intersection problem is described in the header, with the
      solution deferred by name to the Wächter offset item in v0.2

## 2026-09-13

The epsilon ships as 1e-4 m and the header says it is the author's scene written down rather than a tolerance. v0.2 item 0038 replaces it.
