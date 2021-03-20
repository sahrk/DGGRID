#!/bin/sh

llvm-g++ -o dggrid -O1 -g -fsanitize=address -fno-omit-frame-pointer  -I../../lib/dglib/include -I../../lib/shapelib/include -I../../lib/proj4lib/include -I/opt/local/include -DUSE_GDAL -std=c++11 *.cpp ../../lib/dglib/lib/dglib.a ../../lib/shapelib/lib/shapelib.a ../../lib/proj4lib/lib/proj4lib.a /opt/local/lib/libgdal.dylib
