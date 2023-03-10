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

    mkdir build                         # Make a directory to compile into
    cd build                            # Switch into build directory
    cmake -DCMAKE_BUILD_TYPE=Release .. # Prepare to compile
    make -j 4                           # Build code. Change number to compile in parallel
    make install                        # Optionally install on your machine

cmake will automatically detect where libraries are installed on your machine
and include them appropriately.

By default cmake will search for `GDAL` and link __DGGRID__ with it if found. If `GDAL` is present on your system and you want to force a build of __DGGRID__ without `GDAL`, call cmake with `-DWITH_GDAL=OFF`.

By default cmake will use an included vendored version of SHAPELIB. You can build also build and link against a system-provided version of shapelib. You can activate it by calling cmake with `-DWITH_EXT_SHAPELIB=ON`, and cmake search for `shapelib` and link __DGGRID__ with it if found. In addition you can provide `-DSHAPELIB_ROOT_DIR=/usr/..` as the search path.

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

See the manual for more details on the parameters contained in the example
metafiles.



Documentation
------------------

If you have doxygen installed, the source code documentation can be built in PDF and HTML formats using

    make docs

and can be found at `DGGRID/build/docs`.
