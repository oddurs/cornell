// torrance_sparrow.hpp — a surface made of mirrors too small to see.
//
// Kenneth Torrance and Ephraim Sparrow, "Theory for Off-Specular Reflection
// From Roughened Surfaces", Journal of the Optical Society of America 57(9),
// 1967 — the same year and the same journal as Smith's shadowing function
// next door, by people who were measuring how light comes off roughened
// magnesium oxide rather than drawing pictures with it. Robert Cook and
// Torrance brought it into graphics in 1982, and the model is usually named
// for that paper; house rule 9 says the file takes the first name.
//
// It is three files joined and one line of its own:
//
//      f(wo, wi) = D(h) G₂(wo, wi, h) F(wo·h) / (4 cos(theta_o) cos(theta_i))
//
// `trowbridge_reitz.hpp` supplies `D`, `smith.hpp` supplies `G₂`,
// `fresnel.hpp` supplies `F` through whatever spectrum the caller handed in,
// and `h` is the one facet that could have done it.
//
// ── Why there is exactly one facet ───────────────────────────────────────
//
// Light arrives along `wi` and leaves along `wo`, and every facet is a
// perfect mirror. A mirror sends `wi` to `wo` only if its normal bisects
// them, so of all the facets on the surface, the ones that contribute are the
// ones pointing along
//
//      h = normalize(wo + wi)
//
// and there is no integral to do. That is the whole reason a microfacet model
// is cheap: the statistical surface has been reduced to a question about one
// direction, and `D(h)` answers how much of the surface is pointing there.
//
// ── Where the 4 comes from ───────────────────────────────────────────────
//
// It is the Jacobian of the reflection, and `visible_normals.hpp` derives it:
// the map from facet normals to outgoing directions doubles angles, so a
// solid angle of normals becomes `4 (wo·h)` times as much solid angle of
// directions. The `(wo·h)` of that cancels against one in the numerator, and
// the 4 is what is left. It is not a normalisation and nobody chose it.
//
// ── The one this file gets to say about the type system ──────────────────
//
// `spectrum.hpp` allows exactly one way to make a `Brdf`: a reflectance,
// spread over a solid angle. That was written for `lambert.hpp`, where the
// solid angle is the projected hemisphere and the sentence is obvious.
//
// Here the same sentence reads:
//
//      per_steradian(fresnel, 4 cos(theta_o) cos(theta_i) / (D G₂))
//
// The surface keeps `F` of the light — that is the reflectance, it is
// dimensionless, it is what Fresnel returns — and it spreads it over
// `4 cos cos / (D G₂)` steradians, which really is a solid angle: `D` is per
// steradian, `G₂` is a fraction, and the cosines are numbers. It is the
// effective width of the lobe, and it does what a lobe should: as the surface
// smooths, `D` at the mirror direction goes to infinity, the solid angle goes
// to zero, and the BRDF becomes the delta that `specular.hpp` handles instead.
//
// A microfacet BRDF is usually written as a fraction with four things on top
// and three underneath, and in that form it is impossible to see that the
// steradian comes from one place. Writing it as a spreading makes the units
// audible.
//
// ── The Fresnel angle is the one people get wrong ────────────────────────
//
// `F` is evaluated against `wo·h`, the angle at the *facet*, not `cos_theta_o`,
// the angle at the surface. The facet is the thing reflecting; the macroscopic
// normal is a statistical fiction that no photon ever met. Using the macro
// angle makes a rough metal too dark head-on and kills the grazing brightening
// that is most of what makes metal look like metal — and it is an easy
// substitution to make, because both are cosines in scope with plausible names.
//
// ── Measured ─────────────────────────────────────────────────────────────
//
// The estimator this BRDF forms with its own sampler collapses, and
// `./cornell chi2` checks that it collapses to the right thing. Writing out
// `f cos_i / pdf` in full and cancelling gives
//
//      f cos_i / pdf = F G₂ / G₁
//
// — no `D`, no cosines, no 4 — and over 166,025 draws at three roughnesses
// the ratio the code actually computes differs from that by at most 5.55e-16,
// which is one ulp. No draw weighs more than 1.000000, because `G₂ ≤ G₁`: the
// facets that both the eye and the light can see are a subset of the ones the
// eye can see.
//
// House rule 3 is the reason the divides are written anyway. The collapsed
// form is the same image and cannot be given multiple importance sampling in
// v0.8 without reopening every material, because there would be no `pdf` left
// to ask.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// **The light that bounces twice.** `G₂` says a facet is masked, and drops
// the light. That light was not absorbed — it hit the facet doing the masking
// and went on — so this BRDF loses energy, and loses more of it the rougher
// the surface gets. It is the defining flaw of single-scattering microfacet
// models, it is in almost every renderer ever shipped, and it is item 0085
// (measuring it) and item 0086 (what to do). This file is written knowing it
// will fail the furnace.
//
// **Transmission.** A rough dielectric refracts through the same facets and
// has a second lobe, which is item 0109 and v0.9's business. This is a
// reflector, as `specular.hpp` is.
//
// **Any facet smaller than a wavelength**, which is the whole model's limit
// and `trowbridge_reitz.hpp` says so at more length.

#pragma once

#include <cmath>

#include <render/bsdf.hpp>
#include <render/smith.hpp>
#include <render/spectrum.hpp>
#include <render/trowbridge_reitz.hpp>
#include <render/vec.hpp>
#include <render/visible_normals.hpp>

namespace render {

// A rough reflector whose reflectance is a spectrum of wavelength and angle.
//
// The same `Spectrum` shape `specular.hpp` takes, deliberately: a mirror and
// a rough mirror differ in how they scatter and not in what they are made of,
// so `Conductor` and its rough counterpart ask the same table the same
// question. `FlatReflectance` makes the furnace's white one.
template <class Spectrum>
class TorranceSparrow {
public:
    constexpr TorranceSparrow(Spectrum reflectance, TrowbridgeReitz distribution)
        : reflectance_{reflectance}, smith_{distribution} {}

    constexpr double alpha() const { return smith_.distribution().alpha(); }

    Brdf eval(const Vec3& wo, const Vec3& wi, const Wavelengths& lambdas) const {
        if (!same_hemisphere(wo, wi)) return Brdf{};

        // Everything below assumes the upper hemisphere. A path arriving from
        // behind meets the same microsurface from the other side, so the
        // whole problem is mirrored rather than special-cased — the same
        // move `lambert.hpp` and `visible_normals.hpp` make.
        const double side = wo.z < 0.0 ? -1.0 : 1.0;
        const Vec3 o{wo.x * side, wo.y * side, wo.z * side};
        const Vec3 i{wi.x * side, wi.y * side, wi.z * side};

        if (o.z <= 0.0 || i.z <= 0.0) return Brdf{};

        // The facet that could have done it. Exactly opposite directions have
        // no bisector, which is a measure-zero case that arrives anyway when
        // a path grazes.
        const Vec3 sum = o + i;
        if (length_squared(sum) == 0.0) return Brdf{};
        const Vec3 h = normalize(sum).vec();

        const double d = smith_.distribution().d(h);
        const double g = smith_.masking_shadowing(o, i, h);
        if (d <= 0.0 || g <= 0.0) return Brdf{};

        // Fresnel at the facet, not at the surface. See above.
        const double cos_at_facet = dot(o, h);

        Reflectance fresnel;
        for (int k = 0; k < spectral_samples; ++k)
            fresnel[k] = reflectance_.at(lambdas[k], cos_at_facet);

        // And the spreading, which is the whole model in one line.
        const double steradians = 4.0 * o.z * i.z / (d * g);
        return per_steradian(fresnel, steradians);
    }

    // The density `sample` would have drawn this direction with: the
    // visible-normal density at the facet, carried through the reflection by
    // its Jacobian. Both halves are `visible_normals.hpp`'s.
    SolidAngleDensity pdf(const Vec3& wo, const Vec3& wi) const {
        if (!same_hemisphere(wo, wi)) return SolidAngleDensity{};

        const double side = wo.z < 0.0 ? -1.0 : 1.0;
        const Vec3 o{wo.x * side, wo.y * side, wo.z * side};
        const Vec3 i{wi.x * side, wi.y * side, wi.z * side};

        if (o.z <= 0.0 || i.z <= 0.0) return SolidAngleDensity{};

        const Vec3 sum = o + i;
        if (length_squared(sum) == 0.0) return SolidAngleDensity{};
        const Vec3 h = normalize(sum).vec();

        const double facing = dot(o, h);
        if (facing <= 0.0) return SolidAngleDensity{};

        // House rule 3: the Jacobian is written out where it applies rather
        // than folded into the density function next door, because the
        // density over normals and the density over directions are different
        // quantities and only one of them is what an estimator divides by.
        return SolidAngleDensity{visible_normal_density(smith_, o, h) / (4.0 * facing)};
    }

    BsdfSample sample(const Vec3& wo, const Wavelengths& lambdas,
                      double u, double v) const {
        const Vec3 m = sample_visible_normal(smith_.distribution(), wo, u, v);

        // Reflect about the facet. This is `specular.hpp`'s mirror written in
        // a frame where the normal is `m` instead of +z, and it is the only
        // arithmetic in this file that is about geometry rather than physics.
        const Vec3 wi = 2.0 * dot(wo, m) * m - wo;

        // A facet whose mirror direction points into the surface. It is a
        // real facet and it really reflected the light downwards, where this
        // model has nothing for it to hit; the honest sample is a black one,
        // and `pdf` returns zero for the same direction, so the two agree
        // about a draw that carries nothing.
        if (!same_hemisphere(wo, wi)) return BsdfSample{};

        return BsdfSample{wi, eval(wo, wi, lambdas), pdf(wo, wi)};
    }

private:
    Spectrum reflectance_{};
    Smith smith_{TrowbridgeReitz{0.0}};
};

} // namespace render
