// waechter.hpp — the one place in this project where the machine intrudes on
// the physics, and it says so rather than pretending otherwise.
//
// `ray.hpp` describes the problem and admits it was not fixed. This is the
// fix. Wächter and Binder, "A Fast and Robust Method for Avoiding
// Self-Intersection", *Ray Tracing Gems*, 2019, chapter 6.
//
// ── The problem, restated in one paragraph ───────────────────────────────
//
// A path bounces. The renderer computes the hit point as `origin + t·d`,
// which is three multiplies and three adds in floating point, and the result
// lands a few ulps to one side or the other of the surface it is supposed to
// be on. Land on the wrong side and the next ray immediately hits the surface
// it just left, at some t near zero. The path terminates against itself, and
// the image fills with black speckle that looks exactly like noise and does
// not go away with more samples.
//
// ── Why an epsilon cannot be right ───────────────────────────────────────
//
// The universal first fix is `t_min = 1e-4`, and `ray.hpp` shipped exactly
// that, labelled as a placeholder. It is a *length*, compared against a scene
// whose size the author happened to know, and it fails in both directions:
// too small a kilometre away, where the spacing between representable doubles
// is itself large, and too large for a scene in millimetres, where it punches
// visible holes in contact shadows and objects float.
//
// The error being corrected is not a length. It is *a few units in the last
// place*, and the size of a unit in the last place depends on the magnitude
// of the number — which is precisely what an absolute constant cannot know
// and what the floating-point representation already encodes.
//
// ── The construction ─────────────────────────────────────────────────────
//
// Step the hit point's coordinates along the normal by a fixed number of
// ulps, done as integer arithmetic on the bit patterns. Consecutive doubles
// differ by 1 in their integer representation, so adding N to the bits moves
// exactly N representable values — a relative step, automatically the right
// size at 10⁻³ metres and at 10⁶, with no constant that has units.
//
// ── What had to change for doubles ───────────────────────────────────────
//
// The published code is `float`, and three of its constants are float-shaped:
// 256 ulps, an absolute fallback of 2⁻¹⁶, and a threshold of 2⁻⁵ below which
// the fallback is used instead. Ported verbatim into a double renderer they
// are wrong by nine orders of magnitude — 2⁻¹⁶ is 15 micrometres, which in a
// two-metre box is an enormous offset to make for a reason that no longer
// applies.
//
// The ulp count carries over: 256 ulps is a claim about how many roundings
// the intersection arithmetic went through, not about the type, and 256 of
// them is 5.7 × 10⁻¹⁴ relative in double rather than 3 × 10⁻⁵ in float.
//
// The fallback does not carry over, and it needed rethinking rather than
// rescaling. It exists because a coordinate at or near zero has ulps that are
// vanishingly small — `nextafter(0.0, 1.0)` is 5 × 10⁻³²⁴ — so stepping it by
// 256 of them moves nothing, and a floor at exactly y = 0 would spawn its
// bounce rays exactly on itself.
//
// The published fallback is an absolute distance, which is the very thing
// this file exists to avoid. But the error in a coordinate that is nearly
// zero does not come from that coordinate: it comes from the arithmetic that
// produced it, which involved the *other* two. A point at (3.7, 0.0, −1.2)
// has its y computed from quantities of order 1, so the uncertainty in y is
// ulps of 1, not ulps of 0.
//
// So the fallback here is ulps of the point's largest coordinate. It is still
// relative, it still has no units, and it still scales, and it is a better
// answer than the original's for the same reason the original is a better
// answer than an epsilon.
//
// ── Measured ─────────────────────────────────────────────────────────────
//
// A bounce spawned straight up the normal from a computed hit point, 200 000
// times, at four world scales spanning nine orders of magnitude. Without the
// offset — which is now the honest test, since `t_min` is zero rather than an
// epsilon that was hiding this:
//
//      scale      spawned from the hit point    from the offset point
//      1e-3 m     103 292 / 200 000             0
//      1e+0 m     106 005 / 200 000             0
//      1e+3 m     107 940 / 200 000             0
//      1e+6 m     108 727 / 200 000             0
//
// Over half of all bounces immediately re-hit the surface they left, at every
// scale, every one of them at exactly t = 0. None do with the offset.
//
// And the offset is scale-invariant, which is the property an epsilon cannot
// have. The distance the point actually moves:
//
//      scale      moved         as a fraction of the scale
//      1e-3 m     2.776e-17 m   2.78e-14
//      1e+0 m     2.842e-14 m   2.84e-14
//      1e+3 m     2.910e-11 m   2.91e-14
//      1e+6 m     2.980e-08 m   2.98e-14
//
// The same relative step across six decades, from twenty-eight attometres to
// thirty nanometres, with nothing in the code that knows how big the scene
// is. An `EPSILON` of 1e-4 would have been ten million times too coarse at
// the top of that range and ten million times too fine at the bottom.

#pragma once

#include <bit>
#include <cmath>
#include <cstdint>
#include <render/vec.hpp>

namespace render {

namespace detail {

// How many representable doubles to step. Not a tolerance: a count of the
// roundings the intersection arithmetic is allowed to have accumulated.
inline constexpr std::int64_t offset_ulps = 256;

// Move `x` by `n` representable values in the direction of `towards`.
//
// Consecutive positive doubles are consecutive integers when the bits are
// read as an integer — which is a property of the IEEE 754 layout rather
// than a coincidence, and is the whole reason this works. The sign has to be
// handled because for a negative double, increasing the bit pattern makes
// the value *more* negative.
inline double step_ulps(double x, std::int64_t n, double towards) {
    const std::int64_t bits = std::bit_cast<std::int64_t>(x);
    const std::int64_t step = (x < 0.0) ? -n : n;
    return std::bit_cast<double>(towards > 0.0 ? bits + step : bits - step);
}

} // namespace detail

// A ray origin that is on the correct side of the surface it is leaving.
//
// `n` points to the side the new ray departs into — outward for a reflection,
// and flipped by the caller for anything that transmits, which is v0.9's
// problem and not yet anybody's.
inline Vec3 offset_origin(const Vec3& p, const Unit& n) {
    // Ulps of the largest coordinate: the scale the error in *any* of the
    // three actually came from. See the header.
    const double scale = std::fmax(std::fabs(p.x), std::fmax(std::fabs(p.y), std::fabs(p.z)));
    const double floor_step = scale * 0x1p-52 * double(detail::offset_ulps);

    // A point at the world origin exactly. Nothing about it has a scale, so
    // there is nothing to be relative to, and the smallest normal double is
    // as good an answer as exists. It does not arise in a real scene; it is
    // here so that the function is total.
    const double fallback = (scale == 0.0) ? 0x1p-1022 : floor_step;

    const auto axis = [&](double coordinate, double normal_component) {
        if (normal_component == 0.0) return coordinate;
        // A coordinate whose own ulps are finer than the floor gets the
        // floor; otherwise the integer step, which is tighter.
        if (std::fabs(coordinate) * 0x1p-52 * double(detail::offset_ulps) < fallback)
            return coordinate + std::copysign(fallback, normal_component);
        return detail::step_ulps(coordinate, detail::offset_ulps, normal_component);
    };

    return Vec3{axis(p.x, n.x()), axis(p.y, n.y()), axis(p.z, n.z())};
}

} // namespace render
