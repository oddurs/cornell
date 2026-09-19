---
id: 87
title: What to do about the missing energy
type: spike
status: done
milestone: v0.7
assignee: Oddur Sigurdsson
labels:
- admission
created: 2026-09-13
updated: 2026-09-19
priority: p0
area: bsdf
effort: m
---

## Question

Given that the single-scattering model loses energy, what does this project do
about it?

## Why it has to be answered first

Every answer is a compromise and the project's credibility depends on saying
which one it took.

## Options

**Kulla-Conty**: add a compensation lobe fitted to the measured deficit. Cheap,
used in production, and it is a fit — house rule 8 says it would have to be
named for whoever fitted it and marked as a curve.

**Heitz's stochastic multiple scattering**: random-walk the microsurface.
Correct by construction, unbiased, slower, and beautiful — it makes the
microsurface a real place that a ray actually travels through rather than a
statistical abstraction.

**Leave it, and quote the deficit.** Honest, and leaves the furnace test
failing forever, which erodes the instrument.

## What would settle it

The second option is the one that fits this project: it is a derivation rather
than a fit, the furnace test then passes for the right reason, and the cost is
performance, which this project has already said it will spend.

## Answer

**Heitz's stochastic multiple scattering.** Random-walk the microsurface, and
let the energy come back because the light was followed rather than because a
curve was fitted to how much of it went missing.

### Why, now that 0086 has measured it

The accounting in `./cornell furnace --table` settles this more firmly than
the argument from taste did. Every draw ends in one of three states, and at
roughness 1, head on:

    escaped 0.306733    masked 0.192990    below 0.500277

The deficit is not diffuse dissatisfaction with the model. It is two named
populations of rays with known directions: the ones another facet intercepted,
and — the larger group, and not the one the literature usually blames — the
ones the facet reflected into the surface. A random walk follows exactly those
rays and nothing else. It is the fix that is *about the same thing the bug is
about*.

Kulla-Conty is the alternative, and what disqualifies it is not that it is a
fit. It is that it fits a curve to the *total*, which means it puts the energy
back without asking where it went — and 0086 has just demonstrated that the
question has a specific, measurable, surprising answer. Adding a compensation
lobe now would be adding a lobe shaped to cancel a number this project has
already taken apart. House rule 2 in its clearest case: the deficit is derived,
so the repair should be too.

Leaving it and quoting the deficit was never available. `furnace.hpp` opens by
saying a furnace test is the best test in rendering because the object is there
or it is not; an instrument with a permanent known failure in it stops being
read, and the next real failure hides behind this one.

### What it costs, which is not mainly speed

The cost everyone quotes is performance, and that is the part this project has
already said it would spend. An order-of-magnitude prediction from the numbers
above: if the escape probability per bounce were the same at every bounce, the
mean number of scattering events at roughness 1 would be `1/0.307`, about
three. The escape probability almost certainly rises after the first bounce, so
three is a ceiling rather than an estimate, and it is a prediction rather than a
measurement — nothing has been built yet.

The cost that matters is structural, and it is worth saying before anybody
starts rather than discovering it halfway through.

**A stochastic BSDF has no closed form.** Heitz's model defines the BSDF as an
expectation over random walks. There is no `f(wo, wi)` to write down and no
`pdf(wo, wi)` either — both become unbiased *estimators* rather than functions.
That collides with three things this project has already committed to:

- **`bsdf.hpp`'s three-method contract**, which is one of its oldest design
  claims and exists so that `sample` and `pdf` are two independent statements
  a machine can check against each other.
- **`./cornell chi2`**, which compares a sampler against a density integrated
  by quadrature. A density that can only be estimated cannot be integrated
  that way, so the instrument that has caught the most in this project does
  not straightforwardly apply to the new material.
- **v0.8's multiple importance sampling**, which needs to evaluate one
  strategy's density on a direction the other strategy chose. That is the
  reason `pdf` was made a separate method in v0.1, two milestones before
  anything needed it.

The usual resolution is to keep the single-scattering lobe's closed-form
`eval` and `pdf` for weighting and to let the multiple-scattering energy ride
in `sample`'s weight. It is a compromise, it is defensible, and the point of
recording it here is that it must be **taken deliberately and written down**,
not arrived at by whoever is implementing when the compiler complains. That is
its own item.

### What follows

- **0160** — what a stochastic BSDF does to the three-method contract, which
  has to be decided before 0161 can have an interface.
- **0161** — the random walk itself, which is the model.
- **0162** — the furnace, finally vanishing, which is the thing this whole
  milestone was arranged to be able to say.

0160 first. A model whose interface is settled afterwards gets the interface
its implementation happened to need.

## 2026-09-19

Answered: Heitz's stochastic multiple scattering, and the item's own lean was right for a reason it could not have had when it was written. 0086's accounting is what settles it. The deficit is not diffuse - it is two named populations of rays with known directions, 0.193 masked and 0.500 reflected into the surface at roughness 1 head on. A random walk follows exactly those. Kulla-Conty fits a curve to the total, which puts the energy back without asking where it went, and 0086 has just shown the question has a specific and surprising answer.

The finding this spike adds, which is the part worth having: the cost is not mainly speed. A BSDF defined by a random walk has no closed form, so eval and pdf become unbiased estimators rather than functions, and that collides with three things already committed to - bsdf.hpp's three-method contract, chi2's quadrature over a density, and v0.8's MIS, which needs to evaluate one strategy's density on another strategy's direction. That is the real bill, and it is bigger than the performance one everybody quotes.

Deliberately NOT decided here, because it deserves its own item rather than being settled by whoever hits the compiler error first: which of the three gives. That is 0160 and it comes before the walk. A model whose interface is settled afterwards gets the interface its implementation happened to need.

Cost prediction, labelled as a prediction: if the escape probability were the same at every bounce, the mean number of scattering events at roughness 1 would be about 1/0.307, near three. It should come out below that since a ray that has bounced is better oriented to leave. Nothing has been built, so that is arithmetic on 0086's table and not a measurement.

Opened 0160 (contract, p0, m), 0161 (the walk, p0, xl), 0162 (the furnace finally vanishing, p0, s). The decision is recorded in torrance_sparrow.hpp and the README so a reader meets it where the failure is.
