# __DGGRID__

## General Information

__DGGRID__ version 7.0 released September 1, 2019  
Southern Terra Cognita Laboratory / www.discreteglobalgrids.org  


__DGGRID__ is a command-line application designed to generate and manipulate 
icosahedral discrete global grids (DGGs).

Two directories should be included herein:

- `src`: contains complete source code for __DGGRID__ (see `README.MAKE` for 
     instructions on building the application and source code documenation)

- `examples`: contains examples of using __DGGRID__ with pre-computed outputs

User documentation for __DGGRID__ is in `dggridManualV70.pdf`.

## Terms of Use

This documentation is part of __DGGRID__.

__DGGRID__ is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

__DGGRID__ is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with the __DGGRID__ source code (see the file `LICENSE`).  If not, see <https://www.gnu.org/licenses/>.

## Credits

__DGGRID__ was primarily written in C++ by Kevin Sahr. See the file `CHANGELOG.md` 
for additional contributors. 

The original __DGGRID__ specifications were developed by (in alphabetical order): 
A. Ross Kiester, Tony Olsen, Barbara Rosenbaum, Kevin Sahr, Ann Whelan, and 
Denis White.

__DGGRID__ was made possible in part by funding from the __US Environmental Protection Agency__, __PlanetRisk, Inc.__, and __Culmen International__.

## Dependencies

__DGGRID__ requires the following external library (not included):

- The Open Source Geospatial Foundation’s GDAL translator library for raster and vector geospatial data formats (see http://www.gdal.org)

__DGGRID__ uses the following external libraries (included with the __DGGRID__ source 
code):

- Angus Johnson’s Clipper library; see http://www.angusj.com.

- George Marsaglia’s multiply-with-carry “Mother-of-all-RNGs” pseudo-random number generation function.



