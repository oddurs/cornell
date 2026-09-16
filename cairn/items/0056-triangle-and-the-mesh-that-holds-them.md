---
id: 56
title: Triangle, and the mesh that holds them
type: optics
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-15
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

- [x] `geometric_normal()` and `shading_normal()` are separate accessors
- [x] The file says which is used for the offset (geometric, always), and why:
      a ray offset along an interpolated normal can be pushed to the wrong
      side of the surface it is on, which is the bug the whole
      self-intersection machinery exists to prevent

## 2026-09-15

geometric_normal() and shading_normal() are separate, and the header says which consumer wants which and why: the offset in waechter.hpp must use the geometric one, because a ray offset along an interpolated normal can be pushed to the wrong side of the surface it is standing on, which is the bug the whole self-intersection machinery exists to prevent. Verified: on a flat quad the two agree exactly and the shared edge is bit-identical between the two triangles, which is the property item 0055 measured the absence of; on a quad with fanned vertex normals the geometric normal is constant while the shading normal differs from it by 17 to 23 degrees across the face. The mesh's real job here is not memory - the box is 32 triangles - it is that two faces sharing a vertex share it exactly, so a seam is watertight by construction rather than because the same number was typed twice.
