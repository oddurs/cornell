---
id: 76
title: 'schlick.hpp: the 1994 fit, in the file next door'
type: optics
status: done
milestone: v0.6
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-16
priority: p2
area: bsdf
effort: s
---

## What it is

Christophe Schlick, *An Inexpensive BRDF Model for Physically-based Rendering*,
Eurographics 1994. A one-line approximation to the file next to it:

    R(theta) = R0 + (1 - R0)(1 - cos theta)^5

It is in every real-time renderer in the world, it is a fit, and it sits
beside the exact version deliberately so that a reader can see the trade
rather than inherit it. This is house rule 8's clearest case: a law and a
curve-fit must never be spelled the same way.

## What it must derive

Its own error. The file quotes the maximum and mean deviation from exact
Fresnel, measured by this project, for a dielectric and for gold — and the
metal figure is the interesting one, because the approximation assumes a real
index and a metal's is not.

## Acceptance criteria

- [x] Not used by default anywhere; it exists to be compared
- [x] The error figures in the comment come from `./cornell verify`

Nothing calls it. `./cornell verify` is its only caller and what it does with
it is measure how wrong it is, against a bound taken from the comment — so the
comment cannot drift away from the measurement without failing a build.

## What the measurement corrected

The comment was written first, with figures from memory, and every one of them
was wrong. Six rows, 90,001 angles each:

      medium            n        R0       max error   mean error  worst at
      water           1.330    0.02006     0.05992     0.01086     83.8 deg
      window glass    1.500    0.04000     0.03569     0.00922     85.0 deg
      diamond         2.417    0.17197     0.07544     0.01338     84.5 deg
      n=0.2  k=3.0             0.92337     0.01544     0.00461     76.5 deg
      n=1.1  k=7.0             0.91762     0.10112     0.02121     83.4 deg
      n=0.05 k=4.2             0.98933     0.00604     0.00157     79.6 deg

**"Under one percent" is the mean, not the maximum.** The approximation's
reputation is about its average error. Its worst is three to eight percent for
a dielectric, and every maximum is at 84 or 85 degrees — the curve is pinned
at both ends by construction, so the last few degrees before grazing are where
it has the most room to be wrong. The guessed figures had the maxima at 65 to
71 degrees, which is where they would be if the fit were free at the ends.

**And the conductor error is not a multiple of the dielectric error.** The
guess said "two to three times". It is 0.006 at one complex index and 0.101 at
another — better than glass in the first case and three times worse in the
second — because a conductor's reflectance curve is not the shape Schlick
fitted and how badly that shows depends on where the index sits. There is no
correction factor. A renderer using this for metal is accepting an error it
cannot bound without computing the thing it was avoiding.

That is a better argument for the file existing than the one it was written
with, and it only exists because the numbers were measured after being
guessed.
