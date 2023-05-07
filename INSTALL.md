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
    cmake -DCMAKE_BUILD_TYPE=Release ..  # Prepare to compile
    make                                 # Build code.
    make install                         # Optionally install on your machine

cmake will automatically detect where libraries are installed on your machine
and include them appropriately.

By default cmake will search for `GDAL` and link __DGGRID__ with it if found. If `GDAL` is present on your system and you want to force a build of __DGGRID__ without `GDAL`, call cmake with `-DWITH_GDAL=OFF`.

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
