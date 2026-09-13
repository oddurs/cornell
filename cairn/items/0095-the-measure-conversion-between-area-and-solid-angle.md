---
id: 95
title: The measure conversion between area and solid angle
type: optics
status: backlog
milestone: v0.8
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: transport
effort: s
---

A density over a light's surface and a density over directions are not the
same number, and combining them in a MIS weight without converting is a bug
that produces a subtly wrong image rather than an obviously wrong one.

    p_solid_angle = p_area * d^2 / cos(theta_light)

Derive it from the Jacobian, in one place, and make every call site use the
named conversion rather than writing the factor inline — because when it is
inline, one of them will be missing a cosine and it will take an afternoon.

- [ ] One function, called everywhere, never reproduced by hand
