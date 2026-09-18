---
id: 75
title: Aluminium, from a different source, cited differently
type: optics
status: done
milestone: v0.6
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-17
priority: p2
area: bsdf
effort: s
---

Johnson and Christy measured the noble metals and aluminium is not one. It
comes from elsewhere — Rakic, or the Palik handbook — and it is cited
separately, because a project that quietly extends one citation to cover data
it did not come from has begun to rot.

Aluminium is also the useful control: it is nearly neutral, so if the pipeline
tints it, the tint is the pipeline's.

- [x] Separate citation, adjacent to the data, naming the source
- [x] `./cornell swatch` shows it neutral, and the deviation is quoted

      A. D. Rakic, "Algorithm for the determination of intrinsic optical
      constants of metal films: application to aluminum", Applied Optics 34,
      4755-4767 (1995).

Its own line, its own paper, its own range — 29 samples from 206.6 to 1771.2
nm, against Johnson and Christy's 49 from 187.9 to 1937. Extending their
citation to cover a metal they did not measure would be the beginning of the
rot: a table whose provenance is "the same place as the other tables,
probably".

## The deviation

      aluminium   x 0.31161  y 0.32825    0.00133 from D65 white
      silver      x 0.31350  y 0.32967    0.00103 from D65 white
      gold        x 0.38177  y 0.38870    0.09128 from D65 white

0.0013, against gold's 0.0913 — a factor of 68. Its reflectance runs 0.922 at
450 nm to 0.906 at 700, a 1.6-point tilt across the whole visible against
gold's 56.

Which is what makes it the control. If aluminium ever comes out tinted, the
tint is the pipeline's and not the metal's, and there is now a number that
says so rather than an impression.
