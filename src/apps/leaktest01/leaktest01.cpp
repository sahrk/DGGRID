/*******************************************************************************
    Copyright (C) 2021 Kevin Sahr

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
// leaktest01.cpp: simple application to experiment with memory leaks
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

using namespace std;

#include "DgRFNetwork.h"
#include "DgContCartRF.h"

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char* argv[])
{
   // create a reference frame (RF) conversion network
   DgRFNetwork net0;
   cout << "net0 size: " << net0.size() << endl;

   const DgContCartRF* ccPtr1 = new DgContCartRF(net0);
   cout << "net0 size: " << net0.size() << endl;

   const DgContCartRF* ccPtr2 = new DgContCartRF(net0);
   cout << "net0 size: " << net0.size() << endl;

   Dg2WayContAffineConverter conv1(*ccPtr1, *ccPtr2);

   DgLocVector v(*ccPtr1);
   DgDVec2D p(1, 2);
   DgLocation* loc = ccPtr1->makeLocation(p);
   v.push_back(*loc);

   delete loc;

   return 0;

} // main 

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
