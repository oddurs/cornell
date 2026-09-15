// spectrum.hpp — a spectrometer, pointed at the renderer's own data.
//
// `windsor`, the engine this project is a sibling to, has an instrument that
// prints a part as a shop-manual page: the thing itself, quoted, with its
// dimensions beside it. This is that page for a spectrum.
//
// Every claim the colour half of this project makes rests on two tables and a
// pile of integrals, and all of it is invisible. A wrong transcription, a
// misaligned grid, an interpolation that is off by half a step — none of
// those announce themselves in an image, they just tint it slightly and
// forever. So: print the spectrum, print what the observer makes of it, and
// let a reader compare both against a published figure.
//
// ── What it may not do ───────────────────────────────────────────────────
//
// Compute anything `cie.hpp` should compute. The chromaticities below come
// from `chromaticity_of(white_point(...))`, the same call the renderer makes,
// not from a copy of the integral written out again here with a plot next to
// it. An instrument that reimplements the thing it is checking is checking
// its own reimplementation.
//
// The correlated colour temperature is the exception, and it is the exception
// because it is not a colour-science primitive the renderer needs: nothing in
// the transport asks what temperature a light is. It is computed here, in the
// instrument, and marked as the instrument's own.
//
// ── The plot ─────────────────────────────────────────────────────────────
//
// Text, because house rule 4 has no plotting library in it and because a
// spectrum is a single-valued function of one variable, which is the one
// shape ASCII is genuinely adequate for.

#pragma once

#include <cmath>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

#include <render/cie.hpp>
#include <render/illuminant.hpp>
#include <render/si.hpp>
#include <render/srgb.hpp>

namespace app {

namespace detail {

// A column chart, drawn downward, so that the wavelength axis reads across
// the bottom the way every published spectrum does.
inline void plot(const std::vector<double>& values, int rows) {
    const auto biggest = [&] {
        double m = 0.0;
        for (const double v : values) m = std::fmax(m, v);
        return m;
    }();
    if (biggest <= 0.0) { std::printf("  (all zero)\n"); return; }

    for (int r = rows; r > 0; --r) {
        const double high = biggest * double(r) / double(rows);
        const double low  = biggest * double(r - 1) / double(rows);
        std::printf("  %7.3f |", high);
        for (const double v : values) std::printf("%s", v >= low ? "#" : " ");
        std::printf("\n");
    }
    std::printf("          +");
    for (std::size_t i = 0; i < values.size(); ++i) std::printf("-");
    std::printf("\n           ");

    // Tick the round hundreds, which is where a reader's eye goes.
    std::size_t marked = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        const double nm = render::si::as::nm(render::cie::first + double(i) * render::cie::step);
        if (std::fabs(nm - std::round(nm / 100.0) * 100.0) < 2.5 && i >= marked) {
            std::printf("%-5.0f", nm);
            marked = i + 5;
        } else if (i >= marked) {
            std::printf(" ");
            marked = i + 1;
        }
    }
    std::printf("   nm\n");
}

// Correlated colour temperature, by McCamy's 1992 cubic approximation.
//
// It is a *fit*, and to a fit: the real definition of CCT is the temperature
// of the Planckian radiator closest to the sample in the 1960 uv diagram,
// which is a minimisation rather than a formula. McCamy fitted a cubic in one
// variable to that minimisation and it is good to about 2 K over the range
// anybody quotes.
//
// House rule 8 says a law and a curve-fit must never be spelled the same way.
// This is a curve-fit, it is named for the person who fitted it, and it lives
// in the instrument rather than in `cie.hpp` because the renderer never asks
// what temperature a light is.
inline double mccamy_cct(const render::Chromaticity& c) {
    const double n = (c.x - 0.3320) / (0.1858 - c.y);
    return 449.0 * n * n * n + 3525.0 * n * n + 6823.3 * n + 5520.33;
}

inline void report(std::string_view name,
                   const std::vector<double>& values,
                   const render::Xyz& tristimulus,
                   std::string_view units,
                   std::string_view source) {
    std::printf("%.*s\n", int(name.size()), name.data());
    std::printf("  %.*s\n\n", int(source.size()), source.data());

    plot(values, 16);

    const render::Chromaticity c = render::chromaticity_of(tristimulus);
    std::printf("\n  %.*s\n", int(units.size()), units.data());
    std::printf("  XYZ            %.6f  %.6f  %.6f\n",
                tristimulus.x, tristimulus.y, tristimulus.z);
    std::printf("  chromaticity   x = %.5f   y = %.5f\n", c.x, c.y);
    std::printf("  CCT            %.0f K   (McCamy's fit, +-2 K; see the header)\n",
                mccamy_cct(c));

    // And what a display would do with it, which is where out-of-gamut stops
    // being an abstraction.
    const render::Xyz rgb = render::srgb::apply(
        render::srgb::xyz_to_rgb, tristimulus.x, tristimulus.y, tristimulus.z);
    const double scale = std::fmax(rgb.x, std::fmax(rgb.y, rgb.z));
    std::printf("  linear sRGB    %+.4f  %+.4f  %+.4f%s\n",
                rgb.x / scale, rgb.y / scale, rgb.z / scale,
                (rgb.x < 0.0 || rgb.y < 0.0 || rgb.z < 0.0)
                    ? "   <- outside the gamut; see item 0047" : "");
}

} // namespace detail

inline int spectrum(std::string_view which) {
    using namespace render;

    std::vector<double> values;
    values.reserve(cie::samples);

    if (which == "d65" || which == "e") {
        const cie::Illuminant& light = which == "d65" ? cie::d65 : cie::e;
        for (std::size_t i = 0; i < cie::samples; ++i) values.push_back(light.table[i]);
        detail::report(which == "d65" ? "D65 — average daylight, 6504 K"
                                      : "E — equal energy",
                       values, cie::white_point(light),
                       "relative spectral power, normalised to 100 at 560 nm",
                       which == "d65"
                           ? "cvrl.org, Illuminantd65.csv, on the CIE 5 nm grid"
                           : "defined, not measured: the same power at every wavelength");
        return 0;
    }

    if (which == "x" || which == "y" || which == "z") {
        Xyz total;
        for (std::size_t i = 0; i < cie::samples; ++i) {
            const Xyz& m = cie::observer[i];
            values.push_back(which == "x" ? m.x : which == "y" ? m.y : m.z);
            total += m;
        }
        const std::string title =
            std::string(which == "x" ? "x-bar" : which == "y" ? "y-bar" : "z-bar")
            + " — CIE 1931 2-degree standard observer"
            + (which == "y" ? ", which is also V(lambda)" : "");
        detail::report(title, values, total,
                       "matching function, dimensionless; the XYZ below is the "
                       "observer's own integral, i.e. equal-energy white",
                       "Wright's ten observers and Guild's seven, averaged by the "
                       "CIE in 1931. See cie.hpp.");
        return 0;
    }

    std::fprintf(stderr, "cornell spectrum: no spectrum called '%.*s'.\n\n",
                 int(which.size()), which.data());
    std::fprintf(stderr, "  d65   average daylight, as a table\n");
    std::fprintf(stderr, "  e     equal energy, as a definition\n");
    std::fprintf(stderr, "  x y z the three colour matching functions\n");
    return 1;
}

} // namespace app
