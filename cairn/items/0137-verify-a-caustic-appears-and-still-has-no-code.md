---
id: 137
title: 'Verify: a caustic appears, and still has no code'
type: verify
status: backlog
milestone: v1.4
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: verification
effort: m
---

## The claim

The second thesis, at its hardest test. A glass of water on a table produces a
bright caustic, and nothing in the project is named for it.

## How it is checked

Render it, and grep the source for anything that mentions caustics outside
comments explaining that there is nothing.

## What failure looks like

A special case. If the caustic requires a code path that exists only to make
caustics happen, the claim in `transport.hpp` becomes false and must be
rewritten rather than defended.

## Acceptance criteria

- [ ] The photon mapper is a general density estimator, not a caustic feature
- [ ] The image is compared with the bidirectional result where both converge
