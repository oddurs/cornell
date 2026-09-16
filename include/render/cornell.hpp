// cornell.hpp — the box, as somebody measured it.
//
// This is the file the repository is named after, and the reason the project
// exists. Everything before it is machinery; this is the object.
//
// Item 0051 is the spike that had to close before a line of this could be
// written, and it should be read alongside this file. The short version:
//
//      The data is primary and it is measured. The publication is gone from
//      the live web and survives only in the Internet Archive, which is why
//      the numbers are embedded here rather than fetched. The units are not
//      stated anywhere and had to be inferred. The box is not rectangular,
//      because it was measured rather than drawn. And the light's spectrum is
//      four numbers.
//
// Source, cited once, for everything below:
//
//      Cornell University Program of Computer Graphics, "Cornell Box Data",
//      http://www.graphics.cornell.edu/online/box/data.html
//      last updated 2 February 2005; that URL is dead, and the page is at
//      https://web.archive.org/web/2018/http://www.graphics.cornell.edu/online/box/data.html
//
// ── Every figure is marked ───────────────────────────────────────────────
//
// In the manner of `windsor`, which marks a number CALIBRATED when it was
// chosen rather than measured, so that a reader can see exactly how much of
// a model is evidence:
//
//      MEASURED    Cornell measured it and published it
//      INFERRED    not stated, but established from the rest of the data
//      ASSUMED     Cornell assumed it and said so
//      CALIBRATED  this project chose it, because the data does not say
//
// ── The walls ────────────────────────────────────────────────────────────
//
// MEASURED. Three reflectance spectra, 400 to 700 nm at 4 nm, 76 samples
// each, of the actual paint on the actual walls.
//
// This is where the no-RGB rule stops being a principle and starts being
// visible, and the numbers say so before anything is rendered. The red wall
// never rises above 0.657 and never falls below 0.040 — it is a broad gentle
// curve that happens to be higher at long wavelengths, not a saturated
// primary. The green wall peaks at 0.481 around 528 nm and sits near 0.1
// everywhere else. The white wall is not flat either: 0.343 at 400 nm rising
// to about 0.74 and staying there.
//
// Colour bleeding — the single most famous thing about this image — is the
// product of one of these curves with the light's spectrum, integrated
// against the observer. From an RGB triple it is a plausible tint. From these
// numbers it is the answer.
//
// None of these go anywhere near `jakob_hanika.hpp`. That file is for a
// colour somebody invented; these were measured, and mixing the two would
// make the v1.0 comparison meaningless.
//
// ── What holding the endpoints costs ─────────────────────────────────────
//
// The data stops at 400 and 700 nm and this project samples wavelengths from
// 360 to 830, so something must happen outside the measured range.
// `Measured::at` holds the nearest endpoint, and the honest question is how
// much of the observer lives out there:
//
//                      below 400 nm    above 700 nm
//      x-bar              0.0796 %        0.1296 %
//      y-bar              0.0023 %        0.0468 %
//      z-bar              0.3760 %        0.0000 %
//
// A twentieth of a per cent of the luminance and a third of a per cent of the
// blue, and the wall spectra are flat-ish at both ends, so holding them is a
// small extrapolation over a region that barely contributes. Reporting zero
// instead would darken every surface by those fractions, systematically,
// which is worse than an admitted extrapolation — and it would do it
// unevenly across the three channels, which is worse again, because a
// systematic error that is the same everywhere is invisible and one that
// differs by channel is a tint.

#pragma once

#include <array>
#include <cstddef>
#include <render/cie.hpp>
#include <render/spectrum.hpp>

namespace render::cornell {

// The grid the reflectances were measured on. MEASURED.
//
// Stated in nanometres as well as metres, and the reason is the one `si.hpp`
// spends a paragraph on. In nanometres the grid is exact integer arithmetic:
// 400 + 4 x 75 is 700 and there is nothing to discuss. In metres it is not —
// `400.0e-9 + 4.0e-9 * 75` comes out as 7.0000000000000007e-7 against a
// `700.0e-9` of 6.9999999999999997e-7, one ulp apart, because none of those
// three values is representable and the roundings do not cancel.
//
// So the check below is done in the units where the claim is exactly true,
// and the metres are derived from them. This is not pedantry about a
// difference of 1e-22 metres; it is that `static_assert` takes a bool, and a
// check written in the wrong units is a check that has to be softened into a
// tolerance and then means less.
inline constexpr int reflectance_first_nm = 400;
inline constexpr int reflectance_step_nm  = 4;
inline constexpr int reflectance_last_nm  = 700;
inline constexpr std::size_t reflectance_samples = 76;

static_assert(reflectance_first_nm + reflectance_step_nm * int(reflectance_samples - 1)
                  == reflectance_last_nm,
              "the reflectance grid must end at 700 nm");

inline constexpr double reflectance_first = double(reflectance_first_nm) / 1e9;
inline constexpr double reflectance_step  = double(reflectance_step_nm) / 1e9;

using WallSpectrum = Measured<reflectance_samples>;

// MEASURED. The white walls: floor, ceiling, back wall, and both blocks.
inline constexpr WallSpectrum white = {reflectance_first, reflectance_step, {
    0.343, 0.445, 0.551, 0.624, 0.665, 0.687,
    0.708, 0.723, 0.715, 0.710, 0.745, 0.758,
    0.739, 0.767, 0.777, 0.765, 0.751, 0.745,
    0.748, 0.729, 0.745, 0.757, 0.753, 0.750,
    0.746, 0.747, 0.735, 0.732, 0.739, 0.734,
    0.725, 0.721, 0.733, 0.725, 0.732, 0.743,
    0.744, 0.748, 0.728, 0.716, 0.733, 0.726,
    0.713, 0.740, 0.754, 0.764, 0.752, 0.736,
    0.734, 0.741, 0.740, 0.732, 0.745, 0.755,
    0.751, 0.744, 0.731, 0.733, 0.744, 0.731,
    0.712, 0.708, 0.729, 0.730, 0.727, 0.707,
    0.703, 0.729, 0.750, 0.760, 0.751, 0.739,
    0.724, 0.730, 0.740, 0.737,
}};

// MEASURED. The right wall, seen from the camera.
inline constexpr WallSpectrum green = {reflectance_first, reflectance_step, {
    0.092, 0.096, 0.098, 0.097, 0.098, 0.095,
    0.095, 0.097, 0.095, 0.094, 0.097, 0.098,
    0.096, 0.101, 0.103, 0.104, 0.107, 0.109,
    0.112, 0.115, 0.125, 0.140, 0.160, 0.187,
    0.229, 0.285, 0.343, 0.390, 0.435, 0.464,
    0.472, 0.476, 0.481, 0.462, 0.447, 0.441,
    0.426, 0.406, 0.373, 0.347, 0.337, 0.314,
    0.285, 0.277, 0.266, 0.250, 0.230, 0.207,
    0.186, 0.171, 0.160, 0.148, 0.141, 0.136,
    0.130, 0.126, 0.123, 0.121, 0.122, 0.119,
    0.114, 0.115, 0.117, 0.117, 0.118, 0.120,
    0.122, 0.128, 0.132, 0.139, 0.144, 0.146,
    0.150, 0.152, 0.157, 0.159,
}};

// MEASURED. The left wall, seen from the camera.
inline constexpr WallSpectrum red = {reflectance_first, reflectance_step, {
    0.040, 0.046, 0.048, 0.053, 0.049, 0.050,
    0.053, 0.055, 0.057, 0.056, 0.059, 0.057,
    0.061, 0.061, 0.060, 0.062, 0.062, 0.062,
    0.061, 0.062, 0.060, 0.059, 0.057, 0.058,
    0.058, 0.058, 0.056, 0.055, 0.056, 0.059,
    0.057, 0.055, 0.059, 0.059, 0.058, 0.059,
    0.061, 0.061, 0.063, 0.063, 0.067, 0.068,
    0.072, 0.080, 0.090, 0.099, 0.124, 0.154,
    0.192, 0.255, 0.287, 0.349, 0.402, 0.443,
    0.487, 0.513, 0.558, 0.584, 0.620, 0.606,
    0.609, 0.651, 0.612, 0.610, 0.650, 0.638,
    0.627, 0.620, 0.630, 0.628, 0.642, 0.639,
    0.657, 0.639, 0.635, 0.642,
}};

// ── Checked at compile time ──────────────────────────────────────────────
//
// House rule 5, and the transcription of 228 numbers is exactly the kind of
// thing that is silent when it goes wrong. These are the facts about the
// published tables that a reader can verify against the page.

namespace check {

static_assert(white.lowest()  == 0.343, "white starts at 0.343 at 400 nm");
static_assert(white.highest() == 0.777, "white peaks at 0.777");
static_assert(green.lowest()  == 0.092, "green bottoms at 0.092");
static_assert(green.highest() == 0.481, "green peaks at 0.481");
static_assert(red.lowest()    == 0.040, "red bottoms at 0.040");
static_assert(red.highest()   == 0.657, "red peaks at 0.657");

// The endpoints, which pin the tables at both ends.
static_assert(white.table[0] == 0.343 && white.table[reflectance_samples - 1] == 0.737);
static_assert(green.table[0] == 0.092 && green.table[reflectance_samples - 1] == 0.159);
static_assert(red.table[0]   == 0.040 && red.table[reflectance_samples - 1]   == 0.642);

// No reflectance may exceed 1, or the wall is an energy source. Cornell's
// paint does not come close, and the assert is here because the day this file
// gains a fourth surface is the day somebody types a number.
constexpr bool physical(const WallSpectrum& s) {
    return s.lowest() >= 0.0 && s.highest() <= 1.0;
}
static_assert(physical(white) && physical(green) && physical(red),
              "a reflectance outside [0,1] is not a material");

// The red wall is not red the way a display is red. If this ever fails,
// somebody has replaced measured paint with a primary.
static_assert(red.highest() < 0.70, "the red wall reflects at most 0.657, not 1");
static_assert(green.highest() < 0.50, "the green wall reflects at most 0.481, not 1");

} // namespace check

} // namespace render::cornell
