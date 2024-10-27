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
// DgDiscRFSGrids.hpp: DgDiscRFSGrids template class definition.
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgRF.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class DRF, class A, class B, class DB> const char*
DgDiscRFSGrids<DRF, A, B, DB>::str2add (DgResAdd<A>* add, const char* str,
                              char delimiter) const
{
   if (!add) add = new DgResAdd<A>();

   char delimStr[2];
   delimStr[0] = delimiter;
   delimStr[1] = '\0';

   char* tmpStr = new char[strlen(str) + 1];
   strcpy(tmpStr, str);

   char* tok;

   // get the resolution

   tok = strtok(tmpStr, delimStr);
   int res;
   if (sscanf(tok, "%d", &res) != 1)
   {
      ::report("DgDiscRFSGrids<A, B, DB>::str2add() invalid res string " +
               string(tok), DgBase::Fatal);
   }

   // now get the address

   const char* tmp = &(str[strlen(tok) + 1]);
   DgLocation tloc(*grids()[res]);
   tmp = tloc.fromString(tmp, delimiter);
   const A& subAdd = *grids()[res]->getAddress(tloc);
   *add = DgResAdd<A>(subAdd, res);

   return tmp;

} // const char* DgDiscRFSGrids::str2add

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
