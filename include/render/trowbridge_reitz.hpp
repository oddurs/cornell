// trowbridge_reitz.hpp — how a rough surface's facets are pointed, and the
// constant that energy conservation leaves you no room to choose.
//
// T. S. Trowbridge and K. P. Reitz, "Average Irregularity Representation of a
// Rough Surface for Ray Reflection", Journal of the Optical Society of
// America 65(5), 1975. Blinn brought it into graphics two years later, in
// 1977, as one of the three distributions he compared, and the field used the
// other two. Walter, Marschner, Li and Torrance arrived at it independently
// in "Microfacet Models for Refraction through Rough Surfaces", Eurographics
// Symposium on Rendering 2007, called it **GGX**, and the field has used that
// name ever since — including, for a decade, people who had read Blinn.
//
// House rule 9: the file takes the name its authors gave it, and the comment
// carries both, because a reader who searches for `ggx.hpp` and finds nothing
// deserves to be told where it went rather than left to conclude the project
// has not got one. The 2007 paper introduces the letters without saying what
// they stand for; "ground glass unknown" is the usual gloss and it is
// folklore, so it is written down here as folklore.
//
// ── What is being distributed ────────────────────────────────────────────
//
// The surface is not rough in this model. It is *flat*, and it is covered in
// facets too small to see individually, each one a perfect mirror, each one
// pointed somewhere. What a renderer works with is never a facet; it is the
// statistics of them, and `D(m)` is the whole of those statistics: the
// density of facet area whose normal points along `m`.
//
// That word — area — is the one that matters, and it is where the cosine
// below comes from. `D` is a density of *area on the microsurface* per
// steradian of normal direction. A facet tilted away from the macroscopic
// normal presents less of itself to the surface it is standing on, by exactly
// `n·m`, so the microsurface's areas add up to the macrosurface's area only
// when each is counted with that projection:
//
//      ∫ D(m) (n·m) dm = 1
//
// A microsurface must cover the macrosurface it is a model of. One unit of
// flat surface, seen from directly above, is one unit of facets seen from
// directly above. That is the whole normalisation condition, it is geometry
// rather than convention, and it fixes the constant.
//
// ── The shape, and where it is from ──────────────────────────────────────
//
// Trowbridge and Reitz were not doing computer graphics. They were modelling
// a rough surface as a field of randomly oriented ellipsoids of revolution,
// and the function below is the distribution of normals such a field has —
// `alpha` being the ratio of the ellipsoid's axes, so that `alpha = 0` is a
// stack of flat plates and `alpha = 1` is spheres.
//
// Write the shape with its constant still unknown:
//
//      D(m) = k / (alpha² cos²(theta) + sin²(theta))²
//
// where `theta` is the angle between `m` and the macroscopic normal. The
// square in the denominator is theirs; `k` is ours to find, and there is
// exactly one value it can have.
//
// ── Where the constant comes from ────────────────────────────────────────
//
// Put the shape into the normalisation condition. In spherical coordinates
// `dm = sin(theta) d(theta) d(phi)`, and nothing in the integrand depends on
// `phi`, so that integral is `2pi`:
//
//      ∫ D(m) (n·m) dm
//          = 2pi k ∫₀^(pi/2) cos(theta) sin(theta) / (a² c² + s²)² d(theta)
//
// writing `c` for `cos(theta)`, `s` for `sin(theta)` and `a` for `alpha`.
// Substitute `u = cos(theta)`, so `du = -sin(theta) d(theta)` and
// `s² = 1 - u²`:
//
//          = 2pi k ∫₀¹ u / (a² u² + 1 - u²)² du
//
// and then `w = u²`, `dw = 2u du`, which removes the last trace of the angle:
//
//          = pi k ∫₀¹ dw / ((a² - 1) w + 1)²
//
// That is an elementary integral. With `b = a² - 1`:
//
//          = pi k [ -1 / (b (b w + 1)) ]₀¹
//          = (pi k / b) (1 - 1/(b + 1))
//          = (pi k / b) (b / (b + 1))
//          = pi k / a²
//
// Setting it equal to one:
//
//      k = alpha² / pi
//
// and the pi is not a normalisation somebody chose either. It is the integral
// of the cosine over the hemisphere, in steradians — the same quantity
// `lambert.hpp` derives by integrating and `warp.hpp` arrives at as the area
// of the unit disc. Three files, three derivations, one constant, and none of
// them typed it in.
//
// The `b = 0` case is `alpha = 1`, where the substituted integrand is a
// constant and the bracket above is a division by zero. The limit is the same
// answer — `∫₀¹ dw = 1`, so `pi k = 1` — and the code below never evaluates
// the bracket, so the hole is in the derivation's notation rather than in the
// arithmetic.
//
// ── The two ends of alpha, which are both familiar ───────────────────────
//
// At `alpha = 1` the denominator is `(c² + s²)² = 1` and the distribution is
// `1/pi`, in every direction. The facets of a surface of spheres are
// uniformly distributed over the *projected* hemisphere — which is Lambert's constant, arriving in a file that is not
// about Lambert, for the third time and from a fourth direction. The sheet
// compares the two as bit patterns rather than to a tolerance, because both
// are `si::inv_pi` and nothing has rounded either.
//
// Larger alphas are arithmetic rather than geometry. The derivation above
// holds for any positive alpha and the normalisation stays exact, but the
// ellipsoids ran out at spheres, so nothing past 1 has a surface behind it —
// which is not a reason to forbid it and is a reason to say so.
//
// At `alpha = 0` the distribution is a delta at the normal: every facet is
// the surface, and the surface is a mirror. It is not representable as a
// function and `d()` returns zero there rather than the `0/0` the formula
// would produce, because the material that holds the `alpha` is the thing
// that has to notice and reach for `specular.hpp` instead. That switch is
// v0.7's conductor, not this file's business.
//
// ── Why this is not a `SolidAngleDensity` ────────────────────────────────
//
// It has the units of one. `D` is per steradian, dimensionally, and the type
// is right there in `density.hpp` asking to be used.
//
// It would be a lie. `SolidAngleDensity` means one specific thing in this
// project: a density that integrates to one against `dw`. `D` does not — it
// integrates to one against `(n·m) dm`, the projected measure, and the sheet
// prints what `∫ D dm` actually comes to so that the difference is a number
// rather than a claim. Two densities differing by a cosine is exactly the
// confusion `density.hpp` exists to prevent, and borrowing its type for a
// quantity normalised against a third measure would spend the type's whole
// meaning to save a `double`.
//
// The thing that *is* a `SolidAngleDensity` arrives in item 0084: sampling
// the visible normals gives a genuine density over directions, and it has a
// cosine and a masking term in it for precisely this reason.
//
// ── Measured ─────────────────────────────────────────────────────────────
//
// `./cornell verify`, which is the derivation above done a second time by
// machine and without the algebra: `D(m) (n·m)` integrated over the
// hemisphere by a midpoint rule on a 512 x 512 grid in `(theta, phi)`, and
// then again on a grid twice as fine.
//
//      alpha    ∫ D (n·m) dm - 1      at h/2      ∫ D dm
//      0.05          +3.142e-04         / 4.00     1.0095
//      0.10          +7.847e-05         / 4.00     1.0302
//      0.25          +1.260e-05         / 4.00     1.1332
//      0.50          +3.334e-06         / 4.00     1.3802
//      1.00          +1.569e-06         / 4.00     2.0000
//
// The residual is the rule and not the model. Halving the grid spacing
// divides it by four at every alpha, to two decimal places, which is what a
// second-order quadrature does and what a wrong constant does not — a
// distribution that really failed to normalise would sit the same distance
// from one however finely it was integrated. It grows as the lobe narrows
// because a narrow lobe is a peaked integrand, at the rate `h²/alpha²` the
// rule's own error term predicts.
//
// The last column is the same integral without the projection, and it is the
// more interesting one. It is what a reader who assumed `D` was a density
// over solid angle would have been dividing by, and at `alpha = 0.05` it is
// 1.0095 — wrong by one percent, which is to say invisible. The mistake hides
// on a polished surface, because a narrow lobe sits where the cosine is
// nearly one, and it costs a factor of exactly two at `alpha = 1`. A renderer
// with that bug does not look broken. It looks like rough metal is too dark,
// which is a thing people believe about renderers.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// **Beckmann**, which is the one with the better pedigree. Beckmann and
// Spizzichino's 1963 treatment of scattering from rough surfaces assumes the
// heights are Gaussian, which makes the slopes Gaussian too; Cook and
// Torrance used that distribution in 1982, and it was the default in graphics
// for twenty-five years. The field abandoned it anyway, and it abandoned it
// on evidence. Walter and colleagues measured ground glass and found far more
// light at large angles than a Gaussian can account for — the broad dim halo
// around a highlight — because a Gaussian's tails fall as `exp(-tan²)` and
// the tails of this distribution fall as a power of the angle instead. The
// halo is most of what makes rough metal look like metal.
//
// That is worth a reader's attention, because it is the opposite of this
// project's usual moral. The distribution with a physical derivation behind
// it lost to a fit to ellipsoids, on a measurement, and the standard is the
// same standard: the thing that agrees with the photograph is the thing that
// is right.
//
// **Anisotropy.** One `alpha`, so the lobe is round. Brushed metal has two
// and a direction to measure them along, which is item 0088, and it is a
// change to this file rather than a second one.
//
// **Roughness.** There is no `roughness` here and there will not be one in
// this file. `alpha` is the parameter the distribution is written in terms
// of; "roughness" is a number an artist types into a slider, the mapping
// between them is somebody's taste rather than physics, and item 0089 is
// where that gets admitted with a name on it.
//
// **Masking and shadowing.** `D` counts facets that point somewhere. It does
// not ask whether they can be seen from `wo`, or whether they can see `wi`,
// and at grazing angles most of them cannot. A BRDF built from `D` alone
// reflects more light than arrives. `smith.hpp` is item 0083 and it is not a
// free choice either: Heitz showed in 2014 that the masking function is
// determined by the distribution it accompanies, and this file's `D` is what
// determines it.
//
// **That the microsurface exists.** There is no height field here, no facets
// to intersect, and no length scale at all — `alpha` is dimensionless and the
// model cannot answer how big a facet is. That is deliberate and it is also
// the model's limit: a surface whose features approach a wavelength diffracts,
// and diffraction is outside geometric optics, which `transport.hpp` says at
// the top and item 0146 is where this project admits how far outside.

#pragma once

#include <render/si.hpp>
#include <render/vec.hpp>

namespace render {

// The distribution of microfacet normals of a rough surface, isotropic.
//
// `alpha` is the ellipsoid's axis ratio: 0 is a mirror and 1 is spheres,
// where the picture it came from stops. It is not a roughness and it is not
// remapped; see above.
class TrowbridgeReitz {
public:
    constexpr explicit TrowbridgeReitz(double alpha) : alpha_{alpha} {}

    constexpr double alpha() const { return alpha_; }

    // D(m), per steradian of facet-normal direction, per unit of projected
    // area — the density derived above, evaluated for a unit `m` in the local
    // frame where the macroscopic normal is +z.
    constexpr double d(const Vec3& m) const {
        // A facet pointing into the surface is not part of a height field's
        // normal distribution, and the formula would happily give it a value:
        // the denominator is even in `cos(theta)`, so the lower hemisphere is
        // a mirror image of the upper one rather than empty. Zero is the
        // honest answer and it is the one that keeps `∫ D (n·m) dm` over the
        // *sphere* equal to what it is over the hemisphere.
        if (m.z <= 0.0) return 0.0;

        // The delta, refused rather than approximated. See above: this is the
        // caller's switch to make, and returning zero makes a caller that
        // forgot produce a black surface rather than a NaN that spreads.
        if (alpha_ <= 0.0) return 0.0;

        const double cos2 = m.z * m.z;

        // `sin²(theta)` as the length of the tangential part rather than as
        // `1 - cos²`, which is the same number for a unit vector and does not
        // depend on `m` being one to stay non-negative.
        const double sin2 = m.x * m.x + m.y * m.y;

        const double a2 = alpha_ * alpha_;
        const double shape = a2 * cos2 + sin2;

        // `alpha² / pi`, which the derivation above leaves no freedom in, and
        // the pi is the projected hemisphere rather than a tidying constant.
        return (a2 * si::inv_pi) / (shape * shape);
    }

private:
    double alpha_ = 0.0;
};

} // namespace render
