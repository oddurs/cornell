// verify.hpp — the inspection sheet.
//
// Item 0065 in v0.5 is what this file is meant to become: every physical
// claim the project makes, checked in one run, printed as a sheet somebody
// could sign. It arrives early and nearly empty because v0.4's BVH needed a
// check that cannot be a `static_assert` — a million rays is not a compile-
// time quantity — and a check with nowhere to live is a check that does not
// get run.
//
// So: one section now, and the shape for the rest.
//
// ── What a check in here has to be ───────────────────────────────────────
//
// Comparable against something that is not this program's opinion. A
// published figure, an analytic answer, or a second implementation that was
// written to be obviously correct rather than fast. A test that compares the
// renderer against itself passes whatever the renderer does.

#pragma once

#include <cmath>
#include <cstdio>
#include <vector>

#include <render/cornell.hpp>
#include <render/sampler.hpp>
#include <render/scene.hpp>
#include <render/si.hpp>

#include "render.hpp"

namespace app {

namespace detail {

struct Outcome {
    long compared = 0;
    long disagreed = 0;
    double worst_distance = 0.0;
};

// The claim: the acceleration structure changes only the speed of the answer.
//
// Both routes are asked the same question and the answers are compared
// exactly — not to a tolerance. They run the same intersection arithmetic on
// the same primitives in a different order, so the distances should be
// identical bit patterns, and anything else means the index dropped or
// reordered something.
inline Outcome agrees_with_brute_force(const render::Scene& scene,
                                       const std::vector<render::Ray>& rays) {
    Outcome out;
    for (const render::Ray& ray : rays) {
        const auto fast = scene.intersect(ray);
        const auto slow = scene.intersect_exhaustively(ray);
        ++out.compared;

        if (fast.has_value() != slow.has_value()) { ++out.disagreed; continue; }
        if (!fast) continue;

        // The distance, exactly, and the primitive it belongs to.
        if (fast->t != slow->t || fast->surface != slow->surface) {
            ++out.disagreed;
            out.worst_distance = std::fmax(out.worst_distance, std::fabs(fast->t - slow->t));
        }
    }
    return out;
}

inline void report(const char* what, const Outcome& o) {
    std::printf("  %-46s %9ld rays   %s\n", what, o.compared,
                o.disagreed == 0 ? "agree"
                                 : "DISAGREE");
    if (o.disagreed != 0)
        std::printf("      %ld disagreements, worst distance %.3e\n",
                    o.disagreed, o.worst_distance);
}

} // namespace detail

inline int verify() {
    using namespace render;

    std::printf("cornell — inspection sheet\n\n");
    std::printf("The acceleration structure changes only the speed of the answer.\n");
    std::printf("Every ray is intersected through the index and against every\n");
    std::printf("primitive, and the two distances are compared exactly.\n\n");

    Scene scene = cornell::box();
    bool all_agree = true;

    // 1. A million rays, origins inside and outside, directions anywhere.
    {
        Sampler rng{1, 1};
        std::vector<Ray> rays;
        rays.reserve(1000000);
        for (long i = 0; i < 1000000; ++i) {
            const auto [u, v] = rng.next2();
            const auto [w, q] = rng.next2();
            // Origins over a box twice the scene's size, so half of them
            // start outside it and some start inside the blocks.
            const Vec3 origin{(u * 2.0 - 0.5) * 0.56,
                              (v * 2.0 - 0.5) * 0.55,
                              (w * 2.0 - 0.5) * 0.56};
            const double z = 2.0 * q - 1.0;
            const double r = std::sqrt(std::fmax(0.0, 1.0 - z * z));
            const auto [phi_u, unused] = rng.next2();
            (void)unused;
            const double phi = si::two_pi * phi_u;
            rays.push_back(Ray{origin, normalize(Vec3{r * std::cos(phi), r * std::sin(phi), z})});
        }
        const auto o = detail::agrees_with_brute_force(scene, rays);
        detail::report("random origins and directions", o);
        all_agree = all_agree && o.disagreed == 0;
    }

    // 2. Rays exactly along an axis. These are the ones a slab test divides
    //    by zero on, and the ones an axis-aligned scene meets exactly.
    {
        std::vector<Ray> rays;
        const Vec3 axes[6] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (int i = 0; i <= 200; ++i) {
            for (int j = 0; j <= 200; ++j) {
                const double a = double(i) / 200.0 * 0.55;
                const double b = double(j) / 200.0 * 0.55;
                for (const Vec3& d : axes)
                    rays.push_back(Ray{Vec3{a, b, 0.2796}, normalize(d)});
            }
        }
        const auto o = detail::agrees_with_brute_force(scene, rays);
        detail::report("exactly axis-aligned, on a grid of origins", o);
        all_agree = all_agree && o.disagreed == 0;
    }

    // 3. Rays that start on the geometry, which is where the offset in
    //    waechter.hpp lives and where a hierarchy is most likely to disagree
    //    with an exhaustive search about what counts as a hit.
    {
        Sampler rng{5, 5};
        std::vector<Ray> rays;
        for (long i = 0; i < 200000; ++i) {
            const auto [u, v] = rng.next2();
            const Vec3 on_floor{u * 0.55, 0.0, v * 0.559};
            const auto [a, b] = rng.next2();
            const double z = 2.0 * a - 1.0;
            const double r = std::sqrt(std::fmax(0.0, 1.0 - z * z));
            const double phi = si::two_pi * b;
            rays.push_back(Ray{on_floor, normalize(Vec3{r * std::cos(phi), r * std::sin(phi), z})});
        }
        const auto o = detail::agrees_with_brute_force(scene, rays);
        detail::report("starting exactly on the floor", o);
        all_agree = all_agree && o.disagreed == 0;
    }

    // 4. Degenerate primitives. A zero-area triangle has no interior for a
    //    ray to hit, and both routes must say so — but a hierarchy also has
    //    to cope with its bounding box, which is a point or a line.
    {
        Scene degenerate = cornell::box();
        const Vec3 p = cornell::at(300.0, 200.0, 300.0);
        const Bsdf grey = GreyLambert{Flat{0.5}};
        degenerate.add(Surface{Triangle{p, p, p}, grey, Flat{0.0}, 0.0});          // a point
        degenerate.add(Surface{Triangle{p, p + Vec3{0.1, 0, 0}, p}, grey, Flat{0.0}, 0.0});  // a line
        degenerate.add(Surface{Triangle{p, p + Vec3{0.1, 0, 0},
                                        p + Vec3{0.2, 0, 0}}, grey, Flat{0.0}, 0.0}); // collinear
        degenerate.finalise();

        Sampler rng{9, 9};
        std::vector<Ray> rays;
        for (long i = 0; i < 200000; ++i) {
            const auto [u, v] = rng.next2();
            const auto [w, q] = rng.next2();
            const Vec3 origin{u * 0.55, v * 0.55, w * 0.55};
            const double z = 2.0 * q - 1.0;
            const double r = std::sqrt(std::fmax(0.0, 1.0 - z * z));
            const auto [phi_u, unused] = rng.next2();
            (void)unused;
            rays.push_back(Ray{origin, normalize(Vec3{r * std::cos(si::two_pi * phi_u),
                                                      r * std::sin(si::two_pi * phi_u), z})});
        }
        const auto o = detail::agrees_with_brute_force(degenerate, rays);
        detail::report("with degenerate and zero-area triangles", o);
        all_agree = all_agree && o.disagreed == 0;
    }

    // ── Threading ────────────────────────────────────────────────────────
    //
    // The claim is that the thread count changes the speed of the render and
    // nothing else. It is true because `sampler.hpp` addresses a stream by
    // (pixel, sample) rather than dispensing from a shared one, and because
    // tiles write to disjoint pixels — but that is an argument, and this is
    // the check.
    //
    // Item 0035 wrote this criterion in v0.2 and could not satisfy it,
    // because there were no threads to count.
    {
        std::printf("\nThe thread count changes the speed and not the answer.\n\n");

        RenderSettings small;
        small.width = 96;
        small.spp = 8;

        const Scene box = cornell::box();
        const Camera camera = Camera::look_at(cornell::at(278.0, 273.0, -800.0),
                                              cornell::at(278.0, 273.0, 0.0),
                                              Vec3{0.0, 1.0, 0.0},
                                              film_width, film_height, film_distance);
        const int height = height_for(small.width);

        small.threads = 1;
        const Film reference = expose(small, box, camera, height);

        for (const int n : {2, 3, 7, 16}) {
            small.threads = n;
            const Film other = expose(small, box, camera, height);

            long differing = 0;
            for (int y = 0; y < height; ++y)
                for (int x = 0; x < small.width; ++x) {
                    const Xyz a = reference.mean_tristimulus(x, y);
                    const Xyz b = other.mean_tristimulus(x, y);
                    if (a.x != b.x || a.y != b.y || a.z != b.z) ++differing;
                }

            char label[64];
            std::snprintf(label, sizeof label, "1 thread against %d, compared exactly", n);
            std::printf("  %-46s %9d pixels   %s\n", label,
                        small.width * height, differing == 0 ? "agree" : "DISAGREE");
            if (differing != 0)
                std::printf("      %ld pixels differ at %d threads\n", differing, n);
            all_agree = all_agree && differing == 0;
        }
    }

    std::printf("\n%s\n", all_agree
        ? "Every claim above holds."
        : "A CLAIM ABOVE DOES NOT HOLD.");
    return all_agree ? 0 : 1;
}

} // namespace app
