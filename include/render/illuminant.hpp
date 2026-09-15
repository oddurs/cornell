// illuminant.hpp — the lights the CIE defined so that colours could be
// compared at all.
//
// A reflectance is not a colour. A wall reflects some fraction of whatever
// falls on it, and what falls on it decides what comes off — so "the colour
// of the wall" is a statement about a wall *and a light*, and the CIE had to
// standardise some lights before anybody could publish a colour and expect
// somebody else to reproduce it.
//
// This file holds two of them, and they are defined in interestingly
// different ways.
//
// ── D65 is a table, and could not be anything else ───────────────────────
//
// It is meant to represent average daylight at a correlated colour
// temperature of about 6504 K, and daylight is not a blackbody: it is the
// sun, filtered by the atmosphere, with absorption lines in it. Judd,
// MacAdam and Wyszecki derived the D series in 1964 from 622 measured
// daylight spectra, reduced by principal component analysis to a mean and
// two basis vectors, and D65 is what that reconstruction gives at 6504 K.
//
// So there is no formula. There is a table, and the table is the definition —
// the numbers below are not samples of some underlying function that could
// be evaluated more finely, they are the illuminant. The 1 nm values the CIE
// publishes were interpolated from the 5 nm ones, which is why this project
// interpolates the same way and says so in `cie.hpp`.
//
// The CCT is the giveaway: 6504 K rather than 6500. The name says 65 because
// the original definition used a value of c2 in Planck's law that has since
// been revised, and correcting the constant moved the temperature but not the
// spectrum. The illuminant is the table; the temperature is a label that has
// drifted off it.
//
// Taken from cvrl.org, `Illuminantd65.csv`, sampled onto the observer's 5 nm
// grid, and generated into this file rather than transcribed.
//
// ── E is not a table, because there is nothing to tabulate ───────────────
//
// Equal-energy white: the same power at every wavelength. It does not exist,
// no lamp makes it, and it is useful precisely because it is the light that
// plays no favourites — a surface under E has a tristimulus value that
// depends on the surface alone. It is also what the white furnace test in
// v0.5 will put its sphere inside.
//
// One line, and no citation needed, which is the contrast worth noticing:
// one of these two illuminants is a measurement of the sky and the other is
// a definition.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// Every other standard illuminant. A, the tungsten one, arrives with the
// chromatic adaptation item, because that item needs a light that is visibly
// not daylight to have anything to demonstrate. The fluorescent F series is
// eleven more tables and nothing in this project needs them.
//
// Any actual lamp. These are standards, not products: a real fixture has a
// spectrum of its own, and the Cornell box's light has a measured one that
// arrives in v0.4.

#pragma once

#include <array>
#include <cstddef>
#include <render/cie.hpp>
#include <render/si.hpp>

namespace render::cie {

// A light, tabulated on the observer's grid. Relative spectral power: the
// absolute scale is meaningless and every use of it divides the scale out.
struct Illuminant {
    std::array<double, samples> table{};

    constexpr double at(double lambda) const { return interpolate(table, lambda); }
};

// It satisfies `SpectralValue`, so a lamp can emit one directly rather than
// being handed four numbers chosen at scene-build time. That is the whole of
// what the v0.3 signature change buys on the emission side.
static_assert(SpectralValue<Illuminant>);

// The tristimulus value of an illuminant seen directly — that is, of a
// perfect white reflector under it. This is the "white point" everything
// downstream is relative to.
constexpr Xyz white_point(const Illuminant& light) {
    Xyz total;
    for (std::size_t i = 0; i < samples; ++i) total += observer[i] * light.table[i];
    return total;
}

// ── D65 ──────────────────────────────────────────────────────────────────
// cvrl.org, Illuminantd65.csv, on the 5 nm grid. Normalised so that
// S(560 nm) = 100, which is the CIE's convention and is why the numbers look
// like percentages.
inline constexpr Illuminant d65 = {{
    46.638300,   // 360 nm
    49.363700,   // 365 nm
    52.089100,   // 370 nm
    51.032300,   // 375 nm
    49.975500,   // 380 nm
    52.311800,   // 385 nm
    54.648200,   // 390 nm
    68.701500,   // 395 nm
    82.754900,   // 400 nm
    87.120400,   // 405 nm
    91.486000,   // 410 nm
    92.458900,   // 415 nm
    93.431800,   // 420 nm
    90.057000,   // 425 nm
    86.682300,   // 430 nm
    95.773600,   // 435 nm
    104.865000,   // 440 nm
    110.936000,   // 445 nm
    117.008000,   // 450 nm
    117.410000,   // 455 nm
    117.812000,   // 460 nm
    116.336000,   // 465 nm
    114.861000,   // 470 nm
    115.392000,   // 475 nm
    115.923000,   // 480 nm
    112.367000,   // 485 nm
    108.811000,   // 490 nm
    109.082000,   // 495 nm
    109.354000,   // 500 nm
    108.578000,   // 505 nm
    107.802000,   // 510 nm
    106.296000,   // 515 nm
    104.790000,   // 520 nm
    106.239000,   // 525 nm
    107.689000,   // 530 nm
    106.047000,   // 535 nm
    104.405000,   // 540 nm
    104.225000,   // 545 nm
    104.046000,   // 550 nm
    102.023000,   // 555 nm
    100.000000,   // 560 nm
    98.167100,   // 565 nm
    96.334200,   // 570 nm
    96.061100,   // 575 nm
    95.788000,   // 580 nm
    92.236800,   // 585 nm
    88.685600,   // 590 nm
    89.345900,   // 595 nm
    90.006200,   // 600 nm
    89.802600,   // 605 nm
    89.599100,   // 610 nm
    88.648900,   // 615 nm
    87.698700,   // 620 nm
    85.493600,   // 625 nm
    83.288600,   // 630 nm
    83.493900,   // 635 nm
    83.699200,   // 640 nm
    81.863000,   // 645 nm
    80.026800,   // 650 nm
    80.120700,   // 655 nm
    80.214600,   // 660 nm
    81.246200,   // 665 nm
    82.277800,   // 670 nm
    80.281000,   // 675 nm
    78.284200,   // 680 nm
    74.002700,   // 685 nm
    69.721300,   // 690 nm
    70.665200,   // 695 nm
    71.609100,   // 700 nm
    72.979000,   // 705 nm
    74.349000,   // 710 nm
    67.976500,   // 715 nm
    61.604000,   // 720 nm
    65.744800,   // 725 nm
    69.885600,   // 730 nm
    72.486300,   // 735 nm
    75.087000,   // 740 nm
    69.339800,   // 745 nm
    63.592700,   // 750 nm
    55.005400,   // 755 nm
    46.418200,   // 760 nm
    56.611800,   // 765 nm
    66.805400,   // 770 nm
    65.094100,   // 775 nm
    63.382800,   // 780 nm
    63.843400,   // 785 nm
    64.304000,   // 790 nm
    61.877900,   // 795 nm
    59.451900,   // 800 nm
    55.705400,   // 805 nm
    51.959000,   // 810 nm
    54.699800,   // 815 nm
    57.440600,   // 820 nm
    58.876500,   // 825 nm
    60.312500,   // 830 nm
}};

// ── A is a formula, with an obsolete constant fossilised in it ───────────
//
// Incandescent light: the CIE's stand-in for a tungsten bulb, and the third
// kind of definition in this file. D65 is a table because daylight has no
// closed form. E is a line because it is a convention. A is a *formula*, and
// the formula is Planck's law — so this one is derived rather than
// transcribed, which is what this project would prefer for all three and can
// only have for one.
//
// What makes it interesting is the constants. The CIE defines A as
//
//      S(lambda) = 100 (560/lambda)^5 (exp(1.435e7 / (2848 * 560)) - 1)
//                                   / (exp(1.435e7 / (2848 * lambda)) - 1)
//
// with lambda in nanometres — and both of those numbers are wrong now. The
// 1.435e7 nm·K is the second radiation constant c2 as it was measured when A
// was defined, and 2848 K is the temperature that went with it. c2 has since
// been revised to 1.4388e-2 m·K, which moves the *label* to 2856 K without
// moving the spectrum at all.
//
// So illuminant A is a blackbody at 2856 K, described by a formula that says
// 2848 K, because the formula also uses an old value of a physical constant
// and the two errors were designed to cancel. Changing either would change
// the illuminant, and the illuminant is the standard, so neither is changed.
// The file implements the standard, and says what it is doing.
//
// Checked: the formula above gives x = 0.44757, y = 0.40744 against the
// published 0.44758, 0.40745. Evaluating modern Planck at the modern label of
// 2856 K gives 0.44753, 0.40743 — which is the cancellation working, to four
// decimal places. Evaluating modern Planck at the *written* 2848 K gives
// 0.44815, 0.40760, which is visibly a different light: the relabelling is
// not cosmetic, and using the formula's temperature with today's constant
// would be the mistake it exists to prevent.
//
// A real tungsten bulb is not this either. It is a blackbody seen through a
// glass envelope, at whatever temperature the filament is at, and this is a
// standard rather than a lamp.

namespace detail {

// exp, at compile time.
//
// `std::exp` is not `constexpr` until C++26, and CLAUDE.md asks for constexpr
// spectra — so that the checks below can be `static_assert`s like D65's,
// rather than a runtime test somebody can forget to run.
//
// Range-reduce to |r| <= ln2/2, sixteen Taylor terms, and scale back by a
// power of two. Measured against `std::exp` over [0, 20]: worst relative
// error 1.39e-15, which is 6.2 ulps; over [6, 14], the range this file
// actually evaluates, 1.12e-15 or 5.0 ulps.
//
// It becomes one word shorter the day `std::exp` is `constexpr`.
inline constexpr double ln2 = 0.69314718055994530942;

constexpr double exp_(double x) {
    const int k = int(x / ln2 + (x >= 0.0 ? 0.5 : -0.5));
    const double r = x - double(k) * ln2;

    double term = 1.0;
    double sum = 1.0;
    for (int n = 1; n <= 16; ++n) {
        term *= r / double(n);
        sum += term;
    }

    double scale = 1.0;
    for (int i = 0; i < (k < 0 ? -k : k); ++i) scale *= 2.0;
    return k < 0 ? sum / scale : sum * scale;
}

// The CIE's defining formula, with its own constants. See above.
constexpr double illuminant_a_at(double nanometres) {
    constexpr double c2_as_defined = 1.435e7;   // nm·K, the 1931 value
    constexpr double temperature   = 2848.0;    // K, the temperature that went with it
    constexpr double reference     = 560.0;     // nm, where it is normalised to 100

    const double ratio = reference / nanometres;
    const double five = ratio * ratio * ratio * ratio * ratio;
    return 100.0 * five
         * (exp_(c2_as_defined / (temperature * reference)) - 1.0)
         / (exp_(c2_as_defined / (temperature * nanometres)) - 1.0);
}

} // namespace detail

inline constexpr Illuminant a = [] {
    Illuminant lamp;
    for (std::size_t i = 0; i < samples; ++i)
        lamp.table[i] = detail::illuminant_a_at(si::as::nm(first + double(i) * step));
    return lamp;
}();

// ── E ────────────────────────────────────────────────────────────────────
// Equal energy. The `100` matches D65's normalisation so that the two can be
// compared without a scale factor appearing between them.
inline constexpr Illuminant e = [] {
    Illuminant flat;
    for (std::size_t i = 0; i < samples; ++i) flat.table[i] = 100.0;
    return flat;
}();

// ── Checked at compile time ──────────────────────────────────────────────
//
// This is the check the whole colour half of the project rests on, and it is
// the one item 0049 exists for: the illuminant above, integrated against the
// observer in `cie.hpp` by this project's own arithmetic, has to land on the
// chromaticity the CIE published for D65.
//
// If the tables were transcribed wrong, or the interpolation is wrong, or the
// normalisation is wrong, this is where it shows. Every one of those failures
// is silent everywhere else.
//
//      published (CIE 15:2004, 2-degree observer)   x = 0.31272  y = 0.32903
//      computed here                                x = 0.31271  y = 0.32901
//
// Agreeing to four decimal places, with the fifth out by one and two counts
// respectively. That residual is the 5 nm quadrature again — the published
// figure comes from the 1 nm tables — and it is quoted rather than rounded
// away, because "agrees to four decimals" is a claim and "agrees" is not.

namespace check {

inline constexpr Chromaticity d65_chromaticity = chromaticity_of(white_point(d65));

static_assert(d65_chromaticity.x > 0.31262 && d65_chromaticity.x < 0.31282,
              "D65 must land within 1e-4 of its published x");
static_assert(d65_chromaticity.y > 0.32893 && d65_chromaticity.y < 0.32913,
              "D65 must land within 1e-4 of its published y");

// And the tighter statement, so that a change which moves it by 1e-5 is
// noticed rather than absorbed by a loose bound.
static_assert(d65_chromaticity.x > 0.312705 && d65_chromaticity.x < 0.312715,
              "D65 x has moved; re-derive before widening this");
static_assert(d65_chromaticity.y > 0.329005 && d65_chromaticity.y < 0.329015,
              "D65 y has moved; re-derive before widening this");

// Illuminant A, derived from the formula rather than tabulated, against the
// chromaticity the CIE publishes for it.
inline constexpr Chromaticity a_chromaticity = chromaticity_of(white_point(a));

static_assert(a_chromaticity.x > 0.44748 && a_chromaticity.x < 0.44768,
              "illuminant A must land within 1e-4 of its published x");
static_assert(a_chromaticity.y > 0.40735 && a_chromaticity.y < 0.40755,
              "illuminant A must land within 1e-4 of its published y");

// Tighter, so a change that moves it by 1e-5 is noticed. This also checks the
// constexpr exp above: a wrong exponential would move these long before it
// moved anything visible.
static_assert(a_chromaticity.x > 0.447565 && a_chromaticity.x < 0.447575,
              "illuminant A x has moved; re-derive before widening this");
static_assert(a_chromaticity.y > 0.407435 && a_chromaticity.y < 0.407445,
              "illuminant A y has moved; re-derive before widening this");

// A is much warmer than D65, which is the entire point of having it: a render
// lit by it is orange unless something adapts, and `bradford.hpp` is the
// something.
static_assert(a_chromaticity.x > d65_chromaticity.x + 0.13,
              "A must be substantially warmer than D65, or it demonstrates nothing");

// E is flat, so its white point is the observer's own integral, and it must
// land where `cie.hpp` already checked equal-energy white lands.
inline constexpr Chromaticity e_chromaticity = chromaticity_of(white_point(e));

static_assert(e_chromaticity.x > 0.33331 && e_chromaticity.x < 0.33334,
              "illuminant E must be achromatic in x");
static_assert(e_chromaticity.y > 0.33327 && e_chromaticity.y < 0.33331,
              "illuminant E must be achromatic in y");

} // namespace check

} // namespace render::cie
