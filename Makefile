# cornell — a box in a lab, modelled from first principles, for no reason.
#
#     make && ./cornell
#
# A C++23 compiler and make. That is the whole list, and keeping it that short
# is house rule 4, which is not asceticism: this repository should still build
# in fifteen years, and every dependency is a thing that has to still exist,
# still be fetchable, still compile, and still mean what it meant. A PPM is a
# short ASCII header and some bytes. An image library is a promise somebody
# else has to keep.
#
# There is no configure step and nothing to generate, because there is nothing
# to detect. The program uses no platform facility that is not in the standard
# library, so there is no path where a build system asks a question and gets an
# answer this project would do anything differently about.
#
# One translation unit. Everything under include/render is a header, because
# every part of the physics is small enough to read in one sitting, and the
# whole program compiles in well under a second — so there are no object files
# to stale, no link order, and no incremental build to be subtly wrong.
#
# ── The warnings, and which one earns its place ─────────────────────────────
#
#   -Wall -Wextra -Wpedantic   the ordinary floor
#   -Wshadow                   a shadowed name in a nested loop over four
#                              wavelengths is a bug that renders
#   -Wold-style-cast           C casts hide exactly the conversions below
#   -Wconversion               an int that becomes a double, or a size_t that
#                              becomes an int, silently
#   -Wsign-conversion          the same, across the sign, which is where the
#                              array index bugs live
#   -Wdouble-promotion         the one that earns its place here more than in
#                              most projects: a stray float widening in the
#                              middle of a spectral integral is quiet, is
#                              plausible, and is wrong in the eighth digit —
#                              which is exactly the digit the verification in
#                              v0.5 is trying to read
#
# Not -Wfloat-equal, and the refusal is deliberate. This project compares
# doubles for exact equality on purpose and will do it more: `is_black()` on a
# spectrum, a count of zero, and the static_asserts in sphere.hpp whose entire
# point is that one spelling of a discriminant lands on exactly 0.75 and the
# other on exactly 0. A warning you have to suppress in the places it fires is
# a warning that has stopped being read.
#
# Not -Werror either, for the fifteen-year reason. A compiler released in 2034
# will have warnings that do not exist today, and the first thing it will do
# with -Werror is refuse to build a program that is entirely correct. `make
# strict` turns it on for the person who wants it, which is the right default
# for a repository and the wrong one for a checkout.

CXX      ?= c++
CXXFLAGS ?= -std=c++23 -O2 -Iinclude \
            -Wall -Wextra -Wpedantic -Wshadow -Wold-style-cast \
            -Wconversion -Wsign-conversion -Wdouble-promotion

SOURCES  := $(wildcard apps/*.cpp)
HEADERS  := $(wildcard include/render/*.hpp) $(wildcard apps/*.hpp)

cornell: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@

# The same build, with every warning fatal. Not the default; see above.
strict:
	$(MAKE) clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -Werror"

# Another compiler, which is the only portability test this project has until
# somebody runs it somewhere else:
#
#     make CXX=g++-14
#
# Two front ends disagreeing about a program is almost always the program's
# fault, and finding out costs one line.

clean:
	rm -f cornell *.ppm *.pfm

# A link that fails should not leave a binary behind for the next command to
# find and believe.
.DELETE_ON_ERROR:

.PHONY: clean strict
