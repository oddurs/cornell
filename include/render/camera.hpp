// camera.hpp — a dark room with a hole in it.
//
// The oldest instrument there is, and the only one with no aberrations,
// because it is the only one with no glass. Everything a photographer has a
// word for — bokeh, coma, vignetting, curvature of field, the way a fast lens
// draws an out-of-focus highlight as a disc with a bright rim — is a stack of
// ground surfaces being imperfect in a specific way, and a pinhole has no
// stack. It is also the only camera in which everything is in focus at once,
// for the same reason: focus is what a lens does with the cone of light from
// a point, and a hole of zero size admits no cone.
//
// So this file is short, and what is interesting about it is the two things
// it refuses.
//
// ── It refuses a field of view ────────────────────────────────────────────
//
// Field of view is the number every renderer takes. It is also not a property
// of a camera. It is a *consequence* of two lengths — how big the film is and
// how far it sits from the aperture — and the same lens on a different format
// gives a different angle, which is the entire content of the "crop factor"
// that thirty years of camera marketing has managed to make confusing.
//
// Take the two lengths, derive the angle:
//
//      fov = 2 · atan( film / 2·distance )
//
// A 50 mm lens on 35 mm film — 36 × 24 mm, the format the number "50 mm"
// silently assumes — is then
//
//      horizontal   39.5978°
//      vertical     26.9915°
//      diagonal     46.7930°
//
// and nobody typed any of them. Ask for 28 mm and the horizontal opens to
// 65.4705°; ask for 135 mm and it closes to 15.1893°. That is the whole of
// what a lens focal length means to a photographer, and here it falls out of
// a division and an arctangent rather than being a parameter.
//
// ── It refuses to call that distance a focal length ───────────────────────
//
// A focal length is a property of a lens: where it brings parallel rays to a
// point. There is no lens here, so there is no focus, so there is no focal
// length, and the field below is called `film_distance` because that is what
// the quantity is — how far the film sits behind the hole.
//
// The two coincide for a lens focused at infinity, which is why photographers
// use the one word for both and are right to. This project is not entitled to
// the shorthand: house rule 9 says name it what it is, and a `focal_length`
// in a camera with no focus would be the first borrowed word in a program
// whose argument is that words are not borrowed.
//
// ── The image is upside down ──────────────────────────────────────────────
//
// It has to be. Light from the top of the scene goes through the hole and
// lands on the bottom of the film; that is the whole mechanism, and every
// photographic negative ever made came out of the camera inverted. Darkroom
// printing turned it over.
//
// Most renderers avoid the question by putting a virtual image plane in
// *front* of the aperture, where the geometry is identical and the flip never
// happens. It is a fine trick and this file does not use it, because the film
// is behind the hole in the object being modelled. The inversion is applied
// once, here, in the mapping from film coordinates to physical film offsets,
// so that `(u, v)` runs left-to-right and top-to-bottom in reading order and
// everything downstream can stop thinking about it.
//
// ── What is not modelled ──────────────────────────────────────────────────
//
// The lens. All of it: aperture, depth of field, the shape of the out-of-
// focus highlight, the aberrations, the vignetting. v1.5 is about nothing
// else, and it arrives as a stack of refracting surfaces rather than as a
// parameter named `bokeh`, because the shape of an out-of-focus highlight is
// the shape of the aperture and not a setting.
//
// Diffraction, which is what actually limits a real pinhole. The aperture
// here has zero size, so in geometric optics the image is perfectly sharp and
// arbitrarily small holes are arbitrarily good. In reality a hole that small
// spreads the light that passes it, and the two effects trade: geometric blur
// falls with the diameter, diffraction blur rises as it. The optimum, from
// Rayleigh's criterion, is about `d = 1.9·√(λ·distance)` —
//
//      25 mm, 550 nm      0.2228 mm      f/112
//      50 mm, 550 nm      0.3151 mm      f/159
//      100 mm, 550 nm     0.4456 mm      f/224
//
// — which is why pinhole photographs are soft and why the exposures are
// minutes long. This renderer cannot produce that softness at any hole size,
// because diffraction is a wave effect and this is geometric optics. It is
// the assumption named at the top of `transport.hpp`, showing up in the
// simplest file in the project.
//
// Motion, and therefore the shutter. Nothing in this project moves.

#pragma once

#include <cmath>
#include <render/ray.hpp>
#include <render/si.hpp>
#include <render/vec.hpp>

namespace render {

class Camera {
public:
    // Where it is, what it is pointed at, and the two lengths that decide
    // everything else. `up_hint` only has to be roughly up: it is used to
    // find which way is sideways and is then discarded, so that a caller can
    // pass the world's up axis and not think about it.
    //
    // It is degenerate when `up_hint` is parallel to the viewing direction —
    // a camera pointed straight down, given "down" as its idea of up, has no
    // opinion about which way is right — and the cross product then has zero
    // length and `normalize` returns NaN. That is left to propagate rather
    // than be papered over, for the reason `vec.hpp` gives.
    static Camera look_at(const Vec3& eye,
                          const Vec3& target,
                          const Vec3& up_hint,
                          double film_width,
                          double film_height,
                          double film_distance) {
        Camera camera;
        camera.eye_           = eye;
        camera.forward_       = normalize(target - eye);
        camera.right_         = normalize(cross(camera.forward_, up_hint));
        camera.up_            = normalize(cross(camera.right_, camera.forward_));
        camera.film_width_    = film_width;
        camera.film_height_   = film_height;
        camera.film_distance_ = film_distance;
        return camera;
    }

    // A ray through a point on the film, in film coordinates: u from 0 at the
    // left edge to 1 at the right, v from 0 at the top to 1 at the bottom,
    // which is the order a PPM is written in and the order a film reads its
    // own pixels in.
    //
    // The two signs are the inversion. A scene point on the left is imaged on
    // the right-hand side of the physical film, so the film offset for u = 0
    // is +width/2; the ray from there back through the hole therefore leaves
    // in the −right direction, and the two negations cancel into the `u −
    // 0.5` below. The vertical is the same argument with v already counting
    // downward, which is why one of these reads `u − 0.5` and the other reads
    // `0.5 − v` rather than both agreeing.
    Ray ray_through(double u, double v) const {
        const Vec3 across = right_.vec() * ((u - 0.5) * film_width_);
        const Vec3 vertical = up_.vec()    * ((0.5 - v) * film_height_);
        const Vec3 along  = forward_.vec() * film_distance_;
        return Ray{eye_, normalize(across + vertical + along)};
    }

    // Derived, every time, from the two lengths. There is no stored field of
    // view to disagree with the geometry.
    double horizontal_fov() const { return 2.0 * std::atan(film_width_  / (2.0 * film_distance_)); }
    double vertical_fov()   const { return 2.0 * std::atan(film_height_ / (2.0 * film_distance_)); }
    double diagonal_fov()   const {
        return 2.0 * std::atan(std::hypot(film_width_, film_height_) / (2.0 * film_distance_));
    }

    // The film's own shape, which `film.hpp` needs in order to decide how
    // many pixels to put across it without inventing an aspect ratio of its
    // own.
    double film_width()  const { return film_width_;  }
    double film_height() const { return film_height_; }

private:
    Vec3 eye_{};
    Unit forward_{};
    Unit right_{};
    Unit up_{};
    double film_width_    = 0.0;
    double film_height_   = 0.0;
    double film_distance_ = 0.0;
};

// Just the two literal operators this file needs, by name. `si.hpp` made them
// an inline namespace so that they could be pulled in like this; pulling in
// the whole dictionary would put forty operators into `render` for the sake
// of two.
using si::literals::operator""_mm;

// 35 mm film, which is 36 × 24 mm and has not been 35 mm since 1934 — the
// number is the width of the perforated stock, not of the frame. Kept as the
// default because it is the format every focal length in photography is
// quoted against, and quoting a focal length against anything else is how the
// crop factor argument starts.
inline constexpr double film_35mm_width  = 36.0_mm;
inline constexpr double film_35mm_height = 24.0_mm;

} // namespace render
