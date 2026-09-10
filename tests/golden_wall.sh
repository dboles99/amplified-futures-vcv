#!/bin/sh
# Wall Conductor against frozen reference output.
#
#   sh tests/golden_wall.sh            check against the reference
#   sh tests/golden_wall.sh capture    regenerate the reference
#
# Capture only when the sound is deliberately changing, and say so in the
# changelog when you do. Regenerating to make a failing test pass is how a
# reference stops meaning anything.
set -e
RACK_DIR=${RACK_DIR:-/d/Music/dev-vcv/Rack-SDK}
RACK_APP=${RACK_APP:-"/c/Program Files/VCV/Rack2Pro"}
cd "$(dirname "$0")/.."

FLAGS="-std=c++11 -fPIC -O3 -funsafe-math-optimizations -march=nehalem -D_USE_MATH_DEFINES"
INC="-I$RACK_DIR/include -I$RACK_DIR/dep/include"
OBJS=$(ls build/src/*.o | tr '\n' ' ')
LINK="-L$RACK_DIR -lRack -static-libstdc++"

if [ "$1" = "capture" ]; then
	g++ $FLAGS -DAF_CAPTURE $INC -o tests/gw.exe tests/golden_wall.cpp $OBJS $LINK
	PATH="$RACK_APP:$PATH" ./tests/gw.exe > tests/golden_wall_data.hpp
	echo "captured tests/golden_wall_data.hpp"
else
	g++ $FLAGS $INC -o tests/gw.exe tests/golden_wall.cpp $OBJS $LINK
	PATH="$RACK_APP:$PATH" ./tests/gw.exe
fi
