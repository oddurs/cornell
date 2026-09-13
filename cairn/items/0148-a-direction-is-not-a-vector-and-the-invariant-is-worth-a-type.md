---
id: 148
title: A direction is not a vector, and the invariant is worth a type
type: optics
status: done
milestone: v0.1
assignee: Oddur Sigurdsson
labels:
- foundation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: geometry
effort: s
---

## What it is

Three doubles, and the one distinction that earns a second type. The
backlog was written without this item, which is how it went unnoticed that
`ray.hpp` cannot be written until something exists for the origin and the
direction to be.

`Vec3` is free: add it, scale it, cross it, take its length. `Unit` is a
`Vec3` that is known to have length one, and the only way to get one is to
normalise something or to name an axis. Every cosine in the project is a
`dot` of two of them, and a direction that quietly stopped being unit length
is a cosine that is quietly wrong — in a term that is multiplied into the
estimate at every bounce, so the error compounds rather than shows.

It is `spectrum.hpp`'s argument about `Radiance` and `Reflectance`, applied
to the other quantity this renderer cannot afford to confuse.

## What it must derive

Nothing. This is arithmetic, not physics, which is the honest reason it is
listed with the optics rather than as a chore: it is the vocabulary the
optics is written in, the way `si.hpp` is.

## What is not modelled

The affine distinction. A point and a displacement are not the same kind of
thing — a point plus a point is meaningless, and a point under a
transformation picks up the translation where a direction does not — and
plenty of renderers carry `Point3` and `Vec3` separately to say so.

This one does not, and the reason is that the distinction pays only once
there are transformations, which arrive with the triangle meshes in v0.4.
One invariant per file is enough to be read; two is a type system nobody
will use. If v0.4 wants it, it is a change to this file rather than a
discovery.

Normals are also plain `Unit` here. A normal transforms by the inverse
transpose rather than by the matrix, and the day a matrix exists is the day
that becomes a bug rather than a footnote. Noted so that it is a decision.

## Acceptance criteria

- [x] A `Unit` cannot be constructed from arbitrary components without
      passing through `normalize`, and the compiler says so
- [x] `Unit` converts to `Vec3` implicitly and costs nothing: the generated
      code for a `dot` of two units is three multiplies and two adds
- [x] The header says why there is no `Point3`

## 2026-09-13

dot(Unit, Unit) on arm64 -O2: one fmul and two fmadd, with no trace of the wrapper. The invariant costs nothing, which was the claim.

## 2026-09-13

Unit::known is consteval, so the negative test is a compile error rather than a convention: passing runtime doubles gives 'call to consteval function is not a constant expression', and Unit{Vec3{...}} gives 'calling a private constructor'.
