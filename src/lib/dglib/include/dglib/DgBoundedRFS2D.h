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
// DgBoundedRFS2D.h: DgBoundedRFS2D class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGBOUNDEDRFS2D_H
#define DGBOUNDEDRFS2D_H

#include <dglib/DgBoundedRF2D.h>
#include <dglib/DgDiscRFS2D.h>
#include <dglib/DgIVec2D.h>

class DgLocation;
class DgBoundedRFS2D;

////////////////////////////////////////////////////////////////////////////////
class DgBoundedRFS2D
        : public DgBoundedRF<DgResAdd<DgIVec2D>, DgDVec2D, long double > {

   public:

      DgBoundedRFS2D (const DgDiscRFS2D& rf,
                      const DgIVec2D& lowerLeft0, const DgIVec2D& upperRight0);

      virtual DgResAdd<DgIVec2D>& incrementAddress
                                      (DgResAdd<DgIVec2D>& add) const;

      virtual DgResAdd<DgIVec2D>& decrementAddress
                                      (DgResAdd<DgIVec2D>& add) const;

      virtual bool validAddress (const DgResAdd<DgIVec2D>& add) const
          { return add == endAdd() || (add.res() >= 0 &&
                   add.res() < discRFS().nRes() &&
                   grids()[add.res()]->validAddress(add.address())); }

      const DgResAdd<DgIVec2D>& invalidAdd (void) const
              { return discRFS().undefAddress(); }

      const DgDiscRFS2D& discRFS (void) const { return discRFS_; }

      const std::vector<const DgBoundedRF2D*>& grids (void) const { return *grids_; }

      virtual unsigned long long int seqNumAddress (const DgResAdd<DgIVec2D>& add)
                                                                        const;

      virtual DgResAdd<DgIVec2D> addFromSeqNum (unsigned long long int sNum) const;

   private:

      const DgDiscRFS2D& discRFS_;

      std::vector<const DgBoundedRF2D*>* grids_;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif
