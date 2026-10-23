DGGRID Unix Make Instructions
=============================

These instructions assume you are on a Unix/Linux system, though with cmake they
should be broadly compatible.



Acquire the Code
------------------

Acquire the code with:

    git clone https://github.com/sahrk/DGGRID.git



Building DGGRID
------------------

Build the application dggrid by executing:

    cd DGGRID                            # enter the cloned repo
    mkdir build                          # Make a directory to compile into
    cd build                             # Switch into build directory
    cmake ..                             # Prepare to compile
    make                                 # Build code.
    make install                         # Optionally install on your machine

If no build type is given, cmake now defaults to an optimized `Release` build
(`-O3 -DNDEBUG`); passing `-DCMAKE_BUILD_TYPE=...` explicitly still overrides
this. Multi-configuration generators such as Xcode and Visual Studio select the
configuration at build time instead.

cmake will automatically detect where libraries are installed on your machine
and include them appropriately.

By default cmake will search for `GDAL` and link __DGGRID__ with it if found. If `GDAL` is present on your system and you want to force a build of __DGGRID__ without `GDAL`, call cmake with `-DWITH_GDAL=OFF`.

By default cmake will also search for the H3 library (version 4.4.1 or later) and link __DGGRID__ with it if found; H3 is needed only for H3 hierarchical indexing. An older H3, or one that does not export the internal FaceIJK functions __DGGRID__ uses, is ignored. If `H3` is present on your system and you want to force a build of __DGGRID__ without `H3`, call cmake with `-DWITH_H3=OFF`. To use an H3 installed in a nonstandard location, add `-DCMAKE_PREFIX_PATH=/path/to/h3`.

By default cmake will use the `shapelib` library included with the source code.
To build using an external system-provided version of `shapelib` execute `cmake`
with `-DWITH_EXT_SHAPELIB=ON`. To specify a specific directory path to search for
`shapelib` add `-DSHAPELIB_ROOT_DIR=/..`.

You can also build DGGRID with extra debugging info. Doing this requires
emptying your `build/` directory first or making a new `build_debug/` directory.

    # Optimizations + Debugging info
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
    # Optionally, prepare a debugging build with limited or no optimizations
    cmake -DCMAKE_BUILD_TYPE=Debug ..


Upon Success
------------------

If/when the make is successful there will be a command line executable file
`dggrid` in the directory `DGGRID/build/src/apps/dggrid`. This is the DGGRID
program. Assuming dggrid is in your command search path you may execute it
with:

    dggrid someMetaFileName.meta

and/or with either of the informational flags:

    dggrid -v
    dggrid -h

Both flags report whether DGGRID was built with GDAL and with H3.

See the DGGRID documentation and examples for details of metafile contents.

Building also produces the sample application
`DGGRID/build/src/apps/appex/appex`, which is intended as a simple demonstration
of using calls to the dglib library to manipulate DGG cells in source code.



Examples
------------------

The directory `DGGRID/examples` contains a set of example DGGRID runs
with pre-computed output. You can run these examples and automatically compare
your output to the pre-computed output by going into `DGGRID/examples` and
executing:

    ./doexamples.sh

Some of the examples use GDAL and some use H3; if you built DGGRID without
GDAL or without H3 use `./doexamplesNoGDALorH3.sh` instead (see
examples/README.txt).

See the manual for more details on the parameters contained in the example
metafiles.

The developer tool `cmake/h3-version-check` builds DGGRID against several
H3 releases to check the minimum supported version (needs network access,
about 10 minutes):

    cmake -S cmake/h3-version-check -B build-h3check
    cmake --build build-h3check --target check



Documentation
------------------

If you have doxygen installed, the CMake C++ API docs can be built using

    make docs

and can be found at `DGGRID/build/docs`.

The user manual (HTML and PDF) is a separate Sphinx project. See
[documentation/README.md](documentation/README.md). Read the Docs builds the
HTML site from git; you do not need to build HTML locally for that.
