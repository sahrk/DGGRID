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
// DgHierNdx.h: DgHierNdx class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDX_H
#define DGHIERNDX_H

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class DgHierNdx {
   public:
      
      DgHierNdx (const DgHierNdxIntCoord& intNdxIn, const DgHierNdxStringCoord& strNdxIn,
                 bool outModeIntIn = true)
         : intNdx_ (intNdxIn), strNdx_(strNdxIn), outModeInt_ (outModeIntIn)
         { }
    
      DgHierNdx (void);
    
      bool outModeInt (void) const { return outModeInt_; }
    
      const DgHierNdxIntCoord&    intNdx (void) const { return intNdx_; }
      const DgHierNdxStringCoord& strNdx (void) const { return strNdx_; }
      
      void setIntNdx (const DgHierNdxIntCoord& intNdxIn) { intNdx_ = intNdxIn; }
      void setStrNdx (const DgHierNdxStrCoord& strNdxIn) { strNdx_ = strNdxIn; }
    
      void setOutModeInt (bool outModeIntIn) { outModeInt_ = outModeIntIn; }

      operator string (void) const;

   private:

      DgHierNdxIntCoord intNdx_;
      DgHierNdxStringCoord strNdx_;
      bool outModeInt_;
};

////////////////////////////////////////////////////////////////////////////////
#endif
