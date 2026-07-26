# Standalone tests for the pure-logic DSP cores. No Rack SDK involved:
# these headers deliberately have no Rack dependency so they can be tested
# without a plugin host. Run from this directory with `make test`.
CXX      ?= g++
CXXFLAGS ?= -std=c++11 -O2 -Wall -Wextra

BINS = test_clock_core test_env_core

test: $(BINS)
	@for b in $(BINS); do echo "== $$b"; ./$$b || exit 1; echo; done

test_clock_core: test_clock_core.cpp ../src/dsp/ClockCore.hpp
	$(CXX) $(CXXFLAGS) -o $@ test_clock_core.cpp

test_env_core: test_env_core.cpp ../src/dsp/EnvCore.hpp
	$(CXX) $(CXXFLAGS) -o $@ test_env_core.cpp

clean:
	rm -f $(BINS) $(addsuffix .exe,$(BINS))
.PHONY: test clean
