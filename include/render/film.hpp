// film.hpp — where the samples land, and the last place in the program that
// still knows what a wavelength is.
//
// A film here is not an image. It has no pixels in the sense a file format
// means, no gamma, no exposure, no white point, and above all no colour: it
// is an array of accumulators holding spectral radiance in W·m⁻²·sr⁻¹·m⁻¹,
// and the conversion to something a monitor can show happens once, later,
// somewhere else, in the two files that are allowed to know about eyes.
//
// ── The bins, and the argument this file has with spectrum.hpp ────────────
//
// `spectrum.hpp` rejected fixed uniform bins for carrying light, at length
// and correctly: a path that carries 32 bins through a prism produces 32
// coloured stripes, and no number of samples fixes it, because the banding is
// in the representation rather than in the noise.
//
// This file uses fixed uniform bins, and the difference is worth stating
// rather than leaving as an apparent contradiction. A path is a *carrier*: it
// transports light through an optical system that can separate wavelengths,
// so quantising it quantises the physics. A film is a *sensor*: it is the end
// of the line, nothing downstream of it refracts, and what a bin does here is
// exactly what a bin does in a real spectrometer — average the light that
// arrived within it. Every path still carries four continuously-drawn
// wavelengths; the prism in v0.9 still disperses continuously; the bins only
// decide how finely the answer is reported.
//
// Ten nanometres, and that is a choice rather than a derivation, so here is
// what constrains it from both sides. Too coarse and the integration against
// the CIE observer in v0.3 starts to alias against the observer's own
// structure. Too fine and the bins are mostly empty: each path deposits four
// wavelengths, so at N samples per pixel a bin holds about 4N/47 of them, and
// a mean over three samples is not a mean. What would settle it is a
// comparison against a film with twice the resolution at the same sample
// count, which is a v0.5 instrument and is not written yet.
//
// 47 bins, because 830 − 360 is 470 and that is the only part of this that is
// arithmetic. The range is `si::lambda_min` to `si::lambda_max`, which is the
// domain of the 1931 observer, for the reason `si.hpp` gives.
//
// ── What the bins cost, which is why nobody ships this ────────────────────
//
// A bin is a running sum and a count, and there are 47 of them per pixel:
//
//      47 bins       564 bytes/pixel       400×300    67.7 MB
//                                          512×512   147.8 MB
//                                         1024×768   443.5 MB
//
//      XYZ            24 bytes/pixel       400×300     2.9 MB
//                                          512×512     6.3 MB
//                                         1024×768    18.9 MB
//
// Twenty-three times the memory, and that — not colour science — is the real
// reason production spectral renderers convert each sample to tristimulus the
// moment it reaches the sensor and never store a spectrum at all. The
// conversion is unbiased: evaluating the observer at the sampled wavelength
// and accumulating three numbers estimates the same integral as binning first
// and integrating after.
//
// This film keeps the spectra anyway, for exactly as long as it is useful to.
// The reason is v0.1's: there is no observer yet, `cie.hpp` does not arrive
// until v0.3, and a film that could only speak tristimulus could not be
// looked at until then. A film of bins can be written out as a
// single-wavelength image on the first day. When v0.3 arrives it will want
// the three-channel path, and this file will grow it as an alternative rather
// than a replacement, because the `spectrum` instrument in v0.5 needs
// something that still remembers wavelengths.
//
// ── What is not modelled ──────────────────────────────────────────────────
//
// The reconstruction filter. A sample lands in the pixel it lands in and
// contributes to no other, which is a box filter one pixel wide — the worst
// one in common use. A Mitchell or a Gaussian spreads each sample over a
// small neighbourhood with a weight, which is why every accumulator in a real
// film stores a weight alongside the sum rather than a count. That is the
// same arithmetic with a `+= w` where this has a `+= 1`, and it is left for
// whenever edge quality starts to matter more than it does at v0.1.
//
// The sensor. A real one has a quantum efficiency that varies with
// wavelength, a fixed pattern noise, a read noise, a full well that clips,
// and a Bayer mosaic in front of it that throws away two thirds of the light.
// This film is a perfect one: it records what arrives. Modelling any of that
// belongs with the camera body in v1.5, if ever.

#pragma once

#include <cstdint>
#include <vector>
#include <render/si.hpp>
#include <render/spectrum.hpp>

namespace render {

// (830 − 360) nm at 10 nm each. See above for why ten and why not a
// derivation.
inline constexpr int film_bins = 47;
inline constexpr double film_bin_width = (si::lambda_max - si::lambda_min) / film_bins;

class Film {
public:
    Film(int width, int height)
        : width_{width},
          height_{height},
          sums_(std::size_t(width) * std::size_t(height) * film_bins, 0.0),
          counts_(std::size_t(width) * std::size_t(height) * film_bins, 0) {}

    int width()  const { return width_;  }
    int height() const { return height_; }

    // Deposit one path's worth of light. The four wavelengths were drawn
    // together and are a quarter of the visible range apart, so they land in
    // four different bins and one sample contributes to four of them —
    // which is the whole economy of hero wavelength sampling showing up at
    // the sensor.
    void add_sample(int x, int y, const Wavelengths& lambdas, const Radiance& value) {
        const std::size_t pixel = (std::size_t(y) * std::size_t(width_) + std::size_t(x)) * film_bins;
        for (int i = 0; i < spectral_samples; ++i) {
            const std::size_t bin = pixel + std::size_t(bin_of(lambdas[i]));
            sums_[bin]   += value[i];
            counts_[bin] += 1;
        }
    }

    // The estimate, written as the ratio rather than as a number somebody
    // maintained. House rule 3: this is the mean of the spectral radiance
    // samples that fell in the bin, it is unbiased because the wavelengths
    // were drawn uniformly over the range and the bins partition it evenly,
    // and its error falls as the inverse square root of the count.
    //
    // An empty bin is zero rather than a NaN, and that is a small lie of
    // convenience: zero is a legitimate radiance and "nothing arrived here
    // yet" is not. It is tolerable only because the caller can ask for the
    // count and the instruments in v0.5 will.
    double mean_radiance(int x, int y, int bin) const {
        const std::size_t i = index(x, y, bin);
        return counts_[i] == 0 ? 0.0 : sums_[i] / double(counts_[i]);
    }

    std::uint32_t count(int x, int y, int bin) const { return counts_[index(x, y, bin)]; }

    // The middle of a bin, in metres, because everything in here is in
    // metres. `si::as::nm` is how it gets quoted on a page.
    static constexpr double bin_centre(int bin) {
        return si::lambda_min + (double(bin) + 0.5) * film_bin_width;
    }

    // The bin a wavelength falls in, clamped to the range.
    //
    // The clamp is a guard and not a fix for a known case: `Wavelengths`
    // takes `offset - floor(offset)`, which is always in [0, 1), so a
    // wavelength from the sampler cannot reach `lambda_max` and cannot
    // produce an index of 47. An earlier version of this comment claimed a
    // `u` of exactly zero put the fourth wavelength on the upper edge; it
    // puts it at three quarters of the span.
    //
    // It stays because this function is public and nothing stops a caller —
    // `cie.hpp` in v0.3, walking the observer's tabulated wavelengths — from
    // handing it a number from outside the range, where the alternative to a
    // clamp is a silent write past the end of the array.
    static constexpr int bin_of(double lambda) {
        const int bin = int((lambda - si::lambda_min) / film_bin_width);
        return bin < 0 ? 0 : (bin >= film_bins ? film_bins - 1 : bin);
    }

    // Deliberately absent, and this is the list rather than an oversight: no
    // `rgb()`, no `tonemap()`, no `exposure`, no `gamma`, no `srgb()`, no
    // `luminance()`. Every one of them is an eye, and the eye arrives in
    // v0.3, in a file that says whose it was.

private:
    std::size_t index(int x, int y, int bin) const {
        return (std::size_t(y) * std::size_t(width_) + std::size_t(x)) * film_bins + std::size_t(bin);
    }

    int width_  = 0;
    int height_ = 0;
    std::vector<double> sums_;
    std::vector<std::uint32_t> counts_;
};

} // namespace render
