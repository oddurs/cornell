---
id: 33
title: An orthonormal basis from a single normal, branchless
type: optics
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: sampling
effort: s
---

Duff et al., 2017 — the revised Frisvad construction, branchless and stable at
the pole. Three lines, one `copysign`, and a comment explaining that the
obvious version has a discontinuity at `n.z = -1` that produces a seam you
will spend an evening looking for.

- [x] Tested at the pole specifically
- [x] Handedness stated in the comment, once, and obeyed everywhere

## 2026-09-13

The received story - that Frisvad 2012 degrades in the neighbourhood of the pole, not just at it - is not what happens in double precision. Measured, it sits at one ulp (2.22e-16) all the way from n.z = -1 + 1e-2 to -1 + 1e-16, and then is NaN at exactly -1. The gradual degradation is a single-precision effect; the 2017 paper works in float. So what the branchless version buys here is the exact pole and no branch, not accuracy. Worth having anyway: (0,0,-1) is the floor of an axis-aligned box, and NaN propagates to a black pixel that looks like a shadow. Worst orthonormality error over 320800 directions: 4.79e-16. Also: the first version of the test could not see the failure at all, because std::fmax returns the other operand when one is NaN, so a max-of-errors loop scores a NaN as no error.
