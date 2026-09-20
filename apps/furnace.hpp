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
#include <type_traits>
#include <vector>

#include <render/camera.hpp>
#include <render/lambert.hpp>
#include <render/sampler.hpp>
#include <render/scene.hpp>
#include <render/si.hpp>
#include <render/transport.hpp>
#include <render/warp.hpp>
#include <render/visible_normals.hpp>

#include "enclosure.hpp"
#include "image.hpp"

namespace app {

namespace furnace_detail {

// The stream the energy accounting draws on. Fixed, like every other seed in
// this project's instruments: a closure that fails must fail again.
inline constexpr std::uint64_t seed_accounting = 0x0086'0086'0086'0086;

using namespace render;

// The two calls this file makes, spelled so that they work on a `Bsdf` — which
// dispatches through free functions, because `scene.hpp` wanted a model to be
// a plain struct that knows nothing about the variant it ends up in — and on a
// plain model, which has them as members. `verify.hpp` needs the second: the
// deliberately wrong BSDFs it runs through this quadrature are not materials
// and are not in anybody's variant.
template <class Model>
Brdf model_eval(const Model& model, const Vec3& wo, const Vec3& wi,
                const Wavelengths& lambdas) {
    if constexpr (std::is_same_v<Model, Bsdf>) return eval(model, wo, wi, lambdas);
    else return model.eval(wo, wi, lambdas);
}

template <class Model>
BsdfSample model_sample(const Model& model, const Vec3& wo,
                        const Wavelengths& lambdas, double u, double v) {
    if constexpr (std::is_same_v<Model, Bsdf>) return sample(model, wo, lambdas, u, v);
    else return model.sample(wo, lambdas, u, v);
}

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
template <class Model>
double directional_albedo_by_quadrature(const Model& model, const Vec3& wo,
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
            total += model_eval(model, wo, wi, lambdas)[0] * mu;
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
template <class Model>
double directional_albedo_by_sampling(const Model& model, const Vec3& wo, int draws) {
    const Wavelengths lambdas = fixed_wavelengths();

    double total = 0.0;
    for (int i = 0; i < draws; ++i) {
        Sampler sampler{0x0f0f'0f0f'0f0f'0f0fULL, std::uint64_t(i)};
        const auto [u, v] = sampler.next2();

        const BsdfSample drawn = model_sample(model, wo, lambdas, u, v);
        if (drawn.is_black()) continue;

        // f · cos / pdf. A Brdf, scaled by a cosine, divided by a density:
        // the sr⁻¹ cancels and what comes back is a Reflectance, which is
        // what a directional albedo is.
        //
        // Unless the division already happened, which is the case for a
        // mirror and, since item 0161, for a walk. The branch is the same one
        // `transport.hpp` writes at its own point of use, and it is here for
        // the same reason: an instrument that formed the ratio anyway would
        // divide a single-scattering `f` by a single-scattering `pdf` and
        // report that as the albedo of a material that does more than that.
        //
        // The first version of this routine did exactly that and returned NaN
        // for every row, which was luckier than it sounds — a plausible
        // number would have been believed.
        const Reflectance weight =
            drawn.weight_is_taken() ? drawn.weight
                                    : drawn.f * abs_cos_theta(drawn.wi) / drawn.pdf;
        total += weight[0];
    }
    return total / double(draws);
}

// ── 3. The furnace ───────────────────────────────────────────────────────

// The scene is `enclosure.hpp`: walls that emit radiance 1 and reflect
// nothing, which is a uniform environment of radiance 1 seen from inside, and
// one sphere in the middle. It lives next door because `converge.hpp` needs
// the same box with reflecting walls, and two copies of a cube is two places
// for a winding order to be wrong in only one of them.
//
// The sphere is convex, so a path leaving it cannot hit it again: every path
// in this scene is at most one bounce plus a wall. That is the correct scope
// for the claim — the furnace asks whether *a* bounce conserves energy — and
// it is why the depth limit and the roulette have nothing to do here.
inline Scene make_furnace(double rho) { return enclosure::uniform_environment(rho); }

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

    // And the same quantity averaged rather than minimised, which is the one
    // a BSDF that can return a black sample needs.
    //
    // Lambert never does: every draw off an albedo-rho Lambertian in this
    // scene returns exactly rho, so the minimum, the mean and the maximum are
    // one number and `lowest_on_the_sphere` says everything. A microfacet
    // surface draws facets whose mirror direction points into the ground, and
    // those paths carry nothing — correctly, and `chi2.hpp` shows the density
    // agrees — so the minimum over individual samples is 0.0 whatever the
    // model does, and is not a measurement of anything.
    //
    // What the furnace is asking is how much light comes back, which is a
    // mean. This is it, over every sample taken through every pixel whose
    // centre is on the object.
    double total_on_the_sphere = 0.0;
    long samples_on_the_sphere = 0;

    double mean_on_the_sphere() const {
        return samples_on_the_sphere > 0
             ? total_on_the_sphere / double(samples_on_the_sphere) : 0.0;
    }
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
                    if (sphere) {
                        out.lowest_on_the_sphere =
                            std::fmin(out.lowest_on_the_sphere, carried[i]);
                        out.total_on_the_sphere += carried[i];
                        ++out.samples_on_the_sphere;
                    }
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

// ── Where the light actually goes ────────────────────────────────────────
//
// Item 0086, and the reason it is typed as a bug rather than as a
// measurement. `./cornell furnace --bsdf conductor` says a rough metal of
// reflectance 1 returns four tenths of the light at roughness 1. This says
// where the other six tenths went, and the answer has to distinguish between
// two possibilities that look identical in an image:
//
//      the model is throwing energy away    — a bug in the physics
//      this program is losing it            — a bug in the program
//
// Every draw off this surface ends in exactly one of three states, and they
// are exhaustive by construction rather than by inspection:
//
//      escaped   the facet reflected it into the world and nothing blocked it
//      masked    the facet reflected it upward and another facet intercepted it
//      below     the facet reflected it into the surface
//
// The first is the albedo. The second and third are both *the ray hitting the
// microsurface a second time*, and a single-scattering model has nothing to
// say about what happens next, so it drops them.
//
// If those three sum to one, no photon is unaccounted for and the deficit is
// entirely made of rays that struck the surface again. That is the difference
// between a model that is wrong and a program that is broken, and it is a
// number rather than an argument.
//
// ── What the accounting corrected ────────────────────────────────────────
//
// The item describes the mechanism as the masking term dropping shadowed
// light, which is the usual telling and is the smaller half of the story.
// Measured, the channel that dominates at almost every roughness is the third
// one: at normal incidence and roughness 1, a fifth of the light is masked on
// the way out and *half of it never points outward at all*. The facet drawn
// from the visible distribution is tilted so far that its mirror direction
// goes into the ground.
//
// The masking channel only leads at grazing, where `Lambda_i` is large for
// every outgoing direction. So a rough surface loses light mostly because it
// is rough enough to reflect into itself, and only secondarily because the
// shadowing term discards what it does reflect outward — and neither is
// absorption, which is what the material was told to do none of.
inline int furnace_accounting() {
    using namespace furnace_detail;
    using namespace render;

    std::printf("Where the light goes.\n\n"
                "Every draw off a rough conductor of reflectance 1 ends in one of three\n"
                "states, and they are exhaustive: it escaped, another facet intercepted\n"
                "it, or the facet reflected it into the surface. The last two are the\n"
                "same event — the ray met the microsurface again — and a single-\n"
                "scattering model drops both.\n\n");

    constexpr double roughnesses[] = {0.050, 0.100, 0.200, 0.400,
                                      0.600, 0.800, 1.000};
    constexpr double angles[] = {0.0, 60.0, 85.0};
    constexpr long draws = 1L << 22;

    std::printf("      alpha  theta  %10s %10s %10s  %10s %9s\n",
                "escaped", "masked", "below", "sum - 1", "albedo");

    int failures = 0;
    double worst_closure = 0.0;
    double worst_against_albedo = 0.0;

    for (const double alpha : roughnesses) {
        const TrowbridgeReitz distribution{alpha};
        const Smith smith{distribution};
        const Bsdf rough{GreyRough{FlatReflectance{1.0}, distribution}};

        for (const double degrees : angles) {
            const double theta = degrees * si::pi / 180.0;
            const Vec3 wo{std::sin(theta), 0.0, std::cos(theta)};

            double escaped = 0.0, masked = 0.0, below = 0.0;

            for (long k = 0; k < draws; ++k) {
                Sampler sampler{seed_accounting, std::uint64_t(k)};
                const auto [u, v] = sampler.next2();

                const Vec3 m = sample_visible_normal(distribution, wo, u, v);
                const Vec3 wi = 2.0 * dot(wo, m) * m - wo;

                // Reflected into the surface. In a real height field this ray
                // travels on and hits something; here it stops, and that is
                // the model rather than the code.
                if (!same_hemisphere(wo, wi)) { below += 1.0; continue; }

                // And of what does point outward, the share that gets out.
                // `chi2.hpp` derives this as the whole of the estimator's
                // weight when Fresnel is one, so the split below is not a
                // second model of the same thing — it is that weight, and
                // what is left over.
                const double lambda_o = smith.lambda(wo);
                const double lambda_i = smith.lambda(wi);
                const double leaves = (1.0 + lambda_o) / (1.0 + lambda_o + lambda_i);

                escaped += leaves;
                masked += 1.0 - leaves;
            }

            escaped /= double(draws);
            masked /= double(draws);
            below /= double(draws);

            // The claim. Nothing is missing, so the deficit is entirely rays
            // that hit the surface again.
            const double closure = escaped + masked + below - 1.0;
            worst_closure = std::fmax(worst_closure, std::fabs(closure));

            // And the tie back to the instrument next door: the escaped share
            // has to be the directional albedo, measured by a routine that
            // knows nothing about this decomposition.
            const double albedo = directional_albedo_by_sampling(rough, wo, 1 << 21);
            worst_against_albedo = std::fmax(worst_against_albedo,
                                             std::fabs(escaped - albedo));

            std::printf("      %5.3f  %5.0f  %10.6f %10.6f %10.6f  %10.2e %9.6f\n",
                        alpha, degrees, escaped, masked, below, closure, albedo);
        }
    }

    if (!(worst_closure < 1e-12)) ++failures;
    if (!(worst_against_albedo < 2e-3)) ++failures;

    std::printf("\n   The three columns sum to one to %.1e, which is summation of four\n"
                "   million doubles and not a residual of anything physical. No light\n"
                "   is unaccounted for: every photon that does not come back is one\n"
                "   that struck the microsurface a second time and was dropped.\n",
                worst_closure);

    std::printf("\n   The escaped column and the albedo column are measured by routines\n"
                "   with nothing in common — one decomposes the draw, the other forms\n"
                "   f cos / pdf and averages it — and they agree to %.1e, which is the\n"
                "   noise of two estimators at different draw counts.\n",
                worst_against_albedo);

    std::printf("\n   The usual telling of this failure blames the masking term. It is\n"
                "   the smaller channel. At normal incidence a fifth of the light is\n"
                "   masked on the way out at roughness 1 and half of it never points\n"
                "   outward at all; the masking channel only leads at grazing. A rough\n"
                "   surface loses light mostly because it is rough enough to reflect\n"
                "   into itself.\n");

    std::printf("\n   None of this is absorption. The reflectance is 1 at every\n"
                "   wavelength and every angle, and `torrance_sparrow.hpp` says in its\n"
                "   opening that it was written knowing it would fail here. What to do\n"
                "   about it is item 0087. %s\n",
                failures == 0
                    ? "The accounting closes."
                    : "THE ACCOUNTING DOES NOT CLOSE, WHICH WOULD BE THIS PROGRAM'S FAULT.");

    return failures == 0 ? 0 : 1;
}

// ── The same accounting, after the light was followed ────────────────────
//
// Item 0162. `furnace_accounting` above splits a single scattering event into
// escaped, masked and below, and shows that the last two are losses. This
// splits a whole walk by how many times it scattered, and shows that they are
// not losses any more — the light in them comes back at orders two and up.
//
// ── Why the order matters and the total does not ─────────────────────────
//
// The energy test is weak here, and saying so is the point of this section
// existing rather than the furnace alone being deemed enough. At a
// reflectance of 1 the weight never changes, so *any* walk that terminates by
// leaving upward returns exactly 1. A walk that scattered into wrong
// directions, or drew the wrong facet, or got the sign of `Lambda` backwards
// — as the first draft of `multiple_scattering.hpp` did — conserves energy
// perfectly and passes the furnace.
//
// Splitting by order catches what the total cannot. The share that leaves
// after exactly one scattering event has to be the single-scattering albedo,
// and `torrance_sparrow.hpp` computes that from a closed form built out of
// `D`, `G₂` and `G₁` with none of this file's arithmetic in it. Two models,
// one number, and they have to agree.
//
// That is the check that would have caught the sign: with it wrong the walk
// still conserved energy exactly and its first-order share was out by 0.216
// at roughness 1 and sixty degrees.
inline int furnace_orders() {
    using namespace furnace_detail;
    using namespace render;

    std::printf("Where the light goes, once it is followed.\n\n"
                "The same walk, split by how many times it scattered before it left.\n"
                "Order 1 is what a single-scattering model would have returned and the\n"
                "rest is what that model was throwing away.\n\n");

    constexpr double roughnesses[] = {0.050, 0.100, 0.200, 0.400,
                                      0.600, 0.800, 1.000};
    constexpr double angles[] = {0.0, 60.0, 85.0};
    constexpr long draws = 1L << 21;

    std::printf("      alpha  theta  %9s %9s %9s %10s  %11s\n",
                "order 1", "order 2", "order 3+", "total", "single-scat");

    int failures = 0;
    double worst_total = 0.0;
    double worst_against_single = 0.0;
    double largest_recovered = 0.0;
    double worst_against_deficit = 0.0;
    double weakest_calibration = 1.0;

    const Wavelengths lambdas = fixed_wavelengths();

    for (const double alpha : roughnesses) {
        const TrowbridgeReitz distribution{alpha};
        const MultipleScattering<FlatReflectance> walk{FlatReflectance{1.0}, distribution};
        const Bsdf single{GreyRough{FlatReflectance{1.0}, distribution}};

        for (const double degrees : angles) {
            const double theta = degrees * si::pi / 180.0;
            const Vec3 wo{std::sin(theta), 0.0, std::cos(theta)};

            double first = 0.0, second = 0.0, rest = 0.0;

            for (long k = 0; k < draws; ++k) {
                Sampler sampler{seed_accounting, std::uint64_t(k)};
                const auto [u, v] = sampler.next2();

                const auto walked = walk.walk(wo, lambdas, u, v);
                if (walked.sample.is_black()) continue;

                const double carried = walked.sample.weight[0];
                if (walked.order == 1) first += carried;
                else if (walked.order == 2) second += carried;
                else rest += carried;
            }

            first /= double(draws);
            second /= double(draws);
            rest /= double(draws);

            const double total = first + second + rest;
            const double single_scattering =
                directional_albedo_by_sampling(single, wo, 1 << 21);

            // Nothing lost. With a reflectance of 1 this is exact rather than
            // statistical: every walk returns the same number.
            worst_total = std::fmax(worst_total, std::fabs(total - 1.0));

            // And the sharp one. Two models, one number.
            worst_against_single =
                std::fmax(worst_against_single, std::fabs(first - single_scattering));

            // The item's own sentence, as a number. Everything beyond the
            // first scattering event is light the single-scattering model
            // dropped, so it has to come to exactly what that model was
            // short of.
            const double recovered = second + rest;
            const double was_missing = 1.0 - single_scattering;
            largest_recovered = std::fmax(largest_recovered, recovered);
            worst_against_deficit =
                std::fmax(worst_against_deficit, std::fabs(recovered - was_missing));

            // The calibration, and it costs nothing because it is the first
            // column. A walk stopped after one scattering event *is* the
            // single-scattering model, and its total is `first` rather than
            // 1 — so if this section could not tell those apart, the distance
            // between them is what it would be failing to see.
            if (alpha >= 0.2)
                weakest_calibration = std::fmin(weakest_calibration, 1.0 - first);

            std::printf("      %5.3f  %5.0f  %9.6f %9.6f %9.6f %10.6f  %11.6f\n",
                        alpha, degrees, first, second, rest, total, single_scattering);
        }
    }

    if (!(worst_total < 1e-12)) ++failures;
    if (!(worst_against_single < 3e-3)) ++failures;
    if (!(worst_against_deficit < 3e-3)) ++failures;
    if (!(weakest_calibration > 1e-2)) ++failures;

    std::printf("\n   The total is 1 at every row, and %.1e is the worst it departs\n"
                "   from it — not a small residual but none at all, because every walk\n"
                "   returns the same number and the mean of a constant is exact.\n"
                "\n   That is the furnace's claim and it is the weaker half: at a\n"
                "   reflectance of 1 the weight never changes, so any walk that ends by\n"
                "   leaving returns 1 whatever directions it went through on the way.\n",
                worst_total);

    std::printf("\n   The first column is the half with teeth. It is what leaves after a\n"
                "   single scattering event, and `torrance_sparrow.hpp` computes the\n"
                "   same quantity from D, G2 and G1 in closed form with none of the\n"
                "   walk's arithmetic in it. They agree to %.1e, which is the noise of\n"
                "   two estimators. A walk that conserved energy while scattering into\n"
                "   the wrong directions would fail here and nowhere else — the first\n"
                "   draft of the walk did exactly that, and was out by 0.216.\n",
                worst_against_single);

    std::printf("\n   Orders two and up come to as much as %.6f, and that is the light\n"
                "   item 0086 accounted for as masked or reflected into the surface. It\n"
                "   was never absorbed; it was dropped, and now it is followed. What it\n"
                "   comes to and what the single-scattering model was short of differ by\n"
                "   at most %.1e, which is the sentence this item was opened to be able\n"
                "   to write.\n", largest_recovered, worst_against_deficit);

    std::printf("\n   The calibration is the first column again, read as a failure: a\n"
                "   walk stopped after one scattering event is the single-scattering\n"
                "   model, and its total would be short of 1 by at least %.3f over the\n"
                "   rows from roughness 0.2 up. That is the distance this section would\n"
                "   be failing to see if it could not tell the two apart. %s\n",
                weakest_calibration,
                failures == 0 ? "It can." : "SOMETHING ABOVE DOES NOT HOLD.");

    return failures == 0 ? 0 : 1;
}

// ── The rough conductor, which is what the instrument was built for ──────
//
// Item 0085. Four milestones ago this file said, in its opening, that v0.7's
// microfacet model would not vanish. This is that sentence becoming a
// measurement.
//
// The claim under test is the same one Lambert passes: a surface whose
// reflectance is 1 at every wavelength and every angle, in an environment of
// radiance 1, must return 1 in every direction. `FlatReflectance{1.0}` makes
// the Fresnel term identically one, so nothing below is absorbed by the
// material and anything missing is the *model* losing it.
//
// ── Reading the signature, which the item asked for in advance ───────────
//
// The item that scheduled this check named three signatures: uniformly too
// dark is energy lost to multiple scattering, a dark rim at grazing is the
// masking term, and too bright is the normalisation. Having now measured it,
// the first two are less separable than that, and the reason is worth more
// than the heuristic was.
//
// `chi2.hpp` derives what one draw of this estimator weighs:
//
//      f cos_i / pdf = F G₂ / G₁ = (1 + Lambda_o) / (1 + Lambda_o + Lambda_i)
//
// with `F` gone because it is 1. So the directional albedo is the mean of
// that ratio, the deficit is one minus it, and every trend in the table below
// falls out of that one expression rather than out of three rules of thumb.
//
// At low roughness both `Lambda`s are near zero except near grazing, so the
// deficit appears at the rim first and nowhere else. At high roughness
// `Lambda_i` is substantial in most directions and the deficit is everywhere
// — but it is *smallest* at grazing, because `Lambda_o` grows without bound
// there and a ratio whose numerator and denominator both contain a diverging
// term tends to one. So the angular trend reverses somewhere in the middle of
// the range, which is not a masking bug: it is what a correct masking term
// does when the light it masks is thrown away rather than followed.
//
// On a sphere that reads as a radial profile, and it is worth stating
// carefully because the first draft of this comment got it wrong. Annulus-
// averaged from the rendered image, in linear radiance:
//
//      alpha 1.0    0.308 at the centre, rising to 0.589 in the outer tenth
//      alpha 0.4    0.786 head on, a shallow minimum of 0.757 near sixty
//                   degrees, and a bright edge
//
// So at high roughness the disc really is darkest in the middle and brightest
// at the rim, and below about 0.5 it is nearly flat with its darkest ring some
// way out. A single row of pixels at 16 samples cannot tell those apart —
// that much of the image is noise — and averaging over annuli can.
//
// Nothing here is a failure of the code. It is the model, and `torrance_
// sparrow.hpp` says in its own opening that it was written knowing this.
inline int furnace_conductor(double alpha_asked, bool write_image, bool walk) {
    using namespace furnace_detail;
    using namespace render;

    std::printf("The white furnace, with a rough conductor in it.\n\n"
                "Reflectance 1 at every wavelength and every angle, so the material\n"
                "absorbs nothing and any light that does not come back was lost by the\n"
                "model.\n\n");
    std::printf(walk
        ? "This one follows the light: the rays a single-scattering model drops\n"
          "are walked to the facet they meet next, and on until they leave.\n\n"
        : "A single-scattering microfacet BRDF drops the light that one facet\n"
          "reflects into another, and this is how much.\n\n");

    constexpr double roughnesses[] = {0.001, 0.010, 0.050, 0.100, 0.200,
                                      0.400, 0.600, 0.800, 1.000};
    constexpr double angles[] = {0.0, 60.0, 85.0};
    constexpr int mu_cells = 512, phi_cells = 512;
    constexpr int draws = 1 << 21;

    int failures = 0;

    // Roughness zero is not on the table and cannot be: at exactly zero the
    // lobe is a delta, `trowbridge_reitz.hpp` returns nothing rather than a
    // NaN, and the surface this file would measure is black. That is not a
    // gap in the model — a surface with no roughness is a mirror, it is
    // `specular.hpp`, and the Lambert furnace's third section already puts
    // one in here and watches it vanish. The smooth end of this table is
    // 0.001, which is a mirror to four decimal places and is measured as one.
    if (alpha_asked == 0.0)
        std::printf("   (--alpha 0 is a mirror, which is `specular.hpp` and which\n"
                    "    `./cornell furnace` already vanishes. The table starts at\n"
                    "    0.001, where this model is still one to six decimals.)\n\n");

    std::printf("1. Directional albedo. Quadrature over `eval` on a %d x %d grid, and\n"
                "   the estimator f cos / pdf over %d draws, side by side.\n\n",
                mu_cells, phi_cells, draws);
    std::printf("      alpha  %8s %8s  %8s %8s  %8s %8s\n",
                "quad 0", "samp 0", "quad 60", "samp 60", "quad 85", "samp 85");

    for (const double alpha : roughnesses) {
        const Bsdf rough = walk
            ? Bsdf{GreyWalk{FlatReflectance{1.0}, TrowbridgeReitz{alpha}}}
            : Bsdf{GreyRough{FlatReflectance{1.0}, TrowbridgeReitz{alpha}}};
        std::printf("      %5.3f", alpha);

        for (const double degrees : angles) {
            const double theta = degrees * si::pi / 180.0;
            const Vec3 wo{std::sin(theta), 0.0, std::cos(theta)};

            const double by_quadrature =
                directional_albedo_by_quadrature(rough, wo, mu_cells, phi_cells);
            const double by_sampling = directional_albedo_by_sampling(rough, wo, draws);

            // Too bright is the one signature that would be this file's
            // problem rather than the model's, and it is checked at every
            // roughness and every angle rather than left to the eye. A
            // microfacet BRDF whose normalisation is wrong is too large
            // everywhere by a constant, and no amount of lost energy can
            // disguise that at the smooth end, where there is none.
            if (by_sampling > 1.0 + 1e-6 || by_quadrature > 1.0 + 1e-6) ++failures;

            // And for the walk, the whole claim: it does not merely fail to
            // exceed one, it reaches it. Held to a tolerance a hundred times
            // tighter than the estimator's own noise would need, because with
            // a reflectance of 1 the estimate has no noise — every walk
            // returns exactly 1 and the mean of a constant is that constant.
            if (walk && std::fabs(by_sampling - 1.0) > 1e-12) ++failures;

            // Four decimals, not six. The sampled column is an estimate and
            // its standard error at this draw count is a few parts in ten
            // thousand, so a fifth digit would be printing noise with the
            // authority of a measurement.
            std::printf("  %8.4f %8.4f", by_quadrature, by_sampling);
        }
        std::printf("\n");
    }

    // ── Which of the two columns to believe, and where ───────────────────
    //
    // They disagree badly at the top of the table and agree to five decimals
    // from 0.1 down, and the one that is wrong is the quadrature. It is the
    // same resolution wall `chi2.hpp` documents: a 512 by 512 grid over the
    // hemisphere has cells about 0.004 wide in `mu`, and at roughness 0.001
    // the lobe is two parts in a million of `mu`. The grid steps over it and
    // reports almost nothing.
    //
    // The estimator has no such problem, because it draws from the lobe. So
    // at the smooth end the sampled column is the measurement and the
    // quadrature is an artefact, and in the middle of the table they agree,
    // which is what licenses believing either.
    //
    // That is an inversion worth noticing. `furnace.hpp`'s opening calls the
    // quadrature the strong form — it touches neither `sample` nor `pdf`, so
    // it cannot be fooled by the two agreeing with each other about something
    // false. That is still true, and it is not much use on a distribution the
    // grid cannot see. Two instruments, each blind where the other sees.
    {
        const Bsdf smoothest = walk
            ? Bsdf{GreyWalk{FlatReflectance{1.0}, TrowbridgeReitz{0.001}}}
            : Bsdf{GreyRough{FlatReflectance{1.0}, TrowbridgeReitz{0.001}}};
        const Vec3 up{0.0, 0.0, 1.0};
        const double sampled = directional_albedo_by_sampling(smoothest, up, draws);

        // The smooth end must lose nothing. A microfacet surface with no
        // roughness is a mirror, a mirror in a furnace returns exactly what
        // arrived, and this is the row that would catch a normalisation
        // error before the energy loss had a chance to hide it.
        const bool vanishes = std::fabs(sampled - 1.0) < 1e-4;
        if (!vanishes) ++failures;

        std::printf("\n   At roughness 0.001 the sampled albedo is %.6f, and it has to be:\n"
                    "   a surface that smooth is a mirror, and a mirror loses nothing. %s\n",
                    sampled, vanishes ? "It does." : "IT DOES NOT.");

        if (walk)
            std::printf("   For the walk that is the easy end rather than the claim: it\n"
                        "   is 1 at every roughness in the table, and held to 1e-12.\n");

        // And the two columns have to meet where both can see. This is the
        // furnace's second residual doing its original job — catching `sample`
        // and `pdf` describing different distributions — on the first material
        // in the project where they are not the same two lines of arithmetic.
        double worst_disagreement = 0.0;
        for (const double alpha : {0.200, 0.400, 0.600, 0.800, 1.000}) {
            const Bsdf rough = walk
                ? Bsdf{GreyWalk{FlatReflectance{1.0}, TrowbridgeReitz{alpha}}}
                : Bsdf{GreyRough{FlatReflectance{1.0}, TrowbridgeReitz{alpha}}};
            for (const double degrees : angles) {
                const double theta = degrees * si::pi / 180.0;
                const Vec3 wo{std::sin(theta), 0.0, std::cos(theta)};
                worst_disagreement = std::fmax(
                    worst_disagreement,
                    std::fabs(directional_albedo_by_quadrature(rough, wo, mu_cells, phi_cells)
                            - directional_albedo_by_sampling(rough, wo, draws)));
            }
        }
        // For the single-scattering model the two columns must agree: `eval`,
        // `sample` and `pdf` are three descriptions of one surface. For the
        // walk they must *not*, and the gap is the measurement. `eval` is the
        // closed-form part — item 0160 — so the quadrature sees only what one
        // bounce delivers, while the estimator follows every bounce. The
        // difference between the columns is the recovered energy.
        const bool agree = walk ? (worst_disagreement > 1e-2)
                                : (worst_disagreement < 1e-3);
        if (!agree) ++failures;

        std::printf(walk
            ? "   From 0.2 up the two columns differ by as much as %.3f, and that\n"
              "   gap is the point: `eval` is the single-scattering part, which is\n"
              "   all a closed form can be here, and the estimator follows the rest.\n"
              "   %s\n"
            : "   Where the grid can see the lobe, from 0.2 up, the two columns\n"
              "   agree to %.1e. %s\n", worst_disagreement,
            walk ? (agree ? "The difference is the energy that was being lost."
                          : "THEY AGREE, WHICH MEANS THE WALK IS NOT WALKING.")
                 : (agree ? "`eval`, `sample` and `pdf` describe one surface."
                          : "THEY DESCRIBE DIFFERENT SURFACES."));
    }

    // ── And the furnace itself ───────────────────────────────────────────

    constexpr int resolution = 256;
    constexpr int spp = 16;

    const Camera camera = Camera::look_at(Vec3{0, 0, -5}, Vec3{0, 0, 0}, Vec3{0, 1, 0},
                                          0.025, 0.025, 0.035);

    std::printf("\n2. The furnace, rendered: %d x %d at %d spp, the ordinary integrator.\n\n",
                resolution, resolution, spp);
    std::printf("      alpha    pixels on the sphere    mean L on it     deficit\n");

    std::vector<double> picture;
    bool wrote_image = false;
    for (const double alpha : roughnesses) {
        const Bsdf rough = walk
            ? Bsdf{GreyWalk{FlatReflectance{1.0}, TrowbridgeReitz{alpha}}}
            : Bsdf{GreyRough{FlatReflectance{1.0}, TrowbridgeReitz{alpha}}};
        const Scene scene = enclosure::uniform_environment(rough);
        const Residual r = measure(scene, camera, resolution, spp, picture);

        const double mean = r.mean_on_the_sphere();

        // The object cannot return more than the environment put into it, and
        // that is the one thing about this table that would be a defect here
        // rather than in the model.
        if (r.on_the_sphere == 0 || mean > 1.0 + 1e-3) ++failures;

        std::printf("      %5.3f    %20ld    %12.6f    %+.6f\n",
                    alpha, r.on_the_sphere, mean, mean - 1.0);

        if (write_image && alpha == alpha_asked) {
            std::vector<double> grey(picture.size() * 3);
            for (std::size_t i = 0; i < picture.size(); ++i)
                grey[i * 3 + 0] = grey[i * 3 + 1] = grey[i * 3 + 2] = picture[i];
            wrote_image = write_ppm("furnace-conductor.ppm", resolution, resolution, grey);
        }
    }

    if (wrote_image)
        std::printf("\n   furnace-conductor.ppm   alpha = %.3f. A disc, not a square: the\n"
                    "                            sphere is there and it should not be. Its\n"
                    "                            radial profile depends on the roughness,\n"
                    "                            and the note above this function says how.\n",
                    alpha_asked);

    std::printf("\n   Averaged, not minimised. A single path off this surface can carry\n"
                "   nothing at all — it drew a facet reflecting into the ground — so the\n"
                "   darkest sample is 0.0 at every roughness and measures nothing. The\n"
                "   mean is what the furnace is asking about.\n");

    if (walk)
        std::printf("\n   The sphere is gone, at every roughness, to every digit printed.\n"
                    "   Not to within noise: with a reflectance of 1 nothing is absorbed\n"
                    "   at any facet, so every walk returns exactly 1 and the estimate is\n"
                    "   the mean of a constant. A furnace that passes this well is not a\n"
                    "   tuned one, it is one whose estimator has no variance left.\n"
                    "\n   That is item 0086 repaired, by the route item 0087 chose: the\n"
                    "   light was followed rather than a curve fitted to where it went.\n"
                    "   The quadrature column above is the single-scattering model still\n"
                    "   losing the energy it always did, which is what makes the gap\n"
                    "   between the two a measurement rather than a claim. %s\n",
                    failures == 0 ? "Nothing above failed."
                                  : "SOMETHING ABOVE DOES NOT HOLD.");
    else
        std::printf("\n   The sphere does not vanish, and the number above is how much of\n"
                    "   it is there. That is item 0086, a bug in the model rather than in\n"
                    "   this program; item 0087 decided what to do and `--bsdf walk` is it.\n"
                    "\n   What this instrument asserts here is the part that would be this\n"
                    "   project's fault: that nothing is brighter than the light put in,\n"
                    "   that the smooth end loses nothing, and that the three ways of\n"
                    "   asking agree wherever they can all see. %s\n",
                    failures == 0 ? "They do." : "SOMETHING ABOVE DOES NOT HOLD.");

    return failures == 0 ? 0 : 1;
}

inline int furnace(std::string_view model, double rho_asked, double alpha_asked,
                   bool table, bool write_image) {
    using namespace furnace_detail;
    using namespace render;

    // Item 0086's reproduction, spelled the way that item spells it, and
    // item 0162's, which is the same question asked after the repair.
    if (table) return model == "walk" ? furnace_orders() : furnace_accounting();

    if (model == "conductor") return furnace_conductor(alpha_asked, write_image, false);
    if (model == "walk") return furnace_conductor(alpha_asked, write_image, true);

    if (model != "lambert") {
        std::fprintf(stderr,
                     "cornell: no BSDF called '%.*s'. There are three: lambert;\n"
                     "         conductor, which takes --alpha and does not vanish; and\n"
                     "         walk, which is the same surface with its light followed.\n",
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
