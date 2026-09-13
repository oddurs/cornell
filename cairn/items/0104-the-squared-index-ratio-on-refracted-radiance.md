---
id: 104
title: The squared index ratio on refracted radiance
type: optics
status: backlog
milestone: v0.9
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: transport
effort: s
---

Radiance is not conserved across a refractive boundary. A beam entering a
denser medium is compressed in solid angle, so its radiance rises by the
square of the index ratio, and the transmitted BSDF carries a factor of
eta squared.

This is also the honest exception to the reciprocity check written in v0.5,
and the two items must know about each other: the test is not relaxed, it is
taught the physics.

Derive it from the conservation of *flux* through the compressed solid angle.
Four lines, and it turns the most suspicious-looking factor in any renderer
into an obvious one.

- [ ] The derivation is in the comment
- [ ] The v0.5 reciprocity test gains a documented exception, not a waiver
