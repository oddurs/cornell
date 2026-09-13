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
// `sampler.hpp` exists now, so the samples are jittered, and three things
// follow. The sample count is no longer required to be a perfect square, so
// the argument is `--spp` and means what it says. The edge of a shape is no
// longer stepped — the error has become noise. And the noise falls as the
// inverse square root of the sample count, which is a claim v0.5 will measure
// rather than a hope.
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
#include <render/transport.hpp>

#include "box.hpp"
#include "image.hpp"

namespace app {

struct RenderSettings {
    int width = 400;    // the height is derived; see below
    int spp = 64;       // any positive integer now, not a square
};

// Square film, so that the image is square and the box is framed the way it
// was photographed. `camera.hpp` derives the field of view from these two
// lengths and refuses to be told one directly.
inline constexpr double film_side = 0.024;      // 24 mm
inline constexpr double film_distance = 0.018;  // 18 mm, giving 67.4 degrees

// The height is not a setting: it comes from the width and the shape of the
// film, or the pixels are not square. v0.1 learned that by rendering an
// ellipse.
inline int height_for(int width) {
    return int(double(width) * film_side / film_side + 0.5);
}

inline int render(const RenderSettings& settings) {
    using namespace render;

    const Scene scene = box();

    // Looking in through the missing front wall, from just outside it, which
    // is where the camera stood in 1984.
    const Camera camera = Camera::look_at(/* eye    */ Vec3{0.0, 1.0, 1.5},
                                          /* target */ Vec3{0.0, 1.0, -1.0},
                                          /* up     */ Vec3{0.0, 1.0, 0.0},
                                          film_side, film_side, film_distance);

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
                    radiance(scene, camera.ray_through(u, v), sampler);

                film.add_sample(x, y, lambdas, carried);
            }
        }
    }

    const double seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();

    // ── Out ──────────────────────────────────────────────────────────────
    //
    // Still one wavelength of forty-seven, because there is still no
    // observer. 550 nm is where the photopic curve peaks, which is a fact
    // about eyes and is used here only to choose which of the bins to print.
    const int bin = Film::bin_of(550.0e-9);

    std::vector<float> linear(std::size_t(settings.width) * std::size_t(height));
    std::vector<double> preview(linear.size() * 3);

    double brightest = 0.0;
    double total = 0.0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < settings.width; ++x) {
            const std::size_t p = std::size_t(y) * std::size_t(settings.width) + std::size_t(x);
            const double value = film.mean_radiance(x, y, bin);
            linear[p] = float(value);
            brightest = std::fmax(brightest, value);
            total += value;
        }
    }

    // The exposure, stated rather than hidden.
    //
    // Dividing by a reference radiance is exactly what a camera's exposure
    // setting does, and choosing the reference is exactly what a photographer
    // does. One W·m⁻²·sr⁻¹·m⁻¹ maps to white here, which puts the walls — at
    // about 0.4 — near mid-grey and blows the lamp, which is twelve times
    // over, out to pure white.
    //
    // That is what a photograph of the real box looks like, because it is
    // exposed for the walls and the lamp is the brightest thing in the room
    // by an order of magnitude. Exposing for the lamp instead is defensible,
    // and it renders a nearly black picture of a correctly lit room.
    //
    // It is a choice and it is not physics, which is why it is one named
    // constant with a paragraph attached rather than a curve. The proper
    // treatment — and the marking of it as not-physics — is item 0046 in
    // v0.3. No figure is ever quoted from this image; the PFM is for that.
    const double reference = 1.0;
    for (std::size_t p = 0; p < linear.size(); ++p) {
        const double shown = double(linear[p]) / reference;
        preview[p * 3 + 0] = shown;
        preview[p * 3 + 1] = shown;
        preview[p * 3 + 2] = shown;
    }

    if (!write_pfm("cornell.pfm", settings.width, height, linear) ||
        !write_ppm("cornell.ppm", settings.width, height, preview)) {
        std::fprintf(stderr, "cornell: could not write the image files\n");
        return 1;
    }

    const double paths = double(settings.width) * double(height) * double(settings.spp);
    std::printf("%d x %d, %d samples per pixel, %.2f million paths, %.1f s\n",
                settings.width, height, settings.spp, paths / 1e6, seconds);
    std::printf("  %.2f million paths per second\n", paths / 1e6 / seconds);
    std::printf("spectral radiance at %.0f nm, W/m2/sr/m: mean %.4f, brightest %.4f\n",
                si::as::nm(Film::bin_centre(bin)), total / double(linear.size()), brightest);
    std::printf("cornell.pfm   the linear data, which is what a number may be quoted from\n");
    std::printf("cornell.ppm   the same, exposed against %.1f W/m2/sr/m and gamma 2.2,\n"
                "              which clips the lamp at %.0fx over, as a photograph would\n",
                reference, brightest / reference);
    return 0;
}

} // namespace app
