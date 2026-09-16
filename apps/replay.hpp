// replay.hpp — running exactly one path again.
//
// A renderer that traces sixty million paths and produces one wrong pixel has
// a diagnostic problem before it has a physics problem: the failure is a
// single path out of sixty million, it happened inside a thread pool, and the
// usual answer is to add a print statement and re-run until it happens again.
//
// It does not have to be. `sampler.hpp` made a path's entire random state a
// function of two integers — the pixel and the sample index, addressed rather
// than dispensed — which was bought so that the thread count could not change
// the image. This is the second thing it pays for:
//
//      A path has an address, so a path can be run again.
//
// `film.hpp` asserts that no non-finite radiance reaches it and prints that
// address when one does. This is the other half: given the address, run that
// one path, alone, on one thread, and print what it did.
//
// ── What it shows, and what it does not ──────────────────────────────────
//
// It shows the path's inputs and its answer: the camera ray, the four
// wavelengths it carried, the radiance it returned, whether every component
// is a number, and what it would have deposited on the film. It runs the path
// three times and compares bit for bit, because a reproduction that is not
// reproducible is not one.
//
// And it shows the bounces, which it did not when it was written. The first
// version of this file said a bounce-by-bounce trace "needs a hook inside
// `radiance()`, and the transport is not getting one for a diagnostic while
// something cheaper works", and that "the day that stops being enough, the
// hook is a template parameter with a no-op default and it costs nothing —
// but it is not written until it is needed."
//
// It was needed one item later. A firefly is a single path that came back
// enormous, and the question item 0070 has to answer is *why* — which is a
// question about what the path did rather than what it returned. So
// `transport.hpp` has `Bounce` and `NoTrace` now, the default is inlined
// away, and a render timed 1.41 s before and 1.41 s after.
//
// It does not go looking. It replays the address it is given, and the address
// comes from the assertion that fired or from `./cornell render --outliers`.

#pragma once

#include <cstdio>
#include <string_view>
#include <vector>

#include <render/camera.hpp>
#include <render/film.hpp>
#include <render/sampler.hpp>
#include <render/scene.hpp>
#include <render/transport.hpp>

#include "render.hpp"

namespace app {

// The largest component of a throughput, which is what `roulette.hpp` uses
// for the same reason: killing a path kills it at every wavelength at once,
// so the question is whether *any* of the four still carries something.
inline double largest(const render::Reflectance& value) {
    double most = value[0];
    for (int i = 1; i < render::spectral_samples; ++i)
        most = std::fmax(most, value[i]);
    return most;
}

// `x,y,sample`, which is the spelling `film.hpp` prints. Returns false on
// anything else rather than guessing, because a replay of the wrong path is
// worse than a refusal.
inline bool parse_address(std::string_view text, int& x, int& y, std::uint64_t& sample) {
    long long values[3] = {0, 0, 0};
    int index = 0;
    bool any_digits = false;

    for (const char c : text) {
        if (c == ',') {
            if (!any_digits || index == 2) return false;
            ++index;
            any_digits = false;
        } else if (c >= '0' && c <= '9') {
            values[index] = values[index] * 10 + (c - '0');
            if (values[index] > 2147483647LL) return false;
            any_digits = true;
        } else {
            return false;
        }
    }

    if (index != 2 || !any_digits) return false;
    x = int(values[0]);
    y = int(values[1]);
    sample = std::uint64_t(values[2]);
    return true;
}

inline int replay(std::string_view address, const RenderSettings& settings) {
    using namespace render;

    int x = 0, y = 0;
    std::uint64_t sample = 0;
    if (!parse_address(address, x, y, sample)) {
        std::fprintf(stderr, "cornell: replay wants an address, as x,y,sample.\n");
        return 1;
    }

    const int height = height_for(settings.width);
    if (x < 0 || x >= settings.width || y < 0 || y >= height) {
        std::fprintf(stderr,
                     "cornell: (%d, %d) is not on a %d x %d film. The width the path\n"
                     "         was traced at has to be the width it is replayed at.\n",
                     x, y, settings.width, height);
        return 1;
    }

    const Scene box = cornell::box();
    const Camera camera = Camera::look_at(cornell::at(278.0, 273.0, -800.0),
                                          cornell::at(278.0, 273.0, 0.0),
                                          Vec3{0.0, 1.0, 0.0},
                                          film_width, film_height, film_distance);

    std::printf("Replaying pixel (%d, %d), sample %llu, on a %d x %d film.\n\n",
                x, y, static_cast<unsigned long long>(sample), settings.width, height);

    // The same three lines the film loop runs, in the same order, because the
    // order is the state: `sampler.hpp`'s stream is consumed by the jitter,
    // then the wavelength, then the transport, and a replay that draws them in
    // a different order replays a different path.
    Radiance carried{};
    Wavelengths lambdas = Wavelengths::sample(0.0);
    Ray ray{};

    for (int run = 0; run < 3; ++run) {
        Sampler sampler{std::uint64_t(y) * std::uint64_t(settings.width) + std::uint64_t(x),
                        sample};

        const auto [jitter_u, jitter_v] = sampler.next2();
        const double u = (double(x) + jitter_u) / double(settings.width);
        const double v = (double(y) + jitter_v) / double(height);

        const Wavelengths drawn = Wavelengths::sample(sampler.next());
        const Ray through = camera.ray_through(u, v);

        std::vector<Bounce> bounces;
        const Radiance value = radiance(box, through, drawn, sampler,
                                        default_max_depth, settings.roulette_start,
                                        [&](const Bounce& bounce) {
                                            if (run == 0) bounces.push_back(bounce);
                                        });

        if (run == 0) {
            carried = value;
            lambdas = drawn;
            ray = through;
            std::printf("  film point        u = %.17g\n"
                        "                    v = %.17g\n\n"
                        "  camera ray        from (%.9g, %.9g, %.9g)\n"
                        "                    towards (%.9g, %.9g, %.9g)\n\n",
                        u, v,
                        ray.origin.x, ray.origin.y, ray.origin.z,
                        ray.direction.x(), ray.direction.y(), ray.direction.z());

            std::printf("  wavelength        radiance\n");
            for (int i = 0; i < spectral_samples; ++i)
                std::printf("  %8.3f nm      %.17g%s\n", si::as::nm(lambdas[i]), value[i],
                            std::isfinite(value[i]) ? "" : "   <- not a number");

            // ── What it did ──────────────────────────────────────────────
            //
            // One row per bounce. `throughput` is the fraction of whatever
            // the path finds next that survives back to the eye, and it is
            // the column a firefly lives in: an ordinary path's throughput
            // falls monotonically, and a path that survived a roulette coin
            // is scaled *up* by the reciprocal of the probability it
            // survived. Where that is followed by finding the lamp, the two
            // together are the whole mechanism.
            std::printf("\n  depth   distance   throughput      q      emitter\n");
            for (const Bounce& bounce : bounces) {
                char emitted[48] = "";
                if (bounce.emitter)
                    std::snprintf(emitted, sizeof emitted, "   Le = %.4g",
                                  bounce.emitted[0]);

                std::printf("  %5d   %8.4f   %10.4g  %6.4f  %-6s%s\n",
                            bounce.depth, bounce.distance,
                            largest(bounce.after), bounce.survival,
                            bounce.killed ? "ended" : "", emitted);
            }
            if (bounces.empty())
                std::printf("      (the ray left the scene without hitting anything)\n");
        } else {
            bool identical = true;
            for (int i = 0; i < spectral_samples; ++i)
                identical = identical && value[i] == carried[i] && drawn[i] == lambdas[i];
            if (!identical) {
                std::printf("\n  Run %d did not reproduce run 1. The path is not addressed by\n"
                            "  (pixel, sample) after all, which is a worse bug than whatever\n"
                            "  was being looked for.\n", run + 1);
                return 1;
            }
        }
    }

    const Xyz deposited = cie::xyz_estimate(lambdas, carried);
    std::printf("\n  would deposit     X = %.9g   Y = %.9g   Z = %.9g\n",
                deposited.x, deposited.y, deposited.z);

    std::printf("\n  %s\n", finite(carried)
                ? "Every component is a number, and three runs agree bit for bit."
                : "A COMPONENT IS NOT A NUMBER. This is the path to put a breakpoint in.");

    return finite(carried) ? 0 : 1;
}

} // namespace app
