// cornell.hpp — the box, as somebody measured it.
//
// This is the file the repository is named after. Everything before it is
// machinery; this is the object.
//
// ── Who built it, and what they were arguing about ───────────────────────
//
// In 1984 Cindy Goral, Kenneth Torrance, Donald Greenberg and Bennett
// Battaile published "Modeling the Interaction of Light Between Diffuse
// Surfaces". The method was radiosity, borrowed from thermal engineering —
// Torrance's field — and applied to light: divide the surfaces into patches,
// work out how much of each patch every other patch can see, and solve the
// resulting system for the radiosity of each. It produces the soft gradients
// and the colour bleeding that ray tracing of the day could not, because ray
// tracing of the day followed only specular paths.
//
// The problem was that nobody had any reason to believe it. A new method
// produces a picture; a picture is not an argument. The literature was full
// of images that looked plausible and were checked against nothing, and
// radiosity's central quantity — the form factor between two patches — was a
// geometric abstraction with no obvious way to tell whether the number coming
// out was right.
//
// So they built the thing. Plywood, painted, a light in the ceiling, two
// blocks on the floor. They measured the spectral reflectance of the paint,
// photographed the box with a calibrated instrument, rendered the same scene,
// and printed the photograph beside the render.
//
// That is the whole reason this project is named after a box. Not because it
// is a convenient test scene — it has been one for forty years, and almost
// every renderer that draws it has quietly forgotten there is an original —
// but because it is the moment somebody in this field said "here is my
// answer, and here is the world, and you may compare them".
//
// ── What this project owes it ────────────────────────────────────────────
//
// The same comparison, forty years later, with a different algorithm. Goral
// and colleagues solved a linear system over patches; this solves an integral
// equation by following paths, which is a method that did not exist in usable
// form until Kajiya two years afterwards. If both are right they must agree
// with the same photograph, and v1.0 is where that is attempted.
//
// It is worth being precise about which photograph, because item 0051 found
// that there are two eras of them. The 1984 plate is the historical one. The
// data below, and the images this project will actually be compared against,
// come from a re-measurement of the box published by the Program of Computer
// Graphics around 2005, with a liquid-cooled CCD and seven narrow-band
// filters. The argument is unchanged — somebody else measured a real object
// before any of this was written — but the claim has to name the right
// measurement.
//
// ── What could still be wrong ────────────────────────────────────────────
//
// If the comparison in v1.0 fails, the suspects are written down in advance
// rather than found afterwards. The lamp's spectrum is four numbers and its
// absolute scale was never published. The paint is assumed Lambertian and is
// not — item 0061. And this is geometric optics throughout, which
// `transport.hpp` has admitted at length.
//
// A prediction made before the measurement is worth more than an explanation
// offered after it.
//
// Item 0051 is the spike that had to close before a line of this could be
// written, and it should be read alongside this file. The short version:
//
//      The data is primary and it is measured. The publication is gone from
//      the live web and survives only in the Internet Archive, which is why
//      the numbers are embedded here rather than fetched. The units are not
//      stated anywhere and had to be inferred. The box is not rectangular,
//      because it was measured rather than drawn. And the light's spectrum is
//      four numbers.
//
// Source, cited once, for everything below:
//
//      Cornell University Program of Computer Graphics, "Cornell Box Data",
//      http://www.graphics.cornell.edu/online/box/data.html
//      last updated 2 February 2005; that URL is dead, and the page is at
//      https://web.archive.org/web/2018/http://www.graphics.cornell.edu/online/box/data.html
//
// ── Every figure is marked ───────────────────────────────────────────────
//
// In the manner of `windsor`, which marks a number CALIBRATED when it was
// chosen rather than measured, so that a reader can see exactly how much of
// a model is evidence:
//
//      MEASURED    Cornell measured it and published it
//      INFERRED    not stated, but established from the rest of the data
//      ASSUMED     Cornell assumed it and said so
//      CALIBRATED  this project chose it, because the data does not say
//
// ── The walls ────────────────────────────────────────────────────────────
//
// MEASURED. Three reflectance spectra, 400 to 700 nm at 4 nm, 76 samples
// each, of the actual paint on the actual walls.
//
// This is where the no-RGB rule stops being a principle and starts being
// visible, and the numbers say so before anything is rendered. The red wall
// never rises above 0.657 and never falls below 0.040 — it is a broad gentle
// curve that happens to be higher at long wavelengths, not a saturated
// primary. The green wall peaks at 0.481 around 528 nm and sits near 0.1
// everywhere else. The white wall is not flat either: 0.343 at 400 nm rising
// to about 0.74 and staying there.
//
// Colour bleeding — the single most famous thing about this image — is the
// product of one of these curves with the light's spectrum, integrated
// against the observer. From an RGB triple it is a plausible tint. From these
// numbers it is the answer.
//
// None of these go anywhere near `jakob_hanika.hpp`. That file is for a
// colour somebody invented; these were measured, and mixing the two would
// make the v1.0 comparison meaningless.
//
// ── What holding the endpoints costs ─────────────────────────────────────
//
// The data stops at 400 and 700 nm and this project samples wavelengths from
// 360 to 830, so something must happen outside the measured range.
// `Measured::at` holds the nearest endpoint, and the honest question is how
// much of the observer lives out there:
//
//                      below 400 nm    above 700 nm
//      x-bar              0.0796 %        0.1296 %
//      y-bar              0.0023 %        0.0468 %
//      z-bar              0.3760 %        0.0000 %
//
// A twentieth of a per cent of the luminance and a third of a per cent of the
// blue, and the wall spectra are flat-ish at both ends, so holding them is a
// small extrapolation over a region that barely contributes. Reporting zero
// instead would darken every surface by those fractions, systematically,
// which is worse than an admitted extrapolation — and it would do it
// unevenly across the three channels, which is worse again, because a
// systematic error that is the same everywhere is invisible and one that
// differs by channel is a tint.

#pragma once

#include <array>
#include <cstddef>
#include <render/cie.hpp>
#include <cmath>
#include <render/si.hpp>
#include <render/vec.hpp>
#include <render/scene.hpp>
#include <render/spectrum.hpp>

namespace render::cornell {

// The grid the reflectances were measured on. MEASURED.
//
// Stated in nanometres as well as metres, and the reason is the one `si.hpp`
// spends a paragraph on. In nanometres the grid is exact integer arithmetic:
// 400 + 4 x 75 is 700 and there is nothing to discuss. In metres it is not —
// `400.0e-9 + 4.0e-9 * 75` comes out as 7.0000000000000007e-7 against a
// `700.0e-9` of 6.9999999999999997e-7, one ulp apart, because none of those
// three values is representable and the roundings do not cancel.
//
// So the check below is done in the units where the claim is exactly true,
// and the metres are derived from them. This is not pedantry about a
// difference of 1e-22 metres; it is that `static_assert` takes a bool, and a
// check written in the wrong units is a check that has to be softened into a
// tolerance and then means less.
inline constexpr int reflectance_first_nm = 400;
inline constexpr int reflectance_step_nm  = 4;
inline constexpr int reflectance_last_nm  = 700;
inline constexpr std::size_t reflectance_samples = 76;

static_assert(reflectance_first_nm + reflectance_step_nm * int(reflectance_samples - 1)
                  == reflectance_last_nm,
              "the reflectance grid must end at 700 nm");

inline constexpr double reflectance_first = double(reflectance_first_nm) / 1e9;
inline constexpr double reflectance_step  = double(reflectance_step_nm) / 1e9;

using WallSpectrum = Measured<reflectance_samples>;

// MEASURED. The white walls: floor, ceiling, back wall, and both blocks.
inline constexpr WallSpectrum white = {reflectance_first, reflectance_step, {
    0.343, 0.445, 0.551, 0.624, 0.665, 0.687,
    0.708, 0.723, 0.715, 0.710, 0.745, 0.758,
    0.739, 0.767, 0.777, 0.765, 0.751, 0.745,
    0.748, 0.729, 0.745, 0.757, 0.753, 0.750,
    0.746, 0.747, 0.735, 0.732, 0.739, 0.734,
    0.725, 0.721, 0.733, 0.725, 0.732, 0.743,
    0.744, 0.748, 0.728, 0.716, 0.733, 0.726,
    0.713, 0.740, 0.754, 0.764, 0.752, 0.736,
    0.734, 0.741, 0.740, 0.732, 0.745, 0.755,
    0.751, 0.744, 0.731, 0.733, 0.744, 0.731,
    0.712, 0.708, 0.729, 0.730, 0.727, 0.707,
    0.703, 0.729, 0.750, 0.760, 0.751, 0.739,
    0.724, 0.730, 0.740, 0.737,
}};

// MEASURED. The right wall, seen from the camera.
inline constexpr WallSpectrum green = {reflectance_first, reflectance_step, {
    0.092, 0.096, 0.098, 0.097, 0.098, 0.095,
    0.095, 0.097, 0.095, 0.094, 0.097, 0.098,
    0.096, 0.101, 0.103, 0.104, 0.107, 0.109,
    0.112, 0.115, 0.125, 0.140, 0.160, 0.187,
    0.229, 0.285, 0.343, 0.390, 0.435, 0.464,
    0.472, 0.476, 0.481, 0.462, 0.447, 0.441,
    0.426, 0.406, 0.373, 0.347, 0.337, 0.314,
    0.285, 0.277, 0.266, 0.250, 0.230, 0.207,
    0.186, 0.171, 0.160, 0.148, 0.141, 0.136,
    0.130, 0.126, 0.123, 0.121, 0.122, 0.119,
    0.114, 0.115, 0.117, 0.117, 0.118, 0.120,
    0.122, 0.128, 0.132, 0.139, 0.144, 0.146,
    0.150, 0.152, 0.157, 0.159,
}};

// MEASURED. The left wall, seen from the camera.
inline constexpr WallSpectrum red = {reflectance_first, reflectance_step, {
    0.040, 0.046, 0.048, 0.053, 0.049, 0.050,
    0.053, 0.055, 0.057, 0.056, 0.059, 0.057,
    0.061, 0.061, 0.060, 0.062, 0.062, 0.062,
    0.061, 0.062, 0.060, 0.059, 0.057, 0.058,
    0.058, 0.058, 0.056, 0.055, 0.056, 0.059,
    0.057, 0.055, 0.059, 0.059, 0.058, 0.059,
    0.061, 0.061, 0.063, 0.063, 0.067, 0.068,
    0.072, 0.080, 0.090, 0.099, 0.124, 0.154,
    0.192, 0.255, 0.287, 0.349, 0.402, 0.443,
    0.487, 0.513, 0.558, 0.584, 0.620, 0.606,
    0.609, 0.651, 0.612, 0.610, 0.650, 0.638,
    0.627, 0.620, 0.630, 0.628, 0.642, 0.639,
    0.657, 0.639, 0.635, 0.642,
}};

// ── The paint is not Lambertian, and Cornell says so first ───────────────
//
// ASSUMED, and by them rather than by this project. The data page states it
// plainly before giving a single number:
//
//      Surfaces are assumed to be Lambertian.
//
// Assumed. Not measured to be. The reflectances below were measured with a
// spectrometer at one geometry, and the model that turns one number per
// wavelength into a BRDF — scatter it equally in every direction — is an
// assumption laid on top of the measurement.
//
// It is wrong, and in a known direction. Matte paint on plywood
// *retroreflects*: seen from the direction the light comes from it is
// brighter than the cosine law predicts, and at grazing angles brighter
// still. The mechanism is not subtle — a rough surface is a landscape of
// small facets that shadow and mask each other, and when you look along the
// illumination direction you see only the lit faces and none of the shadows.
//
// Oren and Nayar modelled it in 1994, in "Generalization of Lambert's
// Reflectance Model", by treating the surface as a distribution of Lambertian
// V-cavities with a roughness parameter and working out the masking. At zero
// roughness it reduces exactly to Lambert; as roughness rises the surface
// flattens out, loses the limb darkening a Lambertian sphere has, and gains
// the retroreflective peak. It is the model that makes a photograph of the
// Moon look like the Moon.
//
// This project does not implement it. `lambert.hpp` scatters equally and
// says so.
//
// ── Why that is a claim rather than a shrug ──────────────────────────────
//
// The claim is that the error is smaller than the uncertainty in the
// comparison this project is aiming at, and it is a claim precisely because
// the correction is largest exactly where the interesting things are.
//
// Oren-Nayar departs most from Lambert at grazing angles and in corners,
// which is where the colour bleeding is, which is the thing the Cornell box
// is famous for and the thing v1.0 is measuring. If the comparison comes out
// wrong by a few per cent in the corners, this is the first place to look,
// and it will be much easier to look here having said so in advance.
//
// The claim is checkable and it is not checked yet. Item 0115 in v1.0 is
// where every place the model was let off gets stated against a number, and
// this is the first entry on that list.

// ── The light ────────────────────────────────────────────────────────────
//
// MEASURED, and the thinnest part of the data set by a long way.
//
// The emission spectrum is four numbers:
//
//      400 nm    0.0
//      500 nm    8.0
//      600 nm   15.6
//      700 nm   18.4
//
// Everything else here is sampled every 4 nm. The thing that sets the colour
// of the entire image is sampled every 100.
//
// ── What kind of lamp, which took some finding ───────────────────────────
//
// The data page does not say. Cornell's companion measurement library does:
//
//      boxsource.mat: the spectrum of the light source in the Cornell box
//      (tungsten flood light with UV filter and diffusing glass plate)
//
// A tungsten flood, filtered, behind diffusing glass. Three facts, and all
// three show up in those four numbers.
//
// ── The four points check out against Planck ─────────────────────────────
//
// If it is tungsten, the spectrum should be a blackbody. Fitting one to the
// 500, 600 and 700 nm points, with the scale free because the published
// numbers have no units:
//
//      best fit          3350 K, rms residual 0.73 on values of 8 to 18
//
//      predicted   500 nm   8.69    published   8.0
//                  600 nm  14.62               15.6
//                  700 nm  18.84               18.4
//
// A tungsten flood lamp runs at about 3200 to 3400 K. So the shape of the
// published spectrum independently confirms what the other page says the lamp
// was, which is the sort of agreement between two unrelated sources that is
// worth more than either.
//
// And the one place they disagree is the interesting one. The fitted
// blackbody predicts 3.10 at 400 nm; the published value is 0.0. That is the
// UV filter, which the same sentence documents. The data, the lamp
// description and Planck's law corroborate each other, and their single
// disagreement is explained by a component named in the source.
//
// ── The scale is arbitrary, so there is no flux in watts ─────────────────
//
// This item asked for the radiant flux quoted in watts, derived from the
// measured radiance and the area. That cannot be done, and the reason is
// stated by Cornell rather than inferred:
//
//      The following spectra are relative only; their scales are arbitrary.
//
// So the numbers above are a *shape*, not a measurement of how much light
// there is. An absolute flux requires an absolute radiance, and none was
// published.
//
// The derivation is still worth writing down, because it is one line and
// because v1.0 needs it the moment a scale exists. For a Lambertian emitter,
// radiant exitance is pi times radiance, and flux is exitance times area:
//
//      Phi = pi * L * A
//
// With the light's measured area of 0.013650 m^2, that is 0.042882 * L watts.
// Choose L and the flux follows; the choice is CALIBRATED and is made in the
// scene rather than here, so that this file stays a record of what was
// published.
//
// ── The geometry, and the hole it goes in ────────────────────────────────
//
// MEASURED. A quadrilateral at y = 548.8 mm spanning x from 213.0 to 343.0
// and z from 227.0 to 332.0: 130 mm by 105 mm, an area of 0.013650 m^2.
//
// It is exactly coplanar with the ceiling, which is also at 548.8, and that is
// not an oversight in the data — the published ceiling comes with a matching
// "Hole (for disc. mesh)" at precisely these four corners. The lamp fills a
// hole in the ceiling rather than hovering below it.
//
// `box.hpp` in v0.2 moved its placeholder lamp 10 mm down to dodge exactly
// this, and said why: two coplanar surfaces are a coin toss for every ray
// that reaches them. Cornell's answer is better and is the one the geometry
// specifies, so `cornell.hpp` cuts the hole.

inline constexpr int emission_first_nm = 400;
inline constexpr int emission_step_nm  = 100;
inline constexpr int emission_last_nm  = 700;
inline constexpr std::size_t emission_samples = 4;

static_assert(emission_first_nm + emission_step_nm * int(emission_samples - 1)
                  == emission_last_nm,
              "the emission grid must end at 700 nm");

using EmissionSpectrum = Measured<emission_samples>;

// MEASURED, relative. See above: this is a shape, not an amount.
inline constexpr EmissionSpectrum emission = {
    double(emission_first_nm) / 1e9, double(emission_step_nm) / 1e9,
    {0.0, 8.0, 15.6, 18.4}
};

// MEASURED. The lamp, in metres, from the published millimetres.
inline constexpr double light_y      = 548.8 / 1e3;
inline constexpr double light_x_min  = 213.0 / 1e3;
inline constexpr double light_x_max  = 343.0 / 1e3;
inline constexpr double light_z_min  = 227.0 / 1e3;
inline constexpr double light_z_max  = 332.0 / 1e3;

inline constexpr double light_area =
    (light_x_max - light_x_min) * (light_z_max - light_z_min);

// The derivation above, for whenever a radiance is chosen. Lambertian
// emitter: exitance is pi times radiance, flux is exitance times area.
constexpr double light_flux_for(double radiance) {
    return si::pi * radiance * light_area;
}

// ── The geometry ─────────────────────────────────────────────────────────
//
// MEASURED, in millimetres, from the physical box. The vertices below are
// Cornell's, transcribed unchanged — including the three different widths
// that a drawing would have made one.
//
//      floor, far edge     552.8       x at z = 0
//      floor, near edge    549.6       x at z = 559.2
//      ceiling             556.0
//
// That is not noise to be tidied away. The page says the surfaces are
// therefore not perfectly perpendicular, and it is the difference between a
// measurement and a specification. A renderer that squares the box up is
// rendering a different object from the one in the photographs.
//
// The units are INFERRED: nothing on the page states them, and they are
// millimetres because the camera block gives a focal length of 0.035 and a
// position 800 away. See item 0051.
//
// Orientation, so that nothing downstream has to work it out: +y is up, the
// opening is at z = 0 and the back wall at z = 559.2, and looking in from the
// opening puts +x on the *left*. The wall Cornell calls "Left wall" is the
// one at large x, and it is the red one — the naming is from the camera's
// point of view and agrees with the geometry, which is a small reassurance
// that somebody checked.

namespace mm {

// Everything below is in millimetres, converted once at the bottom.
inline constexpr double height = 548.8;
inline constexpr double depth  = 559.2;

// Floor, counter-clockwise seen from inside the box.
inline constexpr double floor_far  = 552.8;   // x at z = 0
inline constexpr double floor_near = 549.6;   // x at z = depth
inline constexpr double ceiling_x  = 556.0;

// The lamp, which fills a hole in the ceiling rather than hanging below it.
inline constexpr double light_x0 = 213.0, light_x1 = 343.0;
inline constexpr double light_z0 = 227.0, light_z1 = 332.0;

} // namespace mm

// A point of the published geometry, in metres. The division rather than a
// multiplication by 1e-3 is `si.hpp`'s rule about negative powers of ten.
constexpr Vec3 at(double x, double y, double z, double scale = 1.0) {
    return Vec3{scale * x / 1e3, scale * y / 1e3, scale * z / 1e3};
}

// ── What follows from it ─────────────────────────────────────────────────
//
// Nothing here is typed. Every quantity below is computed from the vertices
// above, which is the difference between a specification and a data file:
// a reader can check these against a paper, or against their own arithmetic,
// and `./cornell spec` prints them.

// The area of a planar quadrilateral, as two triangles. Correct for the
// non-rectangular floor, which is why it is not width times depth.
constexpr double quad_area(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d) {
    const double first  = 0.5 * std::sqrt(length_squared(cross(b - a, c - a)));
    const double second = 0.5 * std::sqrt(length_squared(cross(c - a, d - a)));
    return first + second;
}

// The solid angle a rectangle subtends from a point on its axis, exactly.
//
//      Omega = 4 arctan( a b / (d sqrt(a^2 + b^2 + d^2)) )
//
// with a and b the half-extents and d the distance. Derived from integrating
// the solid angle element over the rectangle; it is worth having in closed
// form because `./cornell spec` checks the numerical integration against it,
// and two routes to one number is the only kind of check this file can offer.
constexpr double axial_solid_angle(double half_width, double half_depth, double distance) {
    const double numerator = half_width * half_depth;
    const double denominator =
        distance * std::sqrt(half_width * half_width + half_depth * half_depth
                             + distance * distance);
    return 4.0 * std::atan(numerator / denominator);
}

// ── The scene ────────────────────────────────────────────────────────────
//
// CALIBRATED: the lamp's radiance. Cornell published the emission spectrum's
// *shape* and said its scale is arbitrary, so the absolute level is this
// project's choice and is marked as one. Everything else in the scene is
// measured.
//
// It is chosen so that the floor lands near the middle of a display's range
// under the exposure in `apps/render.hpp`, which is the same thing a
// photographer does and is not a claim about the lamp.
inline constexpr double light_radiance = 1.6;

namespace detail {

// Two triangles, wound so the normal comes out on the side asked for.
inline void add_quad(Scene& scene, const Vec3& a, const Vec3& b, const Vec3& c,
                     const Vec3& d, const Vec3& faces, const Bsdf& bsdf,
                     const Emission& emission = Flat{0.0}, double radiance = 0.0) {
    Triangle first{a, b, c};
    Triangle second{a, c, d};
    if (dot(first.geometric_normal(), faces) < 0.0) {
        first = Triangle{a, c, b};
        second = Triangle{a, d, c};
    }
    scene.add(Surface{first, bsdf, emission, radiance});
    scene.add(Surface{second, bsdf, emission, radiance});
}

} // namespace detail

// The box, as published.
//
// `scale` multiplies every length. It exists for one check and is otherwise
// 1: radiance is invariant under a uniform scaling of a scene, so the same
// room a thousand times larger must render identically, and if it does not
// then something in the spawn logic has a length hidden in it. `waechter.hpp`
// claims it does not; `./cornell verify` is where that is tested.
//
// `lamp` replaces the measured emission spectrum. It exists for the other
// check — `bradford.hpp` needs a light that is visibly not daylight to
// demonstrate anything — and using it means this is no longer the measured
// box, which `--lamp` says on the way past.
inline Scene box(double scale = 1.0, const Emission& lamp = emission) {
    using namespace mm;

    Scene scene;
    const Bsdf pale  = MeasuredLambert{white};
    const Bsdf left  = MeasuredLambert{red};
    const Bsdf right = MeasuredLambert{green};

    // Floor, and note the two different far and near widths.
    detail::add_quad(scene, at(floor_far, 0, 0, scale), at(0, 0, 0, scale),
                     at(0, 0, depth, scale), at(floor_near, 0, depth, scale),
                     Vec3{0, 1, 0}, pale);

    // Back wall.
    detail::add_quad(scene, at(floor_near, 0, depth, scale), at(0, 0, depth, scale),
                     at(0, height, depth, scale), at(ceiling_x, height, depth, scale),
                     Vec3{0, 0, -1}, pale);

    // Right wall, green, at x = 0.
    detail::add_quad(scene, at(0, 0, depth, scale), at(0, 0, 0, scale),
                     at(0, height, 0, scale), at(0, height, depth, scale),
                     Vec3{1, 0, 0}, right);

    // Left wall, red, at large x.
    detail::add_quad(scene, at(floor_far, 0, 0, scale), at(floor_near, 0, depth, scale),
                     at(ceiling_x, height, depth, scale), at(ceiling_x, height, 0, scale),
                     Vec3{-1, 0, 0}, left);

    // The ceiling, with a hole in it.
    //
    // The lamp is exactly coplanar with the ceiling and the published data
    // supplies a matching hole rather than expecting the two to be sorted out
    // by tie-breaking. Four quads around it: the strips in front of and
    // behind the lamp, and the two beside it.
    const Vec3 down{0, -1, 0};
    detail::add_quad(scene, at(0, height, 0, scale), at(ceiling_x, height, 0, scale),
                     at(ceiling_x, height, light_z0, scale), at(0, height, light_z0, scale), down, pale);
    detail::add_quad(scene, at(0, height, light_z1, scale), at(ceiling_x, height, light_z1, scale),
                     at(ceiling_x, height, depth, scale), at(0, height, depth, scale), down, pale);
    detail::add_quad(scene, at(0, height, light_z0, scale), at(light_x0, height, light_z0, scale),
                     at(light_x0, height, light_z1, scale), at(0, height, light_z1, scale), down, pale);
    detail::add_quad(scene, at(light_x1, height, light_z0, scale), at(ceiling_x, height, light_z0, scale),
                     at(ceiling_x, height, light_z1, scale), at(light_x1, height, light_z1, scale), down, pale);

    // The lamp, filling the hole.
    detail::add_quad(scene, at(light_x0, height, light_z0, scale), at(light_x1, height, light_z0, scale),
                     at(light_x1, height, light_z1, scale), at(light_x0, height, light_z1, scale),
                     down, Bsdf{MeasuredLambert{white}}, lamp, light_radiance);

    // The two blocks, each a top and four sides, vertices exactly as
    // published.
    const Vec3 up{0, 1, 0};
    detail::add_quad(scene, at(130,165,65, scale), at(82,165,225, scale), at(240,165,272, scale), at(290,165,114, scale), up, pale);
    detail::add_quad(scene, at(290,0,114, scale), at(290,165,114, scale), at(240,165,272, scale), at(240,0,272, scale),
                     Vec3{0.95, 0, 0.3}, pale);
    detail::add_quad(scene, at(130,0,65, scale), at(130,165,65, scale), at(290,165,114, scale), at(290,0,114, scale),
                     Vec3{0.3, 0, -0.95}, pale);
    detail::add_quad(scene, at(82,0,225, scale), at(82,165,225, scale), at(130,165,65, scale), at(130,0,65, scale),
                     Vec3{-0.95, 0, -0.3}, pale);
    detail::add_quad(scene, at(240,0,272, scale), at(240,165,272, scale), at(82,165,225, scale), at(82,0,225, scale),
                     Vec3{-0.3, 0, 0.95}, pale);

    detail::add_quad(scene, at(423,330,247, scale), at(265,330,296, scale), at(314,330,456, scale), at(472,330,406, scale), up, pale);
    detail::add_quad(scene, at(423,0,247, scale), at(423,330,247, scale), at(472,330,406, scale), at(472,0,406, scale),
                     Vec3{0.85, 0, -0.5}, pale);
    detail::add_quad(scene, at(472,0,406, scale), at(472,330,406, scale), at(314,330,456, scale), at(314,0,456, scale),
                     Vec3{0.3, 0, 0.95}, pale);
    detail::add_quad(scene, at(314,0,456, scale), at(314,330,456, scale), at(265,330,296, scale), at(265,0,296, scale),
                     Vec3{-0.95, 0, 0.3}, pale);
    detail::add_quad(scene, at(265,0,296, scale), at(265,330,296, scale), at(423,330,247, scale), at(423,0,247, scale),
                     Vec3{-0.3, 0, -0.95}, pale);

    scene.finalise();
    return scene;
}

// ── Checked at compile time ──────────────────────────────────────────────
//
// House rule 5, and the transcription of 228 numbers is exactly the kind of
// thing that is silent when it goes wrong. These are the facts about the
// published tables that a reader can verify against the page.

namespace check {

static_assert(white.lowest()  == 0.343, "white starts at 0.343 at 400 nm");
static_assert(white.highest() == 0.777, "white peaks at 0.777");
static_assert(green.lowest()  == 0.092, "green bottoms at 0.092");
static_assert(green.highest() == 0.481, "green peaks at 0.481");
static_assert(red.lowest()    == 0.040, "red bottoms at 0.040");
static_assert(red.highest()   == 0.657, "red peaks at 0.657");

// The endpoints, which pin the tables at both ends.
static_assert(white.table[0] == 0.343 && white.table[reflectance_samples - 1] == 0.737);
static_assert(green.table[0] == 0.092 && green.table[reflectance_samples - 1] == 0.159);
static_assert(red.table[0]   == 0.040 && red.table[reflectance_samples - 1]   == 0.642);

// No reflectance may exceed 1, or the wall is an energy source. Cornell's
// paint does not come close, and the assert is here because the day this file
// gains a fourth surface is the day somebody types a number.
constexpr bool physical(const WallSpectrum& s) {
    return s.lowest() >= 0.0 && s.highest() <= 1.0;
}
static_assert(physical(white) && physical(green) && physical(red),
              "a reflectance outside [0,1] is not a material");

// The red wall is not red the way a display is red. If this ever fails,
// somebody has replaced measured paint with a primary.
static_assert(red.highest() < 0.70, "the red wall reflects at most 0.657, not 1");
static_assert(green.highest() < 0.50, "the green wall reflects at most 0.481, not 1");

// ── The light ────────────────────────────────────────────────────────────

// `scene.hpp` has to name this grid to close its variant. If the two ever
// disagree, the walls are being rendered with somebody else's table.
static_assert(reflectance_samples == measured_reflectance_samples,
              "scene.hpp's measured-paint alternative must match Cornell's grid");
static_assert(emission_samples == measured_emission_samples,
              "scene.hpp's measured-emission alternative must match Cornell's grid");

static_assert(emission.table[0] == 0.0,  "the UV filter puts 400 nm at zero");
static_assert(emission.table[3] == 18.4, "700 nm is the brightest published point");

// Monotonically rising, which is what a tungsten spectrum does across the
// visible and what the blackbody fit above depends on.
static_assert(emission.table[0] < emission.table[1] &&
              emission.table[1] < emission.table[2] &&
              emission.table[2] < emission.table[3],
              "a tungsten spectrum rises towards the red across the visible");

// The measured lamp, in metres. 130 mm by 105 mm.
static_assert(light_x_max - light_x_min > 0.1299 && light_x_max - light_x_min < 0.1301);
static_assert(light_z_max - light_z_min > 0.1049 && light_z_max - light_z_min < 0.1051);
static_assert(light_area > 0.013649 && light_area < 0.013651,
              "the lamp is 0.013650 square metres");

// And the flux derivation, at a radiance of 1, which is pi * area.
static_assert(light_flux_for(1.0) > 0.042881 && light_flux_for(1.0) < 0.042883,
              "flux is pi times radiance times area");

} // namespace check

} // namespace render::cornell
