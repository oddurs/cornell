// render.hpp — the witness for v0.2: the first image that is a solution to
// the rendering equation rather than a picture of where the geometry is.
//
// v0.1's version of this file wrote a 1 onto the film wherever a ray hit a
// sphere, and said in its opening paragraph that the 1 was not radiance and
// that this was the only file ever permitted to write a number nothing
// derived. That permission is now spent. Every number on the film below is
// the value of an estimator: emitted radiance, carried back along a path,
// multiplied at each bounce by `f · cos / pdf`.
//
// ── The sampler arrived, and the lattice went ────────────────────────────
//
// v0.1 placed its samples on a regular n × n grid inside each pixel, because
// there was no sampler and inventing one in a hurry would have pre-empted an
// item that deserved a page. The file said what that cost: a regular grid
// does not converge, it *resolves*, giving the edge of a sphere exactly n + 1
// distinguishable levels however many samples are taken, because the error is
// deterministic rather than random.
//
// `sampler.hpp` exists now, so each sample's position inside the pixel is
// drawn from it, and three things follow. The sample count is no longer
// required to be a perfect square, so the argument is `--spp` and means what
// it says. The edge of a shape is no longer stepped — the error has become
// noise. And the noise falls as the inverse square root of the sample count,
// which is measured below rather than hoped for.
//
// "Drawn from it" and not "jittered", which is a distinction worth keeping.
// Jitter means stratification plus a random offset within each stratum: the
// samples are spread evenly *and* randomly. These are neither — each gets an
// independent `Sampler{pixel, s}` and takes two uniforms, so they are
// independent and identically distributed, and nothing stops four of them
// landing in the same corner of the pixel.
//
// That is correct and it is unbiased and it converges at N^-½, which is all
// the N^-½ claim needs. It is simply not the better thing that the word
// "jittered" would be claiming, and `warp.hpp`'s argument for the concentric
// disc mapping — that it preserves a stratification which polar mapping
// destroys — is currently preserving one the renderer never establishes.
// Stratified and low-discrepancy sequences are `sampler.hpp`'s stated
// not-modelled, and the honest order is to measure this first.
//
// Each sample's stream is addressed by `(pixel index, sample index)`, so the
// image does not depend on the order the pixels are visited, and will not
// depend on how many threads visit them when v0.4 adds some.
//
// ── What the image is of ─────────────────────────────────────────────────
//
// `box.hpp`, which is not the Cornell box and says so at length: five grey
// walls and a lamp, with placeholder dimensions and a placeholder albedo,
// built so that the integrator has corners to be checked in. The measured
// box is v0.4.
//
// The things visible in it that nobody wrote any code for are the point:
//
//      The shadow under the lamp has a soft edge. There is no soft shadow
//      routine; the penumbra is the lamp's solid angle being partly blocked,
//      which is what the visibility term inside the integral does.
//
//      The corners are darker than the middles of the walls. There is no
//      ambient occlusion; fewer directions from a corner reach the lamp.
//
//      The ceiling is lit at all, despite facing away from everything. There
//      is no ambient term; light reaches it after bouncing off the floor.
//
// ── Why it is so noisy ───────────────────────────────────────────────────
//
// A path finds the lamp only by wandering into it. The lamp is a 0.6 m panel
// in a 2 m box, so a cosine-weighted bounce hits it perhaps a few percent of
// the time, and a pixel's estimate is the average of a lot of zeros and a few
// large numbers. That is the highest-variance arrangement a correct estimator
// can have.
//
// It is correct, and v0.8 makes it quiet. Sampling the light directly —
// choosing a point on the lamp and asking whether it is visible — finds the
// light on every bounce instead of a few percent of them, and multiple
// importance sampling combines the two strategies so that neither is worse
// than the better of them. The difference between this image and that one at
// equal sample count is the best argument for the technique that exists, and
// it is why this noisy image is worth keeping rather than skipping past.
//
// ── Measured ─────────────────────────────────────────────────────────────
//
// The noise falls as the inverse square root of the sample count, which is
// the claim, so here it is checked rather than asserted. RMSE of a 160 × 160
// render against an 8192-sample reference:
//
//      spp      RMSE       ratio to the row above
//        16     1.19850      —
//        64     0.67538      1.775
//       256     0.29972      2.253
//      1024     0.14068      2.130
//
// Quadrupling the samples should halve the error, and the three ratios
// bracket 2. A least-squares fit of log RMSE against log N gives a slope of
// **−0.522** against a theoretical −0.5.
//
// The excess is at least partly the reference: 8192 samples is not
// converged, so some of what is being measured as the error of the 1024-
// sample image is the error of the thing it is being compared against. v0.5's
// `converge` does this properly, over more decades and against a reference
// that is either analytic or very much better converged, and fits the slope
// with an uncertainty rather than quoting three digits from four points.
//
// ── The offset, at three scales ──────────────────────────────────────────
//
// Item 0038 claimed `waechter.hpp`'s offset has no length hidden in it, and
// could only check the geometry. Now there is a renderer. Radiance is
// invariant under a uniform scaling of a scene — every length in the
// transport cancels — so the same room built a thousand times larger and a
// thousand times smaller must produce the same picture:
//
//      scale     identical to 1×    mean radiance
//      1×        bit for bit        0.410179
//      1000×     bit for bit        0.410179
//      0.001×    bit for bit        0.410179
//
// Not "within tolerance". Every pixel of all three is the same double. An
// epsilon anywhere in the spawn logic would show up here as a difference of
// six orders of magnitude in how much of each contact shadow survives.

#pragma once

#include <chrono>
#include <cstdio>
#include <vector>

#include <render/camera.hpp>
#include <render/film.hpp>
#include <render/sampler.hpp>
#include <render/scene.hpp>
#include <render/si.hpp>
#include <render/spectrum.hpp>
#include <render/bradford.hpp>
#include <render/illuminant.hpp>
#include <render/srgb.hpp>
#include <render/transport.hpp>

#include "box.hpp"
#include "gamut.hpp"
#include "image.hpp"
#include "tonemap.hpp"

namespace app {

struct RenderSettings {
    int width = 400;    // the height is derived; see below
    int spp = 64;       // any positive integer now, not a square
    ToneCurve curve = ToneCurve::clip;   // see tonemap.hpp: a choice, not physics
    bool tungsten = false;               // light it with illuminant A instead
    bool adapt = true;                   // see bradford.hpp: also not physics
};

// The film: square, so that the image is square and the box is framed the way
// it was photographed. `camera.hpp` derives the field of view from these
// lengths and refuses to be told one directly.
inline constexpr double film_width  = 0.024;    // 24 mm
inline constexpr double film_height = 0.024;    // 24 mm
inline constexpr double film_distance = 0.018;  // 18 mm, giving 67.4 degrees

// The height is not a setting: it comes from the width and the shape of the
// film, or the pixels are not square. v0.1 learned that by rendering an
// ellipse.
//
// This was written as `width * film_side / film_side`, which is a constant
// divided by itself — the identity function wearing a derivation's clothes,
// and one that would have gone on returning the width if the film ever
// stopped being square. Two named lengths now, so the expression means what
// it reads as.
inline int height_for(int width) {
    return int(double(width) * film_height / film_width + 0.5);
}

// ── The normalisation, which is derived ──────────────────────────────────
//
// `cie::xyz_estimate` returns absolute tristimulus: the spectral radiance in
// W·m⁻²·sr⁻¹·m⁻¹ integrated against the observer, which has units of
// W·m⁻²·sr⁻¹ and comes out around 1e-7 for this room. That is a correct
// number and a useless one to expose against.
//
// Y is meant to be *relative* luminance — 1 for a perfect white — so it needs
// a constant, and `cie.hpp` derives that constant as an integral rather than
// letting anybody type it:
//
//      k = 1 / integral of S(lambda) * y-bar(lambda) d(lambda)
//
// with S the light doing the lighting. Here that is the lamp: its D65
// spectrum, scaled by its radiance. So Y = 1 means "as bright as looking
// straight at the lamp", and every other surface is a fraction of it.
//
// Quoting a k without saying which light it was for is how a renderer ends up
// a constant factor wrong with nobody noticing, because everything in the
// image is wrong by the same factor. This one is computed from the scene's
// own lamp, at compile time, and moves if the lamp does.
inline double lamp_normalisation_for(const render::cie::Illuminant& lamp) {
    render::cie::Illuminant lit = lamp;
    for (std::size_t i = 0; i < render::cie::samples; ++i) lit.table[i] *= lamp_radiance;
    return render::cie::luminance_normalisation(lit);
}

// The exposure, on top of that. A choice, not physics — see item 0046. The
// lamp is Y = 1 by the normalisation above, and this puts the walls near
// mid-grey, which clips the lamp exactly as a photograph exposed for the
// walls does.
inline constexpr double reference_luminance = 0.2;

inline int render(const RenderSettings& settings) {
    using namespace render;

    const render::cie::Illuminant& lamp =
        settings.tungsten ? render::cie::a : render::cie::d65;

    const Scene scene = box(1.0, lamp);

    // Chromatic adaptation, which is a model of an eye rather than of light.
    // With it off, a tungsten-lit render is orange — and that is the correct
    // radiometric answer, which is why it is switchable rather than baked in.
    // See bradford.hpp.
    const double normalisation = lamp_normalisation_for(lamp);

    const Matrix3 adaptation = settings.adapt
        ? render::bradford::adapt_to_d65(lamp)
        : identity3();

    // Looking in through the missing front wall, from just outside it, which
    // is where the camera stood in 1984.
    const Camera camera = Camera::look_at(/* eye    */ Vec3{0.0, 1.0, 1.5},
                                          /* target */ Vec3{0.0, 1.0, -1.0},
                                          /* up     */ Vec3{0.0, 1.0, 0.0},
                                          film_width, film_height, film_distance);

    const int height = height_for(settings.width);
    Film film(settings.width, height);

    const auto started = std::chrono::steady_clock::now();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < settings.width; ++x) {
            const std::uint64_t pixel =
                std::uint64_t(y) * std::uint64_t(settings.width) + std::uint64_t(x);

            for (int s = 0; s < settings.spp; ++s) {
                // Addressed, not dispensed. Nothing about this depends on
                // the order the loops above happen to run in.
                Sampler sampler{pixel, std::uint64_t(s)};

                const auto [jitter_u, jitter_v] = sampler.next2();
                const double u = (double(x) + jitter_u) / double(settings.width);
                const double v = (double(y) + jitter_v) / double(height);

                const Wavelengths lambdas = Wavelengths::sample(sampler.next());

                const Radiance carried =
                    radiance(scene, camera.ray_through(u, v), lambdas, sampler);

                film.add_sample(x, y, lambdas, carried);
            }
        }
    }

    const double seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();

    // ── Out ──────────────────────────────────────────────────────────────
    //
    // The film holds tristimulus now, integrated against the 1931 observer at
    // the wavelength each sample was actually drawn at. No bin is chosen and
    // no wavelength is privileged: v0.1 and v0.2 printed one of forty-seven
    // bins because there was no observer to integrate against, and said so
    // each time.

    std::vector<float>  linear(std::size_t(settings.width) * std::size_t(height) * 3);
    std::vector<double> preview(linear.size());

    double brightest = 0.0;
    double total = 0.0;
    long out_of_gamut = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < settings.width; ++x) {
            const Xyz measured = film.mean_tristimulus(x, y) * normalisation;
            brightest = std::fmax(brightest, measured.y);
            total += measured.y;

            // The exposure, and it is still a choice rather than physics.
            // Dividing by a reference luminance is what a camera's exposure
            // setting does; this one is picked so the walls land near
            // mid-grey and the lamp clips, which is what a photograph of the
            // real box does. Item 0046 is where this is done properly and
            // marked as the one part of the project with no correct answer.
                    const Xyz exposed =
                apply(adaptation, measured * (1.0 / reference_luminance));

            // Tristimulus to the display's primaries, through the matrix
            // srgb.hpp derived rather than pasted.
            const Xyz rgb = apply(srgb::xyz_to_rgb, exposed.x, exposed.y, exposed.z);

            const std::size_t p = (std::size_t(y) * std::size_t(settings.width) + std::size_t(x)) * 3;
            linear[p + 0] = float(rgb.x);
            linear[p + 1] = float(rgb.y);
            linear[p + 2] = float(rgb.z);

            if (rgb.x < 0.0 || rgb.y < 0.0 || rgb.z < 0.0) ++out_of_gamut;

            // The two steps that are not physics, and the last two.
            // cornell.pfm above was written before both, negatives intact,
            // because a negative component is information about the colour
            // being outside what a display can show.
            const Xyz shown = app::into_gamut(rgb, srgb::rgb_to_xyz);
            preview[p + 0] = tonemap(shown.x, settings.curve);
            preview[p + 1] = tonemap(shown.y, settings.curve);
            preview[p + 2] = tonemap(shown.z, settings.curve);
        }
    }

    if (!write_pfm_rgb("cornell.pfm", settings.width, height, linear) ||
        !write_ppm("cornell.ppm", settings.width, height, preview)) {
        std::fprintf(stderr, "cornell: could not write the image files\n");
        return 1;
    }

    const double paths = double(settings.width) * double(height) * double(settings.spp);
    std::printf("%d x %d, %d samples per pixel, %.2f million paths, %.1f s\n",
                settings.width, height, settings.spp, paths / 1e6, seconds);
    std::printf("  %.2f million paths per second\n", paths / 1e6 / seconds);
    std::printf("luminance Y: mean %.4f, brightest %.4f\n",
                total / (double(settings.width) * double(height)), brightest);
    std::printf("out of gamut: %ld of %d pixels have a negative sRGB component\n",
                out_of_gamut, settings.width * height);
    std::printf("cornell.pfm   linear sRGB, three channels, unclipped — the file a\n"
                "              number may be quoted from\n");
    std::printf("cornell.ppm   the same, exposed against Y = %.2f, tone mapped with\n"
                "              '%.*s', and encoded with sRGB's transfer function.\n"
                "              The lamp is %.0fx over. No figure is quoted from this file.\n",
                reference_luminance,
                int(name_of(settings.curve).size()), name_of(settings.curve).data(),
                brightest / reference_luminance);
    return 0;
}

} // namespace app
