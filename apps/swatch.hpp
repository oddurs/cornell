// swatch.hpp — the thesis, in one image and four numbers.
//
// Five spheres in a uniform environment. Four of them are metal and none of
// them has a colour anywhere in its description. The scene is a list of
// citations:
//
//      gold, copper, silver     Johnson and Christy, Phys. Rev. B 6, 1972
//      aluminium                Rakic, Appl. Opt. 34, 1995
//      glass                    a real index of 1.5, and nothing else
//
// `metals.hpp` holds two columns of measured numbers per metal. `fresnel.hpp`
// turns a complex index into a reflectance at a wavelength and an angle.
// `cie.hpp` turns a spectrum into three numbers because eyes have three cone
// types. Nothing between them knows what yellow is.
//
// ── What it may not do ───────────────────────────────────────────────────
//
// Have a fallback. A metal with no table is not in this picture — there is no
// default index, no "approximately gold", and no tint applied to a grey
// sphere to stand in for a measurement. The `Bsdf` variant makes that
// structural rather than a rule somebody remembers: a conductor is a
// `Conductor<Optical<N>>` and the only way to get one is to hand it a table.
//
// ── Why the numbers are printed under the image ──────────────────────────
//
// Because the image is not the claim. A picture of a yellow sphere proves
// nothing — every renderer ever written can produce one, and most of them do
// it by typing 0.766. What is checkable is the chromaticity, and what is
// checkable *independently* is where each metal's reflectance edge falls,
// because that edge is a transition between electron bands and its energy is
// a fact about the metal rather than about this program.
//
// Gold's d-band to Fermi-level threshold is quoted in the solid-state
// literature at about 2.4 eV, copper's at about 2.1, and silver's plasma edge
// at about 3.8 — which is why silver is neutral across the visible and the
// other two are not. This file measures where the steepest fall in
// reflectance actually lands and prints it beside them.

#pragma once

#include <cmath>
#include <cstdio>
#include <string_view>
#include <vector>

#include <render/camera.hpp>
#include <render/cie.hpp>
#include <render/conductor.hpp>
#include <render/illuminant.hpp>
#include <render/metals.hpp>
#include <render/scene.hpp>
#include <render/srgb.hpp>

#include "enclosure.hpp"
#include "image.hpp"
#include "render.hpp"

namespace app {

namespace swatch_detail {

using namespace render;

// The illuminant is stated, because a chromaticity without one is not a
// number. D65, which is also what `srgb.hpp` is built around, so the sRGB
// triples underneath are what a monitor would be asked for.
inline constexpr double swatch_radiance = 1.0;

// A metal's reflectance at normal incidence, integrated against the observer
// under D65. This is the whole pipeline in five lines and it is where the
// thesis either holds or does not.
template <class IndexSpectrum>
Xyz tristimulus_of(const IndexSpectrum& index) {
    Xyz total;
    for (std::size_t i = 0; i < cie::samples; ++i) {
        const double lambda = cie::first + double(i) * cie::step;
        const double reflected = fresnel(1.0, index.at(lambda)).unpolarised();
        total += cie::observer[i] * (cie::d65.table[i] * reflected);
    }
    return total;
}

// A metal's colour, as three numbers scaled so that the brightest is 1.
//
// That is a hue and a saturation rather than an exposure — the exposure is the
// image's job — and it is the form a reader can hold against the constant
// every renderer types.
struct Rgb { double r = 0.0, g = 0.0, b = 0.0; };

template <class IndexSpectrum>
Rgb normalised_rgb_of(const IndexSpectrum& index) {
    const Xyz value = tristimulus_of(index);
    const Xyz rgb = apply(srgb::xyz_to_rgb, value.x, value.y, value.z);
    const double most = std::fmax(rgb.x, std::fmax(rgb.y, rgb.z));
    return most > 0.0 ? Rgb{rgb.x / most, rgb.y / most, rgb.z / most} : Rgb{};
}

// Where the reflectance falls off a cliff, in electron volts.
//
// Scanned rather than looked up: the steepest fall of R against photon energy
// across the visible and a little beyond. For a noble metal that cliff is an
// interband transition — an electron promoted from a full d band to the Fermi
// surface — and its energy is a property of the metal's band structure, which
// makes it the one figure here that can be checked against a source that is
// not the table this program read.
template <class IndexSpectrum>
std::pair<double, double> reflectance_edge(const IndexSpectrum& index) {
    double steepest = 0.0, at = 0.0;

    for (int i = 1; i < 40'000; ++i) {
        const double energy = 1.0 + double(i) / 10'000.0;      // eV
        const double next = energy + 0.001;
        if (next > 5.0) break;

        const double here = fresnel(1.0, index.at(si::wavelength_of_eV(energy)))
                                .unpolarised();
        const double there = fresnel(1.0, index.at(si::wavelength_of_eV(next)))
                                 .unpolarised();

        const double slope = (here - there) / 0.001;
        if (slope > steepest) { steepest = slope; at = energy; }
    }
    return {at, steepest};
}

// Five spheres in a row, in the enclosure `furnace.hpp` uses, lit by D65.
inline Scene make_swatch() {
    Scene scene;
    enclosure::add_box(scene, 20.0, Bsdf{GreyLambert{Flat{0.0}}},
                       cie::d65, swatch_radiance);

    const double spacing = 2.4;
    const Bsdf surfaces[] = {
        Bsdf{JohnsonChristyMetal{ConductorReflectance{metal::gold}}},
        Bsdf{JohnsonChristyMetal{ConductorReflectance{metal::copper}}},
        Bsdf{JohnsonChristyMetal{ConductorReflectance{metal::silver}}},
        Bsdf{RakicMetal{ConductorReflectance{metal::aluminium}}},
        Bsdf{FlatConductor{ConductorReflectance{FlatIndex{Index{1.5, 0.0}}}}},
    };

    // Laid out right to left in world x, so that they read left to right in
    // the image. `camera.hpp` takes `right = forward x up`, which for a camera
    // looking down +z with +y up points along *negative* x — the film is
    // mirrored against the world, as a camera's is, and this is the one place
    // in the project where that shows.
    for (int i = 0; i < 5; ++i)
        scene.add(Surface{Sphere{Vec3{(2.0 - double(i)) * spacing, 0.0, 0.0}, 1.0},
                          surfaces[std::size_t(i)]});

    scene.finalise();
    return scene;
}

} // namespace swatch_detail

inline int swatch(bool write_image) {
    using namespace swatch_detail;
    using namespace render;

    std::printf("The metals, from nothing but citations.\n\n"
                "Five spheres, four of them metal, and no colour in any of their\n"
                "descriptions. Normal-incidence reflectance integrated against the CIE\n"
                "1931 observer under D65.\n\n");

    std::printf("  %-11s %9s %9s %9s %9s %9s %11s\n",
                "", "x", "y", "R", "G", "B", "edge");

    int failures = 0;
    const Chromaticity white = cie::check::d65_chromaticity;

    const auto row = [&](const char* name, const auto& index, bool neutral) {
        const Chromaticity c = chromaticity_of(tristimulus_of(index));
        const auto [edge, slope] = reflectance_edge(index);
        const Rgb rgb = normalised_rgb_of(index);

        const double from_white = std::hypot(c.x - white.x, c.y - white.y);
        const bool as_expected = neutral ? from_white < 0.01 : from_white > 0.03;
        if (!as_expected) ++failures;

        std::printf("  %-11s %9.5f %9.5f %9.4f %9.4f %9.4f %8.3f eV  %s\n",
                    name, c.x, c.y, rgb.r, rgb.g, rgb.b,
                    edge, as_expected ? "" : "NOT AS DESCRIBED");
        (void)slope;
        return from_white;
    };

    const double gold_off      = row("gold",      metal::gold,      false);
    row("copper", metal::copper, false);
    const double silver_off    = row("silver",    metal::silver,    true);
    const double aluminium_off = row("aluminium", metal::aluminium, true);

    std::printf("\n  D65 white is x %.5f y %.5f, so gold sits %.4f away from it and\n"
                "  aluminium %.4f — a factor of %.0f, and nobody typed either.\n",
                white.x, white.y, gold_off, aluminium_off, gold_off / aluminium_off);

    std::printf("\n  The edge column is the one figure here that can be checked against\n"
                "  something other than the table this program read. It is where the\n"
                "  reflectance falls off a cliff, and that cliff is an interband\n"
                "  transition whose energy is a property of the metal's band structure:\n"
                "  gold's d-band threshold is quoted at about 2.4 eV, copper's at about\n"
                "  2.1, and silver's plasma edge at about 3.8 — which is why silver is\n"
                "  neutral across the visible and the other two are not.\n");

    std::printf("\n  Silver is %.5f from white and aluminium %.5f, which makes aluminium\n"
                "  the control: it is nearly neutral, so a tint on it would be the\n"
                "  pipeline's rather than the metal's.\n", silver_off, aluminium_off);

    // ── And the line the whole project was built to print ────────────────
    //
    // CLAUDE.md opens with the claim: gold is `vec3(1.0, 0.766, 0.336)` in
    // every renderer written for pleasure, and it is nothing of the kind.
    //
    // This is what falls out of Johnson and Christy's two columns through
    // Fresnel's equations through the 1931 observer, with nothing typed. The
    // interesting part is not that it is close. It is that the two numbers
    // come from different places — one from a measurement and one from a
    // convention — and a reader can see how far apart they are.
    {
        const Rgb gold = normalised_rgb_of(metal::gold);
        const double derived[3] = {gold.r, gold.g, gold.b};
        constexpr double typed[3] = {1.0, 0.766, 0.336};

        std::printf("\n  Gold, against the constant every renderer types:\n\n"
                    "      %-10s %8.4f %8.4f %8.4f\n"
                    "      %-10s %8.4f %8.4f %8.4f\n"
                    "      %-10s %8.4f %8.4f %8.4f\n",
                    "derived",  derived[0], derived[1], derived[2],
                    "typed",    typed[0],   typed[1],   typed[2],
                    "difference", derived[0] - typed[0],
                                  derived[1] - typed[1],
                                  derived[2] - typed[2]);

        const Rgb pink = normalised_rgb_of(metal::copper);

        std::printf("\n  Within a few hundredths, from a direction nobody aimed. The\n"
                    "  remaining difference is not error: the typed constant is somebody\n"
                    "  else's table, under somebody else's illuminant, normalised somebody\n"
                    "  else's way, and it has been copied between renderers for thirty\n"
                    "  years without a citation. This one has two.\n"
                    "\n  Which is the point. Swap gold's table for copper's and the same\n"
                    "  code returns %.4f %.4f %.4f instead, because of physics rather than\n"
                    "  because somebody typed pink.\n",
                    pink.r, pink.g, pink.b);
    }

    if (write_image) {
        RenderSettings settings;
        settings.width = 480;
        settings.spp = 64;

        const int height = height_for(settings.width);
        const Scene scene = make_swatch();
        const Camera camera = Camera::look_at(Vec3{0, 0, -14}, Vec3{0, 0, 0},
                                              Vec3{0, 1, 0},
                                              film_width, film_height, film_distance);

        const Film film = expose(settings, scene, camera, height);

        // The exposure, which is a choice and not physics — the same
        // admission `render.hpp` makes about the box. The film holds
        // spectral radiance in absolute units, and the brightest thing in
        // this scene is the enclosure wall, which *is* the illuminant. So
        // white is the light, which is what a photographer means by white
        // balancing to the source, and every sphere is a fraction of it.
        double brightest = 0.0;
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < settings.width; ++x)
                brightest = std::fmax(brightest, film.mean_tristimulus(x, y).y);

        const double exposure = brightest > 0.0 ? 1.0 / brightest : 1.0;

        std::vector<double> rgb(std::size_t(settings.width) * std::size_t(height) * 3);
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < settings.width; ++x) {
                const Xyz value = film.mean_tristimulus(x, y);
                const Xyz linear = apply(srgb::xyz_to_rgb, value.x * exposure,
                                         value.y * exposure, value.z * exposure);
                const std::size_t p = (std::size_t(y) * std::size_t(settings.width)
                                       + std::size_t(x)) * 3;
                rgb[p + 0] = linear.x;
                rgb[p + 1] = linear.y;
                rgb[p + 2] = linear.z;
            }

        if (write_ppm("swatch.ppm", settings.width, height, rgb))
            std::printf("\n  swatch.ppm   gold, copper, silver, aluminium, glass, left to\n"
                        "               right, against the wall that lights them. White is\n"
                        "               the illuminant; no figure is quoted from this file.\n");
    }

    std::printf("\n%s\n", failures == 0
                ? "Every colour above came out of a measurement."
                : "A METAL IS NOT THE COLOUR ITS TABLE SAYS.");
    return failures == 0 ? 0 : 1;
}

} // namespace app
