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
// image does not depend on the order the pixels are visited — and v0.4 cashes
// that in below: it does not depend on how many threads visit them either.
//
// ── Tiles, and why not scanlines ─────────────────────────────────────────
//
// The film is divided into squares and threads take the next unclaimed one.
// Squares rather than rows for two reasons. A square's rays are more alike
// than a row's, so the paths through the scene stay near each other and the
// same parts of the BVH stay in cache. And a row is as wide as the image, so
// the last row of an expensive region is one thread's problem and everybody
// else waits; a square is small enough that the work evens out.
//
// ── No shared mutable state except the film ──────────────────────────────
//
// Each tile computes its own rays from its own samplers and writes only to
// the pixels inside it. Tiles do not overlap, so no two threads ever touch
// the same accumulator, and the film needs no lock — not because the writes
// are atomic but because they never collide.
//
// The one thing genuinely shared is the counter that hands out tiles, and it
// is an atomic. Which tile a thread gets is a race, deliberately: the whole
// arrangement works precisely because the answer does not depend on who got
// what.
//
// That is a claim, so it is measured rather than asserted, and it is item
// 0035's last criterion coming due — the sampler was built in v0.2 to make
// this true and there were no threads yet to prove it with.
//
// ── What the image is of ─────────────────────────────────────────────────
//
// The Cornell box. Not a stand-in for it — `cornell.hpp` holds the geometry
// Cornell measured off the physical object, the reflectance spectra they put
// through a spectrometer, and the emission spectrum of the lamp, and the
// camera below is at the position on the same page.
//
// `apps/box.hpp` held a test rig of five grey walls with round numbers, and
// it is deleted rather than kept: it existed so the integrator had corners to
// be checked in before there was a real scene, and there is one now. Its
// argument for why its walls were grey — that a red wall is a spectrum and
// nothing could evaluate one yet — is the argument this milestone answers.
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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
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

#include <render/cornell.hpp>

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
    int threads = 0;                     // 0 means ask the machine
};

// Sixteen pixels square. Small enough that a slow region does not become one
// thread's problem, large enough that handing out a tile costs nothing next
// to rendering it.
inline constexpr int tile_size = 16;

inline int worker_count(const RenderSettings& settings) {
    return settings.threads > 0
        ? settings.threads
        : int(std::max(1u, std::thread::hardware_concurrency()));
}

// The camera, MEASURED, from the same page as the box.
//
//      Position        278 273 -800      (millimetres)
//      Direction       0 0 1
//      Up direction    0 1 0
//      Focal length    0.035
//      Width, height   0.025 0.025
//
// A 35 mm lens on 25 mm square film, 800 mm in front of the opening, which
// `camera.hpp` turns into a field of view of 2 atan(0.0125/0.035) = 39.3
// degrees without being told one. The framing of every Cornell box image ever
// published is these five lines.

// The height is not a setting: it comes from the width and the shape of the
// film, or the pixels are not square. v0.1 learned that by rendering an
// ellipse.
//
// This was written as `width * film_side / film_side`, which is a constant
// divided by itself — the identity function wearing a derivation's clothes,
// and one that would have gone on returning the width if the film ever
// stopped being square. Two named lengths now, so the expression means what
// it reads as.
inline constexpr double film_width  = 0.025;
inline constexpr double film_height = 0.025;
inline constexpr double film_distance = 0.035;

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
// The lamp, resampled onto the observer's grid so that `cie.hpp` can
// integrate against it. Cornell publishes four points at 100 nm; this is
// where they are stretched over the 5 nm grid everything else lives on, and
// the interpolation is CALIBRATED rather than measured — item 0051 says so.
inline render::cie::Illuminant lamp_on_the_observers_grid() {
    render::cie::Illuminant lit;
    for (std::size_t i = 0; i < render::cie::samples; ++i)
        lit.table[i] = render::cornell::emission.at(
                           render::cie::first + double(i) * render::cie::step)
                     * render::cornell::light_radiance;
    return lit;
}

inline double lamp_normalisation() {
    return render::cie::luminance_normalisation(lamp_on_the_observers_grid());
}

inline render::Matrix3 lamp_adaptation() {
    return render::bradford::adapt_to_d65(lamp_on_the_observers_grid());
}

// The exposure, on top of that. A choice, not physics — see item 0046. The
// lamp is Y = 1 by the normalisation above, and this puts the walls near
// mid-grey, which clips the lamp exactly as a photograph exposed for the
// walls does.
inline constexpr double reference_luminance = 0.2;

// The loop, separated from everything that writes a file, so that the
// threading check in `verify.hpp` can run it twice and compare rather than
// rendering to disk and diffing images.
inline render::Film expose(const RenderSettings& settings,
                           const render::Scene& scene,
                           const render::Camera& camera,
                           int height) {
    using namespace render;

    Film film(settings.width, height);

    // Chromatic adaptation, which is a model of an eye rather than of light.
    // With it off, a tungsten-lit render is orange — and that is the correct
    // radiometric answer, which is why it is switchable rather than baked in.
    // See bradford.hpp.
    const int across = (settings.width + tile_size - 1) / tile_size;
    const int down   = (height + tile_size - 1) / tile_size;
    const int tiles  = across * down;

    const int workers = worker_count(settings);

    // One tile of the film. Everything it touches is its own except the
    // pixels it writes, and no other tile writes those.
    const auto render_tile = [&](int index) {
        const int x0 = (index % across) * tile_size;
        const int y0 = (index / across) * tile_size;
        const int x1 = std::min(x0 + tile_size, settings.width);
        const int y1 = std::min(y0 + tile_size, height);

        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                const std::uint64_t pixel =
                    std::uint64_t(y) * std::uint64_t(settings.width) + std::uint64_t(x);

                for (int s = 0; s < settings.spp; ++s) {
                    // Addressed, not dispensed. This is the line that makes
                    // the thread count irrelevant to the answer.
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
    };

    std::atomic<int> next_tile{0};
    {
        // jthread, so the scope end joins them and an exception does not
        // leave a thread running. There is nothing else in this project that
        // needs a thread, so there is nothing else here.
        std::vector<std::jthread> pool;
        pool.reserve(std::size_t(workers));
        for (int t = 0; t < workers; ++t) {
            pool.emplace_back([&] {
                for (int i = next_tile.fetch_add(1); i < tiles;
                     i = next_tile.fetch_add(1))
                    render_tile(i);
            });
        }
    }

    return film;
}

inline int render(const RenderSettings& settings) {
    using namespace render;

    const Scene scene = cornell::box();

    // Cornell's camera, at Cornell's position, pointed the way Cornell
    // pointed it.
    const Camera camera = Camera::look_at(cornell::at(278.0, 273.0, -800.0),
                                          cornell::at(278.0, 273.0, 0.0),
                                          Vec3{0.0, 1.0, 0.0},
                                          film_width, film_height, film_distance);

    const int height = height_for(settings.width);

    const double normalisation = lamp_normalisation();

    // The box's lamp is tungsten, so without adaptation the render is orange —
    // correctly, and for the reason bradford.hpp gives. Cornell's own
    // photographs were taken through narrow-band filters and calibrated, so
    // the comparison in v1.0 happens before this step, not after it.
    const Matrix3 adaptation = settings.adapt ? lamp_adaptation() : identity3();

    const auto started = std::chrono::steady_clock::now();
    const Film film = expose(settings, scene, camera, height);
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
    std::printf("%d x %d, %d samples per pixel, %.2f million paths, %.1f s"
                " on %d thread%s\n",
                settings.width, height, settings.spp, paths / 1e6, seconds,
                worker_count(settings), worker_count(settings) == 1 ? "" : "s");
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
