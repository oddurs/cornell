// image.hpp — two file formats, written out by hand, because house rule 4
// says no dependencies and an image library is the first one anybody adds.
//
// It is in `apps/` rather than `include/render/` on purpose. The renderer
// does not know what a file is. It answers one question — how much light
// arrives here, from there, at this wavelength — and everything about bytes
// on a disk is an instrument's problem. Nothing in this file may be included
// by anything under `include/render/`, and if it ever needs to be, the thing
// that needed it was in the wrong place.
//
// ── Two formats, because they are for two different readers ───────────────
//
// PFM is the one that matters. Thirty-two-bit floats, linear, no transfer
// function, no clipping, no range: what the film holds, byte for byte. Every
// instrument that quotes a number reads this one, because a number read out
// of an 8-bit image is a number that has been through a tone curve and then
// rounded to one part in 255, and quoting it is quoting the curve.
//
// PPM is the one you can look at. Eight bits, three channels, a display
// encoding applied, and it exists because a project that cannot show you
// anything until v0.3 is a project nobody reads past v0.1.
//
// Both are about fifteen lines. That is the whole argument for writing them:
// a PPM is the four ASCII characters `P6`, a width, a height, a 255, and then
// the bytes, and a dependency that saves you fifteen lines has to be
// maintained, versioned, and still building in fifteen years.
//
// ── They disagree about which way up ──────────────────────────────────────
//
// PPM stores rows top to bottom, the way a page is read. PFM stores them
// bottom to top, the way a graph is drawn. Neither is wrong and there is no
// arguing with either, so the flip lives here, in the one place that knows
// both conventions, and the film is left to index its rows in reading order.
//
// PFM also puts the endianness in the scale line: a negative number means
// little-endian, which is every machine this will run on and is written as
// `-1.0` below rather than detected, because a file that lies about its own
// byte order is worse than a file that only supports one.
//
// ── The transfer function is sRGB's, now that there is one ───────────────
//
// House rule 1 says the transfer function belongs to `srgb.hpp`. Until v0.3
// there was no such file, so this one raised the value to the power 1/2.2 —
// the thing everybody means by "gamma", and not what sRGB does — and said
// that v0.3 would delete the parameter rather than tune it. It has.
//
// The measurement that made the placeholder uncomfortable is kept here
// because it is the argument for the swap. sRGB is a linear segment below
// 0.0031308 spliced to a 1/2.4 power with an offset, and the two differ by up
// to **nine 8-bit codes**, all of it in the shadows:
//
//      linear      sRGB      gamma 2.2     difference
//      0.001          3            11             −8
//      0.005         16            23             −7
//      0.010         25            31             −6
//      0.050         63            65             −2
//      0.100         89            90             −1
//      0.500        188           186             +2
//      1.000        255           255              0
//
// Nine codes is not subtle. It is the difference between a dark corner of the
// Cornell box reading as black and reading as visibly lit, which is the exact
// measurement v1.0 exists to make.
//
// It is still a preview and no figure is ever quoted from it — eight bits and
// a clip are not a measurement — but it is no longer approximate on purpose.

#pragma once

#include <cstdint>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>

#include <render/srgb.hpp>

namespace app {

// ── PFM: linear float, and the format every instrument reads ──────────────
//
// One channel, `Pf`. Three would be `PF` and the only difference is the tag
// and the stride; it is not written until something has three channels to
// put in it, which is v0.3.
inline bool write_pfm(const std::string& path, int width, int height,
                      const std::vector<float>& values) {
    if (int(values.size()) != width * height) return false;

    std::FILE* out = std::fopen(path.c_str(), "wb");
    if (!out) return false;

    // The scale factor's sign is the byte order. Its magnitude is a scale
    // nobody has ever used for anything, and it is 1.
    std::fprintf(out, "Pf\n%d %d\n-1.0\n", width, height);

    // Bottom row first. See above.
    for (int y = height - 1; y >= 0; --y)
        std::fwrite(values.data() + std::size_t(y) * std::size_t(width),
                    sizeof(float), std::size_t(width), out);

    return std::fclose(out) == 0;
}

// ── PPM: eight bits, for looking at ───────────────────────────────────────
//
// Takes linear values in [0, 1] and three of them per pixel, interleaved.
// Anything above 1 is clipped, because eight bits have nowhere else to put
// it; that clipping is the reason the PFM exists and the reason no figure
// comes from here.
inline bool write_ppm(const std::string& path, int width, int height,
                      const std::vector<double>& rgb) {
    if (int(rgb.size()) != width * height * 3) return false;

    std::FILE* out = std::fopen(path.c_str(), "wb");
    if (!out) return false;

    std::fprintf(out, "P6\n%d %d\n255\n", width, height);

    std::vector<std::uint8_t> row(std::size_t(width) * 3);
    for (int y = 0; y < height; ++y) {          // top row first
        for (std::size_t i = 0; i < row.size(); ++i) {
            const double linear = rgb[std::size_t(y) * row.size() + i];
            const double clipped = linear < 0.0 ? 0.0 : (linear > 1.0 ? 1.0 : linear);
            row[i] = std::uint8_t(render::srgb::encode(clipped) * 255.0 + 0.5);
        }
        std::fwrite(row.data(), 1, row.size(), out);
    }

    return std::fclose(out) == 0;
}

} // namespace app
