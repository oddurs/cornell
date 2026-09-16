// roulette.hpp — how a path ends, without anything being thrown away.
//
// Everything else in `transport.hpp` is about the equation being solved. This
// is about stopping, which is a different question and turns out to have a
// better answer than the obvious one: a technique for terminating an
// estimator early that does not change what it estimates.
//
// It is next door rather than inside because it is genuinely separable — its
// own argument, its own trade, its own measurements — and because the file it
// came out of was the longest in the project. The counter-argument was that a
// termination rule read apart from the loop it terminates is worse, and the
// three lines it hands back are the test of that: if the loop in
// `transport.hpp` still reads as one thing with these gone, the split was
// right.
//
// A path in a closed box never ends. Every bounce keeps some fraction of the
// light, the fraction never reaches zero, and something has to stop it — and
// the obvious something, a depth limit, throws away the tail. That is bias:
// the answer is wrong by the discarded remainder, systematically, in the same
// direction, and no number of samples fixes it because every sample is wrong
// the same way. `transport.hpp`'s cavity table measures exactly that at
// `rho = 0.9`.
//
// Roulette stops paths without throwing anything away, and the trick is three
// lines of expectation algebra. Continue with probability `q`, and if the
// path survives, divide its contribution by `q`:
//
//      E[estimate] = q · (X / q)  +  (1 − q) · 0
//                  = X
//
// The survivors are scaled up by exactly the factor that compensates for the
// ones that were killed, so the expectation is unchanged. It is not an
// approximation, a heuristic, or a quality setting. It is the same estimator
// with some of its work replaced by a coin.
//
// What it costs is variance. A path that survives carries `1/q` times the
// weight it would have, so the spread of the estimates grows even though
// their mean does not — which is the trade the whole technique is: bias is
// exchanged for noise, and noise is the one this project can measure and
// drive down with samples.
//
// ── Choosing q, and when to start ────────────────────────────────────────
//
// `q` is the throughput's largest component. A path that has already lost
// most of its light is cheap to kill and expensive to keep; a path still
// carrying nearly all of it should almost certainly continue. Using the
// throughput itself makes `q` close to 1 exactly when the path matters, and
// the `1/q` scaling then barely changes anything.
//
// It does not start at the first bounce, and the reason is the variance
// above. Near the eye the throughput is close to 1, so `q` is close to 1 and
// the roulette is not killing much anyway — but the paths it *does* kill are
// the ones contributing most to the pixel, and each survivor's `1/q` boost
// lands on the largest terms in the sum. Delaying it a few bounces costs
// almost nothing in path length and removes the worst of that.
//
// Three bounces, which is a choice and not a derivation. `converge` in v0.5
// will confirm what is asserted here: that turning roulette on changes the
// noise but not the answer, and does not change the N^-½ slope.
//
// ── What it actually trades ──────────────────────────────────────────────
//
// The same cavity, 10⁵ paths, with the roulette on and with it disabled and
// nothing else changed:
//
//      rho    estimate    sample sd    seconds / 10⁵ paths
//      ────────────────────────────────────────────────────
//      on      0.5   1.997040     0.586          0.05
//      off     0.5   2.000000     2.1e-08        3.27
//      on      0.9   9.952060     8.97           0.15
//      off     0.9  10.000000     0              3.28
//
// Sixty-five times faster at rho = 0.5 and twenty-two times at rho = 0.9,
// and the price is written in the third column: an estimator that had no
// variance at all now has a standard deviation of 0.59 and of 8.97.
//
// That is the trade stated as plainly as it can be. Without roulette every
// path in a closed box runs to the depth limit — 256 bounces, whatever the
// albedo — and returns the same number. With it, a path at rho = 0.9 lasts
// about thirteen bounces and returns a number that is sometimes far too big,
// scaled up by the reciprocal of the probability it survived.
//
// And the mean does not move, which is the whole claim. Twelve independent
// batches of 10⁵ paths, each scored as how many standard errors it lands from
// the exact answer:
//
//      rho = 0.5    mean z = −0.066
//      rho = 0.9    mean z = +0.235
//
// against an expected spread of ±0.289 for twelve batches. No detectable
// bias at either albedo. (The first pair of runs both landed about 1.7
// standard errors low, which looked like something and was not; two numbers
// are not evidence of a direction.)

// ── What is not modelled ─────────────────────────────────────────────────
//
// Roulette on anything but the throughput. The survival probability could
// depend on the surface, the remaining depth, or an estimate of how much
// light is still reachable from here — and a path tracer that samples lights
// directly will want the last of those. It depends on the throughput and
// nothing else, which is the version whose expectation algebra fits in four
// lines above.

#pragma once

#include <cmath>

#include <render/spectrum.hpp>

namespace render {

// How many bounces before the roulette starts. A choice, and the prose above
// says why it is not zero.
inline constexpr int roulette_start_depth = 3;

// The probability a path survives: its throughput's largest component, capped
// at 1.
//
// The largest rather than an average, because killing a path kills it at
// every wavelength at once, and the right question is whether *any* of the
// four still carries something worth following. The cap is because a
// throughput above 1 is possible — an earlier survivor's `1/q` boost — and a
// probability is not.
inline double survival_probability(const Reflectance& throughput) {
    double largest = throughput[0];
    for (int i = 1; i < spectral_samples; ++i)
        largest = std::fmax(largest, throughput[i]);
    return std::fmin(1.0, largest);
}

} // namespace render
