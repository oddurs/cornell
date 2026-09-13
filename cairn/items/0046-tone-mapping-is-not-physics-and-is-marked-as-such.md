---
id: 46
title: Tone mapping is not physics, and is marked as such
type: optics
status: backlog
milestone: v0.3
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: colour
effort: s
---

Radiance is unbounded and a display is not. Everything that happens between
those two facts is a choice about appearance, and none of it belongs in the
renderer.

So: the film writes linear float, always. Tone mapping is a separate pass in a
separate file with a comment saying it is the one part of this project that
has no correct answer. Any figure quoted in the README comes from before this
step.

- [ ] Every number in the README is measured pre-tone-map
- [ ] The default is documented as a choice, with its author named
