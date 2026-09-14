// lambert.hpp — Photometria, Augsburg, 1760.
//
// The oldest result in this project by a hundred and twenty years, and the
// one most often typed into a renderer as `0.31831`.
//
// A Lambertian surface scatters light equally in every direction, whatever
// direction it arrived from. That sentence is the entire model, and the
// interesting part is that it forces a constant on you — there is exactly one
// value the BRDF can have, and nobody gets to choose it.
//
// ── Where the pi comes from ──────────────────────────────────────────────
//
// Energy conservation. A surface that reflects *all* of the light falling on
// it, and scatters it uniformly, must send out exactly as much as it took in.
// Write the BRDF as an unknown constant `k` and integrate what leaves over
// the hemisphere, weighted — as the rendering equation weights it — by the
// cosine, because a direction near the horizon carries away less through a
// given solid angle than one overhead:
//
//      ∫ k cos(theta) dw = 1
//
// So `k` is the reciprocal of the integral of the cosine over the hemisphere,
// and that integral is two lines. In spherical coordinates the solid angle
// element is `dw = sin(theta) d(theta) d(phi)`:
//
//      ∫ cos(theta) dw  =  ∫₀^2pi ∫₀^(pi/2) cos(theta) sin(theta) d(theta) d(phi)
//
//                       =  2pi · ∫₀^(pi/2) cos(theta) sin(theta) d(theta)
//
//                       =  2pi · [ sin²(theta) / 2 ]₀^(pi/2)
//
//                       =  2pi · (1/2 - 0)
//
//                       =  pi
//
// Therefore `k = 1/pi`, and the BRDF of a surface with albedo `rho` is
// `rho/pi`. The pi is not a convention, a normalisation anybody chose, or a
// fudge factor: it is the projected solid angle of a hemisphere, it has units
// of steradians, and the division by it is the surface promising not to emit
// light it never received.
//
// The quantity is spelled `projected_hemisphere` below rather than `pi`,
// because what the code is dividing by is the integral, not the number the
// integral happens to equal.
//
// `warp.hpp` arrives at the same pi from the opposite direction — the area of
// the unit disc, in Malley's construction — and the two agreeing is the
// check that neither of them is a typo.
//
// ── Measured ─────────────────────────────────────────────────────────────
//
// The integral above, done numerically by Simpson's rule over 10⁶ panels
// rather than trusted:
//
//      quadrature   3.14159265358982
//      pi           3.14159265358979
//
// Agreeing to thirteen places, the disagreement being the quadrature rather
// than the derivation.
//
// The furnace, in miniature, which is what v0.5 will build an instrument
// for: a Lambertian with albedo 1 must return exactly the light it is given.
// Integrating `f · cos(theta)` over the hemisphere gives 1.0000000000000173,
// which is the same quadrature error arriving again.
//
// And the estimator, which is the thing that actually runs. Draw a direction
// from `sample`, form `f · cos(theta) / pdf`, and average over 10⁷ draws with
// albedo 1:
//
//      mean 1, min 1, max 1, sample standard deviation 0
//
// Not "1 to within noise". Every single one of ten million samples returned
// exactly 1.0, because for this BRDF the cosine weighting matches the
// integrand exactly and the ratio collapses to the albedo before any
// arithmetic is done. It survives the round trip through floating point
// because `1.0/si::pi` and `si::inv_pi` are the same double — checked, not
// assumed — so the pi that `eval` divides by is bit-for-bit the pi that
// `cosine_hemisphere_pdf` multiplies by.
//
// That is the best advertisement importance sampling will ever get, and it is
// also a warning: it is true for this one material and false for every
// material after it. Item 0032 is about not writing the collapsed version
// down.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// That no real matte surface is Lambertian.
//
// Paint, paper, plaster and unfinished plywood all retroreflect: seen from
// the direction the light is coming from they are brighter than the cosine
// law says, and at grazing angles they are brighter still, because a rough
// surface's pits shadow each other in a way that depends on where you are
// standing. Oren and Nayar modelled it in 1994 by treating the surface as a
// field of V-shaped grooves, and the correction is largest exactly where this
// project will be looking: the corners of a box.
//
// It is deliberately absent. The claim is that Cornell's painted walls are
// close enough to Lambertian that the error sits below their measurement,
// and that is a claim to be *checked in v1.0 against the photograph*, not
// asserted here. Item 0061 is the admission; if the comparison fails, this
// file is the first suspect.

#pragma once

#include <cmath>
#include <render/bsdf.hpp>
#include <render/si.hpp>
#include <render/spectrum.hpp>
#include <render/warp.hpp>

namespace render {

// The integral of the cosine over the hemisphere — the projected solid angle
// of a hemisphere, in steradians. Derived above. It is written as what it is
// rather than as what it equals, so that the division below reads as the
// physics it is doing.
inline constexpr double projected_hemisphere = si::pi;

class Lambert {
public:
    constexpr explicit Lambert(const Reflectance& albedo) : albedo_{albedo} {}

    // The BRDF. Constant, by definition of the model, and equal to the albedo
    // divided by the integral that makes it conserve energy.
    //
    // Nothing below the horizon: this is a reflector, so a direction on the
    // other side of the surface receives nothing. Returning zero rather than
    // refusing is deliberate — `wi` on the wrong side is a perfectly ordinary
    // question for the integrator to ask, and the answer is that no light
    // goes that way.
    constexpr Reflectance eval(const Vec3& wo, const Vec3& wi) const {
        if (!same_hemisphere(wo, wi)) return Reflectance{};
        return albedo_ * (1.0 / projected_hemisphere);
    }

    // The density `sample` would have drawn this direction with. Separate
    // from `sample` on purpose; `bsdf.hpp` says why at length.
    double pdf(const Vec3& wo, const Vec3& wi) const {
        if (!same_hemisphere(wo, wi)) return 0.0;
        return cosine_hemisphere_pdf(abs_cos_theta(wi));
    }

    // Draw a direction, and hand back the density it was drawn with.
    //
    // Cosine-weighted, which for this BRDF is exactly proportional to the
    // integrand — see the measured note above about the variance being zero.
    // The flip is for a `wo` arriving from below, which happens when a path
    // reaches a surface from the inside; the hemisphere sampled is always the
    // one `wo` is in.
    BsdfSample sample(const Vec3& wo, double u, double v) const {
        DirectionSample drawn = cosine_hemisphere(u, v);
        if (wo.z < 0.0) drawn.direction.z = -drawn.direction.z;

        return BsdfSample{drawn.direction, eval(wo, drawn.direction), drawn.pdf};
    }

private:
    Reflectance albedo_{};
};

static_assert(BsdfModel<Lambert>,
              "Lambert must satisfy the three-method contract in bsdf.hpp");

} // namespace render
