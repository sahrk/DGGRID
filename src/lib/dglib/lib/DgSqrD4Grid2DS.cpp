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
// DgSqrD4Grid2DS.cpp: DgSqrD4Grid2DS class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>

#include <dglib/DgContCartRF.h>
#include <dglib/DgDiscRF.h>
#include <dglib/DgSqrD4Grid2D.h>
#include <dglib/DgSqrD4Grid2DS.h>
#include <dglib/DgSqrD8Grid2D.h>

using namespace dgg::topo;

////////////////////////////////////////////////////////////////////////////////
DgSqrD4Grid2DS::DgSqrD4Grid2DS (DgRFNetwork& networkIn,
               const DgRF<DgDVec2D, long double>& backFrameIn, int nResIn,
               unsigned int apertureIn, bool isCongruentIn, bool isAlignedIn,
               const string& nameIn)
        : DgDiscRFS2D (networkIn, backFrameIn, nResIn, apertureIn, Square, D4,
                       isCongruentIn, isAlignedIn, nameIn)
{
   // determine the radix

   radix_ = static_cast<int>(sqrt(static_cast<float>(aperture())));
   if (static_cast<unsigned int>(radix() * radix()) != aperture())
   {
      report(
      "DgSqrD4Grid2DS::DgSqrD4Grid2DS() aperture must be a perfect square",
       DgBase::Fatal);
   }

   if (isAligned() && radix() != 2 && radix() != 3)
   {
      report("DgSqrD4Grid2DS::DgSqrD4Grid2DS() only aligned apertures 4 and 9 "
             " parent/children operators fully implemented", DgBase::Warning);
   }

   // do the grids

   long double fac = 1;

   DgDVec2D trans;
   if (isCongruent())
   {
      trans = DgDVec2D(-0.5, -0.5);
   }
   else if (isAligned())
   {
      trans = DgDVec2D(0.0, 0.0);
   }
   else
   {
      report("DgSqrD4Grid2DS::DgSqrD4Grid2DS() grid system must be either "
             "congruent, aligned, or both", DgBase::Fatal);
   }

   for (int i = 0; i < nRes(); i++)
   {
      string newName = name() + "_" + dgg::util::to_string(i);

      //cout << newName << " " << fac << ' ' << trans << endl;

      const DgContCartRF* ccRF = DgContCartRF::makeRF(network(), newName + string("bf"));

      Dg2WayContAffineConverter(backFrame(), *ccRF, (long double) fac, 0.0, trans);

      (*grids_)[i] = DgSqrD4Grid2D::makeRF(network(), *ccRF, newName);
      Dg2WayResAddConverter<DgIVec2D, DgDVec2D, long double>(*this, *(grids()[i]), i);

      fac *= radix();
   }

} // DgSqrD4Grid2DS::DgSqrD4Grid2DS

////////////////////////////////////////////////////////////////////////////////
DgSqrD4Grid2DS::DgSqrD4Grid2DS (const DgSqrD4Grid2DS& rf)
  : DgDiscRFS2D (rf)
{
   report("DgSqrD4Grid2DS::operator=() not implemented yet", DgBase::Fatal);

} // DgSqrD4Grid2DS::DgSqrD4Grid2DS

////////////////////////////////////////////////////////////////////////////////
DgSqrD4Grid2DS::~DgSqrD4Grid2DS (void)
{ } // DgSqrD4Grid2DS::~DgSqrD4Grid2DS

////////////////////////////////////////////////////////////////////////////////
DgSqrD4Grid2DS&
DgSqrD4Grid2DS::operator= (const DgSqrD4Grid2DS&)
{
   report("DgSqrD4Grid2DS::operator=() ", DgBase::Fatal);

   return *this;

} // DgSqrD4Grid2DS& DgSqrD4Grid2DS::operator=

////////////////////////////////////////////////////////////////////////////////
void
DgSqrD4Grid2DS::setAddParents (const DgResAdd<DgIVec2D>& add,
                               DgLocVector& vec) const
{
//cout << "   setAddParents: " << add << endl;
   if (isCongruent() || radix() == 3)
   {
      DgLocation* tmpLoc = makeLocation(add);
      grids()[add.res() - 1]->convert(tmpLoc);
      convert(tmpLoc);

      vec.push_back(*tmpLoc);

      delete tmpLoc;
   }
   else // must be aligned aperture 4
   {
      // vertices lie in parents

      DgLocation* tmpLoc = makeLocation(add);
      DgPolygon* verts = makeVertices(*tmpLoc);
      delete tmpLoc;

//cout << "   verts 1: " << *verts << endl;

      grids()[add.res() - 1]->convert(*verts);
//cout << "   verts 2: " << *verts << endl;
      convert(*verts);
//cout << "   verts 3: " << *verts << endl;

      for (int i = 0; i < verts->size(); i++)
      {
         bool found = false;

         for (int j = 0; j < vec.size(); j++)
         {
//cout << "  " << i << " " << j << " " << (*verts)[i] << " " << vec[j];
            if ((*verts)[i] == vec[j])
            {
//cout << " YES" << endl;
               found = true;
               break;
            }
//cout << " NO" << endl;
         }

         if (!found) vec.push_back((*verts)[i]);
      }
//cout << "   parents: " << vec << endl;

      delete verts;
   }

} // void DgSqrD4Grid2DS::setAddParents

////////////////////////////////////////////////////////////////////////////////
void
DgSqrD4Grid2DS::setAddInteriorChildren (const DgResAdd<DgIVec2D>& add,
                                        DgLocVector& vec) const
{
   if (isCongruent() || radix() == 3)
   {
      const DgIVec2D& lowerLeft = add.address() * radix();

      vector<DgAddressBase*>& v = vec.addressVec();
      for (int i = 0; i < radix(); i++)
      {
         for (int j = 0; j < radix(); j++)
         {
            v.push_back(new DgAddress< DgResAdd<DgIVec2D> >(
             DgResAdd<DgIVec2D>(DgIVec2D(lowerLeft.i() + i, lowerLeft.j() + j),
                               add.res() + 1)));
         }
      }
   }
   else // must be aligned aperture 4
   {
      // only center square is interior

      DgLocation* tmpLoc = makeLocation(add);
      grids()[add.res() + 1]->convert(tmpLoc);
      vec.push_back(*tmpLoc);

      delete tmpLoc;
   }

} // void DgSqrD4Grid2DS::setAddInteriorChildren

////////////////////////////////////////////////////////////////////////////////
void
DgSqrD4Grid2DS::setAddBoundaryChildren (const DgResAdd<DgIVec2D>& add,
                                        DgLocVector& vec) const
{
   if (isCongruent() || radix() == 3)
   {
      // no boundary children in this topology; leave vec empty
   }
   else // must be aligned aperture 4
   {
      DgLocation* tmpLoc = makeLocation(add);

      // D8 neighbors is what we want

      const DgSqrD8Grid2D& d8 = (* DgSqrD8Grid2D::makeRF(network(), grids()[add.res() + 1]->backFrame(),
                       "dummyD8"));
      d8.convert(tmpLoc);
      d8.setNeighbors(*tmpLoc, vec);

      grids()[add.res() + 1]->convert(vec);
      convert(vec);

      delete tmpLoc;
   }

} // void DgSqrD4Grid2DS::setAddBoundaryChildren

////////////////////////////////////////////////////////////////////////////////
void
DgSqrD4Grid2DS::setAddAllChildren (const DgResAdd<DgIVec2D>& add,
                                   DgLocVector& vec) const
{
   setAddInteriorChildren(add, vec);

   DgLocVector bndVec(vec.rf());
   setAddBoundaryChildren(add, bndVec);

   for (int i = 0; i < bndVec.size(); i++) vec.push_back(bndVec[i]);

} // void DgSqrD4Grid2DS::setAddAllChildren

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
