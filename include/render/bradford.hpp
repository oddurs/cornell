// bradford.hpp — a model of perception, bolted onto a radiometric renderer,
// and labelled as such because it is the first thing in this project that is
// not physics.
//
// A sheet of paper looks white under noon daylight and white under a tungsten
// bulb. Measure the light coming off it in the two cases and the spectra are
// enormously different — the tungsten one has perhaps three times as much red
// as blue. The paper has not changed and the light reaching the eye has, and
// the eye reports "white" both times.
//
// That is chromatic adaptation. It is not a property of the paper, or of the
// light, or of the transport: it happens in the observer, over seconds to
// minutes, and it is why photographs need a white balance and renders need
// this file.
//
// ── What this file is not ────────────────────────────────────────────────
//
// It is not a correction to anything upstream. Everything before this point
// is radiometry, and all of it is right: a white wall under illuminant A
// really does emit a spectrum with three times as much red as blue, and
// `./cornell render --lamp a --adapt off` really does produce an orange
// image, and that image is *correct*. It is what a camera with daylight film
// records in a tungsten-lit room, and it is what the radiometer would
// measure.
//
// What it is not is what a person standing in the room would see, and the
// difference between those two things is a model of an eye rather than a
// model of light.
//
// So: separable, switchable, off by default in anything that quotes a number,
// and in a file of its own with this paragraph at the top.
//
// ── The transform ────────────────────────────────────────────────────────
//
// A von Kries transform. The idea is old and simple: the three cone types
// adapt independently, each scaling its own response so that the illuminant
// comes out looking neutral. So convert tristimulus into something
// cone-shaped, scale the three axes by the ratio of destination white to
// source white, and convert back.
//
//      M_adapt = B^-1 · diag(dest / src) · B
//
// Everything hangs on `B`, the matrix into cone-shaped space, and there are
// several candidates. The one here came out of work at the University of
// Bradford — Lam's 1985 thesis, fitted to observations of colour appearance
// under different illuminants — and it is called the Bradford transform after
// the place rather than the person.
//
// House rule 8: this is a curve-fit and not a law, it is named for where it
// was fitted, and the numbers below are a least-squares result rather than
// anything derived. It is not even a very good description of cone
// fundamentals — its axes are deliberately *sharper* than real cone
// sensitivities, because sharpening turned out to predict appearance better,
// which is an admission that the physiological story is a convenient fiction.
//
// It is here because it is what everybody uses, it is what ICC profiles
// specify, and a render that adapts differently from every other tool is
// harder to check rather than more honest.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// Incomplete adaptation. Real observers do not adapt all the way — a tungsten
// room still looks slightly warm — and the degree depends on luminance,
// duration and how much of the field the illuminant fills. CIECAM02 has a
// parameter for it. This is all-or-nothing.
//
// Everything else about appearance: surround, background, the Hunt effect,
// the Stevens effect, simultaneous contrast. A full colour appearance model
// is a different and much larger thing, and this is the one piece of it the
// project needs in order to answer "what colour is that wall" without the
// answer depending on the bulb.

#pragma once

#include <render/cie.hpp>
#include <render/illuminant.hpp>

namespace render::bradford {

// The matrix into cone-shaped space. Lam, 1985. A fit, not a derivation.
inline constexpr Matrix3 to_cone = matrix3( 0.8951,  0.2664, -0.1614,
                                           -0.7502,  1.7135,  0.0367,
                                            0.0389, -0.0685,  1.0296);

inline constexpr Matrix3 from_cone = inverse(to_cone);

// The adaptation matrix taking tristimulus measured under `from` to what it
// would have been under `to`.
//
// The three ratios are the whole of it: each cone-shaped axis is scaled so
// that the source white lands on the destination white, and everything else
// comes along.
constexpr Matrix3 adapt(const Xyz& from_white, const Xyz& to_white) {
    const Xyz source = apply(to_cone, from_white);
    const Xyz destination = apply(to_cone, to_white);

    const Matrix3 gains = matrix3(destination.x / source.x, 0.0, 0.0,
                                  0.0, destination.y / source.y, 0.0,
                                  0.0, 0.0, destination.z / source.z);

    return multiply(from_cone, multiply(gains, to_cone));
}

// The common case: adapt from whatever lit the scene to the white point the
// display expects, which for sRGB is D65.
constexpr Matrix3 adapt_to_d65(const cie::Illuminant& lit_by) {
    return adapt(cie::white_point(lit_by), cie::white_point(cie::d65));
}

// ── Checked at compile time ──────────────────────────────────────────────

namespace check {

// Adapting an illuminant to itself must be the identity, or the transform is
// inventing a colour shift out of nothing.
constexpr double departure_from_identity(const Matrix3& m) {
    double worst = 0.0;
    for (std::size_t r = 0; r < 3; ++r)
        for (std::size_t c = 0; c < 3; ++c) {
            const double want = (r == c) ? 1.0 : 0.0;
            const double got = m[r][c] - want;
            const double magnitude = got < 0.0 ? -got : got;
            worst = magnitude > worst ? magnitude : worst;
        }
    return worst;
}

static_assert(departure_from_identity(adapt_to_d65(cie::d65)) < 1e-15,
              "adapting D65 to D65 must be the identity");

// And the one that matters: A's white, adapted, must land on D65's white.
// That is what "the paper looks white either way" means, arithmetically.
inline constexpr Xyz adapted_a =
    apply(adapt_to_d65(cie::a), cie::white_point(cie::a));
inline constexpr Chromaticity adapted_a_chromaticity = chromaticity_of(adapted_a);

static_assert(adapted_a_chromaticity.x > cie::check::d65_chromaticity.x - 1e-12 &&
              adapted_a_chromaticity.x < cie::check::d65_chromaticity.x + 1e-12,
              "adapted tungsten white must land on D65 in x");
static_assert(adapted_a_chromaticity.y > cie::check::d65_chromaticity.y - 1e-12 &&
              adapted_a_chromaticity.y < cie::check::d65_chromaticity.y + 1e-12,
              "adapted tungsten white must land on D65 in y");

} // namespace check

} // namespace render::bradford
