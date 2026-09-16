// converge.hpp — the slope is minus one half, or the estimator is biased.
//
// Monte Carlo error falls as the inverse square root of the sample count.
// That is not a rule of thumb and not a property of this renderer; it is the
// central limit theorem, and it holds for any unbiased estimator of anything.
// The consequence is the most useful diagnostic in rendering:
//
//      A renderer whose error falls faster than N^-1/2 is not clever.
//      It is biased, or it is being measured against its own bias.
//
// Clamping a firefly, capping a path at four bounces, adding an ambient term,
// blurring the indirect light — every one of those makes an image look
// quieter at low sample counts and none of them converges to the right
// answer. They show up here as a slope that is too steep at the low end and
// then flattens out, because the estimate stops moving towards the truth and
// starts moving towards something else.
//
// ── The reference is the whole problem ───────────────────────────────────
//
// The usual way to measure this is to render the same image at 16, 64, 256
// and 1024 samples, compare each against a 64k-sample render, and fit a line.
// It is easy and it is half blind: the reference came out of the same code,
// so anything both of them get wrong cancels. A renderer that converges
// beautifully to the wrong picture passes.
//
// So the first table below does not use a rendered reference. It uses a scene
// with a closed form — `enclosure.hpp`'s cavity, where every wall emits `Le`
// and reflects `rho`, the radiance inside is isotropic and satisfies
// `L = Le + rho·L`, and therefore
//
//      L = Le / (1 - rho)
//
// exactly, for the reader, before the renderer is asked. The error in that
// table is a distance from a number nobody rendered. If the transport had a
// systematic error, this is where it would show: the slope would bend, or the
// line would sit at the wrong height and stay there.
//
// The second table is the Cornell box, against a long render, and it is the
// one with the caveat. It is kept because the cavity is not a scene — it has
// no small bright light, no visibility, no geometry to speak of — and a claim
// about the estimator is not automatically a claim about the renderer.
//
// ── What the roulette has to do with it ──────────────────────────────────
//
// Russian roulette is the one thing in this project that was built to be
// unbiased and had no instrument to prove it. `roulette.hpp` argues from
// expectation algebra that killing a path with probability `1-q` and scaling
// the survivors by `1/q` leaves the mean alone, and `transport.hpp` measured
// that the mean does not move. Neither of them measured the *slope*.
//
// That is a separate claim and a stronger one. A biased estimator can have
// the right mean at one sample count by luck; what it cannot do is keep
// falling as N^-1/2 towards the right answer. So every row below is run twice,
// with the roulette on and off, and what should change is the constant and
// not the exponent — roulette trades bias for variance, so the line moves up
// and stays parallel.
//
// ── How the slope is fitted, and what it is not ──────────────────────────
//
// Least squares on `log(rmse)` against `log(N)`. No weighting, because the
// RMSE at each N is estimated from the same number of independent batches and
// the log-scale uncertainty is therefore about the same at every point.
//
// The fit is not a hypothesis test and this file does not dress it as one. An
// RMSE estimated from a few hundred batches has a few percent of noise in it,
// so a measured slope of -0.49 or -0.51 is what a correct estimator looks
// like. What a broken one looks like is -0.7, or -0.5 that goes to -0.1 at
// the top end, and neither of those needs a confidence interval to see.

#pragma once

#include <cmath>
#include <cstdio>
#include <vector>

#include <render/basis.hpp>
#include <render/sampler.hpp>
#include <render/scene.hpp>
#include <render/si.hpp>
#include <render/transport.hpp>
#include <render/warp.hpp>

#include "enclosure.hpp"
#include "render.hpp"

namespace app {

namespace converge_detail {

using namespace render;

// The roulette, off. Not a flag: `transport.hpp` starts the roulette at a
// depth, so a depth beyond the limit is a roulette that never starts, which
// is what off means rather than a special case that has to be maintained.
inline constexpr int roulette_off = default_max_depth + 1;

// ── The least-squares slope of log(rmse) against log(N) ──────────────────

struct Line {
    double slope = 0.0;
    double intercept = 0.0;
};

inline Line fit(const std::vector<double>& n, const std::vector<double>& rmse) {
    const double count = double(n.size());
    double sum_x = 0.0, sum_y = 0.0, sum_xx = 0.0, sum_xy = 0.0;

    for (std::size_t i = 0; i < n.size(); ++i) {
        const double x = std::log(n[i]);
        const double y = std::log(rmse[i]);
        sum_x += x;
        sum_y += y;
        sum_xx += x * x;
        sum_xy += x * y;
    }

    const double slope = (count * sum_xy - sum_x * sum_y) / (count * sum_xx - sum_x * sum_x);
    return Line{slope, (sum_y - slope * sum_x) / count};
}

// ── The cavity, against its closed form ──────────────────────────────────

// One estimate: the mean of `paths` paths leaving the centre of the cavity in
// uniformly random directions. This is the ordinary integrator — the same
// `radiance()` the camera calls, given a ray that did not come from a camera.
inline double estimate(const Scene& scene, int paths, std::uint64_t batch,
                       int roulette_start) {
    double total = 0.0;

    for (int i = 0; i < paths; ++i) {
        Sampler sampler{batch, std::uint64_t(i)};

        // Uniform on the sphere, from `mu` and `phi`, which is the mapping
        // `warp.hpp` derives the cosine-weighted one against.
        const auto [u, v] = sampler.next2();
        const double mu = 2.0 * u - 1.0;
        const double radius = std::sqrt(std::fmax(0.0, 1.0 - mu * mu));
        const double phi = si::two_pi * v;
        const Unit direction = normalize(
            Vec3{radius * std::cos(phi), radius * std::sin(phi), mu});

        const Wavelengths lambdas = Wavelengths::sample(sampler.next());
        const Radiance carried = radiance(scene, Ray{Vec3{0, 0, 0}, direction},
                                          lambdas, sampler, default_max_depth,
                                          roulette_start);
        total += carried[0];
    }
    return total / double(paths);
}

// The root mean square error of that estimate against the exact answer, over
// independent batches. Independent because each batch addresses the sampler
// at a different point rather than continuing a stream, which is the property
// v0.2 bought and this is the first instrument to need.
inline double cavity_rmse(const Scene& scene, double exact, int paths,
                          int batches, int roulette_start, std::uint64_t seed) {
    double sum_squared = 0.0;

    for (int b = 0; b < batches; ++b) {
        const double error = estimate(scene, paths, seed + std::uint64_t(b) * 0x9e37'79b9u,
                                      roulette_start) - exact;
        sum_squared += error * error;
    }
    return std::sqrt(sum_squared / double(batches));
}

// ── The box, against a long render ───────────────────────────────────────

// The difference between two films, over the luminance of every pixel. Y
// rather than all three tristimulus values because a slope is one number and
// three slopes that agree would be three ways of saying it.
//
// It accumulates rather than returning, because a single pair of 48 x 32
// images gives 1536 error samples and that is not enough. See `Spread`.
struct Spread {
    double sum_squared = 0.0;
    double sum_fourth = 0.0;
    long samples = 0;

    double rmse() const { return std::sqrt(sum_squared / double(samples)); }

    // How heavy the tail of the error distribution is. A normal distribution
    // has 3; this one does not, and the number is the reason the error bar
    // below is what it is rather than a guess.
    double kurtosis() const {
        const double variance = sum_squared / double(samples);
        return (sum_fourth / double(samples)) / (variance * variance);
    }

    // The standard error of `log(rmse)`, which is what the slope is fitted
    // to. The variance of a sample second moment is `(kurtosis - 1)/n` times
    // its square, the log halves it, and the square root turns it into a
    // standard error. This is where a heavy tail costs: at kurtosis 290 the
    // same number of pixels buys a tenth of the precision it buys at 3.
    double log_error() const {
        return 0.5 * std::sqrt(std::fmax(0.0, kurtosis() - 1.0) / double(samples));
    }
};

inline void accumulate(Spread& spread, const Film& a, const Film& b) {
    for (int y = 0; y < a.height(); ++y) {
        for (int x = 0; x < a.width(); ++x) {
            const double error = a.mean_tristimulus(x, y).y - b.mean_tristimulus(x, y).y;
            const double squared = error * error;
            spread.sum_squared += squared;
            spread.sum_fourth += squared * squared;
            ++spread.samples;
        }
    }
}

inline double film_rmse(const Film& a, const Film& b) {
    Spread spread;
    accumulate(spread, a, b);
    return spread.rmse();
}

// The standard error of a least-squares slope, given the standard error of
// each `y`. Ordinary least squares makes the slope a fixed linear combination
// of the `y` values, so its variance is that combination applied to theirs.
inline double slope_error(const std::vector<double>& n, const std::vector<double>& log_error) {
    double mean_x = 0.0;
    for (const double value : n) mean_x += std::log(value);
    mean_x /= double(n.size());

    double sxx = 0.0, weighted = 0.0;
    for (std::size_t i = 0; i < n.size(); ++i) {
        const double centred = std::log(n[i]) - mean_x;
        sxx += centred * centred;
        weighted += centred * centred * log_error[i] * log_error[i];
    }
    return std::sqrt(weighted) / sxx;
}

} // namespace converge_detail

// ── The instrument ───────────────────────────────────────────────────────

inline int converge() {
    using namespace converge_detail;
    using namespace render;

    std::printf("Convergence: RMSE falls as N^-1/2, or the estimator is biased.\n\n");

    constexpr int paths[] = {1, 4, 16, 64, 256, 1024};
    constexpr int batches = 2048;

    // ── 1. Against a closed form ─────────────────────────────────────────

    std::printf("1. A closed cavity, against Le / (1 - rho), which is exact.\n"
                "   %d independent batches per point; no reference was rendered.\n\n",
                batches);

    struct Case {
        double rho;
        int roulette_start;
        const char* label;
    };

    // Roulette on only, and the reason is the second table below: with it off
    // this scene has no variance at all, so there is nothing here to converge.
    const Case cases[] = {
        {0.5, roulette_start_depth, "rho = 0.5"},
        {0.9, roulette_start_depth, "rho = 0.9"},
    };

    std::printf("   %-26s", "");
    for (const int n : paths) std::printf("%11d", n);
    std::printf("%12s\n", "slope");

    int failures = 0;
    for (const Case& c : cases) {
        const Scene cavity = enclosure::emitting_cavity(c.rho);
        const double exact = enclosure::cavity_radiance(c.rho);

        std::vector<double> x, y;
        std::printf("   %-26s", c.label);
        for (const int n : paths) {
            const double rmse = cavity_rmse(cavity, exact, n, batches, c.roulette_start,
                                            0x243f'6a88'85a3'08d3);
            std::printf("%11.5f", rmse);
            x.push_back(double(n));
            y.push_back(rmse);
        }

        const Line line = fit(x, y);
        const bool ok = std::fabs(line.slope + 0.5) < 0.05;
        if (!ok) ++failures;
        std::printf("%12.4f  %s\n", line.slope, ok ? "" : "NOT -1/2");
    }

    // ── Why the roulette is not switched off here ────────────────────────
    //
    // Because with it off this scene has no variance. `transport.hpp` says
    // why: cosine-sampling a Lambertian makes `f · cos / pdf` exactly `rho`
    // for every draw, so in a cavity with uniform emission every path returns
    // the identical geometric series and the estimator is deterministic. The
    // only error left is the depth limit cutting the series off.
    //
    // That is worth printing rather than asserting, because a column of zeros
    // in a convergence table looks like a bug and is the correct answer.
    {
        std::printf("\n   Switch the roulette off and this scene stops being random. Every\n"
                    "   path returns the same geometric series, because cosine-sampling a\n"
                    "   Lambertian makes f cos / pdf exactly rho for every draw, so what is\n"
                    "   left is the depth limit cutting the series short at %d bounces:\n\n"
                    "        rho     rmse over %d batches     rho^%d / (1 - rho)\n",
                    default_max_depth, batches, default_max_depth);

        for (const double rho : {0.5, 0.9}) {
            const Scene cavity = enclosure::emitting_cavity(rho);
            const double exact = enclosure::cavity_radiance(rho);
            const double rmse = cavity_rmse(cavity, exact, 64, batches, roulette_off,
                                            0x243f'6a88'85a3'08d3);
            std::printf("       %4.2f          %10.3e            %10.3e\n",
                        rho, rmse, std::pow(rho, double(default_max_depth)) / (1.0 - rho));
        }

        std::printf("\n   Which is why the on-and-off comparison the roulette needs happens\n"
                    "   in the box below, where there is noise either way.\n");
    }

    // ── 2. Against a long render ─────────────────────────────────────────

    std::printf("\n2. The Cornell box, which has no closed form. Independent renders at\n"
                "   the same sample count, differenced against each other — no reference\n"
                "   at all. Section 3 is why not.\n\n");

    RenderSettings settings;
    settings.width = 48;
    const int height = height_for(settings.width);
    const Scene box = cornell::box();
    const Camera camera = Camera::look_at(cornell::at(278.0, 273.0, -800.0),
                                          cornell::at(278.0, 273.0, 0.0),
                                          Vec3{0.0, 1.0, 0.0},
                                          film_width, film_height, film_distance);

    constexpr int spps[] = {8, 32, 128, 512};
    constexpr int pairs = 4;

    // Two renders at N samples, on disjoint stretches of the sampler, differ
    // by the sum of two independent errors: the RMSE between them is sqrt(2)
    // times the RMSE of either against the truth. A constant factor is the
    // one thing a slope does not see, and it costs no reference.
    //
    // Four pairs rather than one, because 1536 pixels is not enough error
    // samples when the errors have the tail this scene's do. The kurtosis
    // column is how that shows.
    std::printf("        spp    roulette on   kurtosis    roulette off   kurtosis\n");

    std::vector<double> x, on, off, on_error, off_error;
    for (const int spp : spps) {
        settings.spp = spp;

        const auto measure = [&](int roulette) {
            settings.roulette_start = roulette;
            Spread spread;
            for (int pair = 0; pair < pairs; ++pair) {
                settings.sample_offset = spp * (2 * pair);
                const Film a = expose(settings, box, camera, height);
                settings.sample_offset = spp * (2 * pair + 1);
                const Film b = expose(settings, box, camera, height);
                accumulate(spread, a, b);
            }
            settings.sample_offset = 0;
            return spread;
        };

        const Spread a = measure(roulette_start_depth);
        const Spread b = measure(roulette_off);

        std::printf("   %8d     %11.4e %10.1f     %11.4e %10.1f\n",
                    spp, a.rmse(), a.kurtosis(), b.rmse(), b.kurtosis());

        x.push_back(double(spp));
        on.push_back(a.rmse());
        off.push_back(b.rmse());
        on_error.push_back(a.log_error());
        off_error.push_back(b.log_error());
    }
    settings.roulette_start = roulette_start_depth;

    const Line line_on = fit(x, on);
    const Line line_off = fit(x, off);
    const double error_on = slope_error(x, on_error);
    const double error_off = slope_error(x, off_error);

    std::printf("\n      slope   %+.4f +/- %.4f    %+.4f +/- %.4f\n",
                line_on.slope, error_on, line_off.slope, error_off);

    // Three standard errors, and the error bar is measured rather than
    // chosen. A tolerance picked by hand is a tolerance picked after seeing
    // the answer; this one comes from the kurtosis of the errors that were
    // actually observed, which is the honest statement of how well 1536
    // pixels times four pairs can pin a slope down in a scene with fireflies
    // in it.
    const bool ok = std::fabs(line_on.slope + 0.5) < 3.0 * error_on
                 && std::fabs(line_off.slope + 0.5) < 3.0 * error_off;
    if (!ok) ++failures;

    std::printf("\n   %s\n", ok
                ? "Both are -1/2 to within three standard errors, and equal to each other."
                : "A SLOPE IS NOT -1/2.");

    std::printf("\n   The roulette moves the line and not its slope, which is the claim\n"
                "   `roulette.hpp` makes from expectation algebra and had no instrument\n"
                "   for. Unbiased means converging to the same answer at the same rate,\n"
                "   not merely having the right mean once.\n"
                "\n   And the kurtosis columns are not decoration. A normal distribution\n"
                "   has 3. These errors have twenty to a few hundred, because a pixel's\n"
                "   error here is mostly a question of whether it caught a rare path that\n"
                "   found the lamp and came back enormous — and one such path is enough\n"
                "   to move an RMSE. A single pair at 2048 samples measured kurtosis 290,\n"
                "   a worst pixel 27 times the RMSE, and an RMSE a quarter too high; the\n"
                "   same pair at 8192 measured kurtosis 20 and landed back on the line.\n"
                "   Those are fireflies, item 0070 is where they get answered, and until\n"
                "   then this instrument pools four pairs and prints its own error bar.\n");

    // ── 3. Why there is no reference render ──────────────────────────────
    //
    // Because the obvious method is wrong twice, and both ways were measured
    // here rather than reasoned about afterwards.
    {
        constexpr int reference_spp = 1024;

        std::printf("\n3. The usual method, and what it does. Render a reference at %d\n"
                    "   samples and measure everything against it:\n\n", reference_spp);

        settings.spp = reference_spp;
        settings.sample_offset = 0;
        const Film shared = expose(settings, box, camera, height);
        settings.sample_offset = reference_spp;
        const Film disjoint = expose(settings, box, camera, height);
        settings.sample_offset = 0;

        std::vector<double> rx, r_shared, r_disjoint;
        std::printf("        spp    same samples    disjoint samples\n");
        for (const int spp : {8, 32, 128, 512}) {
            settings.spp = spp;
            const Film image = expose(settings, box, camera, height);
            const double a = film_rmse(image, shared);
            const double b = film_rmse(image, disjoint);
            std::printf("   %8d     %11.4e         %11.4e\n", spp, a, b);
            rx.push_back(double(spp));
            r_shared.push_back(a);
            r_disjoint.push_back(b);
        }

        std::printf("\n      slope    %+.4f            %+.4f\n",
                    fit(rx, r_shared).slope, fit(rx, r_disjoint).slope);

        std::printf("\n   Neither is -1/2, and they are wrong in opposite directions.\n\n"
                    "   The first column shares draws with its reference. The sampler is\n"
                    "   addressed by (pixel, sample), which is what makes the thread count\n"
                    "   irrelevant to the answer — and it means a %d-sample render\n"
                    "   *contains* a 512-sample render, draw for draw. The errors are not\n"
                    "   independent, they partly cancel, and they cancel more the closer N\n"
                    "   gets to the reference. The slope comes out too steep, which is the\n"
                    "   signature this instrument exists to raise the alarm about, produced\n"
                    "   here by the measurement rather than by the renderer.\n\n"
                    "   The second column fixes that and is still not -1/2, because a\n"
                    "   reference has noise of its own. The measured error is\n"
                    "   sqrt(sigma^2/N + sigma^2/R), and the second term is a floor: at\n"
                    "   N = 512 against R = %d it is a third of the total, so the line\n"
                    "   flattens exactly where the estimate was getting good.\n\n"
                    "   Differencing two equal-length renders has neither problem. Both\n"
                    "   terms are sigma^2/N, the sum is a constant factor of sqrt(2), and a\n"
                    "   constant factor is the one thing a slope does not see.\n",
                    reference_spp, reference_spp);
    }

    std::printf("\n%s\n", failures == 0
                ? "Error falls as the inverse square root of the sample count."
                : "A SLOPE IS NOT -1/2.");
    return failures == 0 ? 0 : 1;
}

} // namespace app
