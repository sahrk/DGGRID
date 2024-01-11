# __DGGRID__

## General Information

__DGGRID__ version 8.1b released January 10, 2024

__Southern Terra Cognita Laboratory__  
www.discreteglobalgrids.org  
[Kevin Sahr](http://www.linkedin.com/in/Kevin-Sahr), Director

__DGGRID__ is a command-line application for generating and manipulating
icosahedral discrete global grids (DGGs).

Three directories should be included herein:

- `src`: contains complete source code for __DGGRID__

   There are two ways that you can build __DGGRID__: you may use `cmake`
   (see [INSTALL.md](INSTALL.md) for instructions),
   or you can use the legacy build system from previous versions of __DGGRID__
   (see the text file README.NOCMAKE for instructions).

- `examples`: contains examples of using __DGGRID__ with pre-computed outputs.

- `dockerfiles`: contains a __DGGRID__ dockerfile and instructions for use

User documentation for __DGGRID__ is in `dggridManualV77.pdf`.

## Terms of Use

This documentation is part of __DGGRID__.

__DGGRID__ is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

__DGGRID__ is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with the __DGGRID__ source code (see the file `LICENSE`).  If not, see <https://www.gnu.org/licenses/>.

## Credits

__DGGRID__ was primarily written in C++ by [Kevin Sahr](http://www.linkedin.com/in/Kevin-Sahr). See the file `CHANGELOG.md` for additional contributors.

The original __DGGRID__ specifications were developed by (in alphabetical order):
A. Ross Kiester, Tony Olsen, Barbara Rosenbaum, Kevin Sahr, Ann Whelan, and
Denis White.

__DGGRID__ was made possible in part by funding from the __US Environmental Protection Agency__, __PlanetRisk, Inc.__, __Culmen International__, __the Ruhr-University Bochum/GeoInsight Project__, __Turtle Conservancy__, and the __University of Tartu Landscape Geoinformatics Lab__.

## Dependencies

__DGGRID__ will make use of the following external library, if available (not included):

- The Open Source Geospatial Foundation’s GDAL translator library for raster and vector geospatial data formats (see http://www.gdal.org)

__DGGRID__ uses the following external libraries (included with the __DGGRID__ source
code):

- Angus Johnson’s Clipper library; see http://www.angusj.com.

- George Marsaglia’s multiply-with-carry “Mother-of-all-RNGs” pseudo-random number generation function.

- The gnomonic projection code is adapted from Gerald Evenden’s PROJ.4 library.

- Frank Warmerdam’s Shapelib library
