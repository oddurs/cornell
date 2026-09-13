---
id: 21
title: Sphere intersection, and the cancellation in the obvious quadratic
type: optics
status: done
milestone: v0.1
assignee: Oddur Sigurdsson
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: geometry
effort: s
---

## What it is

The first thing a ray hits. Also the first place where the algebraically
correct formula and the numerically correct formula differ, which is worth
the half page it takes to explain.

## What it must derive

The stable root. Solving `at² + bt + c = 0` with the quadratic formula loses
most of its significant figures when `b² >> 4ac` — a ray grazing a large
sphere from far away, which is the common case, not the exotic one. The fix
is to compute the well-conditioned root and get the other from `t1·t2 = c/a`.

## Acceptance criteria

- [x] A test that fails with the naive formula and passes with this one
- [x] The comment says what is lost, not what is computed

## 2026-09-13

The fix this item prescribed - well-conditioned root, other from t1*t2=c/a - is the right advice about the textbook quadratic and does not apply to the half-b form. sqrt(disc) <= r always, so the roots can only cancel when the origin is about r from the centre, and in exactly that case c = f.f - r^2 has already lost the same digits. Measured at eps = 1e-6/1e-9/1e-12 above the surface, b-sqrt and c/(b+sqrt) agree to the last digit. The real loss is in the discriminant, not the roots: at 1e8 m, b*b-c evaluates to exactly 0 against a true 0.75. So the file computes the discriminant as r^2 - |f_perp|^2 (Haines, Gunther, Akenine-Moller, Ray Tracing Gems 2019) and never forms c at all.
