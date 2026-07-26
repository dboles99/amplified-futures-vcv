# Standalone tests for the pure-logic DSP cores. No Rack SDK involved:
# these headers deliberately have no Rack dependency so they can be tested
# without a plugin host. Run from this directory with `make test`.
CXX      ?= g++
CXXFLAGS ?= -std=c++11 -O2 -Wall -Wextra

# On Windows the compiler writes foo.exe even when told -o foo, so a target
# named `foo` is never satisfied and make cannot tell fresh from stale. Name
# the targets after the file that actually appears. Without this, an unrelated
# file sitting where the target should be is treated as an up-to-date build:
# three Linux ELF binaries dropped into this directory once did exactly that,
# and `make test` skipped three of four suites while still exiting 0.
EXE     := $(if $(filter Windows_NT,$(OS)),.exe,)
BINS     = test_clock_core test_env_core test_vca_core test_ratchet_core test_bloc_core
TARGETS  = $(addsuffix $(EXE),$(BINS))

test: $(TARGETS)
	@for b in $(TARGETS); do echo "== $$b"; ./$$b || exit 1; echo; done

test_clock_core$(EXE): test_clock_core.cpp ../src/dsp/ClockCore.hpp
	$(CXX) $(CXXFLAGS) -o $@ test_clock_core.cpp

test_env_core$(EXE): test_env_core.cpp ../src/dsp/EnvCore.hpp
	$(CXX) $(CXXFLAGS) -o $@ test_env_core.cpp

test_vca_core$(EXE): test_vca_core.cpp ../src/dsp/VcaCore.hpp
	$(CXX) $(CXXFLAGS) -o $@ test_vca_core.cpp

test_ratchet_core$(EXE): test_ratchet_core.cpp ../src/dsp/RatchetCore.hpp
	$(CXX) $(CXXFLAGS) -o $@ test_ratchet_core.cpp

test_bloc_core$(EXE): test_bloc_core.cpp ../src/dsp/BlocCore.hpp
	$(CXX) $(CXXFLAGS) -o $@ test_bloc_core.cpp

clean:
	rm -f $(BINS) $(addsuffix .exe,$(BINS))
.PHONY: test clean
