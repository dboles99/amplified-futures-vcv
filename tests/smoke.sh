#!/bin/sh
# Build and run the offline smoke test.
#
# Unlike the core suites beside it, this one links the real libRack, so it
# needs a Rack installation and not just the SDK. RACK_APP is the directory
# holding Rack.exe, which is where libRack.dll lives.
#
#   sh tests/smoke.sh
#
set -e
RACK_DIR=${RACK_DIR:-/d/Music/dev-vcv/Rack-SDK}
RACK_APP=${RACK_APP:-"/c/Program Files/VCV/Rack2Pro"}
cd "$(dirname "$0")/.."

# The module classes live inside the .cpp files rather than in headers, so the
# harness reaches them through the exported Model pointers and needs the same
# objects the plugin is built from. Build the plugin first if they are absent.
if [ ! -f build/src/plugin.cpp.o ]; then
	echo "plugin objects missing, run make first"
	exit 1
fi

OBJS=$(ls build/src/*.o | tr '\n' ' ')

g++ -std=c++11 -O2 -g -march=nehalem -D_USE_MATH_DEFINES \
	-Wall -Wextra -Wno-unused-parameter \
	-I"$RACK_DIR/include" -I"$RACK_DIR/dep/include" \
	-o tests/smoke.exe tests/smoke.cpp $OBJS \
	-L"$RACK_DIR" -lRack -static-libstdc++

PATH="$RACK_APP:$PATH" ./tests/smoke.exe
