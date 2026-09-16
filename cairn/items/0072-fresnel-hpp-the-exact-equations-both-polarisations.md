---
id: 72
title: 'fresnel.hpp: the exact equations, both polarisations'
type: optics
status: done
milestone: v0.6
assignee: Oddur Sigurdsson
labels:
- thesis
created: 2026-09-13
updated: 2026-09-16
priority: p0
area: bsdf
effort: m
---

## What it is

The thesis file. Augustin-Jean Fresnel, 1823 — how much light reflects from a
boundary, as a function of angle, index, and polarisation.

The opening comment has the best footnote available to this project: Fresnel
derived these from elastic-ether theory, a physical model in which light is a
mechanical vibration of an invisible solid filling the universe, and which is
entirely wrong. He got the right answer forty years before Maxwell explained
why it is the right answer. Being wrong about the mechanism and right about
the boundary conditions is a thing that happens, and it is worth a reader
knowing it.

## What it must derive

Everything visible. Specifically, all of the following must fall out of these
equations and appear nowhere as a parameter:

- that every material becomes a mirror at grazing incidence
- the sheen on a page held up to a window
- why a wet road is darker and shinier than a dry one
- Brewster's angle, at which reflected light is fully polarised
- the colour of every metal in the project

## What is not modelled

Polarisation, which is the irony of the file: Fresnel's equations are stated
per polarisation and this renderer averages the two because it carries scalar
radiance. That is correct for unpolarised illumination and wrong for light
that has already reflected once at a steep angle. Named, quantified if
possible, and left out.

## Acceptance criteria

- [x] The dielectric and conductor cases are the same function, differing only
      in whether the index is complex
- [x] Total internal reflection falls out rather than being branched on
- [x] The comment derives Brewster's angle from the equations above it

There is one `fresnel()`, it takes `std::complex<double>`, and a dielectric is
the case where the imaginary part is zero. There is no `if` in the file for
total internal reflection: past the critical angle the argument of the square
root goes negative, the cosine is imaginary, and the numerator and denominator
become conjugates whose ratio is 1.

## What the checks are, and where

`fresnel.hpp` is the first file in this project that cannot pin its own
constants with a `static_assert`. Brewster's angle is an arctangent, the
critical angle an arcsine, and the reflectances need a complex square root —
and none of `std::atan`, `std::asin` or `std::sqrt(std::complex)` is
`constexpr` in the standard. libstdc++ provides some as an extension and
libc++ does not, so an assert here compiles on one of this project's two
compilers and fails on the other, which is worse than not having one.

That turned out better rather than worse. A `static_assert` could only have
evaluated the derived formula and compared it with itself. `./cornell verify`
*searches* the reflectance curve for the minimum of the parallel polarisation
and compares where it lands against `arctan(eta_t/eta_i)` — a different
question with the same answer.

## An exactness that had to be arranged

The first version computed each amplitude ratio and then took its squared
magnitude, which is how the equations are written. Under total internal
reflection that rounds twice and returns **1.0000000000000002** at 89 degrees:
a surface reflecting more light than arrives at it.

`|a/b|^2` is `|a|^2/|b|^2`, and taking the norms separately is both cheaper
than a complex division and exact where it matters, because under total
internal reflection the numerator and denominator are conjugates with bit-
identical magnitudes. Measured over 48,140 angles past the critical angle:
half come back as exactly 1.0, none above it, worst 1.5 ulp short. The half
that are not exact are where the complex multiplies had already rounded.
