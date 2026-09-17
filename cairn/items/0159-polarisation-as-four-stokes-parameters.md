---
id: 159
title: Polarisation, as four Stokes parameters and a Mueller matrix
type: chore
status: backlog
milestone: later
created: 2026-09-16
updated: 2026-09-16
priority: p2
area: transport
effort: l
---

Item 0081 measured what averaging the two polarisations costs and filed the
fix here. This is the fix.

## What it would take

A path carries one number per wavelength. Polarised light needs **four** — the
Stokes parameters, which describe an intensity, two axes of linear
polarisation and a circular one, and which have the useful property that they
are all measurable with a detector and a filter rather than being amplitudes
and phases.

Every interaction becomes a four-by-four **Mueller matrix** instead of a
scalar multiply, and each one is expressed in its own surface's plane of
incidence — so a rotation into that plane on the way in and out of every
bounce, which is where the bugs would be.

Roughly four times the state on every path, a matrix multiply where there is
now a multiply, and a rewrite of every material in the project. `spectrum.hpp`
opens by saying light is "a function, not three numbers"; this is the same
argument one level up, and this project has not made it.

## What it would buy

The things `fresnel.hpp` says it cannot produce:

- **Glare that a polarising filter removes.** The reason the model cannot do
  it is not that it lacks a filter — it is that the light it reflects off a
  wet road is not polarised, so there is nothing for a filter to remove.
- **The sky's polarisation pattern**, which is a band of strongly polarised
  light ninety degrees from the sun, and which bees and some birds navigate
  by. v1.2 brings a Rayleigh sky, and Rayleigh scattering is *the* textbook
  polariser.
- **Two crossed reflections cancelling.** Item 0081 measured this: at two
  Brewster-angle bounces in perpendicular planes the true answer is zero and
  this renderer predicts 0.00547. The relative error is unbounded, and what
  is missing is the *absence* of a reflection.
- **Stress birefringence**, the colours in a plastic ruler between polarisers,
  which needs a medium with two indices and is a whole second subject.

## What it would not buy

An obviously different Cornell box. The measured error at one bounce is zero —
averaging is exact for unpolarised light meeting a surface for the first time
— and the box's light is unpolarised, its walls are matte, and the paths that
matter are short. The error is in the second bounce off a *smooth* surface,
and the box has none.

Which is why it is here rather than in a milestone. It is the right thing and
it is not this project's thesis, and doing it would quadruple the state on
every path to fix an error the scene cannot exhibit.

## Acceptance criteria

- [ ] A `Stokes` type, with the four parameters named and their measurement
      described — an intensity and three differences, not an abstraction
- [ ] Mueller matrices for every interaction in the project, derived
- [ ] The rotation into each surface's plane of incidence, written once
- [ ] Item 0081's crossed-polariser check returns zero rather than 0.00547
