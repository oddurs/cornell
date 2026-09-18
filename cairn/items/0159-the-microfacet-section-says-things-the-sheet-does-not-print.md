---
id: 159
title: The microfacet section says things the sheet does not print
type: bug
status: done
milestone: v0.7
assignee: Oddur Sigurdsson
created: 2026-09-18
updated: 2026-09-18
priority: p1
area: verification
effort: s
---

## What the image does

Nothing. This is a defect in the inspection sheet, which is worse, because
the sheet is the thing that catches defects in everything else.

Item 0082 landed `trowbridge_reitz.hpp` with a verify section, and a review of
the merged commit found four things in it:

1. The comment introducing the section quotes `∫ D dm` as "20 at a tight
   lobe, 2 at the widest". The sheet prints 1.0095 at the tight lobe. The 20
   was a guess written before the check was run, corrected in the header next
   door and not here — house rule 6, in the one file whose whole job is to
   stop figures rotting. The same comment calls the unprojected integral "the
   second column" and it is printed third.

2. The row labelled "The calibration" is not one. It builds
   `TrowbridgeReitz{1.0}` — a correct distribution — and re-runs the
   alpha = 1 unprojected integral, so it prints `caught` whatever `d()` does:
   double the constant and it is caught, return zero unconditionally and it
   is caught. Every other calibration in this file instantiates a
   deliberately broken model. This one instantiates a working one.

3. The bit-pattern comparison at alpha = 1 passes because `sin` and `cos`
   happen to return components whose squares sum to exactly 1.0 for the four
   directions chosen. That is a fact about libm's last ulp rather than about
   the model, and CI fails the build on a non-zero exit from `verify`.

4. `d()` promises that a caller who forgot the delta gets zero rather than "a
   NaN that spreads". Below alpha ~1e-77 it returns `inf` at the normal, and
   below ~1e-155 it returns NaN, because `alpha²` underflows.

## What the physics says it should do

Unchanged. The distribution and its normalisation were checked and are
right; what is wrong is what the sheet says about them.

## Reproduction

1. `./cornell verify`, and read the source of the section beside its output.
2. Mutate `d()` to return `0.0` unconditionally. The calibration row still
   says `caught`.

## Acceptance criteria

- [x] Every figure in the section's prose is one the section prints
- [x] The calibration row passes only when it catches a model that is wrong,
      and the liar it uses is normalised against the wrong measure on purpose
- [x] The exactness claim states the precondition it rests on
- [x] `d()` returns what its comment promises for every finite alpha

## 2026-09-18

Mutation-tested after the rewrite: return 0.0 and a doubled constant are both caught by five rows each, and the h/2 ratio drops to 1.00 for a wrong constant, which is the second-order signature the header claims. The liar is now MisMeasured — the alpha=1 distribution at 1/(2pi), normalised so that the unprojected integral is one — and it covers exactly half the surface, so the row prints -5.000e-01. Underflow thresholds measured rather than guessed: 1/0 at alpha 1e-81, 0/0 at 1e-162.
