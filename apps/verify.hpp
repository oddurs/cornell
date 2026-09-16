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

#include "chi2.hpp"
#include "furnace.hpp"
#include "render.hpp"

namespace app {

namespace detail {

struct Outcome {
    long compared = 0;
    long disagreed = 0;
    long tied = 0;
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

        // The distance is the claim. It must be identical — not close.
        if (fast->t != slow->t) {
            ++out.disagreed;
            out.worst_distance = std::fmax(out.worst_distance, std::fabs(fast->t - slow->t));
            continue;
        }

        // Which primitive, when two are at exactly the same distance, is not.
        //
        // Coincident geometry makes "the nearest" genuinely ambiguous, and
        // the two routes visit primitives in different orders, so they are
        // entitled to break the tie differently. Counted and reported rather
        // than ignored: a tie is a fact about the scene, and a sudden change
        // in how many there are would be worth looking at.
        if (fast->surface != slow->surface) ++out.tied;
    }
    return out;
}

inline void report(const char* what, const Outcome& o) {
    std::printf("  %-46s %9ld rays   %s", what, o.compared,
                o.disagreed == 0 ? "agree" : "DISAGREE");
    if (o.tied != 0)
        std::printf("   (%ld ties on coincident geometry)", o.tied);
    std::printf("\n");
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

    // Without this the headline check can be vacuous. `Scene::intersect`
    // falls back to the exhaustive path for a tree of one node, so a scene
    // whose heuristic declines to split would have this section comparing
    // `intersect_exhaustively` against itself and printing "agree" — which is
    // exactly what the header above says a check must never do.
    if (scene.node_count() <= 1) {
        std::printf("  the box built a tree of %zu node(s), so the comparison below\n"
                    "  would be the exhaustive search against itself. Refusing.\n",
                    scene.node_count());
        return 1;
    }
    std::printf("  (the box builds %zu nodes, so the two routes really differ)\n\n",
                scene.node_count());

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

    // 4. A tree deep enough to overflow a fixed traversal stack.
    //
    //    Exponentially spaced centroids make every split peel off one
    //    primitive, so the tree is a chain rather than a hierarchy. 998 of
    //    them once built a tree of depth 107 against a stack of 64 — every
    //    worker thread writing past the end of its own frame, producing
    //    correct-looking images. The build caps depth now; this is the check
    //    that it still does and that traversal still agrees.
    {
        Scene chain;
        const Bsdf grey = GreyLambert{Flat{0.5}};
        for (int i = 0; i < 998; ++i) {
            const double x = std::ldexp(1.0e-3, i % 60);
            chain.add(Surface{Triangle{Vec3{x, 0.0, 0.0},
                                       Vec3{x * 1.001, 0.0, 0.0},
                                       Vec3{x, 1.0e-3, 0.0}},
                              grey, Flat{0.0}, 0.0});
        }
        chain.finalise();

        Sampler rng{11, 11};
        std::vector<Ray> rays;
        for (long i = 0; i < 200000; ++i) {
            const auto [u, v] = rng.next2();
            const auto [w, q] = rng.next2();
            const double z = 2.0 * q - 1.0;
            const double r = std::sqrt(std::fmax(0.0, 1.0 - z * z));
            rays.push_back(Ray{Vec3{u * 1.0e3, v * 1.0e-3, w * 1.0e-3 - 5.0e-4},
                               normalize(Vec3{r * std::cos(si::two_pi * u),
                                              r * std::sin(si::two_pi * u), z})});
        }
        const auto o = detail::agrees_with_brute_force(chain, rays);
        detail::report("a tree built to be as deep as it can get", o);
        all_agree = all_agree && o.disagreed == 0;
    }

    // 5. Primitives no split can separate. The heuristic declines, and what
    //    it declines into must still reach every one of them.
    {
        Scene pile;
        const Bsdf grey = GreyLambert{Flat{0.5}};
        const Vec3 a = cornell::at(300.0, 200.0, 300.0);
        for (int i = 0; i < 5000; ++i)
            pile.add(Surface{Triangle{a, a + Vec3{0.02, 0, 0}, a + Vec3{0, 0.02, 0}},
                             grey, Flat{0.0}, 0.0});
        pile.finalise();

        std::size_t reachable = 0;
        for (const BvhNode& n : pile.nodes())
            if (n.leaf()) reachable += n.count();

        std::printf("  %-46s %9zu of %zu   %s\n",
                    "every coincident primitive is still in the tree",
                    reachable, pile.surfaces().size(),
                    reachable == pile.surfaces().size() ? "reachable" : "LOST");
        all_agree = all_agree && reachable == pile.surfaces().size();
    }

    // 6. Degenerate primitives. A zero-area triangle has no interior for a
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
        // ── Scale ────────────────────────────────────────────────────────────
    //
    // Radiance is invariant under a uniform scaling of a scene: every length
    // in the transport cancels. So the same room, larger, must produce the
    // same image — and if it does not, something in the ray-spawning has a
    // length hidden in it, which is exactly what `waechter.hpp` claims it
    // does not.
    //
    // The scale factors are powers of two, and that is the whole subtlety.
    // Multiplying a coordinate by 1024 changes its exponent and leaves every
    // mantissa bit alone, so the scaled room is *exactly* the original room,
    // larger. Multiplying by 1000 is a rounding: 0.001 x 552.8 is not one
    // thousandth of 552.8, it is the nearest double to it, so the scaled room
    // is a very slightly different room and a pixel on a silhouette is
    // entitled to land on the other side of an edge.
    //
    // Measured, before this was understood: at 0.001x, 105 of 4096 pixels
    // differed — and at 1024x, 1/1024x, 65536x and 1/65536x, none did. Nine
    // orders of magnitude, bit for bit. The transport has no length in it;
    // powers of ten do.
    //
    // This was a table of prose in `render.hpp`, measured once on a scene
    // that has since been deleted, and quoting a mean radiance the program
    // had stopped printing. It is a check now.
    {
        std::printf("\nRadiance does not depend on how big the room is.\n\n");

        RenderSettings tiny;
        tiny.width = 64;
        tiny.spp = 4;
        const int height = height_for(tiny.width);

        const auto film_at = [&](double scale) {
            const Scene box = cornell::box(scale);
            const Camera camera = Camera::look_at(cornell::at(278.0, 273.0, -800.0, scale),
                                                  cornell::at(278.0, 273.0, 0.0, scale),
                                                  Vec3{0.0, 1.0, 0.0},
                                                  film_width, film_height, film_distance);
            return expose(tiny, box, camera, height);
        };

        const Film reference = film_at(1.0);
        for (const double scale : {1024.0, 1.0 / 1024.0, 65536.0, 1.0 / 65536.0}) {
            const Film other = film_at(scale);
            long differing = 0;
            for (int y = 0; y < height; ++y)
                for (int x = 0; x < tiny.width; ++x) {
                    const Xyz a = reference.mean_tristimulus(x, y);
                    const Xyz b = other.mean_tristimulus(x, y);
                    if (a.x != b.x || a.y != b.y || a.z != b.z) ++differing;
                }
            char label[64];
            std::snprintf(label, sizeof label,
                          "the same room at %gx, compared exactly", scale);
            std::printf("  %-46s %9d pixels   %s\n", label, tiny.width * height,
                        differing == 0 ? "agree" : "DISAGREE");
            all_agree = all_agree && differing == 0;
        }
    }

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

    // ── Every sampler agrees with its own density ────────────────────────
    //
    // The short form of `./cornell chi2`, which prints the whole table and
    // its calibration. What belongs on an inspection sheet is the verdict and
    // the worst p-value, because a sheet with a hundred numbers on it is a
    // sheet nobody reads — and because the criterion for the chi-squared item
    // was that a failure be a build-breaking event, which requires it to be
    // here rather than in an instrument somebody remembers to run.
    {
        using namespace chi2_detail;

        std::printf("\nEvery sampler in the project agrees with the density it claims.\n\n");

        const Bsdf grey{GreyLambert{Flat{0.5}}};
        const Bsdf spectral{SpectralLambert{cie::d65}};
        const Bsdf measured{MeasuredLambert{cornell::red}};

        const std::pair<const char*, const Bsdf*> models[] = {
            {"lambert, grey albedo",     &grey},
            {"lambert, D65 as an albedo", &spectral},
            {"lambert, Cornell's red",   &measured},
        };

        constexpr int draws = 1 << 18;

        for (const auto& [name, bsdf] : models) {
            double lowest = 1.0;
            long impossible = 0;
            int index = 0;

            for (const double degrees : {0.0, 30.0, 60.0, 85.0}) {
                const double theta = degrees * si::pi / 180.0;
                const Vec3 wo{std::sin(theta), 0.0, std::cos(theta)};
                const Result r = test(Dispatch{*bsdf}, wo, draws,
                                      seed_base * std::uint64_t(++index));
                lowest = std::fmin(lowest, r.p);
                impossible += r.impossible;
            }

            const bool ok = lowest > 0.01 && impossible == 0;
            all_agree = all_agree && ok;

            char label[64];
            std::snprintf(label, sizeof label, "%s, four angles", name);
            std::printf("  %-46s %9d draws   %s   worst p = %.3f\n", label, draws * 4,
                        ok ? "agree" : "DISAGREE", lowest);
        }

        // And the calibration, because a test that has never failed is a test
        // nobody has checked. `chi2.hpp` explains what these two are.
        const Vec3 up{0.0, 0.0, 1.0};
        //
        // Four times the draws the rows above use, and that is not padding:
        // the two-percent liar is passed at 2^18 with p = 0.25 and caught at
        // 2^20 with p = 6e-05. `./cornell chi2` prints the whole power curve.
        constexpr int liar_draws = 1 << 20;
        const double flat = test(ClaimsUniform{}, up, liar_draws, seed_liar).p;
        const double nearly = test(ClaimsTwoPer{}, up, liar_draws, seed_liar).p;
        const bool caught = flat <= 0.01 && nearly <= 0.01;
        all_agree = all_agree && caught;

        std::printf("  %-46s %9d draws   %s   p = %.1e, %.1e\n",
                    "two deliberate liars, which must be caught", liar_draws * 2,
                    caught ? "caught" : "ESCAPED", flat, nearly);
    }

    // ── Lambert vanishes in the furnace ──────────────────────────────────
    //
    // Item 0066's claim, on the sheet rather than in an instrument somebody
    // remembers to run. `furnace.hpp` prints the argument and the three ways
    // of asking; what belongs here is the sentence and its verdict.
    //
    // The item asked for a residual below 1e-6 and it is zero, so the check
    // is `== 0.0` rather than a tolerance. A tolerance would be a place for
    // an error to hide that nothing in this scene can produce: the estimator
    // divides the same cosine by itself and `rho/pi` against `1/pi` cancels
    // exactly when rho is 1, so the correct answer is a bit pattern.
    {
        using namespace furnace_detail;

        std::printf("\nA Lambertian of reflectance 1 vanishes in a uniform environment.\n\n");

        constexpr int resolution = 96;
        constexpr int spp = 8;

        const Camera furnace_camera =
            Camera::look_at(Vec3{0, 0, -5}, Vec3{0, 0, 0}, Vec3{0, 1, 0},
                            film_width, film_height, film_distance);

        std::vector<double> picture;

        // Three albedos, all binary fractions so that the comparison can be
        // exact — `furnace.hpp` explains why 0.9 cannot be. At rho = 1 the
        // sphere must be gone; below it, it must be there by exactly 1 - rho,
        // which is what stops the check passing on an empty scene.
        for (const double rho : {1.0, 0.5, 0.0}) {
            const Scene empty_but_for_a_sphere = enclosure::uniform_environment(rho);
            const Residual r = measure(empty_but_for_a_sphere, furnace_camera,
                                       resolution, spp, picture);

            const double expected = 1.0 - rho;
            const bool ok = r.worst == expected && r.on_the_sphere > 0;
            all_agree = all_agree && ok;

            char label[64];
            std::snprintf(label, sizeof label,
                          rho == 1.0 ? "reflectance %.2f, which must leave nothing"
                                     : "reflectance %.2f, which must leave a disc", rho);
            std::printf("  %-46s %9ld pixels   %s   |L-1| = %.3e\n",
                        label, r.on_the_sphere, ok ? "agree" : "DISAGREE", r.worst);
        }

        // And the directional albedo, which is the part of the claim an image
        // cannot show: a BRDF integrated against the cosine over the
        // hemisphere is a reflectance, and for this one it is the albedo.
        // Through `eval` alone, so it is blind to the sampling routine.
        const Bsdf white{GreyLambert{Flat{1.0}}};
        double worst = 0.0;
        for (const double degrees : {0.0, 30.0, 60.0, 85.0}) {
            const double theta = degrees * si::pi / 180.0;
            const Vec3 wo{std::sin(theta), 0.0, std::cos(theta)};
            worst = std::fmax(worst, std::fabs(
                directional_albedo_by_quadrature(white, wo, 256, 256) - 1.0));
        }

        // Not exact, and `furnace.hpp` measures why: the integrand is
        // constant so the midpoint rule is analytically exact, and what is
        // left is the summation of 65,536 doubles.
        const bool integrates = worst < 1e-12;
        all_agree = all_agree && integrates;
        std::printf("  %-46s %9d cells    %s   worst %.3e\n",
                    "its BRDF integrates against the cosine to 1", 256 * 256,
                    integrates ? "agree" : "DISAGREE", worst);
    }

    std::printf("\n%s\n", all_agree
        ? "Every claim above holds."
        : "A CLAIM ABOVE DOES NOT HOLD.");
    return all_agree ? 0 : 1;
}

} // namespace app
