// srgb.hpp — the other end of the pipe, and the second of exactly two files
// allowed to break the no-RGB rule.
//
// `cie.hpp` converts a spectrum to tristimulus because eyes have three cone
// types. This converts tristimulus to numbers a monitor will accept, because
// monitors have three phosphors and a transfer function. Between them they
// are the entire boundary; everything upstream is spectra, and `spectrum.hpp`
// spends a page on why.
//
// Two jobs, and both of them are usually done by copying a constant.
//
// ── The matrix, derived ──────────────────────────────────────────────────
//
// A display's primaries are three chromaticities — the colours its red, green
// and blue subpixels produce at full brightness — and its white point is what
// all three at once should look like. Those five numbers determine the 3 × 3
// matrix completely, and the derivation is a page of linear algebra that
// almost nobody performs, because the answer is printed in the standard and
// can be pasted.
//
// Build a matrix whose columns are the primaries as unnormalised tristimulus
// values, `(x/y, 1, (1−x−y)/y)`. That matrix maps RGB to XYZ *up to an
// unknown scale per column*: it has the right directions and the wrong
// brightnesses. Then demand that `(1, 1, 1)` maps to the white point, solve
// the resulting three equations for the three scales, and multiply them in.
// That is the whole derivation, and it is below rather than in this comment.
//
// ── Deriving it turned up a discrepancy in the standard ──────────────────
//
// Doing the algebra rather than pasting the answer means finding out whether
// the two agree, and they do not.
//
// The sRGB standard states a white point of x = 0.3127, y = 0.3290, and
// prints a matrix. Derive the matrix from that white point and it differs
// from the printed one by 2.28e-4 in its worst element — far more than
// rounding at seven decimal places.
//
// Work backwards instead: apply the *published* matrix to (1, 1, 1) and ask
// what white point it implies. It implies x = 0.312727, y = 0.329023, and a
// matrix derived from that reproduces the published one to 1.36e-7, which is
// the rounding.
//
//      stated in the standard          x = 0.3127    y = 0.3290
//      implied by its own matrix       x = 0.312727  y = 0.329023
//      D65 integrated by this project  x = 0.312712  y = 0.329008
//
// So the published matrix was computed from a more precise D65 than the
// document quotes beside it. Nothing is wrong with the matrix; the standard
// simply rounded the white point for the reader and not for the arithmetic,
// and anybody deriving the matrix from the printed chromaticities gets a
// different answer from everybody who pasted it.
//
// ── Which white point this file uses, and why it is the third one ────────
//
// The middle row, not the top. This project's own D65, integrated from the
// CIE tables in `illuminant.hpp` by the code in `cie.hpp`.
//
// That is not pedantry, it is the only self-consistent choice. A perfect
// white reflector lit by *this project's* D65 has to come out as RGB
// (1, 1, 1), or white in a render is slightly not white. The matrix has to be
// built from the same illuminant the scene is lit by, and the scene is lit by
// the table in `illuminant.hpp` rather than by a four-decimal summary of it.
//
// The cost is that this matrix differs from the pasted one by 1.40e-4 in its
// worst element — 0.036 of one 8-bit code, which is invisible — and the file
// says so rather than letting somebody discover it by diffing against a
// reference.
//
// Measured, for the record: encode followed by decode over 10⁶ values round
// trips to within 4.44e-16, which is two ulps at 1.0. And the nine-code gap
// `image.hpp` measured against a 2.2 power in v0.1 reproduces exactly, at
// linear 0.001315.
//
// ── The transfer function, exactly ───────────────────────────────────────
//
// Not a 2.2 gamma. sRGB is a short linear segment near black spliced to a
// 1/2.4 power curve with an offset, and `image.hpp` in v0.1 measured what the
// approximation costs: up to nine 8-bit codes, all of it in the shadows, with
// linear 0.001 encoding to 3 under sRGB and 11 under a 2.2 power. That file
// used 2.2 and said it was provisional and that this one would replace it.
//
// The linear segment exists because a pure power curve has an infinite slope
// at zero, which quantises terribly in the darks and is numerically unpleasant
// to invert.
//
// ── The standard is very slightly discontinuous, and that is fine ────────
//
// The two segments are spliced at 0.0031308, and the value where they
// actually meet is 0.003130668443. At the standard's threshold the linear
// segment gives 0.0404499360 and the power segment gives 0.0404499075, so the
// curve has a step of 2.85e-8 in it.
//
// That is a fortieth of a bit in 16-bit, and it exists because the standard
// rounded the crossover to five decimal places. It is noted rather than
// corrected: this file implements sRGB, and a version that is smoother than
// the standard is a version that disagrees with every other decoder.
//
// The decode threshold has the same history — the standard says 0.04045, the
// exact image of the crossover is 0.0404482363 — and the same treatment.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// Any other display. Display P3, Rec. 2020 and Adobe RGB are three more sets
// of primaries through the same derivation, and adding one is a table of five
// numbers rather than a change to this file's shape.
//
// The viewing conditions sRGB assumes: a 64 lux ambient, a 20 % surround, a
// dim room. They are in the standard because a transfer function is a
// statement about appearance and appearance depends on what else is in the
// room, and none of that is modelled here.
//
// Out-of-gamut colour. A saturated spectral green converts to an RGB with a
// negative component, and what to do about that is item 0047, which is a
// spike rather than a decision because the answer depends on what the v0.9
// prism image needs to be able to show.

#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <render/cie.hpp>
#include <render/illuminant.hpp>

namespace render::srgb {

// ── The published chromaticities ─────────────────────────────────────────
//
// IEC 61966-2-1. These five pairs are the definition of sRGB; everything else
// in this file is derived from them.
inline constexpr Chromaticity red   = {0.6400, 0.3300};
inline constexpr Chromaticity green = {0.3000, 0.6000};
inline constexpr Chromaticity blue  = {0.1500, 0.0600};

// The white point is *not* taken from the standard's printed 0.3127, 0.3290.
// See the header: it is this project's own D65, so that white in a render is
// white on the display.
inline constexpr Chromaticity white =
    chromaticity_of(cie::white_point(cie::d65));

// The 3 x 3 machinery moved to `cie.hpp` when `bradford.hpp` turned out to
// need it too: a transform between illuminants should not have to include a
// display standard to get a matrix inverse.

// ── The derivation ───────────────────────────────────────────────────────

// A chromaticity as an unnormalised tristimulus value: the direction in XYZ
// that this primary points, with its brightness left undecided.
constexpr Xyz direction_of(const Chromaticity& c) {
    return Xyz{c.x / c.y, 1.0, (1.0 - c.x - c.y) / c.y};
}

constexpr Matrix3 derive_rgb_to_xyz() {
    const Xyz r = direction_of(red);
    const Xyz g = direction_of(green);
    const Xyz b = direction_of(blue);

    // The three directions as columns. This maps RGB to XYZ with the right
    // hues and the wrong brightnesses.
    Matrix3 directions;
    directions[0] = {r.x, g.x, b.x};
    directions[1] = {r.y, g.y, b.y};
    directions[2] = {r.z, g.z, b.z};

    // Demand that (1, 1, 1) is the white point, and solve for the three
    // brightnesses that make it so.
    const Xyz w = direction_of(white);
    const Matrix3 undo = inverse(directions);
    const Xyz scale = apply(undo, w.x, w.y, w.z);

    // Scale each column by its own brightness.
    Matrix3 out = directions;
    const double s[3] = {scale.x, scale.y, scale.z};
    for (std::size_t row = 0; row < 3; ++row)
        for (std::size_t col = 0; col < 3; ++col)
            out[row][col] *= s[col];
    return out;
}

inline constexpr Matrix3 rgb_to_xyz = derive_rgb_to_xyz();
inline constexpr Matrix3 xyz_to_rgb = inverse(rgb_to_xyz);

// ── The transfer function ────────────────────────────────────────────────
//
// The four constants below are the standard's, including the two that make it
// very slightly discontinuous. See the header.

inline constexpr double linear_slope    = 12.92;
inline constexpr double encode_threshold = 0.0031308;
inline constexpr double decode_threshold = 0.04045;
inline constexpr double curve_offset    = 0.055;
inline constexpr double curve_exponent  = 2.4;

// Linear light in, display code in [0, 1] out.
inline double encode(double linear) {
    if (linear <= encode_threshold) return linear_slope * linear;
    return (1.0 + curve_offset) * std::pow(linear, 1.0 / curve_exponent) - curve_offset;
}

// And back again.
inline double decode(double code) {
    if (code <= decode_threshold) return code / linear_slope;
    return std::pow((code + curve_offset) / (1.0 + curve_offset), curve_exponent);
}

// ── Checked at compile time ──────────────────────────────────────────────
//
// The matrix is `constexpr`, so the checks on it can be too. The transfer
// function is not — `std::pow` is not `constexpr` before C++26 — so its
// round-trip is measured at runtime by the instruments instead.

namespace check {

// White in must be white out, which is the whole reason the white point above
// is this project's D65 rather than the standard's rounding of it.
inline constexpr Xyz white_in_rgb = apply(rgb_to_xyz, 1.0, 1.0, 1.0);
inline constexpr Chromaticity white_out = chromaticity_of(white_in_rgb);

static_assert(white_out.x > white.x - 1e-12 && white_out.x < white.x + 1e-12,
              "RGB (1,1,1) must land exactly on the white point it was built from");
static_assert(white_out.y > white.y - 1e-12 && white_out.y < white.y + 1e-12,
              "RGB (1,1,1) must land exactly on the white point it was built from");

// Y is relative luminance, so white must have Y = 1 exactly.
static_assert(white_in_rgb.y > 1.0 - 1e-12 && white_in_rgb.y < 1.0 + 1e-12,
              "white must have unit luminance");

// The inverse must be an inverse.
constexpr double round_trip_error() {
    double worst = 0.0;
    const double probes[4][3] = {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0},
                                 {0.0, 0.0, 1.0}, {0.2, 0.7, 0.4}};
    for (const auto& p : probes) {
        const Xyz forward = apply(rgb_to_xyz, p[0], p[1], p[2]);
        const Xyz back = apply(xyz_to_rgb, forward.x, forward.y, forward.z);
        const double d[3] = {back.x - p[0], back.y - p[1], back.z - p[2]};
        for (const double e : d) worst = (e < 0.0 ? -e : e) > worst ? (e < 0.0 ? -e : e) : worst;
    }
    return worst;
}

static_assert(round_trip_error() < 1e-14, "xyz_to_rgb must invert rgb_to_xyz");

// Each primary must land back on the chromaticity it was derived from — the
// check that the column scaling did not disturb the hues.
constexpr bool primary_lands_on(const Chromaticity& c, double r, double g, double b) {
    const Chromaticity got = chromaticity_of(apply(rgb_to_xyz, r, g, b));
    const double dx = got.x - c.x, dy = got.y - c.y;
    return (dx < 1e-12 && dx > -1e-12) && (dy < 1e-12 && dy > -1e-12);
}

static_assert(primary_lands_on(red,   1.0, 0.0, 0.0), "red primary moved");
static_assert(primary_lands_on(green, 0.0, 1.0, 0.0), "green primary moved");
static_assert(primary_lands_on(blue,  0.0, 0.0, 1.0), "blue primary moved");

} // namespace check

} // namespace render::srgb
