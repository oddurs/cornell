---
id: 143
title: 'Polarisation: what it would cost, what it would buy'
type: spike
status: backlog
milestone: later
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: transport
effort: xl
---

## Question

Should this renderer carry polarisation?

## What it would cost

Four Stokes parameters per wavelength instead of one scalar, a 4x4 Mueller
matrix at every interaction instead of a scalar reflectance, a rotation into
and out of a reference frame at every bounce, and roughly four times the state
on every path. Fresnel already computes the two polarisations separately and
throws the distinction away, so the physics is half-present already.

## What it would buy

Correct glare off water and glass at steep angles. The polarisation pattern of
the sky, which bees navigate by. Stress birefringence in plastic, which is the
only way to render the coloured fringes in a clear plastic ruler between two
filters.

## What would settle it

Whether any image the project wants to make requires it. Currently: no. Kept
as an open question rather than a silence, per house rule 7.

## What item 0081 measured, which sharpens the question

v0.6 quantified the error rather than leaving it as "progressively wrong".
Unpolarised light, two bounces off glass, tracking the two states through both
and averaging once, against this renderer's averaging after each:

      first    second      tracked      averaged     ratio
       0.00      0.00     0.00160000   0.00160000    1.0000
      45.00     45.00     0.00426907   0.00252405    0.5912
      56.31     56.31     0.01094149   0.00547075    0.5000
      70.00     70.00     0.04578120   0.02925555    0.6390
      85.00     85.00     0.38981461   0.37552341    0.9633

**Exactly one half** at two bounces at Brewster's angle, which is the worst
case in that plane, and **exactly right** at one bounce — the error is zero at
first contact rather than merely small.

Turn the second surface ninety degrees and it stops being a factor at all. Two
crossed polarisers: the tracked answer is zero and this renderer predicts
0.00547. The relative error is unbounded, and the thing the model cannot
produce is the *absence* of a reflection.

## Which does not settle it

Because the Cornell box cannot exhibit any of it. Its light is unpolarised,
its walls are matte, and the error at one bounce off a matte surface is zero.
The factor of two needs two smooth surfaces at 56 degrees, and there are none
in this scene or in any scene on the roadmap before v1.5's real lenses.

So the answer is still no, and it is now a no with a number attached rather
than a no with a shrug. What would change it: a scene with two smooth
reflections in it, or the v1.2 sky, where Rayleigh scattering is the textbook
polariser and the pattern bees navigate by is a thing the model would be
actively failing to produce rather than merely not producing.

Both figures are checked by `./cornell verify`, so this item's numbers cannot
rot while the question stays open.
