// metals.hpp — two columns of numbers per metal, and nothing else.
//
// This file is the project's answer to its own thesis. `fresnel.hpp` has the
// equations, `conductor.hpp` joins them to a surface, and what is left is the
// only input a metal gets: a measurement of its complex refractive index,
// made by somebody with a spectrometer, transcribed with its citation.
//
// There is no colour here. There is no `vec3`, no tint, no hex string, and
// `grep -r "0.766" include/` returns nothing. Gold is yellow in this
// renderer because n falls from 1.38 to 0.13 across the visible while k rises
// from 1.9 to 4.1, so Fresnel returns 0.41 at 450 nm and 0.97 at 700 nm, so
// the blue is absorbed and the red is not. Swap in copper's table and the
// same code returns 0.54 and 0.96 with the rise starting later, and the
// reflection turns pink. Nobody chose either.
//
// ── Johnson and Christy, 1972 ────────────────────────────────────────────
//
//      P. B. Johnson and R. W. Christy, "Optical constants of the noble
//      metals", Physical Review B 6, 4370-4379 (1972).
//      https://doi.org/10.1103/PhysRevB.6.4370
//
// Copper, silver and gold, measured on films evaporated in a vacuum and
// measured in it, at 49 photon energies from 0.64 to 6.6 eV. It is the most
// used optical data set in computer graphics and one of the most cited papers
// in the field, and it is fifty-four years old.
//
// ── Rakic, 1995, cited separately because it is a different measurement ──
//
//      A. D. Rakic, "Algorithm for the determination of intrinsic optical
//      constants of metal films: application to aluminum", Applied Optics 34,
//      4755-4767 (1995). https://doi.org/10.1364/AO.34.004755
//
// Aluminium is not a noble metal and Johnson and Christy did not measure it.
// Extending their citation to cover it would be the beginning of the rot this
// project exists to avoid — a table whose provenance is "the same place as
// the other tables, probably" — so it has its own line, its own paper, and
// its own range.
//
// It is also the control. Aluminium is nearly neutral: 0.923 at 450 nm and
// 0.900 at 700 nm, a 2.3-point tilt across the whole visible against gold's
// 56. If a rendering of aluminium in this project comes out tinted, the tint
// is the pipeline's and not the metal's, and that is what makes it worth
// having in a file whose other three entries are strongly coloured.
//
// ── Where the numbers came from, which is not the same as who measured ──
//
// Transcribed from refractiveindex.info, retrieved 17 September 2026, which
// mirrors both papers' tables. That is a second claim and it gets its own
// sentence: the citation above is who made the measurement, and this is the
// path the digits took to get here. A reader who wants to check them against
// the printed paper should, and this file is what they would be checking.
//
// One thing the mirror lost. Johnson and Christy's grid is uniform in photon
// energy — that is how a spectrometer of that period was driven — and the
// mirror stores wavelengths rounded to 0.1 nm. Converting back gives steps of
// 0.1177 to 0.1314 eV against a constant 0.124, so the exact energies are not
// recoverable. The wavelengths are transcribed as the mirror gives them,
// which is the honest thing to hold, and `spectrum.hpp`'s `Tabulated` exists
// because that grid is uniform in energy and therefore not in wavelength.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// **Sample dependence**, which is larger than the measurement error. Johnson
// and Christy measured evaporated films; a rolled sheet, an electroplated
// surface, a cast ingot and a sputtered coating differ from those films and
// from each other by more than the error bars on any one of them. This
// renders one piece of metal somebody made in 1972.
//
// **Oxidation.** Real aluminium grows a few nanometres of oxide within
// seconds of meeting air and real silver tarnishes, and both are thin films —
// interference, which `fresnel.hpp` cannot produce because it adds powers
// rather than amplitudes. These tables are clean surfaces that do not exist
// outside a vacuum chamber.
//
// **Temperature**, which shifts both n and k, and matters for a filament.

#pragma once

#include <array>
#include <cstddef>

#include <render/conductor.hpp>
#include <render/fresnel.hpp>
#include <render/spectrum.hpp>

namespace render::metal {

// The tables, in metres, because house rule 1 does not stop at a data file.
inline constexpr std::size_t gold_samples = 49;

inline constexpr std::array<double, gold_samples> gold_lambda = {
    1.879e-07, 1.916e-07, 1.953e-07, 1.993e-07, 2.033e-07, 2.073e-07,
    2.119e-07, 2.164e-07, 2.214e-07, 2.262e-07, 2.313e-07, 2.371e-07,
    2.426e-07, 2.49e-07, 2.551e-07, 2.616e-07, 2.689e-07, 2.761e-07,
    2.844e-07, 2.924e-07, 3.009e-07, 3.107e-07, 3.204e-07, 3.315e-07,
    3.425e-07, 3.542e-07, 3.679e-07, 3.815e-07, 3.974e-07, 4.133e-07,
    4.305e-07, 4.509e-07, 4.714e-07, 4.959e-07, 5.209e-07, 5.486e-07,
    5.821e-07, 6.168e-07, 6.595e-07, 7.045e-07, 7.56e-07, 8.211e-07,
    8.92e-07, 9.84e-07, 1.088e-06, 1.216e-06, 1.393e-06, 1.61e-06,
    1.937e-06,
};

inline constexpr std::array<double, gold_samples> gold_n = {
    1.28, 1.32, 1.34, 1.33, 1.33, 1.3, 1.3, 1.3, 1.3, 1.31, 1.3, 1.32, 1.32,
    1.33, 1.33, 1.35, 1.38, 1.43, 1.47, 1.49, 1.53, 1.53, 1.54, 1.48, 1.48,
    1.5, 1.48, 1.46, 1.47, 1.46, 1.45, 1.38, 1.31, 1.04, 0.62, 0.43, 0.29,
    0.21, 0.14, 0.13, 0.14, 0.16, 0.17, 0.22, 0.27, 0.35, 0.43, 0.56, 0.92,
};

inline constexpr std::array<double, gold_samples> gold_k = {
    1.188, 1.203, 1.226, 1.251, 1.277, 1.304, 1.35, 1.387, 1.427, 1.46,
    1.497, 1.536, 1.577, 1.631, 1.688, 1.749, 1.803, 1.847, 1.869, 1.878,
    1.889, 1.893, 1.898, 1.883, 1.871, 1.866, 1.895, 1.933, 1.952, 1.958,
    1.948, 1.914, 1.849, 1.833, 2.081, 2.455, 2.863, 3.272, 3.697, 4.103,
    4.542, 5.083, 5.663, 6.35, 7.15, 8.145, 9.519, 11.21, 13.78,
};

inline constexpr std::size_t silver_samples = 49;

inline constexpr std::array<double, silver_samples> silver_lambda = {
    1.879e-07, 1.916e-07, 1.953e-07, 1.993e-07, 2.033e-07, 2.073e-07,
    2.119e-07, 2.164e-07, 2.214e-07, 2.262e-07, 2.313e-07, 2.371e-07,
    2.426e-07, 2.49e-07, 2.551e-07, 2.616e-07, 2.689e-07, 2.761e-07,
    2.844e-07, 2.924e-07, 3.009e-07, 3.107e-07, 3.204e-07, 3.315e-07,
    3.425e-07, 3.542e-07, 3.679e-07, 3.815e-07, 3.974e-07, 4.133e-07,
    4.305e-07, 4.509e-07, 4.714e-07, 4.959e-07, 5.209e-07, 5.486e-07,
    5.821e-07, 6.168e-07, 6.595e-07, 7.045e-07, 7.56e-07, 8.211e-07,
    8.92e-07, 9.84e-07, 1.088e-06, 1.216e-06, 1.393e-06, 1.61e-06,
    1.937e-06,
};

inline constexpr std::array<double, silver_samples> silver_n = {
    1.07, 1.1, 1.12, 1.14, 1.15, 1.18, 1.2, 1.22, 1.25, 1.26, 1.28, 1.28,
    1.3, 1.31, 1.33, 1.35, 1.38, 1.41, 1.41, 1.39, 1.34, 1.13, 0.81, 0.17,
    0.14, 0.1, 0.07, 0.05, 0.05, 0.05, 0.04, 0.04, 0.05, 0.05, 0.05, 0.06,
    0.05, 0.06, 0.05, 0.04, 0.03, 0.04, 0.04, 0.04, 0.04, 0.09, 0.13, 0.15,
    0.24,
};

inline constexpr std::array<double, silver_samples> silver_k = {
    1.212, 1.232, 1.255, 1.277, 1.296, 1.312, 1.325, 1.336, 1.342, 1.344,
    1.357, 1.367, 1.378, 1.389, 1.393, 1.387, 1.372, 1.331, 1.264, 1.161,
    0.964, 0.616, 0.392, 0.829, 1.142, 1.419, 1.657, 1.864, 2.07, 2.275,
    2.462, 2.657, 2.869, 3.093, 3.324, 3.586, 3.858, 4.152, 4.483, 4.838,
    5.242, 5.727, 6.312, 6.992, 7.795, 8.828, 10.1, 11.85, 14.08,
};

inline constexpr std::size_t copper_samples = 49;

inline constexpr std::array<double, copper_samples> copper_lambda = {
    1.879e-07, 1.916e-07, 1.953e-07, 1.993e-07, 2.033e-07, 2.073e-07,
    2.119e-07, 2.164e-07, 2.214e-07, 2.262e-07, 2.313e-07, 2.371e-07,
    2.426e-07, 2.49e-07, 2.551e-07, 2.616e-07, 2.689e-07, 2.761e-07,
    2.844e-07, 2.924e-07, 3.009e-07, 3.107e-07, 3.204e-07, 3.315e-07,
    3.425e-07, 3.542e-07, 3.679e-07, 3.815e-07, 3.974e-07, 4.133e-07,
    4.305e-07, 4.509e-07, 4.714e-07, 4.959e-07, 5.209e-07, 5.486e-07,
    5.821e-07, 6.168e-07, 6.595e-07, 7.045e-07, 7.56e-07, 8.211e-07,
    8.92e-07, 9.84e-07, 1.088e-06, 1.216e-06, 1.393e-06, 1.61e-06,
    1.937e-06,
};

inline constexpr std::array<double, copper_samples> copper_n = {
    0.94, 0.95, 0.97, 0.98, 0.99, 1.01, 1.04, 1.08, 1.13, 1.18, 1.23, 1.28,
    1.34, 1.37, 1.41, 1.41, 1.45, 1.46, 1.45, 1.42, 1.4, 1.38, 1.38, 1.34,
    1.36, 1.37, 1.36, 1.33, 1.32, 1.28, 1.25, 1.24, 1.25, 1.22, 1.18, 1.02,
    0.7, 0.3, 0.22, 0.21, 0.24, 0.26, 0.3, 0.32, 0.36, 0.48, 0.6, 0.76,
    1.09,
};

inline constexpr std::array<double, copper_samples> copper_k = {
    1.337, 1.388, 1.44, 1.493, 1.55, 1.599, 1.651, 1.699, 1.737, 1.768,
    1.792, 1.802, 1.799, 1.783, 1.741, 1.691, 1.668, 1.646, 1.633, 1.633,
    1.679, 1.729, 1.783, 1.821, 1.864, 1.916, 1.975, 2.045, 2.116, 2.207,
    2.305, 2.397, 2.483, 2.564, 2.608, 2.577, 2.704, 3.205, 3.747, 4.205,
    4.665, 5.18, 5.768, 6.421, 7.217, 8.245, 9.439, 11.12, 13.43,
};

inline constexpr std::size_t aluminium_samples = 29;

inline constexpr std::array<double, aluminium_samples> aluminium_lambda = {
    2.0664e-07, 2.4797e-07, 3.0996e-07, 3.2628e-07, 3.6466e-07, 4.1328e-07,
    4.428e-07, 4.7687e-07, 5.166e-07, 5.6357e-07, 6.1993e-07, 6.5225e-07,
    6.8881e-07, 7.2932e-07, 7.7491e-07, 7.9478e-07, 8.1569e-07, 8.3774e-07,
    8.8561e-07, 9.1166e-07, 9.3928e-07, 9.6863e-07, 9.9988e-07, 1.0332e-06,
    1.1271e-06, 1.2399e-06, 1.3776e-06, 1.5498e-06, 1.7712e-06,
};

inline constexpr std::array<double, aluminium_samples> aluminium_n = {
    0.12677, 0.18137, 0.28003, 0.31474, 0.39877, 0.52135, 0.6079, 0.7278,
    0.8734, 1.0728, 1.366, 1.5724, 1.8301, 2.1606, 2.6154, 2.7675, 2.7668,
    2.6945, 2.2802, 1.9739, 1.6784, 1.4867, 1.4359, 1.3998, 1.3281, 1.3157,
    1.3899, 1.5782, 1.9205,
};

inline constexpr std::array<double, aluminium_samples> aluminium_k = {
    2.3563, 2.9029, 3.7081, 3.9165, 4.3957, 5.0008, 5.3676, 5.7781, 6.2418,
    6.7839, 7.4052, 7.7354, 8.0601, 8.3565, 8.4914, 8.3866, 8.2573, 8.1878,
    8.1134, 8.3058, 8.597, 9.0655, 9.4939, 9.8914, 10.969, 12.245, 13.784,
    15.656, 17.991,
};
// ── A metal's index, as a spectrum ───────────────────────────────────────
//
// `conductor.hpp` wants something that answers "what is your complex index at
// this wavelength". Two tabulated spectra, one per part, asked together.
template <std::size_t N>
struct Optical {
    Tabulated<N> n{};
    Tabulated<N> k{};

    constexpr Index at(double lambda) const { return Index{n.at(lambda), k.at(lambda)}; }
};

inline constexpr Optical<gold_samples>      gold{{gold_lambda, gold_n},
                                                {gold_lambda, gold_k}};
inline constexpr Optical<silver_samples>    silver{{silver_lambda, silver_n},
                                                  {silver_lambda, silver_k}};
inline constexpr Optical<copper_samples>    copper{{copper_lambda, copper_n},
                                                  {copper_lambda, copper_k}};
inline constexpr Optical<aluminium_samples> aluminium{{aluminium_lambda, aluminium_n},
                                                      {aluminium_lambda, aluminium_k}};

static_assert(ComplexSpectralValue<Optical<gold_samples>>);

// ── Checked at compile time ──────────────────────────────────────────────
//
// House rule 5, and the transcription of 624 numbers is exactly the kind of
// thing that is silent when it goes wrong. These are facts about the tables
// that a reader can check against the paper.

namespace check {

// The grids ascend, which `Tabulated`'s search depends on and which a table
// pasted in the other direction would silently break rather than fail.
static_assert(gold.n.ascending() && gold.k.ascending());
static_assert(silver.n.ascending() && silver.k.ascending());
static_assert(copper.n.ascending() && copper.k.ascending());
static_assert(aluminium.n.ascending() && aluminium.k.ascending());

// Johnson and Christy's three metals share one grid, because they were
// measured in one run on one instrument. If a row is ever dropped from one of
// them this is what says so.
static_assert(gold_samples == 49 && silver_samples == 49 && copper_samples == 49,
              "Johnson and Christy published 49 photon energies");
static_assert(gold_lambda[0] == silver_lambda[0] && gold_lambda[0] == copper_lambda[0],
              "the three noble metals were measured on one grid");
static_assert(gold_lambda[48] == silver_lambda[48] && gold_lambda[48] == copper_lambda[48],
              "and to the same end of it");

// The ends of that grid, which are 6.6 and 0.64 eV.
static_assert(gold_lambda[0] > 187.8e-9 && gold_lambda[0] < 188.0e-9,
              "the blue end is 187.9 nm, which is 6.6 eV");
static_assert(gold_lambda[48] > 1936.9e-9 && gold_lambda[48] < 1937.1e-9,
              "the red end is 1937 nm, which is 0.64 eV");

// An extinction coefficient is not negative, and a refractive index below 1
// is normal for a metal and alarming for anything else — it is why light
// travels faster than c inside one, which is allowed because that is the
// phase velocity and carries no information.
static_assert(gold.k.lowest() > 0.0 && silver.k.lowest() > 0.0
              && copper.k.lowest() > 0.0 && aluminium.k.lowest() > 0.0,
              "k is an absorption and cannot be negative");
static_assert(gold.n.lowest() < 1.0 && silver.n.lowest() < 1.0,
              "a metal's refractive index goes below 1, which a dielectric's never does");

// And the endpoints of the data itself, pinned so that a transcription error
// in the first or last row cannot hide.
// Both ends of gold, both parts. The first version of this file asserted
// `gold_n[48] == 13.78` from reading the bottom of a CSV without noticing it
// was the bottom of the *k* block, and the assert passed — because nothing
// included this header yet, so none of these were compiled. A check in a file
// nobody includes is not a check. That is what `scene.hpp` including it fixed
// and what deleting `mesh.hpp` in v0.4 was about.
static_assert(gold_n[0] == 1.28  && gold_k[0] == 1.188);
static_assert(gold_n[48] == 0.92 && gold_k[48] == 13.78);
static_assert(silver_n[0] == 1.07 && copper_n[0] == 0.94);

// Which is also the physics: at 1937 nm gold's refractive index is 0.92 and
// its extinction coefficient is 13.78. A number near 14 is an absorption and
// never an index, and the two are not interchangeable even when a table
// prints them in adjacent columns.

} // namespace check

} // namespace render::metal
