# cornell

A plywood box in a lab at Cornell, modelled from first principles in C++23,
for no reason.

This is not a renderer that happens to be readable. It is a piece of writing
that happens to run. If a change makes the image more accurate and the source
less beautiful, it is the wrong change.

It is a sibling to `windsor`, and it inherits that project's argument: that a
model earns the right to be believed by deriving what it claims rather than
declaring it, and that the derivation is the thing worth reading.

---

## What the thing is

A box, two coloured walls, a light in the ceiling, and a program that computes
what a camera pointed into it would record. The physics is honest — spectral
radiance, Fresnel's equations at every boundary, microfacet distributions with
their shadowing terms derived rather than chosen, Monte Carlo integration of
the rendering equation with the estimator written out where you can see it —
but accuracy is a means, not the end. The end is that a reader who has never
opened a graphics paper finishes the repository understanding why gold is
yellow.

The box is not a test scene. It is a real object: built, painted, lit and
photographed with a calibrated radiometer at the Cornell Program of Computer
Graphics, and published with its spectral reflectances, so that a renderer has
something to be right *about*. Goral, Torrance, Greenberg and Battaile put it
in front of a camera in 1984 and compared the photograph with the simulation,
which is the only reason anyone should have believed them.

The thesis lives in `fresnel.hpp` and everything else serves it:

> You never type a colour. Gold is `vec3(1.0, 0.766, 0.336)` in every renderer
> written for pleasure, and it is nothing of the kind. It is a table of complex
> refractive index — n and k, per wavelength, measured by Johnson and Christy
> in 1972 — and the yellow falls out of Fresnel's equations or it doesn't.
> Swap the table for copper and the reflection turns pink because of physics,
> not because somebody typed pink.

Change the metal, and the colour changes. If it ever stops doing that — if a
material ever needs a tint multiplied on afterwards to look right — the model
has quietly stopped being true and the project is over.

The second claim, which belongs to the integrator:

> Soft shadows, colour bleeding, caustics, ambient occlusion, depth of field
> and glossy reflection are not features. **None of them has any code.** They
> are consequences of solving one integral honestly. A Whitted ray tracer has
> a `soft_shadow()` routine; this one must never acquire one.

---

## The homage

This is C++, deliberately, for the same reason `windsor` is, and it means the
same specific things.

**Represent ideas directly in code.** `Radiance` and `Irradiance` are distinct
types because they differ by a steradian and because confusing them is the
oldest mistake in the field. A `Spectrum` is not a colour and will not let you
treat it as one. A `Bsdf` that can be sampled must also be able to say with
what density it sampled, and the type system makes that not optional. The
domain is in the type system, and the compiler is an optician checking your
work.

**Zero overhead.** Every abstraction here must compile to the arithmetic you
would have written by hand and no more. No virtual dispatch in the hot loop;
a tagged union is what you would have written for a tagged union, so that is
what we write. If an abstraction costs a cycle it cannot justify, it is the
wrong abstraction.

**Prefer compile-time.** `consteval` literals. `constexpr` spectra. Illuminant
D65 computed at compile time from the figures CIE published. Errors a compiler
can catch should never reach an image.

**Don't make it look like C.** No output parameters, no raw owning pointers,
no `#define`, no arrays that decay. Values, references, `std::array`, and
names.

---

## House rules

### 1. Inside the renderer, everything is SI, and colour is spectral

Metres, watts, steradians, kelvin, nanometres, radians. Not in a field, not in
a parameter, not in an intermediate.

And the stronger form, which is the one that will be tested daily:

> **There is no RGB inside this program.** Light is spectral radiance,
> W·m⁻²·sr⁻¹·nm⁻¹. Reflectance is a spectrum. An index of refraction is a
> spectrum. RGB exists at exactly two surfaces — the albedo a user types in
> and the pixel written out — and `cie.hpp` owns both.

This is not purism. It is what buys dispersion without a prism model, metals
that are the right colour without a tint, and an answer to "what does this
look like under a sodium lamp" that is a computation rather than an apology.
Every renderer that starts in RGB and adds spectra later rewrites every
material it has. We are starting at the expensive end on purpose.

The sanctioned exceptions are two, and both convert at their own boundary and
say so, loudly, in a comment: `cie.hpp`, which must produce tristimulus values
because eyes have three cone types; and `srgb.hpp`, which must produce a
transfer function because monitors are not linear. If you find yourself
wanting a third, you are about to introduce a bug that will take an afternoon
to find and will look, the whole time, like an artistic choice.

### 2. Derive, never declare

The colour of a metal is not an input. You measure n and k, you evaluate
Fresnel, and the colour is whatever falls out — which is why gold and copper
and aluminium differ, and why no artist ever had to be consulted.

The rule applies everywhere, and the list of things it forbids is the spine of
this project:

- **Lambert's 1/π is derived**, by integrating the cosine over the hemisphere,
  in the file that uses it. It is never typed as `0.3183`.
- **The Smith masking function is derived from the normal distribution**, not
  chosen from a menu. Heitz showed in 2014 that it is not a free choice; a
  microsurface must project to the macrosurface, and G follows.
- **A microfacet BRDF's normalisation is derived**, by requiring the
  distribution to integrate to one over the projected hemisphere.
- **The sky is blue because of λ⁻⁴**, and the sunset is the same code with a
  longer path through the same atmosphere. Nobody writes a sunset.
- **Bokeh is the shape of the aperture**, not a parameter named `bokeh`.
- **A caustic is not a feature.**

The moment you type a derived constant in as a literal, the model stops being
a model and becomes a lookup table with opinions.

Corollary: when a derivation reproduces something real — the measured
reflectance of Cornell's red wall, the 550 nm peak of the photopic curve,
Brewster's angle falling out of Fresnel at the right degree — say so in the
commit message. Those are the moments the project is for.

### 3. The estimator is always visible

This is the one house rule `windsor` does not have, because `windsor` is a
deterministic mechanism and this is an experiment you run repeatedly.

Every `sample()` returns a value **and the density it was drawn from**. The
estimate is written as the ratio, in full, at the point of use. It is always
tempting to cancel the cosine against the pdf inside the sampling routine and
return a tidier number — it is algebraically correct, it is faster to type,
and it makes multiple importance sampling impossible to add later without
rewriting every material in the project. The divides cost nothing the
optimiser will not delete. Write them.

Noise is not a defect. It is the variance of an estimator, it falls as the
inverse square root of the sample count, and `./cornell converge` asserts
exactly that. A renderer whose error falls faster than N^-½ is not clever; it
is biased.

### 4. No dependencies

The standard library and nothing else. No CMake requirement, no image library,
no linear algebra library, no BVH library. `make && ./cornell` on a clean
machine with a C++23 compiler. A PPM is a short ASCII header and some bytes;
write it yourself. The repository should still build in fifteen years.

### 5. Verify every physical claim

Before a number appears in a comment or a README, check it. Sampling routines
get a χ² test against their own analytically evaluated pdf. BRDFs get put in a
white furnace and must **vanish**. Estimators get their convergence slope
measured. Spectra get integrated against the CIE observer and compared with
published chromaticity coordinates.

The lab-notebook voice is only earned if the figures are right; a beautiful
comment attached to wrong arithmetic is the worst thing this project could
contain.

There is a stronger standard available here than `windsor` ever had, and it is
the reason the box was chosen: **Cornell photographed the real thing.** The
project's final claim is a comparison against a measurement somebody else
made, in a lab, before any of this was written.

### 6. Every figure quoted outside the code is a copy, and copies rot

The README will quote reflectances, chromaticities, convergence slopes and
furnace residuals. Every one of them is a *copy* of something the program
prints, and the program changes.

After any change to the model, re-run `./cornell verify`, `./cornell furnace`
and `./cornell chi2`, and reconcile every number in the README against what
they actually printed. When editing prose programmatically, assert the anchor
exists. An edit that silently does nothing is worse than one that fails,
because you will believe it worked.

### 7. Say what is not modelled

This is geometric optics: Maxwell's equations in the limit where every feature
is large compared with a wavelength. That limit is a choice, it is stated at
the top of `transport.hpp`, and the things it costs are listed by name —
polarisation, fluorescence, diffraction, interference. The sheen on a CD and
the colours in a soap film are wave effects this model cannot produce, and
saying so is a design decision where not saying so is a lie.

Every header opens with prose explaining *why the part exists and what it is
arguing with*. Not what the code does — the code does that. Never write a
comment that restates the line beneath it.

### 8. One idea per file, and people get named

A file is named for a phenomenon or a person — `fresnel.hpp`, `lambert.hpp`,
`schlick.hpp`, `cie.hpp`. Files named after people are named that way on
purpose, and the naming carries information:

- **`fresnel.hpp`** is a law, and exact. Fresnel derived it in 1823 from
  elastic-ether theory, a physical model that is entirely wrong, forty years
  before Maxwell explained why the answer is right anyway.
- **`schlick.hpp`** is a *fit to the file next door*, and sits beside it
  deliberately, so a reader can see the trade rather than inherit it.
- **`cie.hpp`** is a fit to seventeen people in London in 1931, and the whole
  colour science of every screen you have ever looked at rests on them.
- **`henyey_greenstein.hpp`** was fitted to interstellar dust in 1941 and is
  now used for skin, milk and cloud, which should worry you slightly.

The credit is also a warning. A law and a curve-fit must never be spelled the
same way.

### 9. Name it what its author named it

`trowbridge_reitz.hpp`, not `ggx.hpp`. Trowbridge and Reitz published the
distribution in 1975; Walter and colleagues rediscovered it in 2007 and called
it GGX, and the literature has used the second name since. Both names go in
the comment. The file takes the first.

---

## Commits

Atomic, one subsystem each, written in the same voice as the code. Present
tense, no ceremony, and say what the part *does* rather than what you did to
it.

    Fresnel, and where the colour of gold comes from
    The furnace test, which the rough metal fails
    Admit that this cannot render a soap film

Not `feat: add fresnel module` and not `fix stuff`. The log is part of the
piece, and it is also the pull request: `.claude/propose` passes the commit
message straight through, so there is one thing to write rather than two.

Never commit a state that does not build. That one is not left to
discipline: a hook runs `make` and `cairn check` in front of every commit,
and CI builds every pull request from a clean checkout under both gcc and
clang. The loop the commits are made in — one item, one worktree, one
branch, one pull request — is written down in `AGENTS.md`.

---

## Layout

    include/render/     the optics. header-only, one idea per file.
    apps/               the instruments you point at it.
    cairn/              the roadmap, as items. `cairn next`.
    Makefile            `make`. that's it.

There is no src/. Every part of this is a header, because every part of it is
small enough to be read in one sitting, and splitting a thing that size across
two files buys nothing but a place for them to disagree.

The renderer knows nothing about output, and nothing about images. It is a
sealed mechanism that answers one question — how much light arrives here, from
there, at this wavelength — and the apps are witnesses to it: a camera, a
furnace, a chi-squared test, a convergence plot, a spectrometer, a colour
chart, and an inspection sheet. None of them may reach into the physics to
make their own job easier.

---

## The roadmap

It is in the repository, as Markdown, under `cairn/`.

    cairn next          what is ready to work on
    cairn board         where everything stands
    cairn roadmap       the milestones, in order

Work an item, close it, and let the hook re-render `ROADMAP.md`. Do not invent
a parallel list of things to do; this is the list.
