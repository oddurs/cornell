---
id: 157
title: There is no mesh, and the reason is measurable
type: chore
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
created: 2026-09-15
updated: 2026-09-15
priority: p1
area: geometry
effort: s
---

The last finding from the v0.4 review, and the only one nobody else raised:
`mesh.hpp` was 117 lines that nothing included. Not `cornell.hpp`, not
`scene.hpp`, not `apps/verify.hpp`, not a test. The only occurrences of the
word "mesh" anywhere else in the repository were in prose about meshes.

A header nobody includes is not a part of this project, because the project
is the reading order. It is a chapter the book does not contain.

## The argument it made, and whether it survives

`mesh.hpp` gave two reasons to index triangles and dismissed the first
itself: sharing a vertex between six faces saves memory, and for 32 vertices
it saves a few hundred bytes and costs an indirection per intersection.

The second was the real one, and it was good:

> a mesh is where a *surface* is described rather than a pile of unrelated
> faces. Two triangles that share vertices share them exactly ... which is the
> difference between a seam that is watertight by construction and one that is
> watertight because the numbers happened to be typed identically twice.

That is true in general. It is measurably not load-bearing here. `cornell.hpp`
emits 114 vertices across its 38 triangles, and they fall on:

      32 distinct positions, within 1e-9 m
      32 distinct positions, bit for bit

The two counts being equal is the whole result. There is no pair of corners in
the box that is nearly-but-not-exactly shared. The seams are already exact,
and they are exact for the reason `mesh.hpp` called a coincidence: a corner is
written `at(130, 165, 65, scale)` wherever it appears, the same arithmetic
runs on the same literals, and IEEE 754 is deterministic.

So indexing would convert an exactness that holds textually into one that
holds structurally. That is a real improvement in kind — but it is not an
improvement in the image, and it has a price: `at(290, 165, 114)` can be
checked against Cornell's published table by eye, and `{4, 7, 6}` cannot.
Given a file whose entire job is to be checkable against a 1984 measurement,
that trade goes the other way.

Deleted. The argument moves into `cornell.hpp` next to the geometry it is
about, with the measurement in it, so that the first model this project cannot
transcribe by hand finds the reasoning waiting.

## Two other deaths, while here

`Bvh::order()` returned the permutation and had no callers — `traverse` hands
the caller its own indices back, so the sorted order never leaves the tree.
The comment above `build` said it "returns the order to visit them in", which
had been false since the flat-node layout landed.

And `Triangle::na/nb/nc` are now set by nothing at all, `mesh.hpp` having been
the only thing that ever set them. The branch stays, because the geometric /
shading normal distinction is one of the things `triangle.hpp` exists to
explain, and an explanation is worth more where it applies than in the
abstract. But it now says so out loud rather than implying a smooth surface
somewhere in the repository.

## Acceptance criteria

- [x] No header in `include/render/` is included by nothing
- [x] No public member function in `include/render/` has no caller
- [x] The seam claim is measured, not asserted
- [x] `make strict` passes and `./cornell verify` holds
