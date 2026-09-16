// vec.hpp — three doubles, and the one distinction worth a second type.
//
// This file is arithmetic rather than physics, and it is here anyway, for the
// same reason `si.hpp` is: it is the vocabulary the physics gets written in,
// and a vocabulary settled late is a vocabulary that gets settled twice.
//
// The whole of the argument is one line long:
//
//      A DIRECTION IS NOT A VECTOR. IT IS A VECTOR OF LENGTH ONE.
//
// which sounds like a distinction without a difference until you notice what
// this renderer does with directions. Every cosine in the rendering equation
// is a dot product of two of them. A direction that has quietly stopped being
// unit length is a cosine that is quietly wrong — not by much, and not
// visibly, and in a factor that multiplies into the estimate at every bounce,
// so the error compounds along the path rather than showing up at one of
// them. It is the class of bug that gets found by someone rewriting the file
// for other reasons, eight months later.
//
// So `Vec3` is free and `Unit` is guarded. The only way to obtain a `Unit` is
// to normalise a `Vec3` or to name an axis, and the compiler will not let you
// assemble one out of three numbers you are confident about. It is
// `spectrum.hpp`'s argument about `Radiance` and `Reflectance` — a bore is
// not a stroke — applied to the other pair this renderer cannot afford to
// confuse.
//
// `Unit` compiles to a `Vec3` and `Vec3` compiles to three doubles. There is
// no tag, no length field, no check at runtime. The invariant is established
// once, at the point of normalisation, and after that it is a fact about the
// type rather than a property anybody has to maintain.
//
// ── What is not modelled ──────────────────────────────────────────────────
//
// The affine distinction. A point and a displacement are not the same kind of
// thing: adding two points is meaningless, and under a transformation a point
// picks up the translation where a direction does not. Plenty of renderers
// carry `Point3` and `Vec3` separately to say so, and they are right to.
//
// This one does not, because the distinction only pays once there are
// transformations, which arrive with the triangle meshes in v0.4. One
// invariant per file is enough for the file to be read; two is a type system
// nobody uses. If v0.4 wants it, that is a change to this file rather than a
// discovery in the middle of one.
//
// Normals are plain `Unit` here for the same reason and with the same debt.
// A normal is not a direction either — it transforms by the inverse transpose
// rather than by the matrix — and the day a matrix exists is the day that
// stops being a footnote and becomes a bug. It is written down so that it is
// a decision somebody made.

#pragma once

#include <cmath>

namespace render {

// ── Vec3 ──────────────────────────────────────────────────────────────────
//
// An aggregate, deliberately: `Vec3{1.0, 0.0, 0.0}` with no constructor to
// read, no initialisation order to get wrong, and members you can reach.
// Nothing here is guarded because there is nothing here to guard.

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    constexpr Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    constexpr Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    constexpr Vec3& operator*=(double s)      { x *= s;   y *= s;   z *= s;   return *this; }
    constexpr Vec3& operator/=(double s)      { x /= s;   y /= s;   z /= s;   return *this; }

    constexpr Vec3 operator-() const { return {-x, -y, -z}; }
};

constexpr Vec3 operator+(Vec3 a, const Vec3& b) { return a += b; }
constexpr Vec3 operator-(Vec3 a, const Vec3& b) { return a -= b; }
constexpr Vec3 operator*(Vec3 a, double s)      { return a *= s; }
constexpr Vec3 operator*(double s, Vec3 a)      { return a *= s; }
constexpr Vec3 operator/(Vec3 a, double s)      { return a /= s; }

constexpr double dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Right-handed, which is a convention and not a fact, and is stated here
// because the alternative is every file downstream rediscovering it from the
// sign of a shadow.
constexpr Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

// The squared length is the one to reach for. Comparing two distances, or
// asking whether a vector is shorter than some tolerance, does not need the
// square root, and a square root that nobody needed is the cheapest thing in
// a renderer to not do.
// One component by index, for the few places that genuinely loop over axes —
// a bounding box test, a split-plane choice. Written as a switch rather than
// as pointer arithmetic into the struct, which is the usual trick and is
// undefined behaviour dressed as cleverness.
constexpr double component(const Vec3& v, int axis) {
    return axis == 0 ? v.x : (axis == 1 ? v.y : v.z);
}

constexpr double length_squared(const Vec3& v) { return dot(v, v); }

inline double length(const Vec3& v) { return std::sqrt(length_squared(v)); }

// ── Unit ──────────────────────────────────────────────────────────────────
//
// A `Vec3` that is known to have length one. It converts to `Vec3` silently
// and for nothing, so every function above accepts it and none of them had to
// be written twice; the conversion only goes that way, which is the point.

class Unit {
public:
    constexpr Unit() = default;   // (0,0,0): not a direction, and not usable
                                  // as one. It exists so that a Unit can be a
                                  // member of something that has not decided
                                  // yet, and the arithmetic will produce
                                  // zeroes rather than nonsense if it is used
                                  // before it is set.

    constexpr operator const Vec3&() const { return v_; }
    constexpr const Vec3& vec()      const { return v_; }

    constexpr double x() const { return v_.x; }
    constexpr double y() const { return v_.y; }
    constexpr double z() const { return v_.z; }

    // The escape hatch, and it is `consteval` on purpose.
    //
    // Naming an axis is a claim a reader can check by looking at it. Handing
    // this a vector computed at runtime is a claim nobody can check, and it
    // is exactly how the invariant gets lost — one caller, once, who was
    // right at the time. Restricting it to compile time makes the honest use
    // free and the dishonest one impossible.
    //
    // When something legitimately produces a unit vector at runtime — a
    // reflection about a unit normal, in v0.6 — that routine can be made a
    // friend, deliberately, in a commit that says why. Until then there is no
    // such routine and there is no such door.
    static consteval Unit known(double x, double y, double z) {
        return Unit{Vec3{x, y, z}};
    }

    // Negating a direction preserves its length, so the result is still a
    // `Unit` and says so. This is the one operation that can stay inside the
    // type without an argument.
    constexpr Unit operator-() const { return Unit{-v_}; }

private:
    constexpr explicit Unit(const Vec3& v) : v_{v} {}

    friend Unit normalize(const Vec3& v);

    Vec3 v_{};
};

// The only runtime door into `Unit`, and the place the invariant is
// established.
//
// A zero vector normalises to NaN, and that is left alone rather than
// special-cased to some fallback direction. A direction of zero length is a
// mistake upstream — a degenerate triangle, a light with no orientation, two
// coincident points subtracted — and a NaN propagates to a black pixel and a
// failed assertion in the instruments, where it will be found. A silent
// fallback to +z produces an image that is wrong in a way that looks like a
// modelling decision.
inline Unit normalize(const Vec3& v) {
    return Unit{v / length(v)};
}

// The axes. Right-handed, +y up, which is the convention `camera.hpp` and
// `cornell.hpp` are both written in.
inline constexpr Unit axis_x = Unit::known(1.0, 0.0, 0.0);
inline constexpr Unit axis_y = Unit::known(0.0, 1.0, 0.0);
inline constexpr Unit axis_z = Unit::known(0.0, 0.0, 1.0);

} // namespace render
