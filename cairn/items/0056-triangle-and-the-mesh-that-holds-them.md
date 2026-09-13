---
id: 56
title: Triangle, and the mesh that holds them
type: optics
status: backlog
milestone: v0.4
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: geometry
effort: m
---

Triangles are the core type and everything else is a special case. Indexed
positions, optional shading normals, and a clear separation between the
geometric normal — which is what offsets and the cosine use — and the shading
normal, which is an interpolated lie for the benefit of appearance.

Conflating the two is a classic source of energy loss at silhouettes, and the
comment should say which one each consumer wants.

- [ ] `geometric_normal()` and `shading_normal()` are separate accessors
- [ ] The file says which is used for the offset (geometric, always)
