# cornell — a box in a lab, modelled from first principles, for no reason.
#
# No dependencies and nothing to configure. A C++23 compiler and make.
# This should still work in fifteen years.

CXX      ?= c++
CXXFLAGS ?= -std=c++23 -O2 -Iinclude \
            -Wall -Wextra -Wpedantic -Wshadow -Wold-style-cast -Wdouble-promotion
SOURCES  := $(wildcard apps/*.cpp)
HEADERS  := $(wildcard include/render/*.hpp) $(wildcard apps/*.hpp)

cornell: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@

clean:
	rm -f cornell *.ppm *.pfm

.PHONY: clean
