// tonemap.hpp — the one part of this project that has no correct answer.
//
// Radiance is unbounded. A display is not. Everything that happens between
// those two facts is a choice about appearance, and none of it is physics.
//
// So it lives here, in `apps/`, in its own file, after everything else is
// finished — and the rule that goes with it is the important part:
//
//      NO FIGURE QUOTED ANYWHERE IN THIS PROJECT COMES FROM AFTER THIS STEP.
//
// The film writes linear float, always. `cornell.pfm` is written before any
// of this happens and is the file a number may be read from; `cornell.ppm` is
// written after and is for looking at. The README says which command printed
// each of its figures, and none of those commands is this one.
//
// ── Why there is no right answer ─────────────────────────────────────────
//
// A tone curve is being asked to represent a scene with a contrast ratio of
// perhaps 10⁵ on a display with a contrast ratio of perhaps 10³, in a room
// whose lighting nobody told it about, to an observer who is adapted to
// something. That is not an approximation problem with a best solution; it is
// a question about what the viewer should be made to feel, and it has been
// answered differently by every film stock, every camera manufacturer and
// every colour scientist who has tried.
//
// This project's position is that the renderer should not have an opinion,
// and that the place to have one is clearly marked.
//
// ── The default is `clip`, and clipping has no author ────────────────────
//
// Everything at or below the exposure reference is reproduced exactly;
// everything above it becomes white. It is the most honest curve available,
// in the narrow sense that it does not alter a single value it can represent,
// and the least flattering, because a bright light becomes a flat white shape
// with no detail in it.
//
// That is what a photograph exposed for the walls does to the lamp in the
// ceiling, which is the comparison v1.0 has to make, so it is the default.
//
// ── The alternative is Reinhard's, and it has one ────────────────────────
//
// Erik Reinhard, Michael Stark, Peter Shirley and Jim Ferwerda, "Photographic
// Tone Reproduction for Digital Images", SIGGRAPH 2002. The operator here is
// the simplest of the several in that paper:
//
//      L' = L / (1 + L)
//
// which maps [0, infinity) onto [0, 1) with no clipping anywhere: nothing is
// ever pure white and nothing is ever lost. It is a curve-fit to the
// behaviour of photographic film rather than a model of anything, which is
// house rule 8's distinction, and it is named for its authors because of it.
//
// It is not the default because it changes every value in the image,
// including the ones that were representable, and a render whose mid-greys
// have been moved cannot be compared against a measurement.

#pragma once

#include <string_view>

namespace app {

enum class ToneCurve { clip, reinhard };

inline ToneCurve tone_curve_named(std::string_view name) {
    return name == "reinhard" ? ToneCurve::reinhard : ToneCurve::clip;
}

inline std::string_view name_of(ToneCurve curve) {
    return curve == ToneCurve::reinhard ? "reinhard" : "clip";
}

// Applied per channel, after exposure and before sRGB encoding.
//
// Per channel rather than to luminance, which is itself a choice and a
// criticised one: it desaturates bright colours, because the largest channel
// is compressed hardest. Reinhard's paper applies it to luminance and
// preserves the ratios. This does not, because `clip` cannot do anything else
// and having the two operators differ in more than one respect would make the
// comparison between them useless.
inline double tonemap(double value, ToneCurve curve) {
    if (value <= 0.0) return 0.0;
    switch (curve) {
        case ToneCurve::reinhard: return value / (1.0 + value);
        case ToneCurve::clip:     break;
    }
    return value > 1.0 ? 1.0 : value;
}

} // namespace app
