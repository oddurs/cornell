// sampler.hpp — where the random numbers come from, and why they are not
// random in the way that phrase usually means.
//
// This is not a performance decision. It is the decision that makes the rest
// of the project debuggable at all, and it is worth the page it takes.
//
// ── The shared stream, and why it is the wrong shape ──────────────────────
//
// The obvious arrangement is one generator, and every sample drawn from it in
// turn. It is fine until the renderer has two threads, at which point the
// number a given path receives depends on how far the other thread had got
// when it asked — which is to say, on the operating system's scheduler.
//
// The output is then still correct. That is the trap: Monte Carlo integration
// is unbiased under any of those orderings, so the image looks right, the
// average is right, and nothing announces that anything has changed. What is
// gone is reproducibility. A firefly in the corner of frame 200 cannot be
// re-run; a path that produces a NaN cannot be found twice; and the
// difference between two images is no longer attributable to the change you
// made between them, because it is also attributable to the scheduler.
//
// Debugging a Monte Carlo renderer is hard enough with the noise holding
// still. Every bug becomes a heisenbug the moment it does not.
//
// ── One stream per sample, addressed rather than dispensed ────────────────
//
// So there is no shared stream. A sampler is constructed from the two numbers
// that identify what it is for — which pixel, which sample — and those are
// hashed into a seed. Nothing is handed out in order; a stream is *addressed*.
// Pixel 41 200's ninth sample gets the same numbers whether it is computed
// first, last, on eight threads, or alone in a debugger with a breakpoint in
// it.
//
// The hash has to be good, and this is the part that gets skipped. Seeding a
// generator with `pixel * 1000 + sample` and starting it produces streams
// whose first outputs are strongly correlated with each other, because most
// generators take a while to forget a nearby seed — and the first output is
// the one that matters most, since it picks the position within the pixel.
// Correlated first draws across neighbouring pixels is a structured artefact
// that looks like a pattern in the image and is not noise at all. So the seed
// goes through the SplitMix64 finaliser, whose whole job is to take a
// sequential counter and return something that looks like it was not.
//
// ── PCG32, O'Neill, 2014 ─────────────────────────────────────────────────
//
// A linear congruential generator, which is the oldest and worst family of
// random number generators, with its output permuted by a shift whose *amount
// is drawn from the generator's own high bits*. The LCG supplies the period
// and the cheapness; the permutation destroys the lattice structure that made
// bare LCGs a byword for getting this wrong.
//
// The reason it is here rather than `std::mt19937` is the shape of the
// problem above. Mersenne Twister holds 2.5 kilobytes of state and takes
// thousands of operations to seed; this project constructs a generator per
// sample per pixel, which at 64 samples on a 480 × 320 film is ten million
// constructions. PCG32 is sixteen bytes and two multiplications.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// Low-discrepancy sequences: Sobol, Halton, blue-noise masks. They converge
// faster than plain random numbers — the error falls closer to N⁻¹ than to
// N^-½ for smooth integrands — and they complicate the χ² machinery in v0.5,
// because a stratified sequence is deliberately *not* the independent
// uniform variates that test assumes.
//
// The honest order is: get it right with plain random numbers, prove it with
// the instruments, then make it faster and prove it again. A renderer that
// starts with a clever sequence has no way of telling a bug in the sequence
// from a bug in the physics.
//
// ── Measured ─────────────────────────────────────────────────────────────
//
// Uniformity, χ² over 64 bins, 2 × 10⁶ draws, 20 independent streams: mean
// 63.1 against 63 degrees of freedom, spread 50.2 to 85.6 against an expected
// standard deviation of 11.2. Mean of the variates 0.500144, variance
// 0.083376 against 1/12 = 0.083333.
//
// Addressing, not dispensing: a thousand pixels visited forwards and the same
// thousand visited backwards produce bit-identical first draws.
//
// And the thing this file is actually arguing about — the correlation between
// the first draw of pixel i and the first draw of pixel i + 1, which is the
// draw that positions the sample inside the pixel:
//
//      this sampler                        −0.000236
//      a bare LCG seeded `pixel * 1000`    +0.996906
//
// against a noise floor of 1/√n = 0.001. The second row is not a worse
// sampler, it is not a sampler at all: neighbouring pixels are sampling at
// almost exactly the same place within themselves, which is a fixed pattern
// laid over the whole image that no number of samples removes. It is what the
// mixing step exists to prevent, and it is why the seed is hashed rather than
// constructed.

#pragma once

#include <cstdint>
#include <utility>

namespace render {

namespace detail {

// SplitMix64's finaliser. Its job is to be a bijection on 64 bits with good
// avalanche — flipping one input bit flips about half the output bits — so
// that two seeds differing by one produce streams with nothing in common.
constexpr std::uint64_t mix64(std::uint64_t x) {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

} // namespace detail

class Sampler {
public:
    // The address, not a position in a queue. Two numbers in, one stream out,
    // and the same two numbers always give the same stream.
    constexpr Sampler(std::uint64_t pixel, std::uint64_t sample) {
        // Two different mixes of the same pair: one chooses the sequence, the
        // other the starting point within it. PCG's `inc` must be odd, which
        // is what the `| 1` is for; it selects one of 2⁶³ distinct streams.
        inc_   = (detail::mix64(pixel ^ (sample << 32)) << 1) | 1ULL;
        state_ = 0ULL;
        step();
        state_ += detail::mix64(sample ^ (pixel << 32));
        step();
    }

    // A uniform variate in [0, 1). Never 1.0: the largest `std::uint32_t` is
    // 2³² − 1, so the largest result is 1 − 2⁻³². That matters more than it
    // looks — a warp that takes `sqrt(1 - u)` or `log(1 - u)` has a defined
    // answer everywhere on this half-open interval and not on the closed one.
    constexpr double next() {
        return double(next_u32()) * 0x1p-32;
    }

    // Two of them, in a fixed order, because a warp that consumes a pair
    // should not have to care which of `next()` and `next()` the compiler
    // evaluated first — and in C++ the order of evaluation of function
    // arguments is unspecified, so `warp(s.next(), s.next())` is a real bug
    // that produces a correct-looking image with the axes swapped.
    constexpr std::pair<double, double> next2() {
        const double u = next();
        const double v = next();
        return {u, v};
    }

private:
    constexpr std::uint32_t next_u32() {
        const std::uint64_t previous = state_;
        step();
        // The permutation: an xorshift whose *amount* comes from the top five
        // bits of the state. This is the part that is not a plain LCG.
        const std::uint32_t xorshifted =
            std::uint32_t(((previous >> 18) ^ previous) >> 27);
        const std::uint32_t rot = std::uint32_t(previous >> 59);
        return (xorshifted >> rot) | (xorshifted << ((32 - rot) & 31));
    }

    constexpr void step() {
        state_ = state_ * 6364136223846793005ULL + inc_;
    }

    std::uint64_t state_ = 0;
    std::uint64_t inc_   = 1;
};

} // namespace render
