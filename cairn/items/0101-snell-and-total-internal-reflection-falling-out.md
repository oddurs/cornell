---
id: 101
title: Snell, and total internal reflection falling out
type: optics
status: backlog
milestone: v0.9
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: bsdf
effort: s
---

Refraction, and the case where there is none.

Total internal reflection should not be an `if`. Solve for the transmitted
direction, find the discriminant is negative, and reflect — the branch is the
arithmetic telling you something rather than the programmer anticipating it.
The comment should say that, because it is a small example of the whole
project's argument.

- [ ] The critical angle is derived in the comment, not quoted
- [ ] Verified against Brewster's angle's neighbour: at n=1.5 the critical
      angle from inside is about 41.8 degrees
