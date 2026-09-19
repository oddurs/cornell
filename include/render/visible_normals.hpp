// visible_normals.hpp — draw a facet you can actually see.
//
// Eric Heitz, "Sampling the GGX Distribution of Visible Normals", Journal of
// Computer Graphics Techniques 7(4), 2018. Fifteen lines, no rejection loop,
// no approximation, and a density that is exact — which is the property that
// matters here, because `./cornell chi2` was built two milestones ago
// precisely to catch a density that is nearly right.
//
// ── What is wrong with sampling D ────────────────────────────────────────
//
// `trowbridge_reitz.hpp` says how the facets are oriented, so the obvious way
// to pick one is to draw from `D`. It works, and it wastes most of its
// samples.
//
// A facet drawn from `D` knows nothing about where the viewer is. At a
// shallow angle most of the surface's facets are turned away from `wo`
// entirely — `smith.hpp`'s `A⁻`, which at alpha 1 and 75 degrees is larger
// than the whole projected macrosurface — and every one of those draws is
// discarded. The ones that remain are then weighted by `G₁ ⟨wo·m⟩`, which
// varies by orders of magnitude across what is left, so the surviving samples
// are not merely fewer but badly unequal.
//
// The fix is to sample the distribution the viewer actually sees:
//
//      D_v(m) = G₁(wo, m) ⟨wo·m⟩ D(m) / cos(theta_o)
//
// A facet's share is its orientation times how much of it faces `wo` times
// whether it is hidden — which is exactly the weight it was going to be given
// afterwards, moved into the draw.
//
// ── Why that is a density at all, which is not obvious ───────────────────
//
// It integrates to one. Not approximately, and not by a normalisation
// constant somebody computed: the denominator is `cos(theta_o)`, and
//
//      ∫ G₁(wo, m) ⟨wo·m⟩ D(m) dm = cos(theta_o)
//
// is the covering requirement that `smith.hpp` derived `Lambda` from. The
// masking function was fixed by demanding that the microsurface cover the
// macrosurface from every direction; that demand, read the other way round,
// says the visible-normal distribution is normalised. One statement, used
// twice, and the second use is free.
//
// This is the payment for having derived `G` instead of picking it. A masking
// function chosen from a menu does not satisfy the requirement, so `D_v` built
// on it is not a density, so its "pdf" is wrong by a factor that varies with
// angle — and a renderer in that state still produces an image, slightly and
// invisibly wrong, forever.
//
// ── The geometry, which is the part worth reading ────────────────────────
//
// Heitz's routine is short enough to copy without understanding, so here is
// what it is doing.
//
// **Start with the sphere.** At `alpha = 1` the Trowbridge-Reitz microsurface
// is a surface of spheres, and `D` is `1/pi` in every direction. Put that in
// the definition above, with `Lambda = (1/cos(theta_o) - 1)/2` from
// `smith.hpp`, and almost everything cancels:
//
//      D_v(m) = 2 ⟨wo·m⟩ / (pi (1 + cos(theta_o)))
//
// A clipped cosine about `wo`. And a density proportional to `⟨wo·m⟩` over a
// set of directions is exactly *uniform over that set's projected area* — so
// sampling the visible normals of a sphere means picking a point uniformly on
// the silhouette of the part of it you can see.
//
// **What that silhouette is.** The facets exist only where `m·n > 0`, and are
// visible only where `wo·m > 0`. Project that lune along `wo`. The outer edge
// — the sphere's own silhouette — is a circle of radius 1. The inner edge is
// the equator `m·n = 0`, which is also a circle, seen at an angle, so it
// projects to an *ellipse* with one semi-axis foreshortened to
// `cos(theta_o)`. The shape is a half-disc glued to a half-ellipse.
//
// So: take a uniform point on the unit disc and squash the half of it that
// corresponds to the equator. With `s = (1 + cos(theta_o))/2`,
//
//      t2  ->  (1 - s) sqrt(1 - t1²) + s t2
//
// which is the identity when `wo` is the normal (`s = 1`, the whole disc is
// visible) and folds the disc onto its upper half when `wo` is grazing
// (`s = 1/2`, exactly half the sphere's facets face you). Then lift the point
// onto the sphere along `wo`, and that is the normal.
//
// **Then un-squash the sphere.** A general `alpha` is that same sphere
// flattened by `alpha` in the tangent plane — which is where Trowbridge and
// Reitz's ellipsoid came from in the first place. So the routine flattens the
// problem rather than solving a new one: stretch `wo` into the frame where
// the ellipsoid is a sphere, do the construction above, and stretch the
// resulting normal back.
//
//      stretch     wo -> normalize(alpha wo.x, alpha wo.y, wo.z)
//      unstretch   m  -> normalize(alpha m.x,  alpha m.y,  m.z)
//
// Both multiply by `alpha`, which looks like a mistake and is not. The first
// is a direction and the second is a normal, and under a linear map those
// transform by the matrix and by its inverse transpose respectively — so the
// map that takes the ellipsoid to the sphere carries `wo` one way and carries
// normals back the other, and for `diag(1/alpha, 1/alpha, 1)` the two spell
// the same thing.
//
// ── The density the estimator needs ──────────────────────────────────────
//
// `D_v` is a density over facet normals, and a BRDF samples a *direction*.
// The reflection about `m` maps one to the other, and it is not
// measure-preserving: a small cone of normals about `m` becomes a cone of
// directions twice as wide, and the Jacobian is
//
//      dwi / dm = 4 (wo·m)
//
// — the 4 that appears in the denominator of every microfacet BRDF ever
// written, arriving here as a change of variables rather than as a constant
// to memorise. So the density over directions is
//
//      pdf(wi) = D_v(m) / (4 (wo·m))
//
// and that expression, written out, is `G₁(wo, m) D(m) / (4 cos(theta_o))`,
// which is the integrand of the weak white furnace test on `./cornell
// verify`'s masking section. It integrates to one over the whole sphere of
// `wi`. Over the hemisphere it does not, and the shortfall is not an error:
// it is the facets whose mirror direction points into the surface, which are
// real facets that really reflect light into the ground. They arrive here as
// a sample with no light in it and the density accounts for them exactly.
//
// ── Measured ─────────────────────────────────────────────────────────────
//
// `./cornell chi2`, a million draws per row against the density integrated
// over each histogram cell by a quadrature that never calls the sampler:
//
//      alpha    theta_o      chi2/dof        p
//      0.001          0        0.9165   0.9732
//      0.001         45        0.9308   0.9439
//      0.001         80        0.9643   0.7891
//      0.010         45        0.9982   0.5100
//      0.100         45        0.9512   0.8616
//      1.000         45        0.9634   0.7577
//      1.000         80        0.9293   0.8888
//
// Those rows are histogrammed in the flattened frame, because at roughness
// 0.001 the lobe is ten thousand times narrower than a bin and no grid over
// the sphere can see it; `chi2.hpp` explains the stretch at length, including
// why its factor is deliberately not `1/alpha`. The same BSDF is also tested
// undisguised, in the directions a path really leaves along, from roughness
// 0.1 upwards where the grid has the resolution for it — nine more rows, the
// worst p of them 0.0843.
//
// The sampler and the density were wrong together once, and it is worth
// recording what caught it. The squash above has to be applied along the axis
// lying in the plane of `wo` and the normal, and `basis.hpp` returns a frame
// pointing wherever Duff's branch-free construction happens to point. With
// the wrong axis the routine still produced plausible normals in the right
// hemisphere, and the first three moments of the sampled normals disagreed
// with the same moments integrated against this density by forty percent at
// grazing incidence — while agreeing exactly at normal incidence, where the
// squash is the identity and any frame will do. A chi-squared would have
// caught it too. A picture would not have.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// **Anisotropy.** Heitz's routine takes two roughnesses and stretches the two
// tangent axes by different amounts; this one passes `alpha` twice. Item 0088
// is where the second one arrives, and it is a change to the two stretch
// lines rather than to the geometry above.
//
// **Sampling the light instead.** For a narrow lobe this is the best strategy
// there is; for a rough surface under a small bright lamp it is among the
// worst, and multiple importance sampling in v0.8 is what lets a renderer
// have both. That is the reason `bsdf.hpp` insisted on a separate `pdf` from
// the first material, and this is the first one where the two differ.

#pragma once

#include <cmath>

#include <render/si.hpp>
#include <render/smith.hpp>
#include <render/trowbridge_reitz.hpp>
#include <render/vec.hpp>
#include <render/warp.hpp>

namespace render {

// The visible-normal distribution: how much of the surface, as seen from
// `wo`, is facing along `m`.
//
// Returned as a plain double for the same reason `D` is — it is a density
// over facet normals, not over the directions a path travels in, and
// `density.hpp`'s type means the second thing. The conversion to a
// `SolidAngleDensity` happens where the reflection does, which is the only
// place it is meaningful.
inline double visible_normal_density(const Smith& smith, const Vec3& wo, const Vec3& m) {
    const double cos_theta_o = std::fabs(wo.z);
    if (cos_theta_o == 0.0) return 0.0;

    const double facing = dot(wo, m);
    if (facing <= 0.0) return 0.0;

    return smith.masking(wo, m) * facing * smith.distribution().d(m) / cos_theta_o;
}

// Draw a facet normal from that distribution. Heitz 2018.
//
// `wo` is in the local frame and may be in either hemisphere: a path that
// reaches a surface from behind is an ordinary event, and the microsurface it
// meets is the same one seen from the other side. The routine flips it, does
// the construction, and flips the answer back — which is the same trick
// `lambert.hpp` plays with a negated z, and for the same reason.
inline Vec3 sample_visible_normal(const TrowbridgeReitz& distribution,
                                  const Vec3& wo, double u, double v) {
    const double alpha = distribution.alpha();
    const double side = wo.z < 0.0 ? -1.0 : 1.0;
    const Vec3 facing{wo.x * side, wo.y * side, wo.z * side};

    // A smooth surface has one facet and it is the surface. The construction
    // below would divide by the length of a stretched vector that has gone to
    // zero in its tangential part, so the degenerate case is answered rather
    // than computed.
    if (alpha <= 0.0) return Vec3{0.0, 0.0, side};

    // Stretch, so that the ellipsoid is a sphere.
    const Unit stretched = normalize(Vec3{alpha * facing.x, alpha * facing.y, facing.z});

    // A frame about the stretched direction — and this one cannot come from
    // `basis.hpp`, which is the only place in the project that is true.
    //
    // Duff's construction returns *some* pair of perpendicular directions,
    // chosen for being branch-free rather than for pointing anywhere in
    // particular. The squash below is not symmetric: it foreshortens one axis
    // and leaves the other alone, because the ellipse it is making is the
    // equator seen at an angle, and that ellipse's short axis lies in the
    // plane containing `wo` and the normal. Applied along an arbitrary axis
    // it squashes the wrong direction.
    //
    // So `across` is built perpendicular to that plane and `along` lies in
    // it. The moments of the sampled normals disagreed with the density by
    // forty percent at grazing incidence until this was written this way,
    // and agreed exactly at normal incidence, where the squash is the
    // identity and any frame will do.
    const double tangential = stretched.x() * stretched.x()
                            + stretched.y() * stretched.y();
    const Vec3 across = tangential > 0.0
        ? Vec3{-stretched.y(), stretched.x(), 0.0} * (1.0 / std::sqrt(tangential))
        : Vec3{1.0, 0.0, 0.0};
    const Vec3 along = cross(stretched.vec(), across);

    // A uniform point on the unit disc, then the squash that turns the disc
    // into the silhouette of the visible lune. `concentric_disc` rather than
    // a polar map for the reason `warp.hpp` gives: the shapes stay square, so
    // a stratified sampler stays stratified through the mapping.
    const Vec3 disc = concentric_disc(u, v);
    const double t1 = disc.x;
    const double squash = 0.5 * (1.0 + stretched.z());
    const double t2 = (1.0 - squash) * std::sqrt(std::fmax(0.0, 1.0 - t1 * t1))
                    + squash * disc.y;

    // Lift onto the sphere along the stretched direction.
    const double lift = std::sqrt(std::fmax(0.0, 1.0 - t1 * t1 - t2 * t2));
    const Vec3 on_sphere = across * t1 + along * t2 + stretched.vec() * lift;

    // Unstretch. The clamp is the one place this can produce a normal below
    // the surface — `on_sphere.z` can be a few ulps negative at the silhouette
    // — and a facet pointing into the surface is not one.
    const Vec3 unstretched{alpha * on_sphere.x, alpha * on_sphere.y,
                           std::fmax(0.0, on_sphere.z)};

    const Vec3 m = normalize(unstretched).vec();
    return Vec3{m.x * side, m.y * side, m.z * side};
}

} // namespace render
