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
// It also assumes light travels unchanged between surfaces, which is the
// subject of item 0041 and the next thing this file has to admit.
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
// the integral, and it is written as the ratio it is. For a Lambertian
// surface sampled cosine-weighted, every factor in it is known in closed
// form — `f` is `rho/pi`, `cos/pdf` is `pi` — and the whole thing collapses
// to `rho`. Item 0032 is about why this file does not write `rho`.
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
