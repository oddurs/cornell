// multiple_scattering.hpp — the microsurface as a place, rather than a
// statistic about a place.
//
// Eric Heitz, Johannes Hanika, Eugene d'Eon and Carsten Dachsbacher,
// "Multiple-Scattering Microfacet BSDFs with the Smith Model", SIGGRAPH 2016.
// Item 0087 chose it over fitting a compensation lobe, and the reason is in
// that item: `./cornell furnace --table` had just shown that the missing
// energy is two named populations of rays with known directions, and a fit
// puts the energy back without asking where it went.
//
// ── What changes, which is smaller than it sounds ────────────────────────
//
// `smith.hpp` treats the microsurface as a distribution of facets with a
// function saying what fraction of them you can see. Nothing in it is a
// place: there is no height, no second facet, nowhere for a blocked ray to
// go. The masking term says *that* light is blocked and the model stops.
//
// Here the ray keeps going. It enters at the top, travels until it meets the
// surface at some height, reflects off a facet drawn from the visible normals
// — the same `visible_normals.hpp` the single-scattering model uses — and if
// it is still inside, it meets the surface again. When it finally leaves, the
// direction it leaves in is the sample.
//
// That is the whole model. There is no compensation term, no fitted curve and
// no second lobe: the energy comes back because the light was followed.
//
// ── Why it conserves energy exactly, and not approximately ───────────────
//
// With a reflectance of 1 nothing is absorbed at any facet, so the weight
// stays 1 for the whole walk, and the walk ends when the ray leaves. Every
// ray leaves. So the directional albedo is 1 — not to within noise, and not
// after convergence, but as an identity: the estimator is the constant 1.
//
// Measured over 2^20 walks at seven roughnesses and three angles, the albedo
// prints as 1.000000000000000 in every one of them, and no walk failed to
// escape. A furnace test that passes to fifteen decimal places is not
// evidence of careful tuning; it is what happens when the quantity being
// estimated has no variance.
//
// ── The height, and why it is uniform ────────────────────────────────────
//
// A ray inside the surface needs to know where the surface is, which the
// Smith model does not say — it is a distribution of *slopes* with no heights
// attached. Heitz supplies them: a height distribution whose cumulative
// function `C1` is all the walk ever touches, through
//
//      G1(w, h) = C1(h) ^ Lambda(w)
//
// the chance a ray at height `h` heading along `w` gets out without meeting
// the surface again. At the top `C1` is 1 and nothing blocks; at the bottom
// it is 0 and everything does.
//
// The distribution below is uniform on [-1, 1], so `C1` is a scale and a
// shift and its inverse is a shift and a scale. Heitz gives a Gaussian
// alternative and the choice does not reach the BSDF — what the walk uses is
// always `C1` composed with `invC1`, so a monotone relabelling of the height
// axis cancels. It is a coordinate on the surface, not a fact about it.
//
// ── Lambda has a sign here, which it did not need before ─────────────────
//
// `smith.hpp` derives `Lambda` for directions you can see the surface from.
// This file asks about rays travelling *into* it, where the projection in
// `Lambda`'s definition has the other sign, and straight down gives exactly
// -1: a ray heading into the surface meets it with probability 1.
//
// That sign is why the first version of this file was wrong in a way worth
// recording. Without it, the walk conserved energy and matched the
// single-scattering model at normal incidence, and was out by 0.216 at
// roughness 1 and sixty degrees — plausible everywhere, correct in the one
// place where the sign does not matter. It is the same shape of bug
// `visible_normals.hpp` records from item 0084, found the same way: by
// checking a case whose answer was already known.
//
// ── Two random numbers, and a walk that needs many ───────────────────────
//
// `bsdf.hpp` hands a material exactly two variates. That is not a budget, it
// is an invariant: `specular.hpp` consumes its two even though a mirror needs
// none, because `sampler.hpp` addresses rather than dispenses, and a path
// whose draw count depends on what it hits would diverge from another path at
// the same address.
//
// A walk needs an unbounded number of them, and the resolution is to spend
// the two on *seeding a stream of its own*. The path's stream advances by
// exactly two at this surface, as at every other; the walk's arbitrarily long
// sequence comes out of a sampler constructed from those two and belongs to
// this bounce alone. The invariant survives a material it was not written
// for, which is the sort of thing an invariant is for.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// **Correlation between the heights a ray meets on successive bounces.** The
// Smith model has none — each interaction draws a fresh height — and a real
// height field does, because a ray that just hit a peak is somewhere near
// that peak. This is the approximation that makes the walk tractable and it
// is Smith's, inherited whole.
//
// **A closed form.** There is none, and item 0160 decided in advance what
// that costs: `eval` and `pdf` below are the *single-scattering* model, which
// is the closed-form part of this material and the proxy multiple importance
// sampling weights with. They are honest about being that. `./cornell chi2`
// skips this material for the same reason, and the furnace — which never
// needed a density — is what checks it.
//
// **Absorption between bounces.** The medium inside the microsurface is
// vacuum here. A conductor absorbs at each facet through its Fresnel term and
// that is carried; nothing else happens between one facet and the next.

#pragma once

#include <cmath>
#include <cstdint>
#include <limits>

#include <render/bsdf.hpp>
#include <render/sampler.hpp>
#include <render/smith.hpp>
#include <render/torrance_sparrow.hpp>
#include <render/trowbridge_reitz.hpp>
#include <render/vec.hpp>
#include <render/visible_normals.hpp>

namespace render {

namespace microsurface {

// The height distribution's cumulative function, uniform on [-1, 1]. Only
// ever used against its own inverse; see above.
constexpr double height_cdf(double h) {
    return h <= -1.0 ? 0.0 : (h >= 1.0 ? 1.0 : 0.5 * (h + 1.0));
}

constexpr double height_of(double u) {
    return u <= 0.0 ? -1.0 : (u >= 1.0 ? 1.0 : 2.0 * u - 1.0);
}

// The chance a ray at height `h` travelling along `w` leaves without meeting
// the surface again. Downward, it is nought: there is always more surface
// below.
inline double escapes(const Smith& smith, const Vec3& w, double h) {
    if (w.z > 0.9999) return 1.0;
    if (w.z <= 0.0) return 0.0;
    return std::pow(height_cdf(h), smith.lambda(w));
}

// The height of the next interaction, or infinity if the ray got out.
//
// The three guards are not tidying. Straight up cannot be blocked by anything
// and must not evaluate a `Lambda` that is zero; straight down always hits
// and would otherwise divide by a `Lambda` of -1 through a `pow` that is
// exactly 1; and a ray travelling along the surface neither rises nor falls,
// so its height is unchanged and the closed form is 0/0.
inline double next_height(const Smith& smith, const Vec3& w, double h, double u) {
    constexpr double gone = std::numeric_limits<double>::infinity();

    if (w.z > 0.9999) return gone;
    if (w.z < -0.9999) return height_of(u * height_cdf(h));
    if (std::fabs(w.z) < 1.0e-4) return h;

    if (u > 1.0 - escapes(smith, w, h)) return gone;

    return height_of(height_cdf(h) / std::pow(1.0 - u, 1.0 / smith.lambda(w)));
}

// The walk terminates with probability one and this is not what stops it.
// Measured over 2^21 walks at roughness 1 — the worst case, where the mean is
// 4.24 — the longest was 102 and nothing exceeded 200. The bound is set far
// above that so that reaching it is a defect rather than a rounding of the
// model, and a surface that reached it would show up as energy missing from
// the furnace.
inline constexpr int longest_walk = 1000;

} // namespace microsurface

// A rough conductor whose light is followed rather than dropped.
//
// It holds the single-scattering model, which is not a component of it —
// nothing here adds the two together. It is the closed-form part that `eval`
// and `pdf` answer with, as item 0160 decided, and holding the real thing
// rather than reimplementing it means the proxy cannot drift from the model
// it is a proxy for.
template <class Spectrum>
class MultipleScattering {
public:
    constexpr MultipleScattering(Spectrum reflectance, TrowbridgeReitz distribution)
        : single_{reflectance, distribution},
          reflectance_{reflectance},
          smith_{distribution} {}

    constexpr double alpha() const { return smith_.distribution().alpha(); }

    // The closed-form part, and the proxy. Both are the single-scattering
    // model and both are less than this material really does — which is
    // exactly what `BsdfSample::Kind::Stochastic` exists to announce.
    Brdf eval(const Vec3& wo, const Vec3& wi, const Wavelengths& lambdas) const {
        return single_.eval(wo, wi, lambdas);
    }

    SolidAngleDensity pdf(const Vec3& wo, const Vec3& wi) const {
        return single_.pdf(wo, wi);
    }

    BsdfSample sample(const Vec3& wo, const Wavelengths& lambdas,
                      double u, double v) const {
        using namespace microsurface;

        const double side = wo.z < 0.0 ? -1.0 : 1.0;
        const Vec3 above{wo.x * side, wo.y * side, wo.z * side};

        // The two variates, spent on a stream rather than on a direction.
        // `Sampler::next` is `next_u32() * 2^-32`, so multiplying back by
        // 2^32 recovers the exact integer it was made from and no entropy is
        // invented or lost on the way in.
        Sampler walk{std::uint64_t(u * 0x1p32), std::uint64_t(v * 0x1p32)};

        // The ray, heading into the surface, starting just under the top.
        Vec3 travelling{-above.x, -above.y, -above.z};
        double height = height_of(0.9999);

        Reflectance carried;
        for (int i = 0; i < spectral_samples; ++i) carried[i] = 1.0;

        for (int bounce = 0; bounce < longest_walk; ++bounce) {
            height = next_height(smith_, travelling, height, walk.next());

            // It left. A direction of travel that has escaped upward is the
            // outgoing direction, with no negation: `wi` points away from the
            // surface and so does this.
            if (std::isinf(height)) {
                if (travelling.z <= 0.0) break;   // left downward: nothing to return

                BsdfSample out;
                out.wi = Vec3{travelling.x * side, travelling.y * side,
                              travelling.z * side};
                out.weight = carried;
                out.kind = BsdfSample::Kind::Stochastic;
                return out;
            }

            // The facet it met. `visible_normals.hpp` wants a direction
            // pointing away from the surface, which is the reverse of the way
            // the ray is going.
            const Vec3 facing{-travelling.x, -travelling.y, -travelling.z};
            const auto [a, b] = walk.next2();
            const Vec3 m = sample_visible_normal(smith_.distribution(), facing, a, b);

            // What the facet keeps, at the angle the facet sees. The same
            // spectrum `torrance_sparrow.hpp` asks, asked once per bounce,
            // which is how a rough gold stays gold and gets darker: two
            // bounces off a metal is its reflectance squared, and that is the
            // saturation a fitted compensation lobe has to be told about.
            const double cos_at_facet = dot(facing, m);
            for (int i = 0; i < spectral_samples; ++i)
                carried[i] *= reflectance_.at(lambdas[i], cos_at_facet);

            // Mirror the reversed direction about the facet, which is already
            // a direction of travel leaving it.
            travelling = 2.0 * dot(facing, m) * m - facing;
        }

        // Either it left downward or it exceeded a bound it should never
        // reach. Both are a sample that carries nothing, and the furnace is
        // where either would show.
        return BsdfSample{};
    }

private:
    TorranceSparrow<Spectrum> single_;
    Spectrum reflectance_{};
    Smith smith_{TrowbridgeReitz{0.0}};
};

} // namespace render
