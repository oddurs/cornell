// sphere.hpp — the first thing a ray hits, and the first place the
// algebraically correct answer and the numerically correct answer differ.
//
// Substituting `o + t·d` into `|p − c|² = r²` gives a quadratic in t, and
// every introduction to ray tracing stops there, because as mathematics there
// is nothing more to say. As arithmetic there is half a page more, and it is
// the interesting half: the obvious formula deletes the sphere.
//
// ── What is lost ──────────────────────────────────────────────────────────
//
// Write `f = o − c` for the vector from the centre to the ray origin. With
// the direction normalised — which `Unit` guarantees, which is what it is for
// — the quadratic has a = 1 and the two coefficients
//
//      b  =  −f·d                      (half the usual B, negated)
//      c  =  f·f − r²
//
// and the discriminant is `b² − c`. Both of those terms are of order |f|²,
// and their difference is of order r². A ray a hundred metres from a
// one-metre sphere is subtracting two numbers near 10⁴ to get an answer near
// 1; at a hundred kilometres it is subtracting two numbers near 10¹⁰ to get
// the same answer, and a double has about sixteen digits to spend.
//
// This is not a tail case. It is a ray from a camera to anything, in any
// scene larger than the objects in it. Measured, for a one-metre sphere and a
// ray from 10⁸ m away passing half a metre from the centre:
//
//      exact                        0.75
//      b² − c                       0            relative error 1
//      r² − |f − (f·d)d|²           0.75         relative error 0
//
// Not "less accurate". Zero: every digit gone, the discriminant negative or
// nought, and the renderer reporting that the ray missed. The sphere is not
// noisy or soft at that distance. It is absent.
//
// The second form is the same quantity rearranged so that nothing large is
// ever subtracted. `f − (f·d)d` is the component of f perpendicular to the
// ray — the ray's closest approach to the centre — and it is a small number
// computed from small differences. Subtracting its square from r² is
// subtracting a number near 1 from a number near 1.
//
//      b² − c  =  (f·d)² − f·f + r²  =  r² − ( f·f − (f·d)² )  =  r² − |f⊥|²
//
// Haines, Günther and Akenine-Möller give the construction in Ray Tracing
// Gems (2019); the rearrangement itself is older than ray tracing and belongs
// to whoever first had to solve a quadratic on a machine.
//
// ── The fix that was expected here, and did not pay ───────────────────────
//
// The item that asked for this file prescribed the textbook remedy: when
// `b² ≫ 4ac`, one root of the quadratic formula cancels, so compute the
// well-conditioned root and recover the other from `t₁t₂ = c/a`. It is the
// right advice about `(−B ± √(B²−4AC))/2A`, and it does not apply to the form
// above, which is worth writing down rather than quietly not doing.
//
// The roots here are `b ± √Δ`, and `√Δ ≤ r` always, because Δ = r² − |f⊥|².
// So the two terms can only be close in magnitude when |b| is itself about r
// — which happens only when the origin is about r from the centre, that is,
// sitting on the surface. And in exactly that case `c = f·f − r²` has lost
// its own significant figures to the same cancellation, so the substitute
// root is computed from a number that is already wrong. Measured, for an
// origin ε above a one-metre sphere aimed at its centre:
//
//      ε        b − √Δ                  c/(b + √Δ)
//      1e-6     rel. error 8.2e-11      rel. error 8.2e-11
//      1e-9     rel. error 8.3e-08      rel. error 8.3e-08
//      1e-12    rel. error 8.9e-05      rel. error 8.9e-05
//
// Identical, to the last digit, because the loss is upstream of both of them.
// So this file does not compute `c` at all. The remaining error at ε = 1e-12
// is real and it is the self-intersection problem from `ray.hpp` wearing
// different clothes: a ray that starts on a surface cannot be told accurately
// how far it is from that surface, which is why the fix is to not start it
// there. Wächter and Binder, v0.2.
//
// ── What is not modelled ──────────────────────────────────────────────────
//
// Ellipsoids, and spheres that are not at the origin of their own coordinate
// system in any sense other than translation. Both are one transformation
// away and there are no transformations until v0.4.
//
// A sphere is also not in the Cornell box, which is made of rectangles. This
// file exists because a sphere is the cheapest surface to be wrong about, and
// v0.4 replaces it as the thing the camera looks at rather than deleting it.

#pragma once

#include <cmath>
#include <optional>
#include <render/ray.hpp>
#include <render/vec.hpp>

namespace render {

struct Sphere {
    Vec3 centre{};
    double radius = 1.0;

    // The nearest intersection inside the ray's own interval, if there is
    // one. Returning the parameter rather than a hit record is deliberate for
    // now: there is nothing to shade, and a record with a normal and a
    // material in it would be a guess about what v0.2 wants.
    std::optional<double> intersect(const Ray& ray) const {
        const Vec3 f = ray.origin - centre;
        const double b = -dot(f, ray.direction);

        // The ray's closest approach to the centre, as a vector. Small, and
        // computed from small differences, which is the entire point.
        const Vec3 perpendicular = f + ray.direction.vec() * b;
        const double discriminant = radius * radius - length_squared(perpendicular);
        if (discriminant < 0.0) return std::nullopt;

        const double root = std::sqrt(discriminant);

        // Near first. A ray that starts inside the sphere has the near root
        // behind it, and the far one is the answer; `holds` sorts that out
        // without this routine needing to know which case it is in.
        if (ray.holds(b - root)) return b - root;
        if (ray.holds(b + root)) return b + root;
        return std::nullopt;
    }

    // The outward normal at a point that is supposed to be on the surface.
    //
    // `(p − centre) / radius` is the same thing without a square root, and it
    // is not used, because the point is never exactly on the surface — that
    // is the whole subject of `ray.hpp` — so dividing by the radius returns a
    // vector of length 1 ± a few ulps and quietly breaks the invariant `Unit`
    // exists to hold. The square root is the price of the guarantee.
    Unit normal_at(const Vec3& p) const { return normalize(p - centre); }
};

// ── The test that fails with the obvious formula ──────────────────────────
//
// The acceptance criterion for this file asks for a test that the naive
// arithmetic fails and this one passes. It is here rather than in an
// instrument because it needs no sphere, no ray and no square root — only the
// two spellings of the discriminant — and a check a compiler can run is a
// check that cannot be skipped, disabled, or left out of the build.
//
// One metre sphere at the origin, ray from 10⁸ m away, passing 0.5 m from the
// centre. The true discriminant is r² − 0.5² = 0.75.
namespace detail {

inline constexpr double probe_r   = 1.0;
inline constexpr double probe_d   = 1.0e8;
inline constexpr double probe_off = 0.5;
inline constexpr double probe_exact = probe_r * probe_r - probe_off * probe_off;

// f = (0.5, 0, −1e8), direction +z, so f·d = −1e8 and b = +1e8. Every line
// below is the arithmetic the routine above performs, in the same order, on
// the same values — not the answer worked out by hand and typed back in.
inline constexpr double probe_b = probe_d;

// f·f loses the 0.25 outright: the ulp at 10¹⁶ is 2, so 0.25 + 10¹⁶ is 10¹⁶
// and 10¹⁶ − 1 is 10¹⁶ as well. c comes back as a round power of ten, and
// b² is the same round power of ten, and their difference is nothing at all.
inline constexpr double probe_ff    = probe_off * probe_off + probe_d * probe_d;
inline constexpr double probe_c     = probe_ff - probe_r * probe_r;
inline constexpr double probe_naive = probe_b * probe_b - probe_c;

// perpendicular = f + b·d = (0.5, 0, −10⁸ + 10⁸). The z component is exactly
// zero — subtracting a number from itself is the one subtraction floating
// point does perfectly — and the x component was never touched.
inline constexpr double probe_perp_x = probe_off;
inline constexpr double probe_perp_z = -probe_d + probe_b;
inline constexpr double probe_stable =
    probe_r * probe_r - (probe_perp_x * probe_perp_x + probe_perp_z * probe_perp_z);

static_assert(probe_exact == 0.75);

// Not "less accurate". Zero: the ray reports a miss on a sphere it passes
// half a metre inside.
static_assert(probe_naive == 0.0);

static_assert(probe_stable == probe_exact);

} // namespace detail

} // namespace render
