// render.hpp — the witness for v0.1, and the only one that will ever be
// allowed to put a number on the film that nothing derived.
//
// The milestone this belongs to says what it is for, and it is worth quoting
// rather than paraphrasing:
//
//      the image is wrong in every way except its geometry. No shading, no
//      colour, no light — a ray leaves the aperture, meets a sphere or it
//      does not, and the film records which.
//
// So that is what it does. A ray per sub-pixel sample, an intersection test,
// and a flat spectral radiance of exactly 1 W·m⁻²·sr⁻¹·m⁻¹ written into the
// film where the ray hit something and nothing where it did not.
//
// That 1 is not radiance. There is no emitter, no light, no surface that
// could have emitted it, and no rendering equation yet to be a solution of.
// It is the geometry, poured into a container built to hold radiance, and
// this file is the only place in the project where that is permitted — the
// admission being the price of the permission. In v0.2 the sphere becomes a
// surface, the box acquires a light, and the number on the film becomes the
// value of an integral. Every number after that one is derived.
//
// ── There is no sampler, and these are not random ─────────────────────────
//
// A sampler — reproducible per pixel, per sample, independent of the order
// the threads happen to run in — is a v0.2 item with a page of its own, and
// pre-empting it here with a hasty `rand()` would be the worse kind of
// shortcut: one that works.
//
// So the samples are placed on a regular grid: an n × n lattice inside the
// pixel, each at the centre of its cell, and the wavelength offset stratified
// the same way along the sample index. This is not stochastic sampling and it
// does not behave like it. A regular grid does not converge, it *resolves*:
// the edge of the sphere gets exactly n + 1 distinguishable levels and no
// more, so it will always look faintly stepped, and no sample count fixes it
// because the error is deterministic. Jitter is what turns that structured
// error into noise that falls as N^-½, and jitter needs the sampler.
//
// The file says this rather than shipping a smooth-looking edge, because a
// reader comparing v0.1 against v0.2 should be able to see what randomness
// bought.
//
// ── The edge is quantised by the bins, not by the sample count ────────────
//
// There is a second limit on the edge, and it is the film's, not the
// sampler's. `film.hpp` warns that at N samples per pixel a bin holds about
// 4N/47 of them; the image printed here is one bin, so the number of
// distinguishable greys along the edge of the sphere is that occupancy and
// not N. Measured:
//
//      root    samples/pixel    4N/47 expected    distinct levels
//        8            64              5.4                5
//       12           144             12.3               13
//       20           400             34.0               33
//
// which is the film's own arithmetic showing up in a picture on the first
// day. It goes away in v0.3, when the image becomes an integral over all 47
// bins against the observer rather than a look at one of them.

#pragma once

#include <cstdio>
#include <string>
#include <vector>

#include <render/camera.hpp>
#include <render/film.hpp>
#include <render/si.hpp>
#include <render/sphere.hpp>
#include <render/spectrum.hpp>

#include "image.hpp"

namespace app {

struct RenderSettings {
    int width = 480;    // the height is derived; see below
    int root  = 8;      // samples per pixel is this squared
};

// The height is not a setting.
//
// A film is 36 × 24 mm, and a grid of pixels laid over it has to have the
// same shape or the pixels are not square — at which point a sphere renders
// as an ellipse, and the first thing this program ever draws is wrong in a
// way that looks like a modelling decision. It was, for one commit: 400 × 300
// over 3:2 film produced a disc 287 pixels across and 323 tall, clipped top
// and bottom by an image that was not tall enough to hold it.
//
// So the width is asked for and the height falls out, the same way the field
// of view falls out of the film and the distance in `camera.hpp`. 480 is the
// default because 480 × 320 is exactly 3:2 and needs no rounding; any other
// width rounds to the nearest whole pixel and is anisotropic by whatever that
// rounding was worth.
inline int height_for(int width) {
    return int(double(width) * render::film_35mm_height / render::film_35mm_width + 0.5);
}

inline int render(const RenderSettings& settings) {
    using namespace render;

    // A half-metre sphere three metres away, seen with a 50 mm lens on 35 mm
    // film. Nothing here is the Cornell box: the box is measured geometry and
    // it arrives in v0.4 with its reflectances attached. This is a sphere,
    // and it is here because a sphere is the cheapest thing to be wrong
    // about.
    //
    // Three metres rather than two because the framing has to be derived too.
    // At two metres the sphere subtends 28.955°, the film's vertical field of
    // view is 26.9915°, and the disc is cut off top and bottom by an image
    // that is not tall enough to hold it. At three it subtends 19.188° and
    // fits, with room.
    const Sphere subject{Vec3{0.0, 0.0, -3.0}, 0.5};

    const Camera camera = Camera::look_at(/* eye    */ Vec3{0.0, 0.0, 0.0},
                                          /* target */ Vec3{0.0, 0.0, -2.0},
                                          /* up     */ Vec3{0.0, 1.0, 0.0},
                                          film_35mm_width, film_35mm_height,
                                          0.050);

    const int height = height_for(settings.width);

    Film film(settings.width, height);

    // The number that is not derived. See the top of the file.
    const Radiance marker(1.0);

    const int per_pixel = settings.root * settings.root;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < settings.width; ++x) {
            for (int j = 0; j < settings.root; ++j) {
                for (int i = 0; i < settings.root; ++i) {
                    // The centre of cell (i, j) of the lattice, in film
                    // coordinates. Deterministic, and deliberately so.
                    const double u = (double(x) + (double(i) + 0.5) / settings.root)
                                     / double(settings.width);
                    const double v = (double(y) + (double(j) + 0.5) / settings.root)
                                     / double(height);

                    const int index = j * settings.root + i;
                    const Wavelengths lambdas =
                        Wavelengths::sample((double(index) + 0.5) / double(per_pixel));

                    // Every sample is deposited, including the ones that
                    // hit nothing, and that is not a formality. The film's
                    // mean is a sum over a count, and a missed ray that
                    // deposits *nothing* does not increment the count — so
                    // an edge pixel where one sample in sixty-four hits
                    // averages that one sample against no others and comes
                    // out fully lit. The first version of this loop did
                    // exactly that, and the image had two distinct values in
                    // it: black and white, with no edge at all. A path that
                    // found no emitter carries zero radiance, which is a
                    // measurement, not an absence.
                    const Ray ray = camera.ray_through(u, v);
                    film.add_sample(x, y, lambdas,
                                    subject.intersect(ray) ? marker : Radiance{});
                }
            }
        }
    }

    // ── Out ───────────────────────────────────────────────────────────────
    //
    // One wavelength, because there is no observer. 550 nm is where the
    // photopic curve peaks, which is a fact about eyes and therefore not a
    // fact this file is entitled to use for anything except choosing which of
    // 47 identical bins to print. When `cie.hpp` arrives in v0.3 this becomes
    // an integral against the observer and the choice goes away.
    const int bin = Film::bin_of(550.0e-9);

    std::vector<float>  linear(std::size_t(settings.width) * std::size_t(height));
    std::vector<double> preview(linear.size() * 3);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < settings.width; ++x) {
            const std::size_t p = std::size_t(y) * std::size_t(settings.width) + std::size_t(x);
            const double value = film.mean_radiance(x, y, bin);
            linear[p] = float(value);

            // Dividing by a reference radiance is an exposure. It is 1 here
            // because the only non-zero value on this film is 1, and saying
            // so is cheaper than pretending there is no exposure at all.
            preview[p * 3 + 0] = value;
            preview[p * 3 + 1] = value;
            preview[p * 3 + 2] = value;
        }
    }

    const bool wrote_pfm = write_pfm("cornell.pfm", settings.width, height, linear);
    const bool wrote_ppm = write_ppm("cornell.ppm", settings.width, height, preview);
    if (!wrote_pfm || !wrote_ppm) {
        std::fprintf(stderr, "cornell: could not write the image files\n");
        return 1;
    }

    std::printf("%d x %d, %d samples per pixel, %.1f million rays\n",
                settings.width, height, per_pixel,
                double(settings.width) * double(height) * double(per_pixel) / 1e6);
    std::printf("cornell.pfm   linear spectral radiance at %.0f nm, in W/m2/sr/m\n",
                si::as::nm(Film::bin_centre(bin)));
    std::printf("cornell.ppm   the same thing through a gamma of 2.2, for looking at\n");
    return 0;
}

} // namespace app
