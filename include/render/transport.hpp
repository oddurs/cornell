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
// ── Russian roulette ─────────────────────────────────────────────────────
//
// A path in a closed box never ends. Every bounce keeps some fraction of the
// light, the fraction never reaches zero, and something has to stop it — and
// the obvious something, a depth limit, throws away the tail. That is bias:
// the answer is wrong by the discarded remainder, systematically, in the same
// direction, and no number of samples fixes it because every sample is wrong
// the same way. The table above measures exactly that at `rho = 0.9`.
//
// Roulette stops paths without throwing anything away, and the trick is three
// lines of expectation algebra. Continue with probability `q`, and if the
// path survives, divide its contribution by `q`:
//
//      E[estimate] = q · (X / q)  +  (1 − q) · 0
//                  = X
//
// The survivors are scaled up by exactly the factor that compensates for the
// ones that were killed, so the expectation is unchanged. It is not an
// approximation, a heuristic, or a quality setting. It is the same estimator
// with some of its work replaced by a coin.
//
// What it costs is variance. A path that survives carries `1/q` times the
// weight it would have, so the spread of the estimates grows even though
// their mean does not — which is the trade the whole technique is: bias is
// exchanged for noise, and noise is the one this project can measure and
// drive down with samples.
//
// ── Choosing q, and when to start ────────────────────────────────────────
//
// `q` is the throughput's largest component. A path that has already lost
// most of its light is cheap to kill and expensive to keep; a path still
// carrying nearly all of it should almost certainly continue. Using the
// throughput itself makes `q` close to 1 exactly when the path matters, and
// the `1/q` scaling then barely changes anything.
//
// It does not start at the first bounce, and the reason is the variance
// above. Near the eye the throughput is close to 1, so `q` is close to 1 and
// the roulette is not killing much anyway — but the paths it *does* kill are
// the ones contributing most to the pixel, and each survivor's `1/q` boost
// lands on the largest terms in the sum. Delaying it a few bounces costs
// almost nothing in path length and removes the worst of that.
//
// Three bounces, which is a choice and not a derivation. `converge` in v0.5
// will confirm what is asserted here: that turning roulette on changes the
// noise but not the answer, and does not change the N^-½ slope.
//
// ── What it actually trades ──────────────────────────────────────────────
//
// The same cavity, 10⁵ paths, with the roulette on and with it disabled and
// nothing else changed:
//
//      rho    estimate    sample sd    seconds / 10⁵ paths
//      ────────────────────────────────────────────────────
//      on      0.5   1.997040     0.586          0.05
//      off     0.5   2.000000     2.1e-08        3.27
//      on      0.9   9.952060     8.97           0.15
//      off     0.9  10.000000     0              3.28
//
// Sixty-five times faster at rho = 0.5 and twenty-two times at rho = 0.9,
// and the price is written in the third column: an estimator that had no
// variance at all now has a standard deviation of 0.59 and of 8.97.
//
// That is the trade stated as plainly as it can be. Without roulette every
// path in a closed box runs to the depth limit — 256 bounces, whatever the
// albedo — and returns the same number. With it, a path at rho = 0.9 lasts
// about thirteen bounces and returns a number that is sometimes far too big,
// scaled up by the reciprocal of the probability it survived.
//
// And the mean does not move, which is the whole claim. Twelve independent
// batches of 10⁵ paths, each scored as how many standard errors it lands from
// the exact answer:
//
//      rho = 0.5    mean z = −0.066
//      rho = 0.9    mean z = +0.235
//
// against an expected spread of ±0.289 for twelve batches. No detectable
// bias at either albedo. (The first pair of runs both landed about 1.7
// standard errors low, which looked like something and was not; two numbers
// are not evidence of a direction.)

// ── Measured ─────────────────────────────────────────────────────────────
//
// A closed cavity, every wall emitting `Le` and reflecting `rho`. The
// radiance inside is isotropic and satisfies `L = Le + rho·L`, so there is an
// exact answer to check against: `L = Le / (1 − rho)`. 200 000 paths from the
// centre in uniformly random directions, `Le = 1`:
//
//      rho      exact       estimated      relative error
//      0.00     1.0000       1.000000      0
//      0.25     1.3333       1.333333      2.61e-12
//      0.50     2.0000       2.000000      1.11e-16
//      0.75     4.0000       4.000000      1.11e-16
//      0.90    10.0000      10.000000      3.05e-13
//
// Those are not small errors from a lot of samples. They are *no* error, and
// the reason is the one `lambert.hpp` measured: cosine-sampling a Lambertian
// makes `f · cos / pdf` exactly `rho` for every draw, so in a cavity with
// uniform emission every path returns the same geometric series and the
// estimator has no variance at all.
//
// Which is worth saying plainly, because it means this test proves the
// transport is *right* and proves nothing whatever about how it behaves in
// the presence of noise. The first noisy image is item 0040, and the
// instrument that measures noise properly is v0.5's.
//
// The residual at `rho = 0.9` is the depth limit, and it is the bias this
// section warned about arriving on cue: a path truncated at 256 bounces has
// discarded `rho^256` of its contribution, which for 0.9 is 1.9e-12 — the
// same size as the error observed. At 0.25 it is 1e-154, and what is left
// there is ordinary floating-point accumulation.

#pragma once

#include <cmath>

#include <render/basis.hpp>
#include <render/ray.hpp>
#include <render/sampler.hpp>
#include <render/scene.hpp>
#include <render/spectrum.hpp>
#include <render/vec.hpp>
#include <render/waechter.hpp>

namespace render {

// A diagnostic limit. See above: not a number of bounces light is allowed.
inline constexpr int default_max_depth = 256;

// How many bounces before the roulette starts. A choice, and the header says
// why it is not zero.
inline constexpr int roulette_start_depth = 3;

// The largest of a spectrum's components — how much of this path's light is
// left, taking the most optimistic wavelength, which is the right one to ask
// about because killing a path kills it at every wavelength at once.
inline double largest_component(const Reflectance& r) {
    double largest = r[0];
    for (int i = 1; i < spectral_samples; ++i) largest = std::fmax(largest, r[i]);
    return largest;
}

// What a single path contributes, following it until it escapes, is absorbed,
// or hits the diagnostic limit.
//
// The ray is taken by value because the loop advances it; that is the
// iteration, and handing it back to the caller modified would be a worse lie
// than copying six doubles.
inline Radiance radiance(const Scene& scene,
                         Ray ray,
                         Sampler& sampler,
                         int max_depth = default_max_depth) {
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
        carried += throughput * emitted(*hit, wo);

        const Basis frame = hit->frame();
        const auto [u, v] = sampler.next2();
        const BsdfSample scattered = sample(hit->surface->bsdf, frame.to_local(wo), u, v);

        // Absorbed, or the surface refused this direction. Multiplying by
        // zero and continuing would give the same answer and cost the rest of
        // the loop.
        if (scattered.is_black()) break;

        // ── The estimator ────────────────────────────────────────────────
        //
        // Written as the ratio, in full, at the point of use. House rule 3,
        // and item 0032 for the measurement of what refusing to cancel it
        // costs, which is nothing.
        const double cos_theta_i = std::fabs(scattered.wi.z);
        throughput = throughput * (scattered.f * (cos_theta_i / scattered.pdf));

        // ── Russian roulette ─────────────────────────────────────────────
        //
        // Unbiased because the survivors are divided by the probability that
        // they survived. The division is written out for the same reason
        // every other division by a density in this project is.
        if (depth >= roulette_start_depth) {
            const double survival = std::fmin(1.0, largest_component(throughput));
            if (survival <= 0.0) break;
            if (sampler.next() >= survival) break;
            throughput = throughput * (1.0 / survival);
        }

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
