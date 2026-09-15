// jakob_hanika.hpp — the inverse problem, and the ugliest honest corner of
// this project.
//
// A user types `#c04040` for a wall. That is three numbers. A reflectance is
// a function of wavelength. The mapping from the function to the three
// numbers is an integration and it throws almost everything away; the mapping
// back is one-to-many, with an infinite family of spectra that all produce
// exactly that RGB and look identical under the light they were fitted for
// and different under any other.
//
// So something has to choose, and there is no correct choice. This file makes
// one and says so, which is the only honest thing available.
//
// ── What the Cornell walls do instead ────────────────────────────────────
//
// They do not come through here. This is the path for a colour somebody
// invented; the box's walls were measured with a spectrometer and their
// reflectances are published, and `cornell.hpp` in v0.4 reads those numbers.
// A red wall that went through this file would be a red wall this project
// made up, and comparing a made-up wall against a photograph proves nothing.
//
// That separation is the whole reason the file is allowed to exist at all.
//
// ── Wandering among the choices ──────────────────────────────────────────
//
// Smits, 1999, builds a small basis — white, cyan, magenta, yellow, red,
// green, blue — and mixes it greedily. It is simple and it produces spectra
// with visible kinks at the basis boundaries.
//
// Jakob and Hanika, 2019, "A Low-Dimensional Function Space for Efficient
// Spectral Upsampling", observe that what you want from an invented
// reflectance is that it be *smooth* and *bounded*, and that both can be had
// by construction rather than by constraint. Their model is a sigmoid of a
// quadratic:
//
//      s(lambda) = 1/2 + x / (2 sqrt(1 + x^2))
//
//      where     x = c0 * lambda^2 + c1 * lambda + c2
//
// Three coefficients. The sigmoid maps the whole real line into (0, 1), so
// the reflectance cannot leave the physical range no matter what the fit
// does — which is the property naive fits lack, and the reason a naive fit
// produces a wall that reflects 140 % of the blue light falling on it.
//
// The quadratic gives the shape: one hump or one valley, positioned and
// widened by the coefficients, which is enough for every colour a display can
// show and deliberately not enough for a spiky fluorescent spectrum. Smooth
// by construction, because real reflectances of paint and paper are smooth
// and an invented one should not pretend to structure it has no evidence for.
//
// ── What is taken from the paper and what is not ─────────────────────────
//
// The parameterisation is theirs. The precomputed table is not.
//
// Their implementation fits the three coefficients offline for a 64 x 64 x 64
// grid of RGB values and ships the result, so that a lookup at render time is
// three interpolations. This file fits them when asked instead, by
// Gauss-Newton, which takes a few dozen microseconds and happens once per
// distinct colour in a scene rather than once per sample.
//
// That is the right trade here and would be the wrong one in a renderer with
// textures, where every texel is a different RGB. It is also a smaller thing
// to be responsible for: a table of 786 432 coefficients is a table somebody
// has to have generated correctly, and this project would have to either
// trust it or regenerate it, and regenerating it is this code anyway.
//
// ── Measured ─────────────────────────────────────────────────────────────
//
// The claim this file has to make is that it round-trips: upsample an RGB,
// integrate the result back against the observer under the same light, and
// get the RGB you started with. Over 729 colours on a 9 x 9 x 9 grid of the
// unit cube:
//
//      worst round-trip error         1.43e-07
//      colours missed by over 1e-3    0 of 729
//
// The 1.43e-07 is at pure white, where the sigmoid is asymptoting towards 1
// and the last digits cost an unbounded coefficient. Everywhere else it is
// between 1e-16 and 1e-13, which is the Gauss-Newton converging to the limit
// of the arithmetic.
//
// And the bound, which is the property the parameterisation exists for. Every
// reflectance produced, at every one of the 95 tabulated wavelengths, across
// all 729 colours:
//
//      sigmoid of a quadratic    [0.000000136, 0.999999922]
//      the same fit without it   [-8.860240,   9.860240]
//
// The second row is what "naive fits do not" means. Strip the sigmoid and fit
// a bare quadratic to the same colours and the worst case — pure green, the
// most saturated thing the gamut contains — asks for a wall that reflects 986
// per cent of the light falling on it at some wavelengths and a negative
// amount at others. That wall is not merely inaccurate; it is an energy
// source, and it would sail straight through the furnace test in v0.5 doing
// something no material can do.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// Metamerism as something you can ask for. Two reflectances can match under
// D65 and differ completely under a sodium lamp, which is physically
// interesting and is exactly what this file destroys: it returns one spectrum
// per RGB, chosen for smoothness, and there is no way to request the other.
//
// A fit under any illuminant but the one it was given. The coefficients
// depend on the light the match was made under — that is what "matches under
// D65" means — so a scene lit by illuminant A and coloured through this path
// is matching under the wrong light. `upsample` takes the illuminant as an
// argument so that this is at least a decision.
//
// Emission. A light with a typed colour would want the same treatment and
// does not get it: an emission spectrum is not bounded by 1, so the sigmoid
// is the wrong shape for it entirely.

#pragma once

#include <cmath>
#include <cstddef>
#include <render/cie.hpp>
#include <render/illuminant.hpp>
#include <render/spectrum.hpp>
#include <render/srgb.hpp>

namespace render {

// A reflectance with three coefficients, bounded in (0, 1) by construction.
//
// The wavelength is scaled into roughly [0, 1] before the quadratic is
// evaluated: the coefficients of a quadratic in metres would span forty
// orders of magnitude and the fit below would be conditioned accordingly.
class SigmoidSpectrum {
public:
    constexpr SigmoidSpectrum() = default;
    constexpr SigmoidSpectrum(double c0, double c1, double c2) : c_{c0, c1, c2} {}

    constexpr double at(double lambda) const {
        const double t = (lambda - cie::first) / (cie::last - cie::first);
        const double x = (c_[0] * t + c_[1]) * t + c_[2];

        // The sigmoid. `1 + x*x` is never zero, so this needs no guard, and
        // it saturates rather than clipping — which is why a fit that wants a
        // reflectance of 1.2 gets 0.999 instead of an error.
        return 0.5 + x / (2.0 * std::sqrt(1.0 + x * x));
    }

    constexpr double coefficient(std::size_t i) const { return c_[i]; }

private:
    double c_[3] = {0.0, 0.0, 0.0};
};

static_assert(SpectralValue<SigmoidSpectrum>);

// ── What a reflectance looks like to a display ───────────────────────────
//
// Light it, integrate against the observer, normalise so a perfect white
// reflector is 1, and convert through the matrix `srgb.hpp` derived. This is
// the function the fit below inverts, and it is written once here so that the
// fit cannot be inverting something subtly different from what the renderer
// does.
template <class Spectrum>
inline Xyz reflectance_to_rgb(const Spectrum& reflectance,
                              const cie::Illuminant& light) {
    Xyz total;
    double white = 0.0;
    for (std::size_t i = 0; i < cie::samples; ++i) {
        const double lambda = cie::first + double(i) * cie::step;
        const double lit = light.table[i];
        total += cie::observer[i] * (lit * reflectance.at(lambda));
        white += lit * cie::observer[i].y;
    }
    const Xyz normalised = total * (1.0 / white);
    return apply(srgb::xyz_to_rgb, normalised);
}

// ── The fit ──────────────────────────────────────────────────────────────
//
// Three unknowns, three residuals, so Gauss-Newton is a 3 x 3 solve per
// iteration and `cie.hpp` already has the inverse. The Jacobian is by finite
// differences, because the derivative of the sigmoid through the integral
// through the matrix is a page of chain rule that would have to be kept in
// step with the forward function above, and keeping two expressions of one
// thing in step is what house rule 6 is about.
//
// It is not globally convergent and does not need to be. The residual is
// smooth in the coefficients, the starting point below is the middle of the
// space, and a colour a display can show is always reachable.
inline SigmoidSpectrum upsample(double r, double g, double b,
                                const cie::Illuminant& light = cie::d65,
                                int iterations = 40) {
    SigmoidSpectrum fit{0.0, 0.0, 0.0};   // s == 1/2 everywhere: mid grey

    for (int step = 0; step < iterations; ++step) {
        const Xyz current = reflectance_to_rgb(fit, light);
        const Xyz residual{current.x - r, current.y - g, current.z - b};

        // Close enough that further steps are moving the last bits around.
        if (std::fabs(residual.x) + std::fabs(residual.y) + std::fabs(residual.z) < 1e-12)
            break;

        // The Jacobian, one column per coefficient.
        constexpr double nudge = 1e-5;
        Matrix3 jacobian;
        for (std::size_t c = 0; c < 3; ++c) {
            double moved[3] = {fit.coefficient(0), fit.coefficient(1), fit.coefficient(2)};
            moved[c] += nudge;
            const Xyz shifted =
                reflectance_to_rgb(SigmoidSpectrum{moved[0], moved[1], moved[2]}, light);
            jacobian[0][c] = (shifted.x - current.x) / nudge;
            jacobian[1][c] = (shifted.y - current.y) / nudge;
            jacobian[2][c] = (shifted.z - current.z) / nudge;
        }

        if (std::fabs(determinant(jacobian)) < 1e-30) break;

        const Xyz delta = apply(inverse(jacobian), residual);
        fit = SigmoidSpectrum{fit.coefficient(0) - delta.x,
                              fit.coefficient(1) - delta.y,
                              fit.coefficient(2) - delta.z};
    }

    return fit;
}

} // namespace render
