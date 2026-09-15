// cie.hpp — seventeen people in London, around 1930.
//
// This is one of exactly two files in this project allowed to produce three
// numbers from a spectrum. `spectrum.hpp` spends a page explaining why there
// is no RGB anywhere upstream of here; this is the boundary that argument was
// protecting, and it converts because eyes have three cone types and there is
// no way around that fact.
//
// ── Whose eyes ───────────────────────────────────────────────────────────
//
// Not a law. A fit, to a small number of measurements, on a small number of
// people, made with equipment that would now be considered charming.
//
// W. David Wright measured ten observers at Imperial College between 1928 and
// 1929. John Guild measured seven at the National Physical Laboratory. Each
// observer sat in the dark and turned three knobs — the intensities of three
// primary lights — until a mixture of them matched a monochromatic test
// light, wavelength by wavelength, across the visible range. The two sets of
// results, taken with different primaries and different apparatus, agreed
// closely enough that the CIE adopted their average in 1931.
//
// Seventeen young British men, then, and a hundred years of the colour
// science of every screen, every camera, every print process and every
// television standard rests on the average of what their eyes did.
//
// House rule 8 says a file named for people is named that way on purpose and
// that the credit is also a warning. This is the most load-bearing example of
// it in the project, so the warning goes here rather than in a footnote:
//
//      Individual variation between observers is large — far larger than the
//      precision this file computes to — and "the standard observer" is
//      nobody. It is an average with no person behind it.
//
//      The functions have known problems in the blue. The 1931 x-bar was
//      adjusted to fit the photopic luminosity curve of the day, which was
//      itself too low at short wavelengths; Judd in 1951 and Vos in 1978
//      published corrections, and neither replaced the original, because by
//      then the world was built on it.
//
//      It is a 2-degree observer: it describes colour matching in a patch
//      about the width of a thumbnail at arm's length. The 10-degree observer
//      from 1964 is a different table for a different viewing condition, and
//      the Cornell box subtends rather more than two degrees.
//
// None of which is a sneer. Wright and Guild did extraordinary work with what
// they had, and their result has survived a century of people trying to
// improve on it. It is simply not a law of nature, and a project that makes a
// point of deriving rather than declaring should be clear about which of its
// inputs are measurements of the world and which are measurements of people.
//
// ── The tables ───────────────────────────────────────────────────────────
//
// CIE 1931 2-degree standard observer, x-bar, y-bar and z-bar, tabulated at
// 5 nm from 360 to 830 nm. Taken from the Colour & Vision Research Laboratory
// database at UCL (cvrl.org), file `ciexyz31.csv`, which publishes the CIE's
// values rather than a re-derivation of them.
//
// They are not transcribed by hand. The array below was generated from that
// file, which is the only honest way to move 285 numbers.
//
// Checked against what is published about them, at compile time, below:
// y-bar peaks at 555 nm with a value of exactly 1, z-bar peaks at 445 nm, and
// equal-energy white lands on the achromatic point.
//
// ── y-bar is the photopic luminosity function ────────────────────────────
//
// Worth stating because it is the reason `si.hpp` has no photometric units in
// it. The middle of the three functions *is* V(lambda), the curve that turns
// watts into lumens — so a lumen is a watt weighted by this table, a candela
// is a lumen per steradian, and every photometric unit in existence is a
// radiometric one that has been through the eyes of seventeen people.
//
// `si.hpp` says that putting a lumen in the dictionary would put an eye
// inside it. This is where the eye arrives, once, in a file that says whose.

#pragma once

#include <array>
#include <cstddef>
#include <render/si.hpp>
#include <render/spectrum.hpp>

namespace render {

// Tristimulus values. Not a colour, not an RGB, and not a radiance: three
// numbers that say how strongly a spectrum stimulates the three matching
// functions. `srgb.hpp` is what turns them into something a monitor can show,
// and it is the only file allowed to.
struct Xyz {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    constexpr Xyz& operator+=(const Xyz& o) { x += o.x; y += o.y; z += o.z; return *this; }
    constexpr Xyz& operator*=(double s)     { x *= s;   y *= s;   z *= s;   return *this; }
};

constexpr Xyz operator+(Xyz a, const Xyz& b) { return a += b; }
constexpr Xyz operator*(Xyz a, double s)     { return a *= s; }
constexpr Xyz operator*(double s, Xyz a)     { return a *= s; }

// Chromaticity: the direction of a tristimulus value, with its brightness
// divided out. Two lights with the same chromaticity look the same colour and
// may differ in how bright they are.
struct Chromaticity {
    double x = 0.0;
    double y = 0.0;
};

constexpr Chromaticity chromaticity_of(const Xyz& c) {
    const double sum = c.x + c.y + c.z;
    return sum > 0.0 ? Chromaticity{c.x / sum, c.y / sum} : Chromaticity{};
}

// ── A 3 x 3, because tristimulus keeps needing one ───────────────────────
//
// Written out rather than pulled in: house rule 4 has no linear algebra
// library in it, and a matrix that exists to be inverted once at compile time
// does not need an abstraction. It lives here rather than in `srgb.hpp`,
// where it started, because `bradford.hpp` needs one too and a transform
// between illuminants should not depend on a display standard.

struct Matrix3 {
    std::array<std::array<double, 3>, 3> m{};

    constexpr const std::array<double, 3>& operator[](std::size_t r) const { return m[r]; }
    constexpr std::array<double, 3>&       operator[](std::size_t r)       { return m[r]; }
};

// Nine numbers in reading order. `Matrix3` wraps an array of arrays, so the
// aggregate form needs three levels of braces and gets miscounted; this reads
// like the matrix on the page.
constexpr Matrix3 matrix3(double a, double b, double c,
                          double d, double e, double f,
                          double g, double h, double i) {
    Matrix3 out;
    out[0] = {a, b, c};
    out[1] = {d, e, f};
    out[2] = {g, h, i};
    return out;
}

constexpr Matrix3 identity3() { return matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1); }

constexpr Xyz apply(const Matrix3& a, double x, double y, double z) {
    return Xyz{a[0][0] * x + a[0][1] * y + a[0][2] * z,
               a[1][0] * x + a[1][1] * y + a[1][2] * z,
               a[2][0] * x + a[2][1] * y + a[2][2] * z};
}

constexpr Xyz apply(const Matrix3& a, const Xyz& v) { return apply(a, v.x, v.y, v.z); }

constexpr Matrix3 multiply(const Matrix3& a, const Matrix3& b) {
    Matrix3 out;
    for (std::size_t r = 0; r < 3; ++r)
        for (std::size_t c = 0; c < 3; ++c)
            out[r][c] = a[r][0] * b[0][c] + a[r][1] * b[1][c] + a[r][2] * b[2][c];
    return out;
}

constexpr double determinant(const Matrix3& a) {
    return a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1])
         - a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0])
         + a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]);
}

constexpr Matrix3 inverse(const Matrix3& a) {
    const double d = determinant(a);
    Matrix3 out;
    out[0][0] = (a[1][1] * a[2][2] - a[1][2] * a[2][1]) / d;
    out[0][1] = (a[0][2] * a[2][1] - a[0][1] * a[2][2]) / d;
    out[0][2] = (a[0][1] * a[1][2] - a[0][2] * a[1][1]) / d;
    out[1][0] = (a[1][2] * a[2][0] - a[1][0] * a[2][2]) / d;
    out[1][1] = (a[0][0] * a[2][2] - a[0][2] * a[2][0]) / d;
    out[1][2] = (a[0][2] * a[1][0] - a[0][0] * a[1][2]) / d;
    out[2][0] = (a[1][0] * a[2][1] - a[1][1] * a[2][0]) / d;
    out[2][1] = (a[0][1] * a[2][0] - a[0][0] * a[2][1]) / d;
    out[2][2] = (a[0][0] * a[1][1] - a[0][1] * a[1][0]) / d;
    return out;
}

namespace cie {

// ── The grid the CIE publishes on ────────────────────────────────────────
//
// 5 nm from 360 to 830. Everything tabulated in this project's colour files
// lives on it, so that an illuminant and the observer can be multiplied
// together without either being resampled onto the other.

inline constexpr int    samples = 95;
inline constexpr double first   = 360.0e-9;
inline constexpr double last    = 830.0e-9;
inline constexpr double step    = 5.0e-9;

static_assert(first + step * (samples - 1) == last,
              "the grid constants must describe the table that follows");

// Linear interpolation on that grid, for a table of anything.
//
// Linear rather than anything smoother, deliberately. The CIE publishes 1 nm
// values obtained by interpolating these same 5 nm entries, and the method
// they used for it was linear — so a cubic here would be smoother than the
// standard it claims to implement, and would disagree with everybody else's
// numbers in the fourth decimal for no reason anybody could defend.
template <class T>
constexpr T interpolate(const std::array<T, samples>& table, double lambda) {
    if (lambda <= first) return table[0];
    if (lambda >= last)  return table[samples - 1];

    const double position = (lambda - first) / step;
    const std::size_t index = std::size_t(position);
    const double fraction = position - double(index);

    // `index + 1` is in range because `lambda < last` was handled above.
    return table[index] * (1.0 - fraction) + table[index + 1] * fraction;
}

// ── The 1931 2-degree standard observer ──────────────────────────────────
//
// Generated from cvrl.org's ciexyz31.csv. See the top of the file.
inline constexpr std::array<Xyz, samples> observer = {{
    {0.000129900, 0.000003917, 0.000606100},   // 360 nm
    {0.000232100, 0.000006965, 0.001086000},   // 365 nm
    {0.000414900, 0.000012390, 0.001946000},   // 370 nm
    {0.000741600, 0.000022020, 0.003486000},   // 375 nm
    {0.001368000, 0.000039000, 0.006450001},   // 380 nm
    {0.002236000, 0.000064000, 0.010549990},   // 385 nm
    {0.004243000, 0.000120000, 0.020050010},   // 390 nm
    {0.007650000, 0.000217000, 0.036210000},   // 395 nm
    {0.014310000, 0.000396000, 0.067850010},   // 400 nm
    {0.023190000, 0.000640000, 0.110200000},   // 405 nm
    {0.043510000, 0.001210000, 0.207400000},   // 410 nm
    {0.077630000, 0.002180000, 0.371300000},   // 415 nm
    {0.134380000, 0.004000000, 0.645600000},   // 420 nm
    {0.214770000, 0.007300000, 1.039050100},   // 425 nm
    {0.283900000, 0.011600000, 1.385600000},   // 430 nm
    {0.328500000, 0.016840000, 1.622960000},   // 435 nm
    {0.348280000, 0.023000000, 1.747060000},   // 440 nm
    {0.348060000, 0.029800000, 1.782600000},   // 445 nm
    {0.336200000, 0.038000000, 1.772110000},   // 450 nm
    {0.318700000, 0.048000000, 1.744100000},   // 455 nm
    {0.290800000, 0.060000000, 1.669200000},   // 460 nm
    {0.251100000, 0.073900000, 1.528100000},   // 465 nm
    {0.195360000, 0.090980000, 1.287640000},   // 470 nm
    {0.142100000, 0.112600000, 1.041900000},   // 475 nm
    {0.095640000, 0.139020000, 0.812950100},   // 480 nm
    {0.057950010, 0.169300000, 0.616200000},   // 485 nm
    {0.032010000, 0.208020000, 0.465180000},   // 490 nm
    {0.014700000, 0.258600000, 0.353300000},   // 495 nm
    {0.004900000, 0.323000000, 0.272000000},   // 500 nm
    {0.002400000, 0.407300000, 0.212300000},   // 505 nm
    {0.009300000, 0.503000000, 0.158200000},   // 510 nm
    {0.029100000, 0.608200000, 0.111700000},   // 515 nm
    {0.063270000, 0.710000000, 0.078249990},   // 520 nm
    {0.109600000, 0.793200000, 0.057250010},   // 525 nm
    {0.165500000, 0.862000000, 0.042160000},   // 530 nm
    {0.225749900, 0.914850100, 0.029840000},   // 535 nm
    {0.290400000, 0.954000000, 0.020300000},   // 540 nm
    {0.359700000, 0.980300000, 0.013400000},   // 545 nm
    {0.433449900, 0.994950100, 0.008749999},   // 550 nm
    {0.512050100, 1.000000000, 0.005749999},   // 555 nm
    {0.594500000, 0.995000000, 0.003900000},   // 560 nm
    {0.678400000, 0.978600000, 0.002749999},   // 565 nm
    {0.762100000, 0.952000000, 0.002100000},   // 570 nm
    {0.842500000, 0.915400000, 0.001800000},   // 575 nm
    {0.916300000, 0.870000000, 0.001650001},   // 580 nm
    {0.978600000, 0.816300000, 0.001400000},   // 585 nm
    {1.026300000, 0.757000000, 0.001100000},   // 590 nm
    {1.056700000, 0.694900000, 0.001000000},   // 595 nm
    {1.062200000, 0.631000000, 0.000800000},   // 600 nm
    {1.045600000, 0.566800000, 0.000600000},   // 605 nm
    {1.002600000, 0.503000000, 0.000340000},   // 610 nm
    {0.938400000, 0.441200000, 0.000240000},   // 615 nm
    {0.854449900, 0.381000000, 0.000190000},   // 620 nm
    {0.751400000, 0.321000000, 0.000100000},   // 625 nm
    {0.642400000, 0.265000000, 0.000050000},   // 630 nm
    {0.541900000, 0.217000000, 0.000030000},   // 635 nm
    {0.447900000, 0.175000000, 0.000020000},   // 640 nm
    {0.360800000, 0.138200000, 0.000010000},   // 645 nm
    {0.283500000, 0.107000000, 0.000000000},   // 650 nm
    {0.218700000, 0.081600000, 0.000000000},   // 655 nm
    {0.164900000, 0.061000000, 0.000000000},   // 660 nm
    {0.121200000, 0.044580000, 0.000000000},   // 665 nm
    {0.087400000, 0.032000000, 0.000000000},   // 670 nm
    {0.063600000, 0.023200000, 0.000000000},   // 675 nm
    {0.046770000, 0.017000000, 0.000000000},   // 680 nm
    {0.032900000, 0.011920000, 0.000000000},   // 685 nm
    {0.022700000, 0.008210000, 0.000000000},   // 690 nm
    {0.015840000, 0.005723000, 0.000000000},   // 695 nm
    {0.011359160, 0.004102000, 0.000000000},   // 700 nm
    {0.008110916, 0.002929000, 0.000000000},   // 705 nm
    {0.005790346, 0.002091000, 0.000000000},   // 710 nm
    {0.004109457, 0.001484000, 0.000000000},   // 715 nm
    {0.002899327, 0.001047000, 0.000000000},   // 720 nm
    {0.002049190, 0.000740000, 0.000000000},   // 725 nm
    {0.001439971, 0.000520000, 0.000000000},   // 730 nm
    {0.000999949, 0.000361100, 0.000000000},   // 735 nm
    {0.000690079, 0.000249200, 0.000000000},   // 740 nm
    {0.000476021, 0.000171900, 0.000000000},   // 745 nm
    {0.000332301, 0.000120000, 0.000000000},   // 750 nm
    {0.000234826, 0.000084800, 0.000000000},   // 755 nm
    {0.000166151, 0.000060000, 0.000000000},   // 760 nm
    {0.000117413, 0.000042400, 0.000000000},   // 765 nm
    {0.000083075, 0.000030000, 0.000000000},   // 770 nm
    {0.000058707, 0.000021200, 0.000000000},   // 775 nm
    {0.000041510, 0.000014990, 0.000000000},   // 780 nm
    {0.000029353, 0.000010600, 0.000000000},   // 785 nm
    {0.000020674, 0.000007466, 0.000000000},   // 790 nm
    {0.000014560, 0.000005258, 0.000000000},   // 795 nm
    {0.000010254, 0.000003703, 0.000000000},   // 800 nm
    {0.000007221, 0.000002608, 0.000000000},   // 805 nm
    {0.000005086, 0.000001837, 0.000000000},   // 810 nm
    {0.000003582, 0.000001293, 0.000000000},   // 815 nm
    {0.000002523, 0.000000911, 0.000000000},   // 820 nm
    {0.000001777, 0.000000642, 0.000000000},   // 825 nm
    {0.000001251, 0.000000452, 0.000000000},   // 830 nm
}};

// x-bar, y-bar and z-bar at any wavelength, in metres because everything in
// this project is in metres.
constexpr Xyz match(double lambda) { return interpolate(observer, lambda); }

// ── The estimator ────────────────────────────────────────────────────────
//
// The integral being estimated is
//
//      X = integral of L(lambda) * x-bar(lambda) d(lambda)
//
// and a path carries four wavelengths drawn from a known density, so the
// Monte Carlo estimate is the value over that density, summed and averaged —
// written out as the ratio, at the point of use, which is house rule 3.
//
// This is where `Wavelengths::pdf()` finally gets called. `spectrum.hpp` kept
// it through two milestones with nothing calling it, on the grounds that a
// sample and the density it was drawn from are one object and that this day
// would come.
constexpr Xyz xyz_estimate(const Wavelengths& lambdas, const Radiance& value) {
    Xyz total;
    for (int i = 0; i < spectral_samples; ++i)
        total += match(lambdas[i]) * (value[i] / Wavelengths::pdf());
    return total * (1.0 / double(spectral_samples));
}

// ── The normalisation, which is an integral and not a constant ───────────
//
// A reflectance of 1 at every wavelength, lit by an illuminant S, should come
// out with Y = 1 — that is what makes Y a *relative* luminance and what lets
// a render be compared against a photograph of a white card. The constant
// that arranges it is
//
//      k = 1 / integral of S(lambda) * y-bar(lambda) d(lambda)
//
// which depends on the illuminant, so it is a function of one rather than a
// number in this file. Quoting a `k` without saying which light it was for is
// how a renderer ends up a constant factor wrong and nobody notices, because
// everything in the image is wrong by the same factor.
template <class Illuminant>
constexpr double luminance_normalisation(const Illuminant& light) {
    double integral = 0.0;
    for (int i = 0; i < samples; ++i)
        integral += light.table[std::size_t(i)] * observer[std::size_t(i)].y;
    return integral > 0.0 ? 1.0 / (integral * step) : 0.0;
}

// ── What the tables are checked against, at compile time ─────────────────
//
// House rule 5. These are the facts about the 1931 observer that anybody can
// look up, and if a transcription had gone wrong they are what would catch
// it. A wrong table is silent everywhere else and would poison every colour
// in the project.

namespace check {

constexpr std::size_t index_of(double lambda) {
    return std::size_t((lambda - first) / step + 0.5);
}

// y-bar is the photopic luminosity function, normalised to peak at 1.
static_assert(observer[index_of(555.0e-9)].y == 1.0,
              "y-bar must be exactly 1 at its peak");

constexpr bool peaks_at(double lambda, int component) {
    const std::size_t peak = index_of(lambda);
    for (std::size_t i = 0; i < samples; ++i) {
        const double here  = component == 1 ? observer[i].y : observer[i].z;
        const double there = component == 1 ? observer[peak].y : observer[peak].z;
        if (here > there) return false;
    }
    return true;
}

static_assert(peaks_at(555.0e-9, 1), "y-bar peaks at 555 nm");
static_assert(peaks_at(445.0e-9, 2), "z-bar peaks at 445 nm");

// Equal-energy white — a flat spectrum, CIE illuminant E — must land on the
// achromatic point, x = y = 1/3.
//
// It does not land there exactly, and the tolerance says how far off it is
// rather than hiding it. The residual is the 5 nm quadrature: the tables are
// a sampled version of continuous functions whose integrals are equal by
// definition, and summing 95 samples of them is not the same as integrating.
// Measured, it is 2e-5.
constexpr Chromaticity equal_energy_white() {
    Xyz total;
    for (std::size_t i = 0; i < samples; ++i) total += observer[i];
    return chromaticity_of(total);
}

static_assert(equal_energy_white().x > 0.33331 && equal_energy_white().x < 0.33334,
              "equal-energy white must land on the achromatic point in x");
static_assert(equal_energy_white().y > 0.33327 && equal_energy_white().y < 0.33331,
              "equal-energy white must land on the achromatic point in y");

} // namespace check

} // namespace cie
} // namespace render
