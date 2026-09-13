---
id: 134
title: Light path notation, and the paths we cannot find
type: prose
status: backlog
milestone: v1.4
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: prose
effort: m
---

Heckbert's notation — LDSE and the rest — is the compact way to say which
paths an algorithm can and cannot construct, and this project should adopt it
at the point where the answer stops being "all of them".

The specific villain is LS*DS*E: a caustic seen in a mirror, or through glass.
Say which algorithm handles which class, in a table, and then let the
subsequent items fill the gaps.

- [ ] A table of path classes against algorithms, in the README
- [ ] The classes unidirectional path tracing cannot find are named
