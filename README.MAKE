DGGRID Unix Make Instructions
=============================

These instructions assume you are on a Unix/Linux system.

**1. Go into the DGGRID source directory, DGGRID/src.

**2. Some basic Make settings are given in the text file src/MakeIncludes, 
and are used by the Makefiles in the sub-directories. Edit this file if you
want to add or remove build flags.

In particular, you must set GDAL_HDR_LIB to the location of the gdal 
header files and GDAL_LIB to the gdal library file you want to link to 
DGGRID.

**3. Build the application dggrid by executing: 

   make

**4. If/when the make is successful there will be a command line 
executable file "dggrid" in the directory DGGRID/src/apps/dggrid. 
This is the DGGRID program. Assuming dggrid is in your command search 
path you may execute it with:

dggrid someMetaFileName.meta

See the DGGRID documentation and examples for details of metafile contents.

**5. The directory DGGRID/examples contains a set of example DGGRID runs
with pre-computed output. You can run these examples and automatically compare
your output to the pre-computed output by going into DGGRID/examples and
executing:

   ./doexamples

See the manual for more details on the parameters contained in the example
metafiles.

**6. If you have doxygen installed you can build the source code documentation
by going into DGGRID/src and executing:

   doxygen

This will create a directory DGGRID/src/docs containing the generated
source code documentation in pdf and html formats.

Note: Step 3 also builds the sample application DGGRID/src/apps/appex/appex, 
which is intended as a simple demonstration of using calls to the dglib 
library to manipulate DGG cells in source code.
