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

**v0.5 — it has to be right.** The box is the real one: Cornell's geometry and
their measured spectral reflectances, lit by their measured lamp, and the
instruments that will judge v0.7's physics are built and calibrated.

```
$ make && ./cornell render 400 --spp 64
400 x 400, 64 samples per pixel, 10.24 million paths, 1.0 s on 10 threads
  10.25 million paths per second
luminance Y: mean 0.0107, brightest 1.0694
out of gamut: 24587 of 160000 pixels have a negative sRGB component
cornell.pfm   linear sRGB, three channels, unclipped — the file a
              number may be quoted from
cornell.ppm   the same, exposed against Y = 0.20, tone mapped with
              'clip', and encoded with sRGB's transfer function.
              The lamp is 5x over. No figure is quoted from this file.
```

Two of those lines are the machine's rather than the program's — the elapsed
time and the rate — and will differ on yours. The rest is deterministic: the
sampler is addressed by pixel and sample index rather than drawn from a shared
stream, so every figure above is the same on any machine at any thread count,
and `./cornell verify` checks that rather than asserting it.

Every figure in this file is a copy of something a command printed, and the
command is shown above it. That is house rule 6, which exists because copies
rot: the program changes, and a number quoted here without its provenance is a
claim nobody can check and everybody believes.

**This block was stale twice.** It said v0.1 for three commits after v0.2 was
finished, quoting a line the program had stopped printing — found by a code
review. Then it sat at v0.2 through three more milestones, quoting `spectral
radiance at 555 nm` and `gamma 2.2` from a program that had stopped printing
the first and stopped doing the second, in a section whose own next paragraph
was about exactly that. Found by item 0065, which is the item that says every
number here has to trace to a line of output.

Three things in the picture have no code, which is the point of the second
claim. The shadow under the lamp has a soft edge — the penumbra is the lamp's
solid angle being partly blocked. The corners are darker than the middles of
the walls — fewer directions from a corner reach the lamp. And the white wall
beside the red one is faintly red, because light that reaches it came off the
red one; nobody wrote colour bleeding either.

It is noisy because a path finds the lamp only by wandering into it. Cornell's
lamp is 130 by 105 mm — 0.01365 m² — in a room 0.55 m across, so it subtends
about 0.045 of the 6.28 steradians a bounce can go into: **seven paths in a
thousand** find it per bounce, and everything else returns zero. That is the
highest-variance arrangement a correct estimator can have, and it is measured
rather than described: at 4096 samples per pixel the median lit pixel still
takes five percent of its value from a single sample, against the 0.00024 an
even estimator would give.

```
$ ./cornell render 400 --spp 256 --outliers 10
the largest single sample's share of its own pixel, over 135039 lit
pixels — an even estimator would give every one of them 0.00391:
  median 0.4530, 99th percentile 1.0000, and 59476 pixels over a half
```

Nothing is clamped, and nothing will be: a clamp removes energy, and removed
energy bends the convergence slope that `./cornell converge` measures. v0.8 —
direct light sampling, and Veach's weighting — is the fix, and item 0070 says
in advance what it has to achieve, so that it can fail.

What the five milestones actually settled, which is more than the pictures
suggest: that the inside of this program is SI, that light is carried as four
wavelengths rather than three colours, that a direction is a type the compiler
will not let you forge, that an image file is a fifteen-byte header and some
numbers, that the discriminant of a ray–sphere intersection has to be
rearranged or the sphere vanishes at a hundred kilometres, that a ray's offset
is a count of ulps rather than a length — so the same room renders bit for bit
identically across nine orders of magnitude, at every power of two and *not*
at 1000×, because multiplying a coordinate by 1024 leaves every mantissa bit
alone and multiplying it by 1000 is a rounding — that the estimator is written
out as `f · cos / pdf` rather than collapsed, which was measured to cost
nothing, that a BRDF and a reflectance differ by a steradian and the compiler
now says so, that the sRGB standard's printed matrix disagrees with its own
printed white point, and that Cornell's data page is gone from the live web
and survives only in the Internet Archive.

---

## And one it will not let you forget

The colour matching functions are not a law.

W. David Wright measured ten observers at Imperial College in 1928 and 1929.
John Guild measured seven at the National Physical Laboratory. Each of them
sat in the dark and turned three knobs until a mixture of three primaries
matched a monochromatic light, wavelength by wavelength. The two sets of
results agreed closely enough that the CIE averaged them in 1931, and the
colour science of every screen, camera, print process and television standard
since rests on what seventeen young British men's eyes did.

That table is in this repository, cited, and so is what is wrong with it.
Variation between observers is far larger than the precision it is computed
to, so "the standard observer" is nobody. The 1931 x-bar has known problems in
the blue, because it was fitted to a luminosity curve that was itself too low
at short wavelengths — Judd corrected it in 1951 and Vos in 1978, and neither
replaced it, because by then the world was built on it. And it describes a
patch the width of a thumbnail at arm's length, while the box below subtends
rather more.

None of that is a sneer; it has survived a century of people trying to
improve on it. It is simply the one input to this project that measures
*people* rather than the world, and a repository that makes a point of
deriving rather than declaring should say which of its foundations is which.

```
$ ./cornell spectrum d65
```

prints the daylight spectrum, what the observer makes of it, and a correlated
colour temperature of 6504 K — which is the number in D65's own definition,
arrived at from the other end.

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
The microfacet model they were built to examine arrives at **v0.7**, and one
of the three catches it.

That failure is the point.

A rough metal with reflectance 1, in an environment of uniform radiance 1,
must vanish — render exactly as white as the background, because no energy was
absorbed. The standard single-scattering microfacet model does not. It loses
the light that bounces from one facet to another, the deficit grows with
roughness, and almost every renderer ever shipped has it.

The other two pass, and that is the discrimination worth having rather than a
consolation. The sampler agrees with its own density at every roughness from
0.001 to 1, and the estimator converges at the rate it should; what is wrong
is the model, not the arithmetic, and three instruments that all failed
together would not have been able to say so.

`./cornell furnace --bsdf conductor` puts a rough metal of reflectance 1 in
the furnace and reports what comes back. Mean radiance off the sphere, where
1 is vanished:

```
      alpha    mean L on it     deficit
      0.001        0.999998    -0.000002
      0.050        0.991349    -0.008651
      0.200        0.915234    -0.084766
      0.600        0.631849    -0.368151
      1.000        0.411050    -0.588950
```

At the smooth end it vanishes, because a mirror has no second facet to lose
light to. At the rough end four tenths of the light comes back. Nothing was
absorbed — the reflectance is 1 at every wavelength and every angle — so that
is the model throwing energy away, and the number is what the instrument was
built two milestones early to be able to print.

`./cornell furnace --table` says where it went. Every draw ends in one of
three states, and they are exhaustive, at normal incidence:

```
      alpha     escaped     masked      below     sum - 1
      0.050    0.997310   0.000207   0.002483    7.11e-14
      0.200    0.947618   0.013888   0.038494    7.11e-15
      0.600    0.591360   0.143731   0.264909    5.77e-15
      1.000    0.306733   0.192990   0.500277    7.99e-15
```

`masked` is light another facet intercepted on the way out. `below` is light
the facet reflected into the surface. Both are the ray meeting the
microsurface a second time, which a single-scattering model has nothing to say
about and therefore drops. They sum to one to fourteen decimal places, and
that closure is the whole difference between a model that is wrong and a
program that is broken: no light is missing, it is all in states the model
refuses to follow.

It also corrects the story this repository was going to tell. The usual
account — and the one written into the roadmap item before it was measured —
blames the masking term. That is the smaller channel. At roughness 1 a fifth
of the light is masked and **half of it never points outward at all**. A rough
surface loses light mostly because it is rough enough to reflect into itself.

Finding that with an instrument built two milestones before the model, rather
than discovering it in a paper afterwards, is the single best thing this
project can demonstrate. A check written after the thing it checks is a check
written to pass.

### They test what they test

Which is the half of that argument it is easy to skip. An instrument is not
rigour; it is one question, asked well. So each of the v0.5 checks comes with a
deliberately wrong model written to be caught by it — and then **every liar is
run through every check**, and the misses are printed beside the catches:

```
$ ./cornell verify

  deliberately wrong model            furnace     chi2  density  swapped
  claims a flat density                     -   caught        -        -
  claims one 2% too steep                   -   caught        -        -
  a density that is half of one             -   caught   caught        -
  weights only the incoming ray        caught        -        -   caught
```

Four models, each wrong in one way, none of them caught by every column. A
density can be wrong by two percent and conserve energy *exactly* — it sails
through the furnace, and it is invisible in a rendered image. A BRDF that
weights only the incoming direction samples honestly and has a perfectly valid
density. No column is sufficient, and the matrix is printed rather than quoted
from memory, so it cannot quietly stop being true.

The density column is the honest exception: nothing above is caught by it
alone, and the project says so rather than arranging a fifth liar to justify
it. What it is for arrives in v0.8, where multiple importance sampling
evaluates one strategy's density on another strategy's directions — a use of a
pdf with no sampler attached, which is the one place a chi-squared has nothing
to compare.

It also caught a claim this README used to make. The normalisation check was
justified on the textbook argument that a chi-squared test compares *shapes*
and is therefore blind to a density that is uniformly half of one. That is
true of a chi-squared that renormalises its expectations to the observed
total, and this one does not — it measures the half-density at χ²/dof 515. The
check is still there, for two narrower reasons that survive: it is exact
rather than statistical, and it needs no sampler, which is the only form
available in v0.8 when a density is evaluated on directions another strategy
produced.

The program prints the list itself, so it can only claim what it can do:

```
$ ./cornell
cornell — a box in a lab, modelled from first principles, for no reason.

  render    v0.1  built  the image itself
  spec      v0.4  built  the box as a specification, everything derived
  spectrum  v0.3  built  any spectrum in the project, with its chromaticity
  furnace   v0.5  built  energy conservation, as a pass/fail you can see
  chi2      v0.5  built  that sample() and pdf() describe the same distribution
  converge  v0.5  built  that RMSE falls as N^-1/2, or the estimator is biased
  verify    v0.4  built  every physical claim the project makes, in one run
  swatch    v0.6  built  the metals, rendered from nothing but citations
```

### What the furnace says today

There is one BSDF in the project, and it vanishes.

```
$ ./cornell furnace

      rho    pixels on the sphere    worst |L - 1|      expected     verdict
     1.00                   16824    0.000000e+00    0.000000e+00    vanished
     0.50                   16824    5.000000e-01    5.000000e-01    visible, by exactly 1 - rho
     0.25                   16824    7.500000e-01    7.500000e-01    visible, by exactly 1 - rho
     0.00                   16824    1.000000e+00    1.000000e+00    visible, by exactly 1 - rho
```

A residual of **exactly zero**, not a small number: every path in the scene
returns the same double the walls emit, and `furnace.ppm` at ρ = 1 contains
one distinct byte value. The sphere is not dimmed, it is absent.

That is an easier result than it sounds and the ease is the point. The whole
test on a Lambertian is `(ρ/π)·π == ρ`, and the estimator divides the same
cosine by itself, so the arithmetic cancels rather than nearly cancelling.
Nothing here is evidence that the furnace is a good instrument — it is the
calibration that has to pass before v0.7 points it at a model whose answer
nobody knows.

### And what the chi-squared says

A sampler and its density are two claims written in different code, and
almost every BSDF bug in existence is a disagreement between them. A million
draws, histogrammed over the sphere, against the density integrated over each
cell by a quadrature that never calls the sampler:

```
$ ./cornell chi2

  bsdf                  theta_o       chi2    dof    chi2/dof           p
  lambert (grey)           0°      996.57   1023      0.9742      0.7172  pass
  lambert (grey)          30°      999.47   1023      0.9770      0.6948  pass
  lambert (grey)          60°     1045.63   1023      1.0221      0.3046  pass
  lambert (grey)          85°     1005.59   1023      0.9830      0.6451  pass
  lambert (grey)          30°*    1060.77   1023      1.0369      0.2005  pass
  lambert (d65)           45°     1028.40   1023      1.0053      0.4467  pass
  lambert (measured)      45°     1007.08   1023      0.9844      0.6327  pass
```

The interesting half is underneath. A test that has never failed is a test
nobody has checked, so two deliberately dishonest samplers are run through the
same grid — both draw exactly what Lambert draws, and then misreport the
density:

```
      claimed density                    chi2/dof             p
      cos^0  (flat)                         340.7     0.000e+00
      cos^1.02  (2% too steep)                1.2     5.740e-05
```

The second one is the point. **χ²/dof of 1.2 looks fine.** A two-percent error
in a density is invisible in a rendered image, conserves energy perfectly, and
passes the furnace — and the p-value is 6×10⁻⁵. It is also a matter of how
hard you look: the same liar is *passed* at a quarter as many draws, with
p = 0.04.

```
        draws        cos^1.02 p      cos^0 p
       262144        2.536e-01    0.000e+00
       524288        4.266e-02    0.000e+00
      1048576        5.740e-05    0.000e+00
      4194304        3.766e-15    0.000e+00
```

### And where the colour of gold comes from

The thesis, checked. `metals.hpp` holds two columns of measured numbers per
metal and no colour at all; `fresnel.hpp` turns a complex refractive index
into a reflectance; `cie.hpp` turns a spectrum into three numbers because eyes
have three cone types. Nothing in between knows what yellow is.

```
$ ./cornell swatch

                      x         y         R         G         B        edge
  gold          0.38177   0.38870    1.0000    0.7020    0.3514    2.380 eV
  copper        0.35575   0.34559    1.0000    0.6683    0.5606    2.129 eV
  silver        0.31350   0.32967    1.0000    0.9945    0.9883    3.740 eV
  aluminium     0.31161   0.32825    0.9837    0.9930    1.0000    1.342 eV
```

Gold comes out at **(1.0000, 0.7020, 0.3514)**. The constant every renderer in
the world types for gold is `vec3(1.0, 0.766, 0.336)`. Those two numbers have
never met: one is a table measured by Johnson and Christy in 1972 pushed
through Fresnel's equations and the 1931 observer, and the other has been
copied between renderers for thirty years without a citation. They agree to
0.064.

The **edge** column is the part that can be checked against something other
than the table the program read. It is where each metal's reflectance falls
off a cliff, and that cliff is an interband transition — an electron promoted
from a filled d band to the Fermi surface — whose energy is a property of the
metal's band structure, quoted in the solid-state literature independently of
anybody's optical measurement:

| metal | measured here | literature |
|---|---|---|
| gold | 2.380 eV | ~2.4 eV, d-band threshold |
| copper | 2.129 eV | ~2.1 eV |
| silver | 3.740 eV | ~3.8 eV, plasma edge |
| aluminium | 1.342 eV | ~1.5 eV, parallel-band |

Silver's edge is *above* the visible, which is exactly why silver is neutral
and gold is not. Aluminium sits 0.0013 from D65 white and gold sits 0.0913 — a
factor of 68, and nobody typed either.

Swap gold's table for copper's and the same code returns (1.0000, 0.6683,
0.5606). The reflection turns pink because of physics, not because somebody
typed pink.

### And what the convergence slope says

Monte Carlo error falls as N^-½. Not as a rule of thumb — as the central limit
theorem — and a renderer whose error falls *faster* is not clever, it is
biased or it is being measured against its own bias.

The first measurement uses no reference image at all. A closed cavity where
every wall emits `Le` and reflects ρ has an isotropic interior satisfying
`L = Le + ρL`, so the answer is `Le/(1-ρ)` exactly, before the renderer is
asked:

```
$ ./cornell converge

                       1         4        16        64       256      1024      slope
   rho = 0.5     0.65074   0.27399   0.14873   0.07637   0.03698   0.01905    -0.5014
   rho = 0.9     8.60623   4.37140   2.22045   1.11787   0.54870   0.28633    -0.4931
```

Switch the Russian roulette off and this scene stops being random altogether —
cosine-sampling a Lambertian makes `f·cos/pdf` exactly ρ for every draw, so
every path returns the identical geometric series. What is left is the depth
limit cutting that series short, and it lands where the algebra says:

```
        rho     measured rmse      rho^256 / (1 - rho)
       0.90        1.933e-11              1.932e-11
```

In the Cornell box, which has no closed form, the slope is **−0.5049 ± 0.0114**
with roulette and **−0.5109 ± 0.0123** without. The error bar is measured
rather than chosen: the errors have a kurtosis of twenty to a few hundred
against a normal distribution's three, because a pixel's error is mostly a
question of whether it caught a rare path that found the lamp and came back
enormous. One firefly is enough to move an RMSE by a quarter. They are
[item 0070](ROADMAP.md)'s subject, and until then the instrument pools four
independent pairs and prints its own uncertainty.

---

## The roadmap

158 items across 16 milestones, as Markdown files under `cairn/`, rendered
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
