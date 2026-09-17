// transport.hpp — the assumption the whole program is standing on, written
// down before any of the program is.
//
// This file will hold the rendering equation, and it does not hold it yet:
// that is v0.2, item 0029. What it holds now is the admission, and the
// admission comes first on purpose. A reader who wants to render a soap
// bubble should find out that they cannot in the first file they open, not in
// the third week, and a simplification nobody stated is not a simplification.
// It is a lie with a schedule.
//
//      THIS IS GEOMETRIC OPTICS: MAXWELL'S EQUATIONS IN THE LIMIT WHERE EVERY
//      FEATURE IS LARGE COMPARED WITH A WAVELENGTH.
//
// In that limit light travels in straight lines, energy adds, a surface is
// described by how much it sends where, and the whole of image synthesis
// becomes one integral equation that can be estimated by following paths. It
// is an extraordinarily good approximation — it is why a photograph of a
// pinhole camera looks like a pinhole camera — and it is exact nowhere.
//
// Visible light is 360 to 830 nanometres. So the limit holds when the things
// light meets are much larger than about half a micron, which covers a
// plywood wall, a sphere, a lens, a droplet of water and a grain of pigment,
// and does not cover a soap film, an oil slick, the ridges on a beetle's
// shell, the pits on a CD, the coating on a camera lens, or the edge of any
// aperture at all. Those are the cases below, and each one says what it would
// cost to have.
//
// ── What this model cannot produce, and what each would cost ──────────────
//
// The four are kept as items under the `later` milestone — 0143 to 0146 —
// and the sketches here are the short version of those.
//
//   1. POLARISATION.
//
//      Light has an orientation as well as an intensity, and a surface
//      responds to the two orientations differently. `fresnel.hpp` in v0.6
//      will compute both and then average them, which is the exact moment the
//      information is discarded.
//
//      What it costs: a wavelength stops carrying one number and carries four
//      — the Stokes parameters — so the path state quadruples. Every BSDF
//      stops being a scalar and becomes a 4 × 4 Mueller matrix, which means
//      every material in the project is rewritten, and the matrices have to
//      be rotated into a common reference frame at every bounce, which is a
//      new source of subtle sign errors.
//
//      What it buys: the glare off water at a steep angle, which a polarising
//      filter removes and this renderer cannot; the polarisation pattern of
//      the sky, which bees navigate by; stress birefringence in glass.
//
//   2. FLUORESCENCE.
//
//      A wavelength arrives, is absorbed, and leaves as a longer one. This
//      model has no mechanism for that at all: `Reflectance` multiplies a
//      `Radiance` wavelength by wavelength, which is a diagonal matrix, and
//      fluorescence is precisely the off-diagonal terms.
//
//      What it costs: the diagonal becomes a full re-radiation matrix — a
//      Donaldson matrix — so a surface is described by a square table rather
//      than a curve, and the four wavelengths a path carries can no longer be
//      chosen once at the camera, because the surface decides where the
//      energy goes next. Hero wavelength sampling, which `spectrum.hpp` is
//      built on, does not survive that unchanged.
//
//      What it buys: the optical brighteners in white paper and detergent,
//      which is why a white shirt is *brighter* than white; highlighter pens;
//      anything under a blacklight; most safety clothing.
//
//   3. INTERFERENCE.
//
//      Two beams at the same wavelength add their powers here. Real ones add
//      their amplitudes, which can cancel.
//
//      What it costs: light has to carry a phase as well as a magnitude, and
//      a coherence length, since sunlight only interferes with itself over a
//      few microns. A thin film needs the optical path difference through it
//      and a sum over multiple internal reflections. It is a different
//      quantity being transported, not an extra term.
//
//      What it buys: the colours in a soap film and an oil slick, the
//      structural colour of a beetle or a butterfly or a peacock — none of
//      which is a pigment — and the anti-reflective coating on every lens
//      this project will model in v1.5, which works by interference and will
//      have to be approximated rather than computed.
//
//   4. DIFFRACTION.
//
//      Light bends around edges, and an aperture spreads the beam that passes
//      it by an angle of roughly λ/d. The pinhole in `camera.hpp` has zero
//      size, so in this model it spreads light infinitely well and produces a
//      perfectly sharp image — which is the wrong answer by an amount that
//      goes to infinity as the hole closes.
//
//      What it costs: an aperture stops being a hole that passes rays and
//      becomes a wave propagation, or — cheaply and approximately — a
//      scattering function obtained from the Fourier transform of the
//      aperture. The cheap version is tractable and is what the few renderers
//      that do this at all use.
//
//      What it buys: the starburst around a street light at night, which is
//      the shape of the diaphragm blades; the rainbow off a CD; the softness
//      of every real pinhole photograph; and the resolution limit of every
//      lens ever ground, which is the reason telescopes are large.
//
// ── One more, which is not a wave effect ──────────────────────────────────
//
// Light here arrives instantly. The rendering equation as it will be written
// in v0.2 has no time in it, so every path is in equilibrium and nothing can
// be in flight. Time-of-flight imaging, and the photograph of a light pulse
// crossing a bottle, need a transport equation with a delay term and a film
// with a temporal axis. It is not on the roadmap at all, which is itself
// worth saying, since the absence of an item is the easiest omission to
// mistake for an oversight.
//
// ── The equation ─────────────────────────────────────────────────────────
//
// Kajiya, SIGGRAPH 1986, "The Rendering Equation". As he wrote it:
//
//      I(x, x') = g(x, x') [ e(x, x') + ∫  p(x, x', x'') I(x', x'') dx'' ]
//                                       S
//
//      I   the intensity of light passing from x' to x
//      g   the geometry term: zero if x and x' cannot see each other, and
//          otherwise falling off with the square of the distance between them
//      e   the light emitted from x' towards x
//      p   the scattering term: how much light arriving at x' from x''
//          continues towards x
//      S   the union of all surfaces in the scene
//
// It is an integral over *surfaces*, which is the formulation that makes the
// visibility explicit — `g` is where the shadow lives — and it is the form
// that bidirectional methods in v1.4 will need, because a path built from
// both ends is naturally a sequence of points rather than of directions.
//
// The form this project computes in is the same equation with the variable
// changed from points to directions:
//
//      L(x, wo) = Le(x, wo) + ∫ f(x, wi, wo) L(x, wi) (n·wi) dwi
//                             H²
//
// The two are related by the Jacobian of that change of variables,
//
//      dw = cos(theta') dx'' / r²
//
// which is exactly Kajiya's `g`. It has not gone anywhere: it has been
// absorbed into the measure, and the visibility it carried is now performed
// by casting a ray. That is the whole reason the directional form is the one
// to implement — the integral runs over a hemisphere that is always the same
// shape, instead of over a set of surfaces that changes with the scene, and
// the occlusion test is a function call rather than a term.
//
// ── Which file owns which term ───────────────────────────────────────────
//
//      Le(x, wo)       `scene.hpp` — `Surface::emission`, and `emitted()`,
//                      which returns it only on the side the normal faces
//
//      f(x, wi, wo)    `bsdf.hpp` — the three-method contract every surface
//                      signs. `lambert.hpp` is the only model so far and
//                      `fresnel.hpp` in v0.6 is the one the project is for
//
//      (n·wi)          `basis.hpp` — `cos_theta`, and the tangent frame the
//                      BSDF is evaluated in
//
//      L(x, wi)        the recursion, and therefore this file. It is resolved
//                      by casting a ray — `scene.hpp` finds what is there,
//                      `waechter.hpp` makes sure the ray does not find the
//                      surface it started on
//
//      ∫ … dwi         `warp.hpp` chooses the directions and states their
//                      density; `sampler.hpp` supplies the variates that
//                      choosing consumes. The division by that density is
//                      written out at the point of use and never cancelled,
//                      which is house rule 3 and item 0032
//
// ── The list with no code ────────────────────────────────────────────────
//
// Soft shadows. Colour bleeding. Caustics. Ambient occlusion. Depth of field.
// Glossy reflection. Indirect illumination. Contact shadows.
//
// **None of them has any code.** Not a routine, not a flag, not a term. They
// are what solving the equation above honestly *looks like*, and a reader
// will not believe that without being told plainly, because every renderer
// they have used has a checkbox for at least four of them.
//
// A soft shadow is the light's solid angle being partly blocked, which falls
// out of an area emitter and a visibility test. Colour bleeding is `f` being
// spectral and the recursion having more than one bounce. A caustic is a path
// that happens to go light → specular → diffuse → eye. Ambient occlusion is
// an approximation *to* this equation that exists because the equation was
// too expensive in 1998; computing the equation and then adding ambient
// occlusion to it is adding an approximation of a thing to the thing.
//
// The acceptance criterion for this file is that a search of `include/` and
// `apps/` for the names of those features finds nothing, and it is checked on
// every pull request rather than asserted here — the pattern lives in
// `.github/workflows/ci.yml`, outside the tree it searches, which is the only
// way a grep for a word can be run over a file that has to be free of it.
//
// If it ever fires, the project has stopped making its second claim and
// should stop printing it.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// The equation as written assumes light leaves a surface from the point it
// arrived at (no subsurface transport — v1.3), at the same instant (nothing
// is in flight, and nothing fluoresces), and at the same wavelength (no
// fluorescence — see the list above). The wavelength assumption is why
// `Reflectance` multiplies a `Radiance` component by component: a diagonal
// matrix, where fluorescence would need a full one.
//
// ── And it assumes the room is empty ─────────────────────────────────────
//
// The largest assumption in the equation is the one easiest to miss, because
// it is not a term that was dropped — it is a term that was never written.
// `L(x, wi)` is the radiance arriving at x from direction wi, and the
// implementation obtains it by casting a ray and asking what it hit. That
// step contains the whole of the claim:
//
//      LIGHT TRAVELS IN A STRAIGHT LINE BETWEEN TWO SURFACES AND ARRIVES
//      WITH EXACTLY THE RADIANCE IT LEFT WITH.
//
// Which is to say the space between surfaces is a vacuum. It does not absorb,
// it does not scatter, and it does not glow. `Ray::at(t)` is a straight line,
// `Scene::intersect` returns the first surface, and nothing at all happens in
// between — the distance the light travelled does not appear anywhere in the
// arithmetic, which is exactly what "unchanged" means.
//
// No photograph was ever taken in a vacuum. Air scatters; the Cornell box was
// photographed in a room with air in it. The assumption is extremely good
// over two metres and it is not free, and here is what it costs, by name:
//
//      Fog, haze and mist. A distant hill is pale not because it is painted
//      pale but because air between it and you has scattered sunlight into
//      your line of sight. This model renders it at full contrast at any
//      distance.
//
//      Smoke, steam and dust. Anything whose whole appearance is what it does
//      to light passing through it rather than what it does at a boundary.
//
//      The shaft of light through a window, and every searchlight, and the
//      beam of a torch in fog. A beam is only visible side-on because
//      something in the air is scattering it towards you. In here a beam of
//      light is invisible unless it lands on something, which is the correct
//      answer for a vacuum and is why an image from this renderer never has
//      one.
//
//      The colour of deep water, which is not a surface tint. Water absorbs
//      red light over metres, so a thing is blue-green at ten metres and
//      grey-blue at forty, and the colour is a property of the distance
//      rather than of the water's surface. A renderer without media has to
//      approximate that with a tint, and a tint does not change with depth.
//
//      Why a glass of milk is white. Milk is water with fat and protein
//      droplets in it, each of which is nearly transparent; the white comes
//      from light scattering off thousands of them before it leaves. There is
//      no white surface anywhere in a glass of milk. The same mechanism
//      makes clouds white, and skin the colour it is, which is why v1.3
//      depends on v1.1 rather than being independent of it.
//
// v1.1, "Between the surfaces", is where the straight line becomes a
// participating medium: a ray acquires a transmittance, an in-scattering
// term appears inside the integral, and `Ray::at` stops being the whole
// story. It is a change to this file rather than an addition beside it.
//
// It is worth being clear that this is a different kind of admission from the
// four at the top. Polarisation, fluorescence, interference and diffraction
// are outside geometric optics — this model cannot represent them in
// principle. Participating media are squarely inside it. The equation for
// them is standard, it composes with everything here, and the only reason
// there are none is that they have not been written yet.
//
// ── Solving it: recursion, and then not ──────────────────────────────────
//
// The equation is recursive — `L` appears on both sides — so the obvious
// implementation is recursive too, and it is worth writing down once, here,
// so that the loop below can be read as what it is rather than as a trick.
//
//      Radiance radiance(scene, ray, depth) {
//          hit = scene.intersect(ray);
//          if (!hit || depth == 0) return black;
//
//          [wi, f, pdf] = sample(hit.bsdf, wo);
//
//          return emitted(hit, wo)
//               + f * radiance(scene, Ray(hit.point, wi), depth - 1)
//                   * cos(theta) / pdf;
//      }
//
// That is correct, it is four lines, and it is the wrong shape. Two reasons,
// and the second is the one that matters.
//
// The small reason is the stack. One frame per bounce, a frame holding a
// scene reference, a hit record, a basis and a sample, and a path that in a
// closed white box can run to hundreds of bounces before Russian roulette
// takes it.
//
// The real reason is that everything this project is going to do next needs
// the recursion's accumulated state to be *a variable it can look at*. In the
// recursive form the product of all the `f · cos / pdf` factors so far only
// exists implicitly, spread across the stack, as the pending multiplications
// in half-finished frames. Russian roulette needs that product to decide a
// survival probability from it. Multiple importance sampling in v0.8 needs it
// to weight a light sample against a BSDF sample. A wavefront formulation,
// if this project ever wants one, needs it in memory rather than on a stack
// so that a thousand paths can be advanced one bounce at a time.
//
// So it is unrolled, the product is carried explicitly as a *throughput*, and
// the two forms are the same computation with the stack made into a local:
//
//      L = 0, throughput = 1
//      loop:
//          L += throughput · emitted
//          throughput *= f · cos / pdf
//
// That is the whole transformation. It is mentioned here once and never
// again.
//
// ── The estimator, uncancelled ───────────────────────────────────────────
//
// The line `throughput *= f · cos(theta) / pdf` is a Monte Carlo estimate of
// the integral, and it is written as the ratio it is. For the one material
// this project currently has, every factor in it is known in closed form, and
// they all cancel:
//
//      f            = rho / pi                  (lambert.hpp)
//      pdf          = cos(theta) / pi           (warp.hpp)
//
//      f · cos / pdf  =  (rho/pi) · cos · (pi / cos)  =  rho
//
// So the whole line could read `throughput *= rho`. It would compute the same
// number — verified below, to the last digit — with one multiply instead of
// nine and no division. Every tutorial renderer writes it that way, and it is
// not wrong.
//
// It is not written that way here, and the reason is not performance. It is
// that the three quantities are what v0.8 needs and the product is not.
// Multiple importance sampling weighs a BSDF sample against a light sample by
// asking each strategy how likely it would have been to produce the other's
// direction, and that question is answered with `f` and with `pdf`
// *separately*. A renderer that collapsed them has not lost a few
// instructions; it has lost the two quantities, from the design rather than
// from the code, and gets them back by reopening every material it has
// written. The collapse is also specific to this pairing — it happens because
// cosine sampling matches a Lambertian exactly — so every material after this
// one would have to un-collapse anyway.
//
// ── What refusing to cancel costs ────────────────────────────────────────
//
// Measured, because "the divides cost nothing the optimiser will not delete"
// is the kind of claim that ages badly. A 200 × 200 render at 256 samples per
// pixel, best of three runs, against the same renderer with the line replaced
// by `throughput *= rho` and `sample` no longer forming `f` or its density at
// all:
//
//      written out    3.757 s    2.73 Mpaths/s    mean radiance 0.410850
//      collapsed      3.774 s    2.71 Mpaths/s    mean radiance 0.410850
//
// The written-out version is half a percent *faster*, which is to say the
// difference is run-to-run noise and there is no cost to measure. Nine
// multiplies and a division per bounce disappear beside one ray-scene
// intersection, and the optimiser has the whole loop in front of it.
//
// The identical mean radiance is the other half of the result: the two are
// the same computation, so the algebra above is right, and the only thing
// separating them is which quantities survive to be asked about later.
//
// ── Depth ────────────────────────────────────────────────────────────────
//
// `max_depth` below is a *diagnostic* limit and not a physical one. There is
// no bounce count at which light stops bouncing; the physical termination is
// Russian roulette, which is unbiased. A path stopped by hitting the depth
// limit has had its remaining contribution silently discarded, which is bias,
// which is exactly the thing this project has instruments to detect.
//
// It exists so that a bug that produces an infinite path — a normal facing
// the wrong way, a surface that scatters into itself — terminates rather
// than hangs. If the limit is ever reached in a correct scene, the roulette
// is misconfigured, and the default is set high enough that reaching it is
// evidence of a mistake rather than of a bright room.
//
// ── Russian roulette, which is next door ─────────────────────────────────
//
// The physical termination — a coin that stops a path without throwing away
// what it would have carried — is `roulette.hpp`. It is the one piece of this
// file that is about ending an estimator rather than about the equation the
// estimator is solving, it has its own argument and its own measurements, and
// the loop below calls it in three lines.
//
// ── Measured ─────────────────────────────────────────────────────────────
//
// A closed cavity, every wall emitting `Le` and reflecting `rho`. The
// radiance inside is isotropic and satisfies `L = Le + rho·L`, so there is an
// exact answer to check against: `L = Le / (1 − rho)`. 200 000 paths from the
// centre in uniformly random directions, `Le = 1`:
//
//      rho    exact     estimated    error       std error   error / se
//      0.00   1.0000     1.00000     0            0            0
//      0.25   1.3333     1.33355    +2.17e-04    2.13e-04    +1.02
//      0.50   2.0000     2.00088    +8.75e-04    1.34e-03    +0.65
//      0.75   4.0000     4.00314    +3.14e-03    6.06e-03    +0.52
//      0.90  10.0000     9.98713    −1.29e-02    2.02e-02    −0.64
//
// Every one within about one standard error of the exact answer, at albedos
// from nothing to nine tenths. That is the transport being right, and the
// errors being ordinary Monte Carlo noise rather than a systematic lean.
//
// ── What this table used to say, and why it was wrong ────────────────────
//
// It is worth leaving the correction in the file rather than quietly
// restating the numbers, because the mistake is the exact one house rule 6
// exists to catch and it took a code review to find.
//
// The first version of this table was measured before Russian roulette
// existed, and reported relative errors of 1e-12 to 1e-16 with the words
// "they are *no* error … the estimator has no variance at all". That was true
// of the code at the time. Cosine-sampling a Lambertian makes `f · cos / pdf`
// exactly `rho` for every draw, so in a cavity with uniform emission every
// path returned the identical geometric series and the only residual was the
// depth limit's truncation.
//
// Roulette then changed the model two commits later, and the table was not
// re-run. The rule is that after any change to the model, every figure gets
// reconciled against what the program actually prints; this one sat there for
// four commits claiming an accuracy nine orders of magnitude better than the
// code could deliver, in the same file as a second table reporting a sample
// standard deviation of 0.586 — which is flatly incompatible with "no
// variance at all". The file contradicted itself and nobody noticed, because
// nobody re-ran it.
//
// The physics in the old paragraph survives and is worth keeping straight.
// The *BSDF* estimator still has zero variance here: `f · cos / pdf` is still
// exactly `rho` every time. All of the variance in the table above is the
// roulette's `1/q` scaling — paths that survive carry more weight than they
// otherwise would — which is precisely the trade `roulette.hpp` measures.
//
// Two things went with the correction. The depth limit no longer contributes
// anything: at `rho = 0.9` a path now ends by coin after about thirteen
// bounces, and the probability of reaching 256 is around 1e-12, so the
// truncation bias the old table was accidentally measuring is gone rather
// than merely smaller. And this test no longer proves only that the transport
// is right — it now exercises the noise as well, which the old one explicitly
// could not.

#pragma once

#include <cmath>

#include <render/basis.hpp>
#include <render/ray.hpp>
#include <render/roulette.hpp>
#include <render/sampler.hpp>
#include <render/scene.hpp>
#include <render/spectrum.hpp>
#include <render/vec.hpp>
#include <render/waechter.hpp>

namespace render {

// A diagnostic limit. See above: not a number of bounces light is allowed.
inline constexpr int default_max_depth = 256;


// What a single path contributes, following it until it escapes, is absorbed,
// or hits the diagnostic limit.
//
// The ray is taken by value because the loop advances it; that is the
// iteration, and handing it back to the caller modified would be a worse lie
// than copying six doubles.
// ── The trace, which is nothing by default ───────────────────────────────
//
// `replay.hpp` said, in v0.5, that a bounce-by-bounce trace was not being
// written until something needed it, and that when it was it would be "a
// template parameter with a no-op default and it costs nothing". Item 0070
// needed it one item later: a firefly is a single path that returned an
// enormous number, and the only way to say *why* is to watch what it did.
//
// So `radiance` takes a callable, called once per bounce with everything the
// loop knows at that moment. The default does nothing, has no members, and is
// inlined away — the generated code for the ordinary call is what it was, and
// item 0058's timing is the check on that claim rather than this sentence.
//
// It is deliberately not an interface for changing anything. The trace is
// handed values, not references it could write through; an integrator that a
// diagnostic can reach into is an integrator with two behaviours.
struct Bounce {
    int depth = 0;
    Vec3 point{};
    double distance = 0.0;
    bool emitter = false;
    Radiance emitted{};         // what this hit contributed, before throughput
    Reflectance before{};       // the throughput arriving
    Reflectance after{};        // the throughput leaving, roulette included
    double survival = 1.0;      // 1 where the roulette did not run
    bool killed = false;
};

struct NoTrace {
    constexpr void operator()(const Bounce&) const {}
};

// `roulette_start` is a parameter for the same reason `max_depth` is: it is a
// knob an instrument has to turn. `converge.hpp`'s claim is that switching the
// roulette off changes the noise and not the slope, and a claim about a switch
// needs a switch. Passing a depth beyond `max_depth` turns it off — not as a
// trick, but because "start the roulette after more bounces than there are" is
// what off means.
template <class Trace = NoTrace>
inline Radiance radiance(const Scene& scene,
                         Ray ray,
                         const Wavelengths& lambdas,
                         Sampler& sampler,
                         int max_depth = default_max_depth,
                         int roulette_start = roulette_start_depth,
                         const Trace& trace = Trace{}) {
    Radiance carried{};

    // The product of every `f · cos / pdf` so far: the fraction of whatever
    // this path finds next that will survive back to the eye. One, because
    // nothing has happened yet.
    Reflectance throughput{1.0};

    for (int depth = 0; depth < max_depth; ++depth) {
        const auto hit = scene.intersect(ray);

        // Escaped. There is no environment light in this project yet — v1.2
        // brings a sky — so a ray that leaves the scene found nothing, which
        // is different from finding black and happens to look the same.
        if (!hit) break;

        // `wo` points back the way the ray came, towards where this path's
        // light is headed. Every direction in `bsdf.hpp` points away from the
        // surface, and this is the first of them.
        const Vec3 wo = -ray.direction.vec();

        // The Le term. Emission is one-sided, so a light seen from behind
        // contributes nothing and the path continues past it.
        const Radiance arriving = emitted(*hit, wo, lambdas);
        carried += throughput * arriving;

        Bounce bounce;
        bounce.depth = depth;
        bounce.point = hit->point;
        bounce.distance = hit->t;
        bounce.emitter = hit->surface->emits();
        bounce.emitted = arriving;
        bounce.before = throughput;

        const Basis frame = hit->frame();
        const auto [u, v] = sampler.next2();
        const BsdfSample scattered =
            sample(hit->surface->bsdf, frame.to_local(wo), lambdas, u, v);

        // Absorbed, or the surface refused this direction. Multiplying by
        // zero and continuing would give the same answer and cost the rest of
        // the loop.
        if (scattered.is_black()) {
            bounce.after = Reflectance{};
            bounce.killed = true;
            trace(bounce);
            break;
        }

        // ── The estimator ────────────────────────────────────────────────
        //
        // Written as the ratio, in full, at the point of use. House rule 3,
        // and item 0032 for the measurement of what refusing to cancel it
        // costs, which is nothing.
        // A delta lobe is the one case where there is no ratio to write:
        // `bsdf.hpp` explains the convention, and the short version is that
        // the two infinities cancelled analytically and the result arrived
        // already divided. Both branches are here rather than one behind a
        // member function, so that the place where the division did not
        // happen is visible at the point of use like every other estimate.
        const double cos_theta_i = abs_cos_theta(scattered.wi);
        throughput = throughput * (scattered.specular
                                   ? scattered.weight
                                   : scattered.f * cos_theta_i / scattered.pdf);

        // ── Russian roulette ─────────────────────────────────────────────
        //
        // Unbiased because the survivors are divided by the probability that
        // they survived. The division is written out for the same reason
        // every other division by a density in this project is.
        if (depth >= roulette_start) {
            const double survival = survival_probability(throughput);
            bounce.survival = survival;
            if (survival <= 0.0 || sampler.next() >= survival) {
                bounce.after = throughput;
                bounce.killed = true;
                trace(bounce);
                break;
            }
            throughput = throughput * (1.0 / survival);
        }

        bounce.after = throughput;
        trace(bounce);

        // Leave along the sampled direction, from a point that is on the
        // correct side of the surface. The normal handed to the offset is the
        // one facing the way the new ray is going, which for a reflector is
        // the outward normal and for anything that transmits — v0.9 — is not.
        const Vec3 direction = frame.to_world(scattered.wi);
        const Unit away = dot(hit->normal, direction) > 0.0 ? hit->normal : -hit->normal;
        ray = Ray{offset_origin(hit->point, away), normalize(direction)};
    }

    return carried;
}

} // namespace render
