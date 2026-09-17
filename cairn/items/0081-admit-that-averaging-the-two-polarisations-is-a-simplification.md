---
id: 81
title: Admit that averaging the two polarisations is a simplification
type: prose
status: done
milestone: v0.6
assignee: Oddur Sigurdsson
labels:
- admission
created: 2026-09-13
updated: 2026-09-16
priority: p2
area: prose
effort: s
---

Fresnel's equations give two reflectances, one per polarisation state. This
renderer carries scalar radiance, so it averages them, which is exactly right
for unpolarised light arriving at a surface for the first time and
progressively less right for light that has already reflected.

What it costs: glare off water and glass at steep angles is wrong in a way
that a polarising filter would reveal; the sky's polarisation pattern, which
bees navigate by, cannot be represented at all.

What it would cost to fix: four Stokes parameters per wavelength, a Mueller
matrix at every interaction, and roughly four times the state on every path.

- [x] Stated in fresnel.hpp, with both costs named
- [x] An item exists under `later` for doing it properly

Item **0143**, which already existed and was already under `later` — this item
first filed a second one saying the same thing, which is the duplication the
roadmap exists to prevent. 0143 now carries the measurement, and the honest
half with it: the box's light is unpolarised, its walls are matte, and the
error at one bounce is exactly zero.

## The size of it, which turned out to be exact

"Quantified if possible" was the item's hedge. It is possible, and the answer
is a round number.

Unpolarised light, two bounces off glass, tracking the two states through both
and averaging once, against this renderer's averaging after each:

      first    second      tracked      averaged     ratio
       0.00      0.00     0.00160000   0.00160000    1.0000
      45.00     45.00     0.00426907   0.00252405    0.5912
      56.31     56.31     0.01094149   0.00547075    0.5000
      70.00     70.00     0.04578120   0.02925555    0.6390
      85.00     85.00     0.38981461   0.37552341    0.9633

At two bounces at Brewster's angle the approximation returns **exactly half**
the right answer, and 0.5000 is the worst case over the whole grid of angle
pairs. The reason is what Brewster's angle is: after one bounce at 56.31
degrees the light is completely s-polarised, and a second surface at the same
angle reflects s strongly and p not at all — but the model has forgotten the
light is polarised, applies the average a second time, and loses the
correlation.

And one bounce is exactly right, which is the other half of the admission. The
error is not "small", it is **zero** at first contact and grows from there.

## The case that is not a factor

Same-plane is the mild one. Turn the second surface ninety degrees and the
tracked answer at two Brewster angles is **zero** — two crossed polarisers,
the oldest demonstration in optics — while this renderer predicts 0.00547.

There the relative error is unbounded, and what the model cannot produce is
the *absence* of a reflection. No correction factor exists for that, which is
the argument for item 0159 rather than for a fudge.
