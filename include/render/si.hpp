// si.hpp — the unit convention of this renderer.
//
// There is exactly one rule here, and every other file depends on it:
//
//      INSIDE THE RENDERER, EVERYTHING IS SI.
//
// Metres, kilograms, seconds, kelvin, watts, joules, steradians, radians.
// Always. No nanometres in a field, no degrees in a parameter, no electron
// volts in an intermediate. Those units exist only at the two surfaces of
// this program: the specification a user types in, and the number printed
// back out. Conversion happens at the surface and nowhere else.
//
// This is not pedantry. Optics is a field where a wavelength is quoted in
// nanometres, a photon energy in electron volts, a lens in millimetres, a
// filter in per-centimetre, an illuminance in lux and a star's brightness in
// magnitudes — all correct in their own context, several of them on the same
// page. The only way to keep the arithmetic in the middle honest is to refuse
// to let any of them in.
//
// A wavelength is the one that will hurt. It is 550 nm everywhere in the
// literature and 5.5e-7 m in here, forever, and the temptation to keep "just
// the wavelength" in nanometres because the numbers are friendlier is the
// first step toward a Sellmeier fit off by twelve orders of magnitude.
//
// Most of the literals below are never used. That is deliberate and it is the
// one place in this project where the rule against dead code does not apply,
// because this file is not a call graph — it is a dictionary. It is the
// statement of what units this renderer will and will not accept, and a
// statement with the inconvenient half left out is not one. They are
// `consteval`, so an unused one generates nothing at all: it costs a line of
// text and buys the completeness of the claim.

#pragma once

#include <numbers>

namespace render::si {

// ── Physical constants ────────────────────────────────────────────────────
//
// `windsor`, the engine this project is a sibling to, says of its own
// constants that they are "the engineering ones, not the CODATA ones; this is
// a machine, not a metrology lab". The opposite applies here, and for a
// reason worth a paragraph.
//
// Since the 2019 redefinition, the three constants below are not measured.
// They are *exact*, by definition: the SI fixes their numerical values and
// derives the metre, the kilogram and the kelvin from them. The second is
// defined by a caesium transition, the speed of light is then declared to be
// 299 792 458 m/s exactly, and the metre is whatever distance makes that
// true. There is no uncertainty to quote because there is no measurement.
//
// So these digits are not a precision this project has chosen. They are the
// whole number, and there are no more.

inline constexpr double pi        = std::numbers::pi;
inline constexpr double two_pi    = 2.0 * pi;
inline constexpr double inv_pi    = std::numbers::inv_pi;

inline constexpr double c_light   = 299792458.0;        // m/s     exact, 1983
inline constexpr double h_planck  = 6.62607015e-34;     // J·s     exact, 2019
inline constexpr double k_boltz   = 1.380649e-23;       // J/K     exact, 2019

// The visible range this project works in. 360 to 830 nm is the domain of the
// CIE 1931 colour matching functions, which is the only principled place to
// put the edges: outside it the observer is zero, so a wavelength out there
// cannot contribute to an image no matter how much energy it carries.
//
// It is wider than "visible" in any useful sense — the observer is below a
// ten-thousandth of its peak at both ends — and the width costs sampling
// efficiency. It is kept because the alternative is a cutoff chosen by
// somebody's judgement, and this one is chosen by the table.
inline constexpr double lambda_min = 360.0e-9;          // m
inline constexpr double lambda_max = 830.0e-9;          // m

// ── Literals: the surface where non-SI enters ─────────────────────────────
// Written so that a specification reads the way it is published.
//
//      auto peak    = 555.0_nm;       // where the photopic observer peaks
//      auto sodium  = 589.3_nm;       // the sodium D line
//      auto fifty   = 50.0_mm;        // a normal lens on 35 mm film
//      auto tungsten = 2700.0_K;

inline namespace literals {

// A note on how these are written, because the compiler caught this file out
// on its first afternoon and the correction is more interesting than the
// mistake.
//
// The obvious spelling of `_nm` is `v * 1e-9`, and it is wrong by one bit:
//
//      555.0e-9        9a1926e8699fa23e
//      555.0 * 1e-9    9b1926e8699fa23e
//      555.0 / 1e9     9a1926e8699fa23e
//
// `1e-9` is not representable in binary, so multiplying by it rounds twice —
// once when the constant is formed and once in the product. `1e9` *is*
// representable exactly, every power of ten up to 1e22 is, and IEEE division
// is correctly rounded, so dividing lands on the same double the decimal
// literal would have produced. So: negative powers are spelled as division.
//
// One bit in the ninth significant figure does not matter to an image. It
// matters to `static_assert(555.0_nm == 555.0e-9)`, which is the kind of
// check this project wants to be able to write, and a unit system that
// cannot survive being compared against the thing it converts is not one.
//
// What this does NOT fix, and the file should say so rather than imply a
// guarantee it has not got: a wavelength like 589.3 nm — the sodium D line —
// is not exactly representable to begin with. The literal operator is handed
// a `long double` that the compiler has already rounded, and on arm64 that
// type is a double, so the digits are gone before this code runs. The result
// is one ulp from `589.3e-9` and no amount of care in here can recover it.
// A raw literal operator taking `const char*` could parse the digits itself.
// That is a real fix, it is twenty lines of machinery, and it is not worth it
// for an error four hundred times smaller than the width of an atom.

// Length. `_nm` is the one that matters; the rest are here so that the file
// is a dictionary rather than a convenience.
consteval double operator""_m  (long double v) { return double(v);           }
consteval double operator""_cm (long double v) { return double(v) / 1e2;         }
consteval double operator""_mm (long double v) { return double(v) / 1e3;         }
consteval double operator""_um (long double v) { return double(v) / 1e6;         }
consteval double operator""_nm (long double v) { return double(v) / 1e9;         }
consteval double operator""_nm (unsigned long long v) { return double(v) / 1e9;         }
consteval double operator""_km (long double v) { return double(v) * 1e3;     }
consteval double operator""_in (long double v) { return double(v) * 0.0254;  }

// Area, volume
consteval double operator""_m2 (long double v) { return double(v);           }
consteval double operator""_m3 (long double v) { return double(v);           }
consteval double operator""_L  (long double v) { return double(v) / 1e3;         }

// Angle. Radians inside, degrees on the page, as everywhere.
consteval double operator""_deg(long double v) { return double(v) * pi / 180.0; }
consteval double operator""_rad(long double v) { return double(v);           }

// Solid angle. Steradians, and there is no other unit for it — which is why
// this literal exists only to let a specification say `4.0 * pi * 1.0_sr` and
// mean it. The square degree, used in astronomy, is deliberately absent.
consteval double operator""_sr (long double v) { return double(v);           }

// Energy and power
consteval double operator""_J  (long double v) { return double(v);           }
consteval double operator""_W  (long double v) { return double(v);           }
consteval double operator""_mW (long double v) { return double(v) / 1e3;         }
consteval double operator""_kW (long double v) { return double(v) * 1e3;     }

// Photon energy, quoted in electron volts by everyone who works on materials,
// and the reason Johnson and Christy's tables in v0.6 are indexed in eV
// rather than in nanometres. The conversion is exact, being a defined charge
// times a defined joule.
consteval double operator""_eV (long double v) { return double(v) * 1.602176634e-19; }

// Temperature. Absolute only: a blackbody at a negative kelvin is not a
// dimmer blackbody, it is an error that will propagate quietly through
// Planck's law and emerge as a negative radiance three files later.
consteval double operator""_K  (long double v) { return double(v);           }
consteval double operator""_C  (long double v) { return double(v) + 273.15;  }

// Time and frequency
consteval double operator""_s  (long double v) { return double(v);           }
consteval double operator""_ms (long double v) { return double(v) / 1e3;         }
consteval double operator""_Hz (long double v) { return double(v);           }
consteval double operator""_THz(long double v) { return double(v) * 1e12;    }

// Mass
consteval double operator""_kg (long double v) { return double(v);           }
consteval double operator""_g  (long double v) { return double(v) / 1e3;         }

} // namespace literals

// ── The other surface: SI back out to the page ────────────────────────────
// These are the only functions in the project permitted to divide by a
// conversion factor. Read them as "quote this in".

namespace as {
constexpr double m   (double m_)   { return m_;                    }
constexpr double mm  (double m_)   { return m_  * 1e3;             }
constexpr double um  (double m_)   { return m_  * 1e6;             }
constexpr double nm  (double m_)   { return m_  * 1e9;             }
constexpr double in  (double m_)   { return m_  / 0.0254;          }
constexpr double deg (double rad)  { return rad * 180.0 / pi;      }
constexpr double eV  (double J)    { return J   / 1.602176634e-19; }
constexpr double mW  (double W)    { return W   * 1e3;             }
constexpr double C   (double K)    { return K   - 273.15;          }
constexpr double THz (double Hz)   { return Hz  / 1e12;            }
} // namespace as

// ── Two derivations, because they are one line each ───────────────────────
// A wavelength, a frequency and a photon energy are three names for one
// quantity, and a file that holds all three constants should say so rather
// than leave the next reader to remember which way round it goes.

constexpr double frequency_of  (double lambda) { return c_light / lambda;  }
constexpr double wavelength_of (double nu)     { return c_light / nu;      }
constexpr double photon_energy (double lambda) { return h_planck * c_light / lambda; }

// And back, because a band structure is quoted in electron volts and a
// renderer works in metres. `swatch.hpp` scans in energy because that is the
// axis the physics is flat in; this is the only conversion between them and
// it is here rather than repeated at the call sites.
constexpr double wavelength_of_eV(double eV) {
    return h_planck * c_light / (eV * 1.602176634e-19);
}

// ── What is deliberately absent ───────────────────────────────────────────
//
// There are no photometric units here. No lumens, no lux, no candela, no
// nits. They are the radiometric units above weighted by the photopic
// luminosity function — which is to say, weighted by a curve fitted to a
// small number of human observers — and that curve is a `cie.hpp` concern.
//
// Putting a lumen in this file would put an eye inside the dictionary, and
// the whole argument of the project is that the eye arrives at the end, once,
// in a file that says whose eye it was.

} // namespace render::si
