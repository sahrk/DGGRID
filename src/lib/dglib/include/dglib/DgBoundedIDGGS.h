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
// DgBoundedIDGGS.h: DgBoundedIDGGS class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGBOUNDEDIDGGS_H
#define DGBOUNDEDIDGGS_H

#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgIDGG.h>
#include <dglib/DgIDGGS.h>

class DgLocation;
class DgBoundedIDGGS;

////////////////////////////////////////////////////////////////////////////////
class DgBoundedIDGGS
        : public DgBoundedRF<DgResAdd<DgQ2DICoord>, DgGeoCoord, long double > {

   public:

      DgBoundedIDGGS (const DgIDGGS& rf);

      virtual DgResAdd<DgQ2DICoord>& incrementAddress
                                      (DgResAdd<DgQ2DICoord>& add) const;

      virtual DgResAdd<DgQ2DICoord>& decrementAddress
                                      (DgResAdd<DgQ2DICoord>& add) const;

      virtual bool validAddress (const DgResAdd<DgQ2DICoord>& add) const
          { return add == endAdd() || (add.res() >= 0 &&
                   add.res() < IDGGS().nRes() &&
                   grids()[add.res()]->validAddress(add.address())); }

      const DgResAdd<DgQ2DICoord>& invalidAdd (void) const
              { return IDGGS().undefAddress(); }

      const DgIDGGS& IDGGS (void) const { return IDGGS_; }

      const std::vector<const DgBoundedIDGG*>& grids (void) const { return *grids_; }

      virtual unsigned long long int seqNumAddress
                                  (const DgResAdd<DgQ2DICoord>& add) const;

      virtual DgResAdd<DgQ2DICoord> addFromSeqNum
                                  (unsigned long long int sNum) const;

   private:

      const DgIDGGS& IDGGS_;

      std::vector<const DgBoundedIDGG*>* grids_;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif
