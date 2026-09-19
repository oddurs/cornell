// smith.hpp — what fraction of a rough surface you can actually see, and why
// nobody gets to pick it.
//
// Bruce Smith, "Geometrical shadowing of a random rough surface", IEEE
// Transactions on Antennas and Propagation 15(5), 1967, working on radar
// returns from rough ground two decades before graphics had a microfacet BRDF
// to put it in. Eric Heitz, "Understanding the Masking-Shadowing Function in
// Microfacet-Based BRDFs", Journal of Computer Graphics Techniques 3(2),
// 2014, is the paper that showed it was never a choice.
//
// ── The menu ─────────────────────────────────────────────────────────────
//
// Facets hide each other. Looking at a rough surface from a shallow angle,
// some of the facets pointing at you are behind other facets, and the
// fraction that survive is `G`. Every renderer written for pleasure picks one
// from a list — Smith, Cook-Torrance's V-cavity, the implicit one, Kelemen's
// — the way one picks a font, and the literature cheerfully reports that the
// difference is small and mostly matters at grazing angles.
//
// There is no list. Heitz's result is that `G` is *determined* by `D`, by one
// requirement with no freedom left in it, and the four functions above are
// not four options: one of them solves the requirement for this distribution
// and the others do not. The sheet puts a published, respectable, widely used
// masking function — Beckmann's, which is correct for Beckmann's `D` — next
// to this one and shows it failing, because that is the clearest way to say
// that the pairing is what matters rather than the pedigree.
//
// This file is the purest case of house rule 2 in the project. The most
// arbitrary-looking term in the whole model turns out to be a consequence.
//
// ── The requirement ──────────────────────────────────────────────────────
//
// A microsurface is a model of a macrosurface, so it has to cover it — not
// only from directly above, which is what `trowbridge_reitz.hpp` normalises
// `D` against, but *from every direction*. Look at one unit of flat surface
// from `v` and you see `cos(theta_v)` of projected area. Look at the facets
// standing on it from the same `v` and you must see the same:
//
//      cos(theta_v) = ∫ G₁(v, m) ⟨v·m⟩ D(m) dm
//
// The angle brackets are a clamp to zero: a facet whose normal points away
// from `v` shows you its back and contributes nothing. `G₁` is the fraction
// of the facets that do face `v` which are not hidden behind another one.
//
// That is the whole input. Everything below is arithmetic.
//
// ── The one approximation, stated where it is made ───────────────────────
//
// `G₁` is taken to depend on `v` but not on `m`: a facet's chance of being
// hidden does not depend on which way it points. That is false — a facet
// tilted towards the viewer is likelier to stand clear of its neighbours, and
// on a real height field the two are correlated. It is Smith's assumption,
// and it is what makes the answer unique instead of underdetermined.
//
// With it, `G₁` comes out of the integral:
//
//      cos(theta_v) = G₁(v) ∫ ⟨v·m⟩ D(m) dm
//
// and `G₁` is a ratio of two things that can be computed.
//
// ── Where Lambda comes from ──────────────────────────────────────────────
//
// Write the integral over the facets facing `v` as `A⁺`, and the one over the
// facets facing away — the *magnitude* of what they would contribute — as
// `A⁻`:
//
//      A⁺ = ∫ ⟨v·m⟩ D(m) dm          A⁻ = ∫ ⟨-v·m⟩ D(m) dm
//
// A height field seen from `v` projects to `cos(theta_v)` whichever way the
// signs fall, which is the same covering statement as before with the clamp
// taken off:
//
//      ∫ (v·m) D(m) dm = cos(theta_v)      so      A⁺ - A⁻ = cos(theta_v)
//
// For `v = n` that is exactly `trowbridge_reitz.hpp`'s normalisation, with
// nothing facing away and `A⁻` zero. Substituting:
//
//      G₁ = cos(theta_v) / A⁺ = cos(theta_v) / (cos(theta_v) + A⁻)
//
// and dividing through by `cos(theta_v)` gives the form the literature uses:
//
//      G₁ = 1 / (1 + Lambda)       where      Lambda(v) = A⁻ / cos(theta_v)
//
// `Lambda` is not notation. It is the back-facing facet area per unit of
// projected macrosurface — how much of the microsurface is turned away from
// `v` for every unit of surface you are looking at. At normal incidence
// nothing is turned away and it is zero. At grazing incidence it diverges,
// and `G₁` goes to zero with it, which is the statement that a rough surface
// seen edge-on is entirely self-hidden.
//
// ── Lambda for Trowbridge-Reitz ──────────────────────────────────────────
//
// Now the page of algebra, which is where the distribution enters and the
// only place it does.
//
// Slopes are the natural variable. A facet normal `m` is the normal of a
// plane of slope `(x, y)`, so `m ∝ (-x, -y, 1)` and `cos(theta_m)` is
// `1/sqrt(1 + x² + y²)`. The distribution of normals and the distribution of
// slopes are the same fact twice, related by
//
//      D(m) = P(x, y) / cos⁴(theta_m)       and      dm = dx dy / cos³(theta_m)
//
// so that `D(m) dm = P(x, y) dx dy / cos(theta_m)`. The covering condition
// `∫ D(m) (n·m) dm = 1` becomes `∫ P dx dy = 1` — the microsurface covering
// the macrosurface and the slope density being a density are, in these two
// coordinate systems, one statement.
//
// Trowbridge-Reitz in slopes is obtained by substituting `tan²(theta) =
// x² + y²` into `D` and cancelling the `cos⁴`:
//
//      P(x, y) = 1 / (pi alpha² (1 + (x² + y²)/alpha²)²)
//
// Take `v = (sin(theta), 0, cos(theta))`. Then
//
//      v·m = (cos(theta) - x sin(theta)) · cos(theta_m)
//
// and that `cos(theta_m)` cancels the one left over from the measure, which
// is the whole reason for working here:
//
//      A⁻ = ∫∫_{x > cot(theta)} (x sin(theta) - cos(theta)) P(x, y) dx dy
//         = sin(theta) ∫_{cot(theta)}^{∞} (x - cot(theta)) P₁(x) dx
//
// where `P₁` is `P` with `y` integrated out. That integral is elementary —
// write `a² = alpha² + x²`, so that `P(x, y) = alpha²/(pi (a² + y²)²)`, and
// use `∫ dy/(a² + y²)² = pi/(2a³)`:
//
//      P₁(x) = alpha² / (2 (alpha² + x²)^{3/2})
//
// which is a density in its own right and integrates to one. Writing `c` for
// `cot(theta)`, the remaining integral splits into two standard ones:
//
//      ∫_c^∞ x (alpha²+x²)^{-3/2} dx = 1 / sqrt(alpha² + c²)
//      ∫_c^∞   (alpha²+x²)^{-3/2} dx = 1/alpha² - c / (alpha² sqrt(alpha²+c²))
//
// so
//
//      ∫_c^∞ (x - c) P₁(x) dx = (alpha²/2) [ 1/sqrt(alpha²+c²)
//                                          - c/alpha²
//                                          + c²/(alpha² sqrt(alpha²+c²)) ]
//                             = (1/2) [ (alpha² + c²)/sqrt(alpha²+c²) - c ]
//                             = (1/2) [ sqrt(alpha² + c²) - c ]
//
// and `Lambda` is that times `sin(theta)/cos(theta)`, which turns the `c`
// back into a tangent and leaves:
//
//      Lambda(v) = ( sqrt(1 + alpha² tan²(theta_v)) - 1 ) / 2
//
// One square root. Everything arbitrary-looking about `G` — the 2, the 1, the
// square root, the fact that it is a function of `alpha tan(theta)` and of
// nothing else — arrived from the requirement that the facets cover the
// surface they stand on.
//
// Two checks fall out without any work. At `alpha = 0` the square root is 1
// and `Lambda` is 0 at every angle, so `G₁` is 1 everywhere: a mirror has no
// facets to hide behind. As `theta` approaches 90 degrees the tangent
// diverges and so does `Lambda`, so `G₁` goes to zero.
//
// ── Masking and shadowing are the same function, twice ───────────────────
//
// Light has to get in and back out. The facet must be visible from `wo` and
// also from `wi`, and there is no second derivation for the second one:
// reciprocity says the path does not know which end it started from, so the
// shadowing term is `Lambda` evaluated at `wi`.
//
// The lazy way to combine them is to multiply — `G₁(wo) G₁(wi)` — which
// assumes that being hidden from the eye and being hidden from the light are
// independent events. They are not. Both depend on the same heights: a facet
// down in a pit is hidden from nearly everywhere, and a facet on a peak is
// hidden from nearly nowhere, so the two events are strongly and positively
// correlated, and multiplying two probabilities that are correlated
// underestimates their joint chance.
//
// Heitz's height-correlated form adds the two `Lambda`s instead of
// multiplying their `G`s:
//
//      G₂(wo, wi) = 1 / (1 + Lambda(wo) + Lambda(wi))
//
// It is never smaller than the product — checked below, over every pair of
// angles at four roughnesses — and at alpha 1 and 85 degrees it is larger by
// a factor of 3.39, which is most of the light at the edge of a rough object
// that the separable form throws away.
//
// The separable form is not offered here. It is one line at a call site if
// anybody wants it, `./cornell verify` forms it to print the difference, and
// a renderer with both in it and a flag to choose has reintroduced the menu
// this file exists to argue does not exist.
//
// ── Measured ─────────────────────────────────────────────────────────────
//
// `./cornell verify`. One sweep over the hemisphere of facet normals, at
// 512 x 512 cells, accumulating the area facing `v` and the area facing away,
// answers both halves of the derivation at once. The second column is the
// closed form against `A⁻/cos(theta_v)` — the definition it came from, with
// none of the algebra in it — and the third is the requirement itself,
// `G₁ A⁺` against `cos(theta_v)`:
//
//                            Lambda  vs its sum    covering   at h/2
//      alpha 0.30, 15 deg  0.001612826   -3.47e-08   +8.44e-06     4.00
//      alpha 0.30, 45 deg  0.022015325   -2.46e-08   +6.06e-06     4.00
//      alpha 0.30, 75 deg  0.250589483   -1.09e-07   +1.80e-06     3.94
//      alpha 1.00, 15 deg  0.017638090   -3.86e-07   +1.12e-06     3.97
//      alpha 1.00, 45 deg  0.207106781   -3.33e-07   +7.24e-07     4.17
//      alpha 1.00, 75 deg  1.431851653   -4.11e-07   +1.23e-07     3.86
//
// Both residuals divide by four when the grid spacing is halved, which is
// what a second-order rule does and what a wrong closed form does not.
//
// The same identity in the variables a BRDF actually works in is Heitz's weak
// white furnace test, and the sheet estimates it rather than integrating it
// on a grid — the only Monte Carlo row there, for a reason given beside it.
// Every estimate lands within 1.6 standard errors of one.
//
// The two liars, at alpha 1 and 75 degrees, where the covering residual
// should be 1.2e-07 and is not:
//
//      no masking at all                   +3.71e-01
//      Beckmann's Lambda, against this D   +1.28e-01
//
// The second is the one to look at. It is not a broken function — it is the
// exact solution of the requirement above for a Gaussian slope distribution,
// and against this distribution it finds a third more surface than there is.
//
// The height-correlated form against the separable one:
//
//      alpha    theta    correlated    separable    ratio
//       0.10       85      0.658456     0.630530    1.044
//       0.50       85      0.172359     0.086458    1.994
//       1.00       70      0.342020     0.259804    1.316
//       1.00       85      0.087156     0.025708    3.390
//
// and the correlated form is never the smaller of the two, over 32400 pairs
// of angles at four roughnesses, the closest approach being 3.6e-14.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// **The correlation between orientation and visibility**, which is Smith's
// assumption above and the one place this file approximates rather than
// derives. Heitz measures the error against ray-traced height fields and
// finds it small; "small" is somebody else's measurement and this project has
// not made it, so it is named here rather than quoted.
//
// **Multiple scattering.** `G` says a facet is hidden. It does not say where
// the light went — and it went on to hit the facet that was hiding it, and
// bounced again. A single-scattering microfacet BRDF simply loses that
// energy, which is why the rough conductor will fail the white furnace test,
// and that failure is item 0085 and what to do about it is item 0086. It is
// the reason this file does not claim to conserve energy: it conserves
// *area*, which is all the requirement above asks of it.
//
// **Shadowing between separate objects**, which is not this function's job
// and belongs to the visibility term in `transport.hpp`.

#pragma once

#include <cmath>
#include <limits>

#include <render/trowbridge_reitz.hpp>
#include <render/vec.hpp>

namespace render {

// The masking-shadowing function of a Trowbridge-Reitz microsurface.
//
// It holds the distribution rather than a bare `alpha`, and that is the whole
// argument of the file expressed as a member: `Lambda` below is the solution
// of the covering requirement *for that distribution*, and a different `D`
// means doing the integral again rather than reusing this one. A class that
// took an `alpha` would let the two drift apart.
class Smith {
public:
    constexpr explicit Smith(TrowbridgeReitz distribution)
        : distribution_{distribution} {}

    constexpr const TrowbridgeReitz& distribution() const { return distribution_; }

    // Lambda: the back-facing microsurface area per unit of projected
    // macrosurface, derived above.
    double lambda(const Vec3& v) const {
        const double cos2 = v.z * v.z;
        const double sin2 = v.x * v.x + v.y * v.y;

        // Grazing, where the tangent diverges. Returning the infinity rather
        // than a large number is not bravado: every caller divides into
        // `1 + Lambda`, IEEE gives zero for that, and zero is the physics —
        // a surface seen exactly edge-on hides all of itself. A zero vector
        // leaves by the same door and makes the surface black, which is the
        // failure that shows up rather than the one that does not.
        if (cos2 == 0.0) return std::numeric_limits<double>::infinity();

        const double alpha = distribution_.alpha();
        const double tan2 = sin2 / cos2;

        return 0.5 * (std::sqrt(1.0 + alpha * alpha * tan2) - 1.0);
    }

    // G₁: the fraction of the facets facing `v` that are not hidden.
    //
    // `m` is here for the clamp and for nothing else, which is Smith's
    // assumption made visible — if the orientation mattered to the
    // visibility, it would appear in the arithmetic below rather than only in
    // the test above it.
    double masking(const Vec3& v, const Vec3& m) const {
        if (dot(v, m) <= 0.0) return 0.0;
        return 1.0 / (1.0 + lambda(v));
    }

    // G₂, height-correlated: the fraction visible from `wo` *and* `wi`.
    //
    // Both clamps, because a facet the light cannot reach is as dark as one
    // the eye cannot see, and the two are the same rejection written twice
    // rather than a special case.
    double masking_shadowing(const Vec3& wo, const Vec3& wi, const Vec3& m) const {
        if (dot(wo, m) <= 0.0 || dot(wi, m) <= 0.0) return 0.0;
        return 1.0 / (1.0 + lambda(wo) + lambda(wi));
    }

private:
    TrowbridgeReitz distribution_;
};

} // namespace render
