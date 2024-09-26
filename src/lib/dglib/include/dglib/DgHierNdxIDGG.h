/*******************************************************************************
    Copyright (C) 2023 Kevin Sahr

    This file is part of DGGRID.

    DGGRID is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DGGRID is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
//
// DgHierNdxIDGG.h: DgHierNdxIDGG header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXIDGG_H
#define DGHIERNDXIDGG_H

#include <climits>
#include <iostream>

#include <dglib/DgDiscRF.h>
#include <dglib/DgIDGGS.h>
#include <dglib/DgIDGG.h>

class DgHierNdxRFInt;
class DgHierNdxIDGGSBase;

using namespace std;

////////////////////////////////////////////////////////////////////////////////
template <class TINT, class TSTR> class DgHierNdxIDGG : 
                   public DgHierNdxIDGGBase {

   protected:

      DgHierNdxIDGG<TINT, TSTR> (const DgHierNdxIDGGSBase& ndxHierDggsIn, int resIn, 
               const string& nameIn = "HierNdxIDGG");

};

////////////////////////////////////////////////////////////////////////////////
#include "../lib/DgHierNdxRFS.hpp"

////////////////////////////////////////////////////////////////////////////////
#endif
