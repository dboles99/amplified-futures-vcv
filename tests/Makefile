# Standalone tests for the pure-logic DSP cores. No Rack SDK involved:
# these headers deliberately have no Rack dependency so they can be tested
# without a plugin host. Run from this directory with `make test`.
CXX      ?= g++
CXXFLAGS ?= -std=c++11 -O2 -Wall -Wextra

test: test_clock_core
	./test_clock_core

test_clock_core: test_clock_core.cpp ../src/dsp/ClockCore.hpp
	$(CXX) $(CXXFLAGS) -o $@ test_clock_core.cpp

clean:
	rm -f test_clock_core test_clock_core.exe
.PHONY: test clean
