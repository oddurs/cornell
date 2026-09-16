// chi2.hpp — catching a sampling routine lying about its own density.
//
// Every BSDF in this project makes two claims that are written in different
// code and have to agree. `sample` draws a direction. `pdf` says how likely
// that draw was. `bsdf.hpp` argues at length that the second must exist
// separately from the first — v0.8's multiple importance sampling needs to
// evaluate one strategy's density on another strategy's direction — and the
// argument has a second half that arrives here:
//
//      Two independent claims about one distribution can be checked against
//      each other by a machine.
//
// Draw a few million directions. Histogram them. Integrate the density over
// each bin by a quadrature that never calls the sampler. If the two describe
// the same distribution, the counts are multinomial with those expectations,
// and Pearson's statistic has a chi-squared distribution whose p-value is a
// number rather than an impression.
//
// Almost every BSDF bug in existence is a disagreement between these two
// functions, and almost none of them are visible in an image until they are
// compared against something. A sampler that is slightly too fond of grazing
// angles renders a slightly wrong material, and the material looks like a
// material.
//
// ── What makes it a test and not a tautology ─────────────────────────────
//
// The reference must not come from the sampling routine. If the expected
// counts were obtained by drawing more samples, the test would compare a
// histogram against a histogram of the same thing and pass whatever either
// one did. So the expectation is a quadrature of `pdf` over the bin — `sample`
// is not called while computing it, and the two halves of the comparison
// share no code.
//
// ── Where the bins come from, and why they move ──────────────────────────
//
// The natural grid is uniform in `mu = cos(theta)` and `phi`, because `dw =
// dmu dphi` makes every cell the same solid angle and a uniform density
// produce a flat histogram. That grid is wrong at exactly the place a BSDF is
// most likely to be wrong.
//
// A cosine-weighted hemisphere puts almost nothing near the horizon, and a
// specular lobe in v0.7 will put almost nothing anywhere except one cell.
// Pearson's statistic needs an expected count of about five per cell to be
// chi-squared at all — below that the discreteness of the counts shows
// through and the p-value stops meaning what it says — so a fixed grid either
// has starved cells or is too coarse to see anything.
//
// So the cells are pooled: sweep the grid in order, accumulate observed and
// expected together, and close a cell when its expectation reaches the
// threshold. Cells in the dense part of the distribution stay at one bin
// each; the grazing tail becomes a few wide cells instead of hundreds of
// starved ones. The degrees of freedom are counted after pooling, from what
// the test actually used, which is the part that is easy to get wrong and
// silently generous.
//
// ── What a failure looks like ────────────────────────────────────────────
//
// A p-value is not a verdict and this file does not pretend otherwise. Under
// the null hypothesis it is uniform on [0, 1], so a run of correct samplers
// produces p = 0.03 about one time in thirty, and a threshold at 0.01 fails a
// correct BSDF once in a hundred. That is the cost of the test having power,
// and the answer is not to loosen the threshold until nothing ever fails:
// the seeds here are fixed, so a failure is reproducible and can be looked
// at rather than re-rolled.
//
// The failures that matter are not marginal. A missing Jacobian or a
// hemisphere flipped the wrong way produces p = 0 to every digit a double
// has, and the number below is 1e-300, not 0.004.
//
// ── What it cannot do ────────────────────────────────────────────────────
//
// It says nothing about whether `eval` is right. `sample` and `pdf` can agree
// perfectly with each other and with nothing in physics — a BSDF that samples
// uniformly and says it samples uniformly passes this test and is not a model
// of anything. `furnace.hpp` is the one that asks whether the value is right;
// this one asks whether the sampler is honest about itself.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string_view>
#include <vector>

#include <render/cornell.hpp>
#include <render/lambert.hpp>
#include <render/sampler.hpp>
#include <render/scene.hpp>
#include <render/si.hpp>

namespace app {

namespace chi2_detail {

using namespace render;

// ── The chi-squared distribution, from scratch ───────────────────────────
//
// House rule 4 leaves nowhere to get this from, and it is two functions.
//
// The p-value of Pearson's statistic is the probability that a correct
// sampler would produce a statistic at least this large, which is the upper
// tail of a chi-squared with k degrees of freedom:
//
//      p = Q(k/2, x/2)
//
// where Q is the regularised upper incomplete gamma function. The standard
// pair of expansions converges everywhere between them: a series for the
// lower tail when `x` is small relative to `a`, and a continued fraction for
// the upper tail otherwise. Both are Abramowitz and Stegun 6.5, and the
// crossover at `x < a + 1` is the usual one.

inline constexpr int gamma_iterations = 512;
inline constexpr double gamma_epsilon = 3.0e-16;

// The series, which computes the *lower* tail P(a, x) and is used where it
// converges quickly.
inline double gamma_p_series(double a, double x) {
    if (x <= 0.0) return 0.0;

    double term = 1.0 / a;
    double sum = term;
    for (int n = 1; n < gamma_iterations; ++n) {
        term *= x / (a + double(n));
        sum += term;
        if (std::fabs(term) < std::fabs(sum) * gamma_epsilon) break;
    }
    return sum * std::exp(-x + a * std::log(x) - std::lgamma(a));
}

// The continued fraction, in Lentz's modified form, which computes the upper
// tail directly. The tiny floor is Lentz's guard against a zero denominator,
// which is the failure mode this formulation is chosen to avoid.
inline double gamma_q_fraction(double a, double x) {
    constexpr double tiny = 1.0e-300;

    double b = x + 1.0 - a;
    double c = 1.0 / tiny;
    double d = 1.0 / b;
    double h = d;

    for (int i = 1; i < gamma_iterations; ++i) {
        const double an = -double(i) * (double(i) - a);
        b += 2.0;
        d = an * d + b;
        if (std::fabs(d) < tiny) d = tiny;
        c = b + an / c;
        if (std::fabs(c) < tiny) c = tiny;
        d = 1.0 / d;
        const double delta = d * c;
        h *= delta;
        if (std::fabs(delta - 1.0) < gamma_epsilon) break;
    }
    return h * std::exp(-x + a * std::log(x) - std::lgamma(a));
}

// The upper tail, whichever way round converges.
inline double gamma_q(double a, double x) {
    if (x < 0.0 || a <= 0.0) return std::nan("");
    if (x < a + 1.0) return 1.0 - gamma_p_series(a, x);
    return gamma_q_fraction(a, x);
}

// The p-value of a chi-squared statistic.
inline double chi_squared_p(double statistic, int degrees_of_freedom) {
    if (degrees_of_freedom <= 0) return std::nan("");
    return gamma_q(0.5 * double(degrees_of_freedom), 0.5 * statistic);
}

// ── The grid ─────────────────────────────────────────────────────────────
//
// Uniform in `mu` and `phi` over the whole sphere, not the hemisphere. A
// reflector puts zero density in half of it, and that half is worth binning
// anyway: a sample landing where the density says nothing can land is not a
// statistical event, it is a bug, and it is reported separately rather than
// being allowed to inflate a cell's chi-squared contribution.

inline constexpr int mu_bins = 32;      // over [-1, 1]
inline constexpr int phi_bins = 64;     // over [0, 2pi)
inline constexpr int bins = mu_bins * phi_bins;

// Where a direction falls. `mu` from -1 to 1, `phi` from 0 to 2pi.
inline int bin_of(const Vec3& w) {
    const double mu = std::clamp(w.z, -1.0, 1.0);
    double phi = std::atan2(w.y, w.x);
    if (phi < 0.0) phi += si::two_pi;

    int i = int((mu + 1.0) * 0.5 * double(mu_bins));
    int j = int(phi / si::two_pi * double(phi_bins));
    i = std::clamp(i, 0, mu_bins - 1);
    j = std::clamp(j, 0, phi_bins - 1);
    return i * phi_bins + j;
}

// The expected fraction of draws in each bin: the density integrated over the
// bin's solid angle. `dw = dmu dphi`, so this is a plain double integral over
// a rectangle, by a midpoint rule on a sub-grid.
//
// `sample` is not called anywhere in here. That is what makes this a test.
inline constexpr int sub_cells = 6;     // per axis, per bin

template <class Model>
std::vector<double> expected_fractions(const Model& model, const Vec3& wo) {
    std::vector<double> out(std::size_t(bins), 0.0);

    const double mu_width = 2.0 / double(mu_bins);
    const double phi_width = si::two_pi / double(phi_bins);
    const double cell = (mu_width / double(sub_cells)) * (phi_width / double(sub_cells));

    for (int i = 0; i < mu_bins; ++i) {
        for (int j = 0; j < phi_bins; ++j) {
            double total = 0.0;
            for (int a = 0; a < sub_cells; ++a) {
                const double mu = -1.0 + mu_width * (double(i) + (double(a) + 0.5) / double(sub_cells));
                const double sin_theta = std::sqrt(std::fmax(0.0, 1.0 - mu * mu));

                for (int b = 0; b < sub_cells; ++b) {
                    const double phi =
                        phi_width * (double(j) + (double(b) + 0.5) / double(sub_cells));
                    const Vec3 wi{sin_theta * std::cos(phi), sin_theta * std::sin(phi), mu};
                    total += model.pdf(wo, wi).per_steradian();
                }
            }
            out[std::size_t(i * phi_bins + j)] = total * cell;
        }
    }
    return out;
}

// A `Bsdf` variant, wearing the three-method contract its own alternatives
// wear. `scene.hpp` dispatches through free functions so that a model is a
// plain struct that knows nothing about the variant; this puts the variant
// back into the shape the test wants, which is the shape everything else in
// the project already has.
struct Dispatch {
    const Bsdf& bsdf;

    BsdfSample sample(const Vec3& wo, const Wavelengths& lambdas, double u, double v) const {
        return render::sample(bsdf, wo, lambdas, u, v);
    }
    Brdf eval(const Vec3& wo, const Vec3& wi, const Wavelengths& lambdas) const {
        return render::eval(bsdf, wo, wi, lambdas);
    }
    SolidAngleDensity pdf(const Vec3& wo, const Vec3& wi) const {
        return render::pdf(bsdf, wo, wi);
    }
};

// ── Two liars, so that a pass means something ────────────────────────────
//
// A test that has never failed is a test nobody has checked. These are not
// materials and they are not in `scene.hpp`'s variant; they exist here, in an
// app, to be caught.
//
// Both draw exactly what `lambert.hpp` draws — cosine-weighted, through the
// same concentric mapping — and then misreport the density. One does it
// grossly and one does it by two percent, which is the interesting one: a
// two-percent error in a density is invisible in an image, survives every
// furnace test, and is the size of mistake that actually happens.
template <int Numerator, int Denominator>
struct CosinePowerLiar {
    // The claim: cos^n, normalised over the hemisphere. It integrates to 1
    // for any n, so it is a perfectly good density — it is simply not the one
    // being drawn from, which is the only thing chi-squared is looking for.
    static constexpr double n = double(Numerator) / double(Denominator);

    BsdfSample sample(const Vec3& wo, const Wavelengths& lambdas, double u, double v) const {
        DirectionSample drawn = cosine_hemisphere(u, v);
        if (wo.z < 0.0) drawn.direction.z = -drawn.direction.z;
        return BsdfSample{drawn.direction, eval(wo, drawn.direction, lambdas),
                          pdf(wo, drawn.direction)};
    }

    Brdf eval(const Vec3& wo, const Vec3& wi, const Wavelengths&) const {
        if (!same_hemisphere(wo, wi)) return Brdf{};
        return per_steradian(Reflectance{1.0}, si::pi);
    }

    SolidAngleDensity pdf(const Vec3& wo, const Vec3& wi) const {
        if (!same_hemisphere(wo, wi)) return SolidAngleDensity{};
        const double mu = abs_cos_theta(wi);
        return SolidAngleDensity{(n + 1.0) * std::pow(mu, n) / si::two_pi};
    }
};

using ClaimsUniform  = CosinePowerLiar<0, 1>;      // cos^0: flat over the hemisphere
using ClaimsTwoPer   = CosinePowerLiar<102, 100>;  // cos^1.02, which is nearly right

// A third liar, for the check chi-squared cannot make. This one has the right
// *shape* — cosine-weighted, exactly what it draws — and half the magnitude,
// so it agrees with its own sampler perfectly and is not a density at all.
//
// Pearson's statistic is blind to it: the expected counts are the density
// times the number of draws, and scaling every expectation by a half and then
// renormalising to the draw count gives back the same expectations. It is
// `verify.hpp`'s normalisation section that catches this one, which is why
// that section exists next to a test that appears to cover it.
struct HalfADensity {
    BsdfSample sample(const Vec3& wo, const Wavelengths& lambdas, double u, double v) const {
        DirectionSample drawn = cosine_hemisphere(u, v);
        if (wo.z < 0.0) drawn.direction.z = -drawn.direction.z;
        return BsdfSample{drawn.direction, eval(wo, drawn.direction, lambdas),
                          pdf(wo, drawn.direction)};
    }

    Brdf eval(const Vec3& wo, const Vec3& wi, const Wavelengths&) const {
        if (!same_hemisphere(wo, wi)) return Brdf{};
        return per_steradian(Reflectance{1.0}, si::pi);
    }

    SolidAngleDensity pdf(const Vec3& wo, const Vec3& wi) const {
        if (!same_hemisphere(wo, wi)) return SolidAngleDensity{};
        return SolidAngleDensity{0.5 * abs_cos_theta(wi) * si::inv_pi};
    }
};

// And a fourth, for reciprocity. It weights the incoming direction and not
// the outgoing one, which is the shape of every BRDF somebody has invented by
// multiplying a cosine into the wrong place — and it is a perfectly good
// density's worth of nonsense: it conserves nothing in particular, it samples
// honestly, and swapping its arguments changes the answer.
struct NotReciprocal {
    BsdfSample sample(const Vec3& wo, const Wavelengths& lambdas, double u, double v) const {
        DirectionSample drawn = cosine_hemisphere(u, v);
        if (wo.z < 0.0) drawn.direction.z = -drawn.direction.z;
        return BsdfSample{drawn.direction, eval(wo, drawn.direction, lambdas),
                          pdf(wo, drawn.direction)};
    }

    Brdf eval(const Vec3& wo, const Vec3& wi, const Wavelengths&) const {
        if (!same_hemisphere(wo, wi)) return Brdf{};
        return per_steradian(Reflectance{0.5 * (1.0 + abs_cos_theta(wi))}, si::pi);
    }

    SolidAngleDensity pdf(const Vec3& wo, const Vec3& wi) const {
        if (!same_hemisphere(wo, wi)) return SolidAngleDensity{};
        return cosine_hemisphere_pdf(abs_cos_theta(wi));
    }
};

// ── The test ─────────────────────────────────────────────────────────────

struct Result {
    double statistic = 0.0;
    int cells = 0;                  // after pooling
    int degrees_of_freedom = 0;
    double p = 0.0;
    long draws = 0;
    long impossible = 0;            // samples where the density says zero
    double total_density = 0.0;     // the quadrature's own integral of pdf
};

// The seeds below are spelled as `std::uint64_t` constants rather than as
// `ULL` literals, and that is not fussiness. On Linux `std::uint64_t` is
// `unsigned long` and `ULL` is `unsigned long long` — the same width, a
// different type — so `literal * value` is a conversion between them, and
// `-Wconversion` is right to say so. clang's headers make the two agree and
// clang says nothing, which is how a warning survives a local build and dies
// in CI under the other compiler.
inline constexpr std::uint64_t seed_base = 0x9e37'79b9'7f4a'7c15;
inline constexpr std::uint64_t seed_liar = 0x5'deec'e66d;
inline constexpr std::uint64_t seed_same = 0xd1b5'4a32'd192'ed03;

// The minimum expected count a pooled cell must reach. Five is the textbook
// figure and it is a rule of thumb rather than a theorem; what it is
// protecting is the approximation of a multinomial by a normal, which is what
// makes Pearson's statistic chi-squared.
inline constexpr double minimum_expected = 5.0;

template <class Model>
Result test(const Model& model, const Vec3& wo, int draws, std::uint64_t seed) {
    Result out;
    out.draws = draws;

    const std::vector<double> fraction = expected_fractions(model, wo);
    for (const double f : fraction) out.total_density += f;

    // The histogram. Addressed rather than dispensed, so that this figure
    // does not depend on the order anything ran in.
    std::vector<long> observed(std::size_t(bins), 0);
    const Wavelengths lambdas = Wavelengths::sample(0.5);

    for (int i = 0; i < draws; ++i) {
        Sampler sampler{seed, std::uint64_t(i)};
        const auto [u, v] = sampler.next2();

        const BsdfSample drawn = model.sample(wo, lambdas, u, v);
        if (drawn.is_black()) continue;

        const int b = bin_of(drawn.wi);
        if (fraction[std::size_t(b)] <= 0.0) ++out.impossible;
        ++observed[std::size_t(b)];
    }

    // Pooling, in grid order. A cell closes when its expectation reaches the
    // threshold; whatever is left at the end joins the last closed cell,
    // because a final starved cell is exactly the thing being avoided.
    double pooled_observed = 0.0;
    double pooled_expected = 0.0;
    std::vector<std::pair<double, double>> cells;

    for (int b = 0; b < bins; ++b) {
        pooled_observed += double(observed[std::size_t(b)]);
        pooled_expected += fraction[std::size_t(b)] * double(draws);
        if (pooled_expected >= minimum_expected) {
            cells.emplace_back(pooled_observed, pooled_expected);
            pooled_observed = 0.0;
            pooled_expected = 0.0;
        }
    }
    if (pooled_expected > 0.0 && !cells.empty()) {
        cells.back().first += pooled_observed;
        cells.back().second += pooled_expected;
    }

    for (const auto& [o, e] : cells) {
        const double residual = o - e;
        out.statistic += residual * residual / e;
    }

    out.cells = int(cells.size());

    // One constraint: the counts sum to the number of draws. Nothing here is
    // fitted from the data, so that is the only one.
    out.degrees_of_freedom = out.cells - 1;
    out.p = chi_squared_p(out.statistic, out.degrees_of_freedom);
    return out;
}

} // namespace chi2_detail

// ── The instrument ───────────────────────────────────────────────────────

inline int chi2() {
    using namespace chi2_detail;
    using namespace render;

    std::printf("Chi-squared: that sample() and pdf() describe the same distribution.\n\n"
                "A million draws per row, histogrammed over %d x %d cells of the whole\n"
                "sphere, against the density integrated over each cell by a %d x %d\n"
                "midpoint quadrature that never calls the sampler. Cells are pooled in\n"
                "grid order until each expects at least %.0f draws, and the degrees of\n"
                "freedom are counted from what is left.\n\n",
                mu_bins, phi_bins, sub_cells, sub_cells, minimum_expected);

    // Every BSDF in the project. They share a sampling routine, and that is
    // worth saying rather than testing one and assuming: the variant is what
    // `scene.hpp` closes, so the variant is what gets tested, and the day an
    // alternative stops sharing it this table is what notices.
    const Bsdf grey{GreyLambert{Flat{0.5}}};
    const Bsdf spectral{SpectralLambert{cie::d65}};
    const Bsdf measured{MeasuredLambert{render::cornell::red}};

    struct Row {
        std::string_view name;
        const Bsdf* bsdf;
        double degrees;     // incident angle from the normal
        bool from_below;    // wo in the lower hemisphere
    };

    // One seed per row, fixed. Fixed matters twice: a failure is reproducible
    // rather than re-rollable, and the whole table is deterministic, so a
    // p-value that passes here passes in CI forever. The one-in-a-hundred
    // false failure a threshold of 0.01 admits is a single draw taken once,
    // not a flake waiting in every build.
    const Row rows[] = {
        {"lambert (grey)",     &grey,     0.0,  false},
        {"lambert (grey)",     &grey,     30.0, false},
        {"lambert (grey)",     &grey,     60.0, false},
        {"lambert (grey)",     &grey,     85.0, false},
        {"lambert (grey)",     &grey,     30.0, true },
        {"lambert (d65)",      &spectral, 45.0, false},
        {"lambert (measured)", &measured, 45.0, false},
    };

    constexpr int draws = 1 << 20;

    std::printf("  %-20s %8s  %9s  %5s  %10s  %10s  %s\n",
                "bsdf", "theta_o", "chi2", "dof", "chi2/dof", "p", "");

    int failures = 0;
    for (const Row& row : rows) {
        const double theta = row.degrees * si::pi / 180.0;
        const double z = std::cos(theta) * (row.from_below ? -1.0 : 1.0);
        const Vec3 wo{std::sin(theta), 0.0, z};

        // A different seed per row, fixed, so that a failure is reproducible
        // and a pass is not the third attempt.
        const Result r = test(Dispatch{*row.bsdf}, wo, draws,
                              seed_base * (std::uint64_t(&row - rows) + 1));

        const bool ok = r.p > 0.01 && r.impossible == 0;
        if (!ok) ++failures;

        std::printf("  %-20s %5.0f%s  %9.2f  %5d  %10.4f  %10.4f  %s\n",
                    row.name.data(), row.degrees, row.from_below ? "°*" : "° ",
                    r.statistic, r.degrees_of_freedom,
                    r.statistic / double(r.degrees_of_freedom), r.p,
                    ok ? "pass" : (r.impossible > 0 ? "IMPOSSIBLE DRAW" : "FAIL"));
    }

    std::printf("\n  * wo in the lower hemisphere: the sampler follows it, and must.\n");

    // ── Two things a p-value can only hint at ────────────────────────────
    //
    // Both of these would show up above as suspiciously equal chi-squareds,
    // and "suspiciously equal" is not a measurement. Asked directly they are
    // exact statements with an exact answer.
    {
        const Wavelengths lambdas = Wavelengths::sample(0.5);
        const Vec3 up{std::sin(0.6), 0.0, std::cos(0.6)};
        const Vec3 down{up.x, up.y, -up.z};

        long alternatives_differ = 0;
        long mirror_differs = 0;

        for (int i = 0; i < draws; ++i) {
            Sampler sampler{seed_same, std::uint64_t(i)};
            const auto [u, v] = sampler.next2();

            const Vec3 a = sample(grey, up, lambdas, u, v).wi;
            const Vec3 b = sample(spectral, up, lambdas, u, v).wi;
            const Vec3 c = sample(measured, up, lambdas, u, v).wi;
            if (a.x != b.x || a.y != b.y || a.z != b.z) ++alternatives_differ;
            if (a.x != c.x || a.y != c.y || a.z != c.z) ++alternatives_differ;

            const Vec3 m = sample(grey, down, lambdas, u, v).wi;
            if (m.x != a.x || m.y != a.y || m.z != -a.z) ++mirror_differs;
        }

        std::printf("\n  The three alternatives of `Bsdf` share Lambert's sampling routine, so\n"
                    "  from one seed they must draw one direction. Over %d draws, compared\n"
                    "  bit for bit:  %ld differ.\n", draws, alternatives_differ);

        std::printf("\n  And a `wo` below the horizon draws the mirror image of the one above,\n"
                    "  because `lambert.hpp` negates a z rather than sampling again — which is\n"
                    "  exact, or it is a second distribution nobody tested:  %ld differ.\n",
                    mirror_differs);
    }

    // ── The pooling, made visible ────────────────────────────────────────
    //
    // At a million draws nothing is starved and every cell is one bin, so the
    // adaptive part of the binning is doing nothing that can be seen. Run it
    // where it matters.
    {
        std::printf("\n  Cells are pooled until each expects %.0f draws, so the number of them\n"
                    "  depends on how many draws there are. Same BSDF, same grid:\n\n"
                    "      draws     cells    dof      chi2/dof         p\n", minimum_expected);

        for (int draw_count : {256, 1024, 8192, 65536, 1 << 20}) {
            const Result r = test(Dispatch{grey}, Vec3{0.0, 0.0, 1.0}, draw_count, seed_liar);
            std::printf("    %7d  %8d %6d  %12.4f  %8.4f\n", draw_count, r.cells,
                        r.degrees_of_freedom, r.statistic / double(r.degrees_of_freedom), r.p);
        }

        std::printf("\n  %d bins is the grid; half of it has zero density because a reflector\n"
                    "  scatters into one hemisphere, and those bins are absorbed into the next\n"
                    "  cell rather than contributing a division by zero.\n", bins);
    }

    // The quadrature's own claim, which is item 0067's and is free here: a
    // density integrated over its whole domain is 1. If this were wrong every
    // expectation above would be wrong by the same factor, and the chi-squared
    // would not notice, because Pearson compares shapes after both sides have
    // been scaled by the number of draws.
    std::printf("\n  The same quadrature, integrated over the whole sphere — which is the\n"
                "  one thing chi-squared cannot see, because it compares shapes. Shown as\n"
                "  the deviation from 1, because a printed 1.000000 is not evidence:\n\n");
    for (const Row& row : rows) {
        const double theta = row.degrees * si::pi / 180.0;
        const double z = std::cos(theta) * (row.from_below ? -1.0 : 1.0);
        const Vec3 wo{std::sin(theta), 0.0, z};
        const std::vector<double> fraction = expected_fractions(Dispatch{*row.bsdf}, wo);
        double total = 0.0;
        for (const double f : fraction) total += f;
        std::printf("    %-20s %5.0f%s   %+.3e\n", row.name.data(), row.degrees,
                    row.from_below ? "°*" : "° ", total - 1.0);
    }

    // ── What it catches ──────────────────────────────────────────────────
    //
    // The calibration. Two samplers that draw exactly what Lambert draws and
    // then misreport the density, at the same draw count as the table above.
    // If these passed, nothing above would mean anything.
    {
        std::printf("\n  And the calibration: two samplers that draw what Lambert draws and\n"
                    "  then misreport the density. Same grid, same %d draws.\n\n"
                    "      claimed density                    chi2/dof             p\n", draws);

        const Vec3 wo{0.0, 0.0, 1.0};
        const Result uniform = test(ClaimsUniform{}, wo, draws, seed_liar);
        const Result nearly  = test(ClaimsTwoPer{},  wo, draws, seed_liar);

        std::printf("      cos^0  (flat)                  %12.1f  %12.3e\n"
                    "      cos^1.02  (2%% too steep)       %12.1f  %12.3e\n",
                    uniform.statistic / double(uniform.degrees_of_freedom), uniform.p,
                    nearly.statistic / double(nearly.degrees_of_freedom), nearly.p);

        std::printf("\n  How many draws it takes to catch them, which is the reason the tables\n"
                    "  above are a million rows deep and not a thousand:\n\n"
                    "        draws        cos^1.02 p      cos^0 p\n");
        for (int n : {1 << 14, 1 << 16, 1 << 18, 1 << 19, 1 << 20, 1 << 21, 1 << 22}) {
            std::printf("    %9d       %10.3e   %10.3e\n", n,
                        test(ClaimsTwoPer{},  wo, n, seed_liar).p,
                        test(ClaimsUniform{}, wo, n, seed_liar).p);
        }
        std::printf("\n  A gross error is caught by the first row and every row after it. A two\n"
                    "  percent one needs about a million draws, and would have been passed at\n"
                    "  a quarter of that — which is what a chi-squared test's *power* means,\n"
                    "  and is a property of the sample count rather than of the threshold.\n");

        if (uniform.p > 0.01 || nearly.p > 0.01) {
            std::printf("\n  Both of those should be zero. The test has no power and the table\n"
                        "  above proves nothing.\n");
            ++failures;
        } else {
            std::printf("\n  Both refused. A two-percent error in a density is invisible in an\n"
                        "  image and survives the furnace; it does not survive this.\n");
        }
    }

    std::printf("\n%s\n", failures == 0
                ? "Every sampler agrees with its own density, and the two that do not were caught."
                : "A sampler and its density disagree.");
    return failures == 0 ? 0 : 1;
}

} // namespace app
