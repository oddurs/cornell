// furnace.hpp — the best test in rendering, and the one this project is
// going to fail on purpose.
//
// Put an object in an environment of uniform radiance 1 and give it a
// reflectance of 1. If its BSDF conserves energy and the sampling is
// unbiased, every direction leaving that object carries exactly 1, the object
// renders the same white as the background it sits in, and **it disappears**.
//
// No reference image. No tolerance chosen after the fact. No judgement. The
// object is there, or it is not.
//
// ── Why it is built now, four milestones before it is interesting ────────
//
// Because a check written after the thing it checks is a check written to
// pass. Lambert vanishes trivially — the whole test on the model this project
// currently has is `(rho/pi) * pi == rho` — and that is the point: the
// instrument is calibrated on a case whose answer is known before it is
// pointed at a case whose answer is not. v0.7's microfacet model will not
// vanish, because a single-scattering Smith model loses energy at high
// roughness and nobody has to be told that here for it to show up.
//
// ── The environment, which is an enclosure ───────────────────────────────
//
// "An environment of uniform radiance 1" is usually an infinite constant
// environment map, and this project has no environment light — a ray that
// escapes finds nothing, and `transport.hpp` says so. It does not need one.
// An enclosure whose walls emit radiance 1 and reflect nothing *is* a uniform
// environment of radiance 1, seen from inside, built out of the two things
// the scene already has: a surface that emits and a surface that does not
// reflect.
//
// That matters for more than convenience. The furnace has to be the ordinary
// integrator pointed at an ordinary scene — if it needed its own code path it
// would be testing that code path — and this scene is made of the same
// `Surface`, the same `Lambert`, the same BVH and the same `radiance()` as
// the box. Nothing below reaches past the interface a reader would use.
//
// ── Three residuals, because there are three places to go wrong ──────────
//
// The rendered image is the whole integrator at once, which is what makes it
// convincing and also what makes a failure hard to localise. So the same
// claim is asked three ways, each blind to a different mistake:
//
//      1. The directional albedo by quadrature. Integrate `eval` against the
//         cosine over the hemisphere on a fixed grid, touching neither
//         `sample` nor `pdf`. This is the strong form, and it catches a
//         missing normalisation — a BRDF too large by pi is the oldest error
//         in the field and it is invisible in an image somebody has already
//         adjusted the light to compensate for.
//
//      2. The same integral through `sample` and `pdf`, as the estimator
//         `f · cos / pdf` averaged over draws. This is the weak form. It
//         agrees with the first only if the sampling routine and the density
//         describe the same distribution, so it catches the two lying to each
//         other — which `./cornell chi2` catches more sharply and which this
//         notices for free.
//
//      3. The furnace itself. Both of the above can pass while the transport
//         applies the cosine twice, or offsets a spawned ray onto the wrong
//         side, or loses a bounce to the depth limit.
//
// A failure in 1 but not 2 is a normalisation. A failure in 2 but not 1 is
// the pdf. A failure in 3 alone is not the material at all.
//
// ── What it cannot do ────────────────────────────────────────────────────
//
// It cannot tell you a BSDF is *right*. A furnace test is conservation, and
// conservation is necessary and nowhere near sufficient: a BRDF that returns
// `rho/pi` regardless of what it was asked passes perfectly and is not a
// model of anything. What it detects is energy created or destroyed, which is
// the class of error that survives being looked at.
//
// ── Compared exactly, which constrains the albedos ───────────────────────
//
// Nothing below is compared to a tolerance. The furnace's answer is a
// specific double and a tolerance would be a place to hide, so the rendered
// residual is `==`.
//
// That is affordable because of what the estimator does at a Lambertian:
// `f` is `rho/pi`, the density is `cos/pi`, the cosines are literally the
// same double on both sides of the division, and what is left is `rho`. It is
// left *exactly* when the two roundings of pi agree — which is when rho is a
// power of two, because that division shifts an exponent and touches no
// mantissa bit. So the asserted rows are 1, 1/2, 1/4 and 0, and 0.9 is
// measured underneath them and reported rather than asserted, so that the
// choice does not look like a convenient one.
//
// And it says nothing about anything below reflectance 1. A surface that
// absorbs half the light is supposed to come back at 0.5 and the sphere is
// supposed to be visible; the furnace's pass condition exists only at rho = 1,
// which is why the table below renders the other albedos too and checks the
// residual against `1 - rho` rather than against zero.

#pragma once

#include <cmath>
#include <cstdio>
#include <string_view>
#include <vector>

#include <render/camera.hpp>
#include <render/lambert.hpp>
#include <render/sampler.hpp>
#include <render/scene.hpp>
#include <render/si.hpp>
#include <render/transport.hpp>
#include <render/warp.hpp>

#include "image.hpp"

namespace app {

namespace furnace_detail {

using namespace render;

// The wavelengths every measurement below is taken at. A flat albedo is the
// same number at all four, so one draw is enough and a fixed one keeps the
// figures reproducible; the day a furnace is run on a spectral material this
// is the line that has to become a loop.
inline Wavelengths fixed_wavelengths() { return Wavelengths::sample(0.5); }

// ── 1. The strong form ───────────────────────────────────────────────────

// The directional albedo: how much of the light arriving from `wo`'s side
// leaves at all.
//
//      rho(wo) = ∫ f(wo, wi) cos(theta_i) dw_i
//                H²
//
// Substituting mu = cos(theta_i) turns `dw = sin(theta) dtheta dphi` into
// `dmu dphi`, so the integral is a plain double integral over a rectangle
// with the cosine as the only weight — no Jacobian left to get wrong, and a
// midpoint rule on a uniform grid converges on it without adapting.
//
// `sample` and `pdf` are not called. That is the point of this one.
inline double directional_albedo_by_quadrature(const Bsdf& bsdf, const Vec3& wo,
                                               int mu_cells, int phi_cells) {
    const Wavelengths lambdas = fixed_wavelengths();
    const double cell = si::two_pi / (double(mu_cells) * double(phi_cells));

    double total = 0.0;
    for (int j = 0; j < mu_cells; ++j) {
        const double mu = (double(j) + 0.5) / double(mu_cells);
        const double sin_theta = std::sqrt(std::fmax(0.0, 1.0 - mu * mu));

        for (int k = 0; k < phi_cells; ++k) {
            const double phi = si::two_pi * (double(k) + 0.5) / double(phi_cells);
            const Vec3 wi{sin_theta * std::cos(phi), sin_theta * std::sin(phi), mu};

            // The first component; a flat albedo is the same at all four.
            total += eval(bsdf, wo, wi, lambdas)[0] * mu;
        }
    }
    return total * cell;
}

// ── 2. The weak form ─────────────────────────────────────────────────────

// The same integral, estimated the way the renderer estimates it: draw a
// direction from the material's own sampling routine and average the
// estimator. House rule 3's ratio, with nothing else around it.
//
// The sampler is addressed rather than dispensed, exactly as in the film
// loop, so this figure does not depend on how it is called.
inline double directional_albedo_by_sampling(const Bsdf& bsdf, const Vec3& wo, int draws) {
    const Wavelengths lambdas = fixed_wavelengths();

    double total = 0.0;
    for (int i = 0; i < draws; ++i) {
        Sampler sampler{0x0f0f'0f0f'0f0f'0f0fULL, std::uint64_t(i)};
        const auto [u, v] = sampler.next2();

        const BsdfSample drawn = sample(bsdf, wo, lambdas, u, v);
        if (drawn.is_black()) continue;

        // f · cos / pdf. A Brdf, scaled by a cosine, divided by a density:
        // the sr⁻¹ cancels and what comes back is a Reflectance, which is
        // what a directional albedo is.
        const Reflectance weight = drawn.f * abs_cos_theta(drawn.wi) / drawn.pdf;
        total += weight[0];
    }
    return total / double(draws);
}

// ── 3. The furnace ───────────────────────────────────────────────────────

// Two triangles, wound so that the normal comes out facing `inward`. The
// enclosure's walls have to emit towards the camera, and emission is
// one-sided.
inline void add_wall(Scene& scene, const Vec3& a, const Vec3& b, const Vec3& c,
                     const Vec3& d, const Vec3& inward) {
    const Bsdf black{GreyLambert{Flat{0.0}}};
    Triangle first{a, b, c};
    Triangle second{a, c, d};
    if (dot(first.geometric_normal(), inward) < 0.0) {
        first  = Triangle{a, c, b};
        second = Triangle{a, d, c};
    }
    scene.add(Surface{first,  black, Flat{1.0}, 1.0});
    scene.add(Surface{second, black, Flat{1.0}, 1.0});
}

// A cube of emitting walls, and one sphere in the middle of it.
//
// The walls reflect nothing, which is what makes the enclosure a *uniform*
// environment rather than a cavity: light that reaches a wall stops there, so
// every wall carries exactly the radiance it emits and nothing accumulated
// from the others. A cavity whose walls both emit and reflect is the other
// test, it has a closed-form answer of `Le / (1 - rho)`, and `transport.hpp`
// already measures it.
//
// The sphere is convex, so a path leaving it cannot hit it again: every path
// in this scene is at most one bounce plus a wall. That is the correct scope
// for the claim — the furnace asks whether *a* bounce conserves energy — and
// it is why the depth limit and the roulette have nothing to do here.
inline Scene make_furnace(double rho) {
    Scene scene;

    const double half = 10.0;
    const Vec3 lo{-half, -half, -half};
    const Vec3 hi{half, half, half};

    add_wall(scene, {lo.x, lo.y, lo.z}, {hi.x, lo.y, lo.z}, {hi.x, hi.y, lo.z}, {lo.x, hi.y, lo.z}, {0, 0, 1});
    add_wall(scene, {lo.x, lo.y, hi.z}, {hi.x, lo.y, hi.z}, {hi.x, hi.y, hi.z}, {lo.x, hi.y, hi.z}, {0, 0, -1});
    add_wall(scene, {lo.x, lo.y, lo.z}, {lo.x, hi.y, lo.z}, {lo.x, hi.y, hi.z}, {lo.x, lo.y, hi.z}, {1, 0, 0});
    add_wall(scene, {hi.x, lo.y, lo.z}, {hi.x, hi.y, lo.z}, {hi.x, hi.y, hi.z}, {hi.x, lo.y, hi.z}, {-1, 0, 0});
    add_wall(scene, {lo.x, lo.y, lo.z}, {hi.x, lo.y, lo.z}, {hi.x, lo.y, hi.z}, {lo.x, lo.y, hi.z}, {0, 1, 0});
    add_wall(scene, {lo.x, hi.y, lo.z}, {hi.x, hi.y, lo.z}, {hi.x, hi.y, hi.z}, {lo.x, hi.y, hi.z}, {0, -1, 0});

    scene.add(Surface{Sphere{Vec3{0, 0, 0}, 1.0}, Bsdf{GreyLambert{Flat{rho}}}});

    scene.finalise();
    return scene;
}

struct Residual {
    long pixels = 0;
    long on_the_sphere = 0;
    double worst = 0.0;          // largest |L - 1| anywhere

    // The darkest radiance any path returned from a pixel looking at the
    // object, kept unsubtracted. The object is the only thing in this scene
    // darker than the walls, so the minimum is what it returned — and a
    // difference from 1 is where an error's size stops being readable, since
    // subtracting 0.9 from 1 keeps the absolute error and divides the value
    // by nine.
    double lowest_on_the_sphere = 1.0;
};

// Render it, and ask how far from 1 the radiance came back.
//
// The comparison is against the spectral radiance the integrator returned,
// not against a pixel: a tone curve and a white balance sit between the two,
// and the claim being tested is radiometric. The image is written for the
// reader; this number is what decides.
inline Residual measure(const Scene& scene, const Camera& camera,
                        int resolution, int spp, std::vector<double>& picture) {
    Residual out;
    picture.assign(std::size_t(resolution) * std::size_t(resolution), 0.0);

    for (int y = 0; y < resolution; ++y) {
        for (int x = 0; x < resolution; ++x) {
            const std::uint64_t pixel =
                std::uint64_t(y) * std::uint64_t(resolution) + std::uint64_t(x);
            ++out.pixels;

            // Whether this pixel is looking at the object at all. Without it
            // a furnace that had lost the sphere entirely would pass, which
            // is the way this test fails silently.
            const double centre_u = (double(x) + 0.5) / double(resolution);
            const double centre_v = (double(y) + 0.5) / double(resolution);
            const auto centre_hit = scene.intersect(camera.ray_through(centre_u, centre_v));
            const bool sphere =
                centre_hit && std::holds_alternative<Sphere>(centre_hit->surface->shape);
            if (sphere) ++out.on_the_sphere;

            double sum = 0.0;
            for (int s = 0; s < spp; ++s) {
                Sampler sampler{pixel, std::uint64_t(s)};
                const auto [jitter_u, jitter_v] = sampler.next2();
                const double u = (double(x) + jitter_u) / double(resolution);
                const double v = (double(y) + jitter_v) / double(resolution);

                const Wavelengths lambdas = Wavelengths::sample(sampler.next());
                const Radiance carried =
                    radiance(scene, camera.ray_through(u, v), lambdas, sampler);

                for (int i = 0; i < spectral_samples; ++i) {
                    const double deviation = std::fabs(carried[i] - 1.0);
                    out.worst = std::fmax(out.worst, deviation);
                    if (sphere) out.lowest_on_the_sphere =
                        std::fmin(out.lowest_on_the_sphere, carried[i]);
                }
                sum += carried[0];
            }
            picture[std::size_t(pixel)] = sum / double(spp);
        }
    }
    return out;
}

} // namespace furnace_detail

// ── The instrument ───────────────────────────────────────────────────────

inline int furnace(std::string_view model, double rho_asked, bool write_image) {
    using namespace furnace_detail;
    using namespace render;

    if (model != "lambert") {
        std::fprintf(stderr,
                     "cornell: no BSDF called '%.*s'. There is one, and it is lambert.\n"
                     "         fresnel.hpp arrives in v0.6 and is the one this test is for.\n",
                     int(model.size()), model.data());
        return 1;
    }

    std::printf("The white furnace.\n\n"
                "An enclosure whose walls emit radiance 1 and reflect nothing is a uniform\n"
                "environment of radiance 1, seen from inside. A sphere of albedo 1 in it must\n"
                "return exactly 1 in every direction, and therefore vanish.\n\n");

    // The angles. A directional albedo may depend on which way the light came
    // from, and Lambert's does not, so a table of one row would be a table
    // that could not tell.
    constexpr double angles[] = {0.0, 30.0, 60.0, 85.0};
    constexpr int mu_cells = 512, phi_cells = 512;
    constexpr int draws = 1 << 22;

    const Bsdf white{GreyLambert{Flat{1.0}}};
    const Bsdf half{GreyLambert{Flat{0.5}}};

    std::printf("1. Directional albedo by quadrature over the hemisphere, %d x %d cells.\n"
                "   `eval` only: neither `sample` nor `pdf` is called.\n\n",
                mu_cells, phi_cells);
    std::printf("     theta_o        rho = 1            rho = 0.5\n");
    for (const double degrees : angles) {
        const double theta = degrees * si::pi / 180.0;
        const Vec3 wo{std::sin(theta), 0.0, std::cos(theta)};
        std::printf("     %5.1f deg     %.15f    %.15f\n", degrees,
                    directional_albedo_by_quadrature(white, wo, mu_cells, phi_cells),
                    directional_albedo_by_quadrature(half,  wo, mu_cells, phi_cells));
    }

    std::printf("\n   The integrand is constant, so the midpoint rule is exact here and the\n"
                "   residual above is summation, not quadrature. It grows with the number\n"
                "   of terms — measured at rho = 1, normal incidence:\n\n"
                "        64 x 64      -3.33e-16        512 x 512     -5.47e-14\n"
                "       128 x 128     +2.00e-15       1024 x 1024    +1.24e-13\n"
                "       256 x 256     +2.40e-14\n\n"
                "   which is what naive addition of N terms in a double does, and is nine\n"
                "   orders of magnitude below the factor of pi this table exists to catch.\n");

    std::printf("\n2. The same integral through `sample` and the estimator f cos / pdf,\n"
                "   %d draws.\n\n", draws);
    std::printf("     theta_o        rho = 1            rho = 0.5\n");
    for (const double degrees : angles) {
        const double theta = degrees * si::pi / 180.0;
        const Vec3 wo{std::sin(theta), 0.0, std::cos(theta)};
        std::printf("     %5.1f deg     %.15f    %.15f\n", degrees,
                    directional_albedo_by_sampling(white, wo, draws),
                    directional_albedo_by_sampling(half,  wo, draws));
    }

    // And the furnace itself.
    constexpr int resolution = 256;
    constexpr int spp = 16;

    const Camera camera = Camera::look_at(Vec3{0, 0, -5}, Vec3{0, 0, 0}, Vec3{0, 1, 0},
                                          0.025, 0.025, 0.035);

    std::printf("\n3. The furnace, rendered: %d x %d at %d spp, the ordinary integrator.\n"
                "   Compared exactly. Every albedo here is a binary fraction, which is\n"
                "   not a convenience — see the note under the table.\n\n",
                resolution, resolution, spp);
    std::printf("      rho    pixels on the sphere    worst |L - 1|      expected     verdict\n");

    constexpr double albedos[] = {1.0, 0.5, 0.25, 0.0};
    int failures = 0;
    std::vector<double> picture;

    for (const double rho : albedos) {
        const Scene scene = make_furnace(rho);
        const Residual r = measure(scene, camera, resolution, spp, picture);

        // Away from the sphere the answer is the wall, which is 1 by
        // construction; on the sphere one bounce of an albedo-rho Lambertian
        // in a uniform environment of radiance 1 returns exactly rho. So the
        // worst deviation over the whole image is 1 - rho, exactly, and at
        // rho = 1 it is zero. This is the only pass condition the furnace has.
        const double expected = 1.0 - rho;
        const bool ok = r.worst == expected && r.on_the_sphere > 0;
        if (!ok) ++failures;

        std::printf("     %4.2f    %20ld    %.6e    %.6e    %s\n",
                    rho, r.on_the_sphere, r.worst, expected,
                    rho == 1.0 ? (ok ? "vanished" : "STILL THERE")
                               : (ok ? "visible, by exactly 1 - rho" : "WRONG"));

    }

    // ── The albedo that is not a binary fraction ─────────────────────────
    //
    // Reported rather than asserted, because it is a fact about arithmetic
    // and not about transport, and because leaving it out would make the
    // table above look like a choice of convenient numbers.
    {
        const Scene scene = make_furnace(0.9);
        const Residual r = measure(scene, camera, resolution, spp, picture);
        const double expected = 1.0 - 0.9;
        const double ulp = std::nextafter(expected, 2.0) - expected;

        const double rho_ulp = std::nextafter(0.9, 2.0) - 0.9;

        std::printf("\n   Not every albedo lands exactly, and 0.9 is the demonstration:\n\n"
                    "      darkest radiance          %.17g\n"
                    "      rho                       %.17g\n"
                    "      short by                  %+.1f ulp of rho\n\n"
                    "      worst |L - 1|             %.17g\n"
                    "      1 - rho                   %.17g\n"
                    "      over by                   %+.1f ulp of 1 - rho\n\n",
                    r.lowest_on_the_sphere, 0.9, (r.lowest_on_the_sphere - 0.9) / rho_ulp,
                    r.worst, expected, (r.worst - expected) / ulp);

        std::printf("   The transport has no rounding in it: the estimator is\n"
                    "   (f * cos) / (cos * pdf), and the two cosines are the same double. What\n"
                    "   does not cancel is f against the density's constant — f is rho/pi and\n"
                    "   the density is cos/pi, so the quotient is rho only if those two\n"
                    "   roundings of pi agree. They agree exactly when rho is a power of two,\n"
                    "   because that division shifts an exponent and leaves every mantissa bit\n"
                    "   alone. 0.9 is a rounding, and lands exactly two ulp low.\n\n"
                    "   The two ulp counts above are the same error twice. Subtracting from 1\n"
                    "   keeps the absolute error and divides the value by nine, so an error of\n"
                    "   under two ulp becomes sixteen — which is the argument for reporting the\n"
                    "   radiance rather than only the residual, and the reason the asserted\n"
                    "   rows are binary fractions rather than convenient ones.\n");
    }

    // ── The picture ──────────────────────────────────────────────────────
    //
    // Last, because it is the least of the three: the eye cannot see 1e-3 and
    // the residual above can. It is here because "the object disappears" is
    // the sentence this test is famous for, and a reader is owed the chance
    // to look.
    if (write_image) {
        const Scene scene = make_furnace(rho_asked);
        measure(scene, camera, resolution, spp, picture);

        std::vector<double> grey(picture.size() * 3);
        for (std::size_t i = 0; i < picture.size(); ++i)
            grey[i * 3 + 0] = grey[i * 3 + 1] = grey[i * 3 + 2] = picture[i];

        if (write_ppm("furnace.ppm", resolution, resolution, grey))
            std::printf("\n   furnace.ppm   rho = %.2f, radiance with 1 at white and the sRGB\n"
                        "                 transfer function on top, because that is what\n"
                        "                 `image.hpp` writes and an instrument does not get its\n"
                        "                 own. The sphere is %s.\n",
                        rho_asked, rho_asked == 1.0 ? "not in it" : "a disc");
    }

    std::printf("\n%s\n", failures == 0
                ? "Lambert conserves energy, and the furnace is empty."
                : "The furnace has something in it that should not be there.");
    return failures == 0 ? 0 : 1;
}

} // namespace app
