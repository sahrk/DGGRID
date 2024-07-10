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
// DgNdxHierCPI.h: DgNdxHierCPI class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGNDXHIERCPI_H
#define DGNDXHIERCPI_H

////////////////////////////////////////////////////////////////////////////////
class DgNdxHierCPI : public DgNdxHierHexIDGGS {

   public:
    
      virtual operator string (void) const
      {
         string s = "*** DgNdxHierCPI";
         return s;
      }

   protected:

     DgNdxHierCPI (const DgHexIDGGS& hexDggsIn)
        : DgNdxHierHexIDGGS(hexDggsIn) { }

     // pure virtual functions passed down from above
     // defined here
     
     // parent is always center cell
     virtual void setAddNdxParent (const DgResAdd<DgQ2DICoord>& add,
                                   DgLocation& parent) const;

     // default definition returns all 7 CPI children
     virtual void setAddNdxChildren (const DgResAdd<DgQ2DICoord>& add,
                                     DgLocVector& children) const;

};

#endif
