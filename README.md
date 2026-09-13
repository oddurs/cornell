# cornell

A box in a lab, modelled from first principles in C++23, for no reason.

```
make && ./cornell
```

No dependencies. Nothing to configure. No website.

---

## You never type a colour

Gold is `vec3(1.0, 0.766, 0.336)` in every renderer written for pleasure, and
it is nothing of the kind. It is a table of complex refractive index — n and
k, per wavelength, measured by Johnson and Christy in 1972 on evaporated films
with a spectrometer — and the yellow falls out of Fresnel's equations from
1823 or it does not fall out at all.

Swap the table for copper and the reflection turns pink because of physics.
Swap it for aluminium and it goes neutral. Nobody consulted an artist.

That is the thesis, and everything else in this repository is in service of
being allowed to make it. There is no RGB inside this program: light is
spectral radiance, reflectance is a spectrum, an index of refraction is a
spectrum, and tristimulus values exist in exactly two files — the one holding
the 1931 standard observer and the one holding the sRGB transfer function.

The cost is a line of every file. What it buys is dispersion through a prism
with no prism model, metals that are the right colour with no tint, and an
answer to "what does this look like under a sodium lamp" that is a computation
rather than an apology.

If a material ever needs a tint multiplied on afterwards to look right, the
model has quietly stopped being true and the project is over.

---

## Where it has got to

**v0.1 — a dark room with a hole in it.** It builds, it runs, and it draws a
sphere on a black background. That is all it draws. There is no shading, no
colour, no light and no rendering equation yet: a ray leaves the aperture,
meets a sphere or it does not, and the film records which.

```
$ make && ./cornell render
480 x 320, 64 samples per pixel, 9.8 million rays
cornell.pfm   linear spectral radiance at 555 nm, in W/m2/sr/m
cornell.ppm   the same thing through a gamma of 2.2, for looking at
```

Every figure in this file is a copy of something a command printed, and the
command is shown above it. That is house rule 6, which exists because copies
rot: the program changes, and a number quoted here without its provenance is a
claim nobody can check and everybody believes.

What v0.1 actually settled, which is more than the picture suggests: that the
inside of this program is SI, that light is carried as four wavelengths rather
than three colours, that a direction is a type the compiler will not let you
forge, that an image file is a fifteen-byte header and some numbers, and that
the discriminant of a ray–sphere intersection has to be rearranged or the
sphere disappears at a hundred kilometres.

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

## The other thing it refuses to let you type

A feature.

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

The program prints the list itself, so it can only claim what it can do:

```
$ ./cornell
cornell — a box in a lab, modelled from first principles, for no reason.

  render    v0.1  built  the image itself
  spectrum  v0.3         any spectrum in the project, with its chromaticity
  furnace   v0.5         energy conservation, as a pass/fail you can see
  chi2      v0.5         that sample() and pdf() describe the same distribution
  converge  v0.5         that RMSE falls as N^-1/2, or the estimator is biased
  verify    v0.5         every physical claim the project makes, in one run
  swatch    v0.6         the metals, rendered from nothing but citations
```

---

## The roadmap

133 items across 16 milestones, as Markdown files under `cairn/`, rendered
into [`ROADMAP.md`](ROADMAP.md). `cairn board` prints where everything stands
and `cairn next` prints what is ready to work on.

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
the top of [`transport.hpp`](include/render/transport.hpp) before that file
contains any transport, and it costs, by name:

- **polarisation** — glare off water at a steep angle is wrong; the sky's
  polarisation pattern, which bees navigate by, cannot be represented
- **fluorescence** — the optical brighteners in white paper, highlighter pens,
  a white shirt under a blacklight
- **interference** — the colours in a soap film, an oil slick, a beetle's shell
- **diffraction** — the starburst around a street light, the rainbow off a CD,
  and the resolution limit of every lens ever ground

Each has an item under `later` saying what it would cost *and* what it would
buy, and `transport.hpp` carries the short version of both. An unstated
simplification is a lie; a stated one is a design decision.

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

The workflow the commits are made in — one item, one worktree, one branch, one
pull request — is in [`AGENTS.md`](AGENTS.md). Every change goes through a
pull request, including the one-line ones, because that is the only place a
change is checked by something that is not the machine that wrote it: a hook
runs `make` and `cairn check` in front of every commit, and CI builds each
pull request from a clean checkout on Linux under gcc and on macOS under
clang, runs it, and builds it again with every warning fatal. The gcc half of
that is the only portability test this project has.

---

## A sibling

[`windsor`](https://github.com/oddurs/windsor) is a Ford 302, modelled from
first principles in C++23, for no reason. It refuses to let you type in a
firing order: you forge a crank, you hang eight rods on four journals, and the
firing order is whatever falls out.

This is the same argument about light.

---

MIT.
