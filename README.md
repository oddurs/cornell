# cornell

A box in a lab, modelled from first principles in C++23, for no reason.

```
make && ./cornell
```

No dependencies. Nothing to configure. No website.

> **Status: nothing is built yet.** This repository currently contains an
> argument, a set of rules, and a roadmap of 147 items across 16 milestones.
> The first line of physics has not been written. Everything below describes
> what the thing is meant to be and how it will be checked, and every figure
> quoted here once there are figures will be a copy of something the program
> printed — see house rule 6, which exists because copies rot.

---

## The box

In 1984, four people at Cornell — Goral, Torrance, Greenberg and Battaile —
published a paper on how light bounces between diffuse surfaces. The method
was new and the reason anyone believed it was not the mathematics. It was that
they built the scene out of plywood, painted one wall red and one wall green,
put a light in the ceiling, photographed it with a calibrated radiometer, and
printed the photograph next to the simulation.

That box is now the most rendered object in the history of computer graphics,
and almost every renderer that draws it has quietly forgotten that it is a
real thing with measurements attached. It is not a test scene. It is a *claim*
that can be checked.

This project's last milestone is checking it again, forty years later, with a
different algorithm.

---

## Two things it refuses to let you type

### A colour

Gold is `vec3(1.0, 0.766, 0.336)` in every renderer written for pleasure, and
it is nothing of the kind. It is a table of complex refractive index — n and
k, per wavelength, measured by Johnson and Christy in 1972 on evaporated films
with a spectrometer — and the yellow falls out of Fresnel's equations from
1823.

Swap the table for copper and the reflection turns pink because of physics.
Swap it for aluminium and it goes neutral. Nobody consulted an artist.

There is no RGB inside this program. Light is spectral radiance, reflectance
is a spectrum, an index of refraction is a spectrum, and tristimulus values
exist in exactly two files: the one holding the 1931 standard observer, and
the one holding the sRGB transfer function. That decision costs something on
every line, and it buys dispersion through a prism with no prism model, metals
that are the right colour with no tint, and an answer to "what does this look
like under a sodium lamp" that is a computation rather than an apology.

### A feature

Soft shadows, colour bleeding, caustics, ambient occlusion, depth of field and
glossy reflection are not features. **None of them has any code.**

They are consequences of solving one integral honestly:

    L(x, wo) = Le(x, wo) + ∫ f(x, wi, wo) L(x, wi) (n·wi) dwi

A Whitted ray tracer has a `soft_shadow()` routine. This one must never
acquire one, and `grep -ri "soft_shadow\|ambient_occlusion" include/` returning
nothing is an acceptance criterion rather than a boast.

---

## The instruments come first

The roadmap's most deliberate feature is its ordering. The white furnace test,
the chi-squared sampling test and the convergence plot are built at **v0.5**.
The microfacet model that will fail all three arrives at **v0.7**.

That failure is the point.

A rough metal with reflectance 1, in an environment of uniform radiance 1,
must vanish — render exactly as white as the background, because no energy was
absorbed. The standard single-scattering microfacet model does not. It loses
the light that bounces from one facet to another, the deficit grows with
roughness, and almost every renderer ever shipped has it.

Finding that with an instrument built two milestones before the model, rather
than discovering it in a paper afterwards, is the single best thing this
project can demonstrate. A check written after the thing it checks is a check
written to pass.

| instrument | what it witnesses |
| --- | --- |
| `furnace` | energy conservation, as a pass/fail you can see |
| `chi2` | that `sample()` and `pdf()` describe the same distribution |
| `converge` | that RMSE falls as N^-½, or the estimator is biased |
| `spectrum` | any spectrum in the project, with its chromaticity |
| `swatch` | the metals, rendered from nothing but citations |
| `spec` | the box, as a specification, with everything derived from it |
| `verify` | every physical claim the project makes, in one run |

---

## The roadmap

[`ROADMAP.md`](ROADMAP.md), generated from items under `cairn/`.

    v0.1  A dark room with a hole in it       units, spectra, a first image
    v0.2  The cosine law                      Lambert, the rendering equation
    v0.3  Eyes have three cone types          CIE 1931, sRGB, colour at last
    v0.4  The box, as measured                triangles, a BVH, cornell.hpp
    v0.5  It has to be right                  the instruments, before the physics
    v0.6  You never type a colour             Fresnel, and the thesis
    v0.7  Roughness                           microfacets, and the furnace failing
    v0.8  Both ends of the path               Veach, and multiple importance sampling
    v0.9  Glass, and the prism nobody wrote   dispersion from measured glass
    v1.0  Compared against the photograph     the claim, checked
    v1.1  Between the surfaces                participating media
    v1.2  Outside the box                     Rayleigh, and the sunset nobody wrote
    v1.3  Skin, marble, milk                  subsurface scattering
    v1.4  Paths from both ends                bidirectional, and the caustic
    v1.5  The camera as an object             real lenses, real aberration
    later Not modelled                        polarisation, fluorescence, diffraction

---

## What it cannot do

This is geometric optics: Maxwell's equations in the limit where every feature
is large compared with a wavelength. That limit is a choice, it is stated at
the top of the first header a reader opens, and it costs, by name:

- **polarisation** — glare off water at a steep angle is wrong; the sky's
  polarisation pattern, which bees navigate by, cannot be represented
- **fluorescence** — the optical brighteners in white paper, highlighter pens,
  a white shirt under a blacklight
- **interference** — the colours in a soap film, an oil slick, a beetle's shell
- **diffraction** — the starburst around a street light, the rainbow off a CD,
  and the resolution limit of every lens ever ground

Each has an item under `later` saying what it would cost and what it would
buy. An unstated simplification is a lie; a stated one is a design decision.

---

## Conventions

The rules are in [`CLAUDE.md`](CLAUDE.md) and they are not suggestions. The
short version:

1. Everything inside is SI, and colour is spectral
2. Derive, never declare
3. The estimator is always visible — every sample carries its density
4. No dependencies
5. Verify every physical claim
6. Every figure quoted outside the code is a copy, and copies rot
7. Say what is not modelled
8. One idea per file, and people get named — a law and a curve-fit must never
   be spelled the same way
9. Name it what its author named it (`trowbridge_reitz.hpp`, not `ggx.hpp`)

---

## A sibling

[`windsor`](https://github.com/oddurs/windsor) is a Ford 302, modelled from
first principles in C++23, for no reason. It refuses to let you type in a
firing order: you forge a crank, you hang eight rods on four journals, and the
firing order is whatever falls out.

This is the same argument about light.

---

MIT.
