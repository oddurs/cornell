// gamut.hpp — what a display should do with a colour it cannot show.
//
// This file exists because of a spike, item 0047, and the spike was right to
// ask: every monochromatic colour is outside sRGB. Not most of them, not the
// saturated ones. All 641 wavelengths sampled across the visible range have a
// negative sRGB component, because the spectral locus is a curve and the
// gamut is a triangle inside it, and no three real primaries can enclose it.
//
// So the question is not whether this happens but what to do when it does,
// and it has to be answered before v0.9 renders a prism, because the answer
// decides whether that image is a spectrum or three stripes.
//
// ── The experiment ───────────────────────────────────────────────────────
//
// The spike said the deciding test was to render the prism. It is not
// necessary to wait: what the prism shows is a continuous sweep through the
// spectral colours, and that sweep can be produced directly from the observer
// without any refraction at all. 380 to 700 nm in 641 steps, each converted
// to sRGB, each run through a candidate strategy, encoded to 8 bits:
//
//      strategy                  distinct  longest flat  worst hue shift
//      clip per channel               265           184         33.2 deg
//      desaturate to the gamut        402            50          0.0 deg
//
// "Longest flat" is the number of consecutive wavelengths that come out as
// exactly the same 8-bit colour. Clipping produces a run of 184 — twenty-nine
// per cent of the visible spectrum rendered as one unchanging block — which
// is precisely the "three bands rather than a spectrum" the spike was worried
// about, arriving without a prism being involved.
//
// And it moves the hue by up to 33 degrees. A clipped violet is not a dimmer
// violet, it is a blue.
//
// ── The answer ───────────────────────────────────────────────────────────
//
// Desaturate towards the white point at constant luminance: slide the colour
// along the line joining it to white until nothing is negative.
//
// The hue shift is 0.0 degrees and that is by construction rather than by
// luck. A line towards `(y, y, y)` in linear RGB maps to a line towards the
// white point in chromaticity, and a point's hue is its direction from the
// white point, so moving along that line cannot change it. What is lost is
// saturation — a spectral green becomes the most saturated green the display
// can make, which is the honest answer, because that is the most saturated
// green the display can make.
//
// It is the option the spike listed second, and the reason it wins is the one
// the spike gave: it is the one that lets a reader see that the spread is
// continuous.
//
// ── What this is not ─────────────────────────────────────────────────────
//
// Not physics, and not applied to anything a number is quoted from.
// `cornell.pfm` is written before this, unclipped and with its negatives
// intact, because a negative sRGB component is *information* — it says the
// colour is outside what the display can represent — and the file that exists
// to preserve evidence should not be the file that destroys it.
//
// Not a full gamut mapping algorithm either. The serious ones — sigmoidal
// compression, spatial methods that consider neighbouring pixels, anything in
// the CIE's own recommendations — also compress colours that are *inside* the
// gamut, so that the ones being brought in have somewhere to go and the
// relationships between them survive. This clamps only what is outside and
// leaves everything else exactly where it was, which is the right trade for a
// renderer whose interior colours are measurements.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// A wider gamut. The fourth option the spike listed was to write a file in a
// larger space — Display P3, Rec. 2020, ACEScg — and say so. It is a better
// answer in every respect except that it requires the reader to have a
// display and a viewer that honour it, and this project writes a PPM. The
// PFM already carries the unmapped values for anybody who wants them.

#pragma once

#include <cmath>
#include <render/cie.hpp>

namespace app {

// The luminance coefficients of the sRGB primaries — the middle row of the
// matrix `srgb.hpp` derives, which is what "relative luminance" means for
// this display.
inline double luminance_of(const render::Xyz& rgb, const render::Matrix3& to_xyz) {
    return to_xyz[1][0] * rgb.x + to_xyz[1][1] * rgb.y + to_xyz[1][2] * rgb.z;
}

// Slide towards the achromatic axis until nothing is negative, keeping the
// luminance and therefore the hue.
inline render::Xyz into_gamut(const render::Xyz& rgb, const render::Matrix3& to_xyz) {
    if (rgb.x >= 0.0 && rgb.y >= 0.0 && rgb.z >= 0.0) return rgb;

    const double y = luminance_of(rgb, to_xyz);
    if (y <= 0.0) return render::Xyz{};

    // The smallest blend towards grey that brings every channel up to zero.
    // Solved directly rather than by bisection: for each negative channel,
    // c + t(y - c) = 0 gives t = c / (c - y), and the largest of those wins.
    double t = 0.0;
    const double channels[3] = {rgb.x, rgb.y, rgb.z};
    for (const double c : channels)
        if (c < 0.0) t = std::fmax(t, c / (c - y));

    return render::Xyz{rgb.x + t * (y - rgb.x),
                       rgb.y + t * (y - rgb.y),
                       rgb.z + t * (y - rgb.z)};
}

} // namespace app
