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

#include <chrono>
#include <cmath>
#include <limits>
#include <cstdio>
#include <vector>

#include <render/cornell.hpp>
#include <render/fresnel.hpp>
#include <render/schlick.hpp>
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

    const auto started = std::chrono::steady_clock::now();

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

        // And the one it declines to test, which it has to say rather than
        // quietly pass. A mirror's density is a delta; there is no histogram
        // to compare it against, and a chi-squared that ran anyway would
        // report p = 0 for a perfectly correct surface.
        const Bsdf polished{GreySpecular{FlatReflectance{1.0}}};
        const Result specular = test(Dispatch{polished}, up, draws, seed_liar);
        const bool declined = specular.skipped_delta;
        all_agree = all_agree && declined;

        std::printf("  %-46s %9s        %s\n",
                    "a mirror, whose density is a delta", "n/a",
                    declined ? "skipped, and says so" : "TESTED ANYWAY");
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

        // ── And a mirror, which is the same claim about a delta lobe ─────
        //
        // Item 0079. A perfect mirror of reflectance 1 has to vanish in the
        // furnace exactly as a Lambertian of reflectance 1 does, and it will
        // not if the estimator divided by the delta lobe's zero density, or
        // forgot to divide at all, or took `f` instead of `weight`. Each of
        // those makes the mirror too dark or too bright by a factor that
        // looks like a choice somebody made about how shiny things should be.
        //
        // The convention it is testing is stated once in `bsdf.hpp` and obeyed
        // in `transport.hpp`, `chi2.hpp` and, in v0.8, by Veach's weighting.
        // This is the check that it is obeyed in the one place an error would
        // be invisible.
        for (const double reflectance : {1.0, 0.5, 0.25}) {
            const Scene mirrored = enclosure::uniform_environment(
                Bsdf{GreySpecular{FlatReflectance{reflectance}}});
            const Residual r = measure(mirrored, furnace_camera, resolution, spp, picture);

            const double expected = 1.0 - reflectance;
            const bool ok = r.worst == expected && r.on_the_sphere > 0;
            all_agree = all_agree && ok;

            char label[72];
            std::snprintf(label, sizeof label,
                          reflectance == 1.0
                              ? "a mirror of reflectance %.2f, which must leave nothing"
                              : "a mirror of reflectance %.2f, which must leave a disc",
                          reflectance);
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

    // ── Every density integrates to one ──────────────────────────────────
    //
    // Item 0067. A density function that does not integrate to one is not a
    // density function, and the image it produces is uniformly too bright or
    // too dark by a constant factor — which is exactly the error that gets
    // compensated for by turning the light down until it looks right, and
    // then never found.
    //
    // It was written in the belief that `./cornell chi2` could not see this,
    // on the textbook reasoning that Pearson's statistic compares shapes. That
    // is true of a chi-squared that renormalises its expectations to the
    // observed total, and this project's does not: the expected count is the
    // density integrated over the bin times the number of draws, so a missing
    // factor of two is a chi2/dof of 515 rather than a perfect fit. The
    // blindness-matrix section at the end of this sheet is what found that
    // out, by running every liar through every check instead of each one
    // through its own.
    //
    // This section is still the one worth having for it, for two reasons that
    // survive the correction. It is exact rather than statistical — a
    // deviation of -5.000e-01 rather than a p-value — and it needs no sampler
    // at all, which is the only form available in v0.8, where multiple
    // importance sampling evaluates one strategy's density on directions
    // another strategy produced.
    //
    // Each is integrated over its own domain by a midpoint rule that does not
    // call the sampling routine it belongs to. And the domains are not the
    // same domain, which is the point `density.hpp` exists to make: two of
    // the rows below are per steradian and one is per metre of wavelength,
    // and nothing in this project may divide one into the other.
    {
        using namespace chi2_detail;

        std::printf("\nEvery density in the project integrates to one over its own domain.\n\n");

        // Over the sphere: dw = dmu dphi, so a midpoint grid in (mu, phi) is
        // a plain double integral with no Jacobian left to get wrong.
        const auto over_the_sphere = [](const auto& model, const Vec3& wo, int cells) {
            const double mu_width = 2.0 / double(cells);
            const double phi_width = si::two_pi / double(cells);
            double total = 0.0;

            for (int i = 0; i < cells; ++i) {
                const double mu = -1.0 + mu_width * (double(i) + 0.5);
                const double sin_theta = std::sqrt(std::fmax(0.0, 1.0 - mu * mu));
                for (int j = 0; j < cells; ++j) {
                    const double phi = phi_width * (double(j) + 0.5);
                    const Vec3 wi{sin_theta * std::cos(phi), sin_theta * std::sin(phi), mu};
                    total += model.pdf(wo, wi).per_steradian();
                }
            }
            return total * mu_width * phi_width;
        };

        constexpr int cells = 1024;
        const Vec3 wo{std::sin(0.6), 0.0, std::cos(0.6)};

        const Bsdf grey{GreyLambert{Flat{0.5}}};
        const Bsdf spectral{SpectralLambert{cie::d65}};
        const Bsdf measured{MeasuredLambert{cornell::red}};

        const std::pair<const char*, const Bsdf*> models[] = {
            {"lambert, grey albedo",      &grey},
            {"lambert, D65 as an albedo", &spectral},
            {"lambert, Cornell's red",    &measured},
        };

        for (const auto& [name, bsdf] : models) {
            const double total = over_the_sphere(Dispatch{*bsdf}, wo, cells);
            const bool ok = std::fabs(total - 1.0) < 1e-12;
            all_agree = all_agree && ok;

            char label[64];
            std::snprintf(label, sizeof label, "%s, over the sphere", name);
            std::printf("  %-46s %9d cells    %s   %+.3e\n", label, cells * cells,
                        ok ? "agree" : "DISAGREE", total - 1.0);
        }

        // The warp underneath them, asked directly rather than through a
        // material. It is the same arithmetic and a different caller, and the
        // day a second BSDF stops using it this row is what stays honest.
        {
            const double mu_width = 1.0 / double(cells);
            const double phi_width = si::two_pi / double(cells);
            double total = 0.0;
            for (int i = 0; i < cells; ++i) {
                const double mu = mu_width * (double(i) + 0.5);
                for (int j = 0; j < cells; ++j)
                    total += cosine_hemisphere_pdf(mu).per_steradian();
            }
            total *= mu_width * phi_width;

            const bool ok = std::fabs(total - 1.0) < 1e-12;
            all_agree = all_agree && ok;
            std::printf("  %-46s %9d cells    %s   %+.3e\n",
                        "warp.hpp's cosine hemisphere, over the hemisphere", cells * cells,
                        ok ? "agree" : "DISAGREE", total - 1.0);
        }

        // And the one that is not a density over directions at all. The hero
        // wavelength draw is uniform over the visible range and its density
        // is per *metre*, which is why `density.hpp` makes the type say which
        // — two densities in one program, both spelled `pdf`, differing by
        // sr·m⁻¹.
        {
            const double width = (si::lambda_max - si::lambda_min) / double(cells);
            double total = 0.0;
            for (int i = 0; i < cells; ++i) total += Wavelengths::pdf();
            total *= width;

            const bool ok = std::fabs(total - 1.0) < 1e-12;
            all_agree = all_agree && ok;
            std::printf("  %-46s %9d cells    %s   %+.3e\n",
                        "the hero wavelength draw, over 360-830 nm", cells,
                        ok ? "agree" : "DISAGREE", total - 1.0);
        }

        // The calibration. An unnormalised density is the failure this
        // section exists for, and it is invisible to every other check on
        // this sheet: it conserves energy, it agrees with its own sampler,
        // and it renders an image that is uniformly wrong.
        {
            const double total = over_the_sphere(HalfADensity{}, wo, cells);
            const bool caught = std::fabs(total - 1.0) > 1e-3;
            all_agree = all_agree && caught;
            std::printf("  %-46s %9d cells    %s   %+.3e\n",
                        "a density that is half of one, which must fail", cells * cells,
                        caught ? "caught" : "ESCAPED", total - 1.0);
        }

        // The exception, which is now in the project and has to be named
        // rather than left out of the loop above. A mirror's density is a
        // delta: it integrates to 1 over its domain in the sense that matters
        // and to *zero* over any quadrature, because the one direction it
        // occupies has no width. The claim in this section's heading is about
        // densities that are functions, and this is the one that is not.
        {
            const Bsdf polished{GreySpecular{FlatReflectance{1.0}}};
            const double total = over_the_sphere(Dispatch{polished}, wo, cells);
            const bool as_expected = total == 0.0;
            all_agree = all_agree && as_expected;
            std::printf("  %-46s %9d cells    %s   %+.3e\n",
                        "a mirror, which integrates to nothing and should", cells * cells,
                        as_expected ? "delta" : "NOT A DELTA", total);
        }

        std::printf("\n  The last row is not a failure. `bsdf.hpp`'s convention has a delta\n"
                    "  lobe return zero from `pdf`, so a quadrature of it is zero — which\n"
                    "  is the correct answer to the wrong question, and the reason the\n"
                    "  convention returns zero rather than something plausible.\n");

        std::printf("\n  The residuals are summation rather than quadrature: every integrand\n"
                    "  here is exactly integrated by a midpoint rule, so what is left is a\n"
                    "  million doubles added end to end. The tolerance is 1e-12 and the\n"
                    "  largest is 1.2e-13.\n"
                    "\n  There is no light-sampling density yet, so there is no row for the\n"
                    "  surface of a lamp. It arrives with v0.8, and so does this row.\n");
    }

    // ── Reciprocity ──────────────────────────────────────────────────────
    //
    // Item 0068. Helmholtz reciprocity: light does not care which end of a
    // path it started from, so
    //
    //      f(wo, wi) == f(wi, wo)
    //
    // for every BSDF in the project. `bsdf.hpp` chose the physicists'
    // convention — both directions point away from the surface — precisely so
    // that this is a symmetry in the code rather than a fact to remember.
    //
    // It matters later rather than now. A non-reciprocal BSDF breaks
    // bidirectional path tracing in v1.4 in ways that are close to
    // undiagnosable from an image: a path built from the eye and the same
    // path built from the light disagree about their own weight, and the
    // result is an image that is subtly wrong in a way nothing points at.
    //
    // ── The exception, stated now rather than excused later ──────────────
    //
    // Refraction across an interface between media of different index is
    // *not* reciprocal in radiance. It carries a factor of the squared index
    // ratio, for the good reason that radiance itself is not conserved across
    // such a boundary — a beam entering glass is compressed into a smaller
    // solid angle and its radiance rises by n^2, which is why a fish looks
    // closer than it is and why the bottom of a swimming pool does not.
    //
    // That is real physics and it arrives in v0.9. When it does, this check
    // must learn about it rather than be relaxed: what stays exactly true is
    // `f(wo, wi) / n_o^2 == f(wi, wo) / n_i^2`, and a reflector is the case
    // where the two indices are the same one.
    {
        using namespace chi2_detail;

        std::printf("\nEvery BSDF returns the same value with its arguments swapped.\n\n");

        const Wavelengths lambdas = Wavelengths::sample(0.5);
        const Bsdf grey{GreyLambert{Flat{0.5}}};
        const Bsdf spectral{SpectralLambert{cie::d65}};
        const Bsdf measured{MeasuredLambert{cornell::red}};

        constexpr int pairs = 1 << 20;

        // Directions over the whole sphere, so that the pairs include ones on
        // opposite sides of the surface. A reflector returns zero for those,
        // and zero on both sides is a claim worth checking: a hemisphere test
        // written with the wrong comparison is how light leaks through a wall.
        const auto sweep = [&](const auto& model) {
            long compared = 0, differing = 0, both_zero = 0;
            double worst = 0.0;

            for (int i = 0; i < pairs; ++i) {
                Sampler sampler{0x51ed'2701'a9e3'31b7, std::uint64_t(i)};
                const auto [u1, v1] = sampler.next2();
                const auto [u2, v2] = sampler.next2();

                const auto on_the_sphere = [](double u, double v) {
                    const double mu = 2.0 * u - 1.0;
                    const double r = std::sqrt(std::fmax(0.0, 1.0 - mu * mu));
                    const double phi = si::two_pi * v;
                    return Vec3{r * std::cos(phi), r * std::sin(phi), mu};
                };

                const Vec3 a = on_the_sphere(u1, v1);
                const Vec3 b = on_the_sphere(u2, v2);

                const Brdf forward = model.eval(a, b, lambdas);
                const Brdf backward = model.eval(b, a, lambdas);
                ++compared;

                if (forward.is_black() && backward.is_black()) { ++both_zero; continue; }

                for (int k = 0; k < spectral_samples; ++k) {
                    if (forward[k] != backward[k]) {
                        ++differing;
                        worst = std::fmax(worst, std::fabs(forward[k] - backward[k]));
                        break;
                    }
                }
            }
            return std::tuple{compared, differing, both_zero, worst};
        };

        const std::pair<const char*, const Bsdf*> models[] = {
            {"lambert, grey albedo",      &grey},
            {"lambert, D65 as an albedo", &spectral},
            {"lambert, Cornell's red",    &measured},
        };

        for (const auto& [name, bsdf] : models) {
            const auto [compared, differing, both_zero, worst] = sweep(Dispatch{*bsdf});
            const bool ok = differing == 0 && both_zero > 0 && both_zero < compared;
            all_agree = all_agree && ok;

            std::printf("  %-46s %9ld pairs   %s   (%ld across the surface)\n",
                        name, compared, ok ? "agree" : "DISAGREE", both_zero);
            if (differing != 0)
                std::printf("      %ld pairs differ, worst %.3e\n", differing, worst);
        }

        // Compared exactly, and it can be: a Lambertian's BRDF does not
        // depend on either direction once both are on the same side, so the
        // two calls return the same double rather than nearly the same one.
        // A tolerance here would be a tolerance for an error that cannot
        // exist yet and would hide one that can.
        const auto [compared, differing, both_zero, worst] = sweep(NotReciprocal{});
        const bool caught = differing > 0;
        all_agree = all_agree && caught;
        (void)both_zero;
        std::printf("  %-46s %9ld pairs   %s   worst %.3e\n",
                    "one that weights only wi, which must fail", compared,
                    caught ? "caught" : "ESCAPED", worst);

        std::printf("\n  Compared exactly. Refraction is the legitimate exception and does\n"
                    "  not exist yet: crossing into a medium of different index multiplies\n"
                    "  radiance by the squared index ratio, so what stays true there is\n"
                    "  f(wo,wi)/n_o^2 == f(wi,wo)/n_i^2. v0.9 teaches this check that,\n"
                    "  rather than relaxing it when it starts failing.\n");
    }

    // ── No NaN survives to the film ──────────────────────────────────────
    //
    // Item 0069. One non-finite value in an accumulation buffer poisons that
    // pixel for the rest of the render, and it propagates silently through
    // every operation that touches it: the sum is a NaN, the mean is a NaN,
    // and the image has a hole in it that says nothing about where it came
    // from. `film.hpp` asserts against it at the point of deposit and names
    // the `(pixel, sample)` that produced it, which `./cornell replay` turns
    // back into one path on one thread.
    //
    // This is the other half: the assertion fires on the way in, and this
    // scans what came out. An assertion only speaks when it is violated, so
    // a sheet that relies on it alone cannot tell "no NaN occurred" from "the
    // assertion was compiled out".
    {
        std::printf("\nNo non-finite value reaches the film.\n\n");

        const auto scan = [](const Film& film) {
            long bad = 0, checked = 0;
            for (int y = 0; y < film.height(); ++y)
                for (int x = 0; x < film.width(); ++x) {
                    const Xyz value = film.mean_tristimulus(x, y);
                    checked += 3;
                    if (!std::isfinite(value.x) || !std::isfinite(value.y)
                        || !std::isfinite(value.z)) ++bad;

                    for (int bin = 0; bin < film_bins; ++bin) {
                        ++checked;
                        if (!std::isfinite(film.mean_radiance(x, y, bin))) ++bad;
                    }
                }
            return std::pair{checked, bad};
        };

        RenderSettings scanned;
        scanned.width = 96;
        scanned.spp = 16;
        const int scanned_height = height_for(scanned.width);
        const Camera box_camera = Camera::look_at(cornell::at(278.0, 273.0, -800.0),
                                                  cornell::at(278.0, 273.0, 0.0),
                                                  Vec3{0.0, 1.0, 0.0},
                                                  film_width, film_height, film_distance);

        const auto [checked, bad] =
            scan(expose(scanned, cornell::box(), box_camera, scanned_height));
        all_agree = all_agree && bad == 0;
        std::printf("  %-46s %9ld values   %s\n",
                    "the box, every bin and every tristimulus", checked,
                    bad == 0 ? "all finite" : "NOT FINITE");

        // The predicate itself, because `x != x` catches a NaN and lets an
        // infinity through, and an infinity poisons a running sum just as
        // thoroughly. `film.hpp` uses `std::isfinite` for exactly that.
        Radiance poisoned;
        poisoned[2] = std::nan("");
        Radiance unbounded;
        unbounded[1] = std::numeric_limits<double>::infinity();

        const bool predicate = !finite(poisoned) && !finite(unbounded)
                            && finite(Radiance{1.0});
        all_agree = all_agree && predicate;
        std::printf("  %-46s %9d values   %s\n",
                    "a NaN and an infinity, which must both fail", 3,
                    predicate ? "caught" : "ESCAPED");

        // And the one NaN this project knows how to make, which turns out not
        // to reach the film at all.
        {
            const Camera degenerate =
                Camera::look_at(Vec3{0, 0, -1}, Vec3{0, 0, 0}, Vec3{0, 0, 1},
                                film_width, film_height, film_distance);
            const Ray ray = degenerate.ray_through(0.5, 0.5);

            Sampler sampler{0, 0};
            const Wavelengths lambdas = Wavelengths::sample(0.5);
            const Radiance carried = radiance(cornell::box(), ray, lambdas, sampler);

            const bool direction_is_nan = !std::isfinite(ray.direction.x());
            const bool result_is_finite = finite(carried) && carried.is_black();
            const bool as_described = direction_is_nan && result_is_finite;
            all_agree = all_agree && as_described;

            std::printf("  %-46s %9d path     %s\n",
                        "a camera with no idea which way is up", 1,
                        as_described ? "black, not NaN" : "NOT AS DESCRIBED");
        }

        std::printf("\n  The last row is a correction. `camera.hpp` says a degenerate `up`\n"
                    "  hint produces a NaN direction left to propagate rather than be\n"
                    "  papered over, and it does — into the ray. It stops there. Every\n"
                    "  comparison against a NaN is false, so the intersection finds\n"
                    "  nothing, the path escapes, and what arrives at the film is a\n"
                    "  perfectly finite zero. The failure is a black image rather than a\n"
                    "  poisoned one, and no assertion at the film can catch it.\n");
    }

    // ── What each instrument is blind to ─────────────────────────────────
    //
    // Item 0071. The roadmap's most deliberate feature is that the furnace,
    // the chi-squared test and the convergence plot are built at v0.5 and the
    // microfacet model that will fail all three arrives at v0.7. A check
    // written after the thing it checks is a check written to pass.
    //
    // The other half of that argument is the one it is easy to skip: the
    // instruments test what they test and no more, and saying so is the
    // difference between a project that is careful and a project that claims
    // to be. So rather than four sections each asserting that its own liar
    // was caught, every liar is run through every check, and the misses are
    // printed beside the catches.
    //
    // The result is a matrix with no row and no column that could be removed,
    // which is a stronger statement than any of the sections above makes on
    // its own — and it is measured here rather than asserted in a README,
    // because a claim about what a test cannot see is exactly the kind that
    // rots when the test changes.
    {
        using namespace chi2_detail;
        using namespace furnace_detail;

        std::printf("\nNo instrument catches everything, and none of them is redundant.\n\n");

        const Vec3 wo{std::sin(0.6), 0.0, std::cos(0.6)};
        const Wavelengths lambdas = Wavelengths::sample(0.5);
        // A million, because the power curve in `./cornell chi2` says a two
        // percent error needs about that many and is passed at a quarter of
        // it. A matrix taken at 2^18 shows that row escaping every column,
        // which would be a true statement about a weaker test than this one.
        constexpr int draws = 1 << 20;
        constexpr int cells = 256;

        // Each column asks its own question of a model, and answers whether
        // that model was caught by it.
        const auto conserves_energy = [&](const auto& model) {
            return std::fabs(directional_albedo_by_quadrature(model, wo, cells, cells) - 1.0)
                   < 1e-9;
        };

        const auto agrees_with_its_density = [&](const auto& model) {
            return test(model, wo, draws, seed_liar).p > 0.01;
        };

        const auto is_a_density = [&](const auto& model) {
            const double mu_width = 2.0 / double(cells);
            const double phi_width = si::two_pi / double(cells);
            double total = 0.0;
            for (int i = 0; i < cells; ++i) {
                const double mu = -1.0 + mu_width * (double(i) + 0.5);
                const double sin_theta = std::sqrt(std::fmax(0.0, 1.0 - mu * mu));
                for (int j = 0; j < cells; ++j) {
                    const double phi = phi_width * (double(j) + 0.5);
                    total += model.pdf(wo, Vec3{sin_theta * std::cos(phi),
                                                sin_theta * std::sin(phi), mu})
                                  .per_steradian();
                }
            }
            return std::fabs(total * mu_width * phi_width - 1.0) < 1e-6;
        };

        const auto is_reciprocal = [&](const auto& model) {
            for (int i = 0; i < 4096; ++i) {
                Sampler sampler{0x51ed'2701'a9e3'31b7, std::uint64_t(i)};
                const auto [u1, v1] = sampler.next2();
                const auto [u2, v2] = sampler.next2();

                const auto on_the_sphere = [](double u, double v) {
                    const double mu = 2.0 * u - 1.0;
                    const double r = std::sqrt(std::fmax(0.0, 1.0 - mu * mu));
                    return Vec3{r * std::cos(si::two_pi * v), r * std::sin(si::two_pi * v), mu};
                };

                const Vec3 a = on_the_sphere(u1, v1);
                const Vec3 b = on_the_sphere(u2, v2);
                const Brdf forward = model.eval(a, b, lambdas);
                const Brdf backward = model.eval(b, a, lambdas);
                for (int k = 0; k < spectral_samples; ++k)
                    if (forward[k] != backward[k]) return false;
            }
            return true;
        };

        std::printf("  %-34s %8s %8s %8s %8s\n",
                    "deliberately wrong model", "furnace", "chi2", "density", "swapped");

        int caught_total = 0;
        const auto row = [&](const char* name, const auto& model) {
            const bool furnace_misses = conserves_energy(model);
            const bool chi2_misses = agrees_with_its_density(model);
            const bool density_misses = is_a_density(model);
            const bool swap_misses = is_reciprocal(model);

            const int caught = int(!furnace_misses) + int(!chi2_misses)
                             + int(!density_misses) + int(!swap_misses);
            caught_total += caught;
            all_agree = all_agree && caught > 0;

            std::printf("  %-34s %8s %8s %8s %8s\n", name,
                        furnace_misses ? "-" : "caught", chi2_misses ? "-" : "caught",
                        density_misses ? "-" : "caught", swap_misses ? "-" : "caught");
        };

        row("claims a flat density",        ClaimsUniform{});
        row("claims one 2% too steep",      ClaimsTwoPer{});
        row("a density that is half of one", HalfADensity{});
        row("weights only the incoming ray", NotReciprocal{});

        std::printf("\n  Four models, each wrong in one way, none caught by every column.\n"
                    "  The furnace and the swap each catch something nothing else does,\n"
                    "  and the chi-squared catches two the furnace cannot see at all: a\n"
                    "  density can be wrong by two percent and conserve energy exactly.\n"
                    "\n  The density column is the honest exception, and it is worth saying\n"
                    "  rather than arranging a fifth liar to justify it. Nothing here is\n"
                    "  caught by it alone. A sampler that draws from a normalised\n"
                    "  distribution and reports an unnormalised density is the row above,\n"
                    "  and the chi-squared sees it because this implementation compares\n"
                    "  against the density times the draw count rather than renormalising\n"
                    "  to the observed total — so a missing factor of two is a chi2/dof of\n"
                    "  515 rather than a perfect fit. A density that is unnormalised and\n"
                    "  whose sampler agrees with it cannot exist, because the missing half\n"
                    "  has to go somewhere.\n"
                    "\n  What the column is for arrives in v0.8. Multiple importance\n"
                    "  sampling evaluates one strategy's density on another strategy's\n"
                    "  directions, which is a use of a pdf with no sampler of its own\n"
                    "  attached — and there a chi-squared has nothing to compare. It is\n"
                    "  also exact where the chi-squared is statistical: -5.000e-01 against\n"
                    "  a p-value.\n");
    }

    // Item 0065 asked for this to run in under a minute or to grow a
    // `--quick` that says what it skipped. It runs in about two seconds, so
    // there is nothing to skip and no flag to explain — and the figure is
    // printed rather than claimed, because the day it stops being true is the
    // day somebody adds a check that renders at 4096 samples and nobody
    // notices until CI does.
    const double seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();

    // ── Fresnel, and the things that fall out of it ──────────────────────
    //
    // Items 0072 and 0077. `fresnel.hpp` cannot pin its own constants with a
    // `static_assert` — an arctangent is not `constexpr` in libc++ and is in
    // libstdc++, so an assert there would compile on one of this project's
    // two compilers — so its claims are here, where they can be stronger than
    // an assert anyway.
    //
    // The important one is Brewster's angle. The file *derives* the formula
    // `tan(theta_B) = eta_t / eta_i` from the equations above it, and
    // evaluating that formula and comparing it with itself would check
    // nothing. So the angle is found by searching the reflectance curve for
    // the minimum of the parallel polarisation, which is a different
    // question, and the formula is what it is compared against.
    {
        std::printf("\nFresnel, and the three things nobody wrote.\n\n");

        // Where `r_p` is smallest, found rather than computed. A ten
        // thousandth of a degree, which is finer than the agreement being
        // claimed and coarse enough to finish.
        const auto smallest_parallel = [](const Index& eta_t) {
            double best = 2.0, at = 0.0;
            for (int i = 0; i <= 900'000; ++i) {
                const double degrees = double(i) / 10'000.0;
                const double parallel = fresnel(std::cos(degrees * si::pi / 180.0), eta_t).p;
                if (parallel < best) { best = parallel; at = degrees; }
            }
            return std::pair{at, best};
        };

        for (const double index : {1.33, 1.5, 2.417}) {
            const auto [found, value] = smallest_parallel(Index{index, 0.0});
            const double derived = si::as::deg(brewster_angle(index_of_air, index));
            const bool ok = std::fabs(found - derived) < 1e-3 && value < 1e-9;
            all_agree = all_agree && ok;

            char label[80];
            std::snprintf(label, sizeof label,
                          "Brewster at n = %.3f: searched %.4f, arctan %.4f",
                          index, found, derived);
            std::printf("  %-58s %s\n", label, ok ? "agree" : "DISAGREE");
        }

        std::printf("      water, window glass, diamond. The middle one is 56.31 degrees\n"
                    "      and is why polarising sunglasses work on a wet road.\n\n");

        // Past the critical angle, glass to air, the reflectance must be 1 —
        // and it is 1 exactly, which is the reason `fresnel.hpp` squares the
        // numerator and denominator separately instead of dividing first.
        // Dividing first rounds twice and returns 1.0000000000000002 at 89
        // degrees, which is a surface that reflects more light than reaches
        // it.
        {
            const Index glass{1.5, 0.0};
            const Index air{index_of_air, 0.0};
            long compared = 0, exactly_one = 0, above_one = 0;
            double worst = 0.0;
            const double critical = si::as::deg(critical_angle(1.5, index_of_air));

            for (int i = 0; i <= 90'000; ++i) {
                const double degrees = double(i) / 1000.0;
                if (degrees <= critical + 0.05) continue;
                const Reflected r = fresnel(std::cos(degrees * si::pi / 180.0), glass, air);
                ++compared;
                if (r.s == 1.0 && r.p == 1.0) ++exactly_one;
                if (r.s > 1.0 || r.p > 1.0) ++above_one;
                worst = std::fmax(worst, std::fmax(std::fabs(r.s - 1.0),
                                                   std::fabs(r.p - 1.0)));
            }

            const double ulp = std::nextafter(1.0, 2.0) - 1.0;
            const bool ok = above_one == 0 && worst <= 2.0 * ulp;
            all_agree = all_agree && ok;
            std::printf("  %-58s %9ld angles   %s   %.0f%% exact, worst %.1f ulp\n",
                        "past the critical angle, glass to air, R is 1",
                        compared, ok ? "agree" : "DISAGREE",
                        100.0 * double(exactly_one) / double(compared), worst / ulp);
        }

        // The general function against the closed form it collapses to at
        // normal incidence, which is where a metal's reflectance is usually
        // quoted from. Two routes to one number, which is the only kind of
        // check a derivation can have.
        {
            double worst = 0.0;
            const Index indices[] = {Index{1.33, 0.0}, Index{1.5, 0.0}, Index{2.417, 0.0},
                                     Index{0.2, 3.0}, Index{1.1, 7.0}, Index{0.05, 4.2}};
            for (const Index& eta : indices)
                worst = std::fmax(worst,
                                  std::fabs(fresnel(1.0, eta).unpolarised()
                                            - normal_incidence(eta)));

            const bool ok = worst < 1e-15;
            all_agree = all_agree && ok;
            std::printf("  %-58s %9zu indices  %s   worst %.1e\n",
                        "at normal incidence the general form meets the closed one",
                        std::size(indices), ok ? "agree" : "DISAGREE", worst);
        }

        // Energy. A reflectance above 1 is a surface that emits.
        //
        // Which is only what `|r|^2` means when the medium the light arrives
        // *through* does not absorb. The quantity being squared is an
        // amplitude ratio; turning it into a ratio of energy flux needs the
        // real part of a Poynting vector, and those two agree exactly when
        // the incident index is real and do not when it is complex. So the
        // sweep below crosses every boundary in the direction light actually
        // travels here — out of a transparent medium — and the reverse only
        // for the transparent ones, where total internal reflection is the
        // thing being checked.
        //
        // That distinction was found by CI rather than by thinking. The first
        // version swept both directions for all six indices; it passed under
        // clang, and under g++-14 a conductor-to-air case at 90 degrees came
        // back a few ulp above 1. Both compilers were right. The check was
        // asking a question that has no answer.
        {
            double worst = 0.0;
            long compared = 0;
            const Index transparent[] = {Index{1.33, 0.0}, Index{1.5, 0.0},
                                         Index{2.417, 0.0}};
            const Index absorbing[] = {Index{0.2, 3.0}, Index{1.1, 7.0},
                                       Index{0.05, 4.2}};

            for (int i = 0; i <= 90'000; ++i) {
                const double cosine = std::cos(double(i) / 1000.0 * si::pi / 180.0);

                for (const Index& eta : transparent) {
                    const Reflected into = fresnel(cosine, eta);
                    const Reflected out = fresnel(cosine, eta, Index{index_of_air, 0.0});
                    worst = std::fmax(worst, std::fmax(into.s, into.p));
                    worst = std::fmax(worst, std::fmax(out.s, out.p));
                    compared += 2;
                }
                for (const Index& eta : absorbing) {
                    const Reflected into = fresnel(cosine, eta);
                    worst = std::fmax(worst, std::fmax(into.s, into.p));
                    ++compared;
                }
            }

            const bool ok = worst <= 1.0;
            all_agree = all_agree && ok;
            std::printf("  %-58s %9ld pairs    %s   largest %.17g\n",
                        "no reflectance exceeds 1, arriving through anything clear",
                        compared, ok ? "agree" : "DISAGREE", worst);
        }

        // Grazing incidence: everything becomes a mirror. Checked as a limit
        // rather than at a point, because the claim is that it approaches 1
        // and not that it reaches it.
        {
            bool rising = true;
            double last_s = 0.0, last_p = 0.0, at_grazing = 0.0;
            for (int i = 0; i <= 8999; ++i) {
                const double degrees = 80.0 + double(i) / 900.0;
                const Reflected r = fresnel(std::cos(degrees * si::pi / 180.0),
                                            Index{1.5, 0.0});
                rising = rising && r.s >= last_s && r.p >= last_p;
                last_s = r.s;
                last_p = r.p;
                at_grazing = r.unpolarised();
            }

            const bool ok = rising && at_grazing > 0.999;
            all_agree = all_agree && ok;
            std::printf("  %-58s %9s        %s   R = %.5f\n",
                        "from 80 to 90 degrees both polarisations rise to 1", "glass",
                        ok ? "agree" : "DISAGREE", at_grazing);
        }

        // And the one the complex case gives for free: a conductor has no
        // Brewster angle at all. It has a pseudo-Brewster angle, where `r_p`
        // is least rather than zero, and reporting the depth of that minimum
        // is the difference between the two phenomena.
        {
            const auto [at, value] = smallest_parallel(Index{0.2, 3.0});
            const bool ok = value > 0.5;
            all_agree = all_agree && ok;

            char label[80];
            std::snprintf(label, sizeof label,
                          "a conductor's r_p bottoms at %.2f degrees, not zero", at);
            std::printf("  %-58s %9s        %s   R = %.5f\n", label, "n=0.2 k=3",
                        ok ? "agree" : "DISAGREE", value);
        }

        // ── And the fit next door ────────────────────────────────────────
        //
        // Item 0076. `schlick.hpp` quotes its own error against the exact
        // equations, and the figures in that comment come from here, so that
        // the file cannot go on claiming an accuracy it stopped having.
        //
        // Nothing else in the project calls Schlick. This is its only caller,
        // and what it does with it is measure how wrong it is.
        {
            std::printf("\n  The fit next door, against the law. Unpolarised, 90,001 angles:\n\n"
                        "    %-22s %9s %11s %11s %10s\n",
                        "index", "R0", "max error", "mean error", "worst at");

            const auto compare = [](const Index& eta) {
                const double r0 = schlick::r0_of(eta);
                double worst = 0.0, at = 0.0, total = 0.0;
                long angles = 0;

                for (int i = 0; i <= 90'000; ++i) {
                    const double degrees = double(i) / 1000.0;
                    const double cosine = std::cos(degrees * si::pi / 180.0);
                    const double error = std::fabs(schlick::reflectance(r0, cosine)
                                                   - fresnel(cosine, eta).unpolarised());
                    if (error > worst) { worst = error; at = degrees; }
                    total += error;
                    ++angles;
                }
                return std::tuple{r0, worst, total / double(angles), at};
            };

            struct Row { const char* name; Index eta; double allowed; };
            // The bound beside each is the figure quoted in `schlick.hpp`,
            // rounded up. It is not a tolerance on a physical claim — the
            // error is whatever it is — it is the mechanism that stops that
            // comment drifting away from this measurement.
            const Row rows[] = {
                {"water,        1.330", Index{1.330, 0.0}, 0.0600},
                {"window glass, 1.500", Index{1.500, 0.0}, 0.0357},
                {"diamond,      2.417", Index{2.417, 0.0}, 0.0755},
                {"n=0.2  k=3.0",        Index{0.20, 3.0},  0.0155},
                {"n=1.1  k=7.0",        Index{1.10, 7.0},  0.1012},
                {"n=0.05 k=4.2",        Index{0.05, 4.2},  0.0061},
            };

            for (const Row& row : rows) {
                const auto [r0, worst, mean, at] = compare(row.eta);
                const bool ok = worst < row.allowed;
                all_agree = all_agree && ok;
                std::printf("    %-22s %9.5f %11.5f %11.5f %7.1f deg  %s\n",
                            row.name, r0, worst, mean, at, ok ? "" : "WORSE THAN QUOTED");
            }

            std::printf("\n    About one percent on average and three to eight at the worst,\n"
                        "    from one line, which is why it won. Every maximum is at 84 or\n"
                        "    85 degrees: the fit is pinned at both ends by construction and\n"
                        "    the last few degrees before grazing are where it has the most\n"
                        "    room to be wrong.\n"
                        "\n    The complex rows are the ones worth reading. The error is not\n"
                        "    a multiple of the dielectric error — it is 0.006 at one index\n"
                        "    and 0.101 at another, better and worse than glass — because a\n"
                        "    conductor's curve is not the shape Schlick fitted and how badly\n"
                        "    that shows depends on the index. There is no factor to apply.\n");
        }

        // ── What averaging the two polarisations costs ───────────────────
        //
        // Item 0081. `fresnel.hpp` returns two reflectances and this renderer
        // carries one number, so it averages them. That is exact for
        // unpolarised light meeting a surface for the first time and wrong
        // afterwards, because reflection polarises what it reflects.
        //
        // The size of the error is not an estimate. Two bounces, tracking the
        // states through both and averaging once, against averaging after
        // each — the difference is the correlation the model threw away.
        {
            std::printf("\n  What averaging the two polarisations costs. Unpolarised light,\n"
                        "  two bounces off glass, same plane of incidence:\n\n"
                        "    %8s %8s %13s %13s %8s\n",
                        "first", "second", "tracked", "averaged", "ratio");

            const Index glass{1.5, 0.0};
            const auto at = [&](double degrees) {
                return fresnel(std::cos(degrees * si::pi / 180.0), glass);
            };

            const double brewster = si::as::deg(brewster_angle(index_of_air, 1.5));
            const double pairs[][2] = {{0.0, 0.0}, {45.0, 45.0}, {brewster, brewster},
                                       {70.0, 70.0}, {85.0, 85.0}};

            double worst_ratio = 1.0;
            for (const auto& pair : pairs) {
                const Reflected first = at(pair[0]);
                const Reflected second = at(pair[1]);

                const double tracked = 0.5 * (first.s * second.s + first.p * second.p);
                const double averaged = first.unpolarised() * second.unpolarised();
                const double ratio = averaged / tracked;
                worst_ratio = std::fmin(worst_ratio, ratio);

                std::printf("    %8.2f %8.2f %13.8f %13.8f %8.4f\n",
                            pair[0], pair[1], tracked, averaged, ratio);
            }

            // Exactly a half at two Brewster bounces, and that is the worst
            // it gets in this plane. Asserted, because it is a consequence of
            // r_p being exactly zero there and not a measurement that could
            // drift.
            const bool exactly_half = std::fabs(worst_ratio - 0.5) < 1e-6;
            all_agree = all_agree && exactly_half;
            std::printf("\n  %-58s %9s        %s   %.6f\n",
                        "the worst of those is exactly one half", "at Brewster",
                        exactly_half ? "agree" : "DISAGREE", worst_ratio);

            // And the crossed case, which is worse than a factor and is the
            // reason this is filed rather than tolerated. Rotating the second
            // surface ninety degrees swaps which state is which, so the
            // tracked answer pairs s against p — and at two Brewster angles
            // both p terms are zero.
            const Reflected b = at(brewster);
            const double crossed_tracked = 0.5 * (b.s * b.p + b.p * b.s);
            const double crossed_averaged = b.unpolarised() * b.unpolarised();
            const bool unbounded = crossed_tracked == 0.0 && crossed_averaged > 0.0;
            all_agree = all_agree && unbounded;

            std::printf("  %-58s %9s        %s   %.8f vs 0\n",
                        "turn the second surface ninety degrees and it is unbounded",
                        "crossed", unbounded ? "agree" : "DISAGREE", crossed_averaged);

            std::printf("\n  Two crossed polarisers, the oldest demonstration in optics. The\n"
                        "  truth is that nothing comes back; this renderer predicts half a\n"
                        "  percent of it. What it cannot produce is the *absence* of a\n"
                        "  reflection, and no factor corrects for that — it needs four\n"
                        "  Stokes parameters per wavelength and a Mueller matrix at every\n"
                        "  interaction, which is item 0143 and is under `later`.\n");
        }

        std::printf("\n  The last row is why the dielectric and the conductor are one\n"
                    "  function here. Brewster's angle exists because `r_p` can reach\n"
                    "  zero; with a complex index it cannot, and nothing had to be\n"
                    "  written for that — the same equation stops having a root.\n");
    }

    std::printf("\n%s\n", all_agree
        ? "Every claim above holds."
        : "A CLAIM ABOVE DOES NOT HOLD.");
    std::printf("%.1f s. There is no --quick, because there is nothing worth skipping.\n",
                seconds);
    return all_agree ? 0 : 1;
}

} // namespace app
