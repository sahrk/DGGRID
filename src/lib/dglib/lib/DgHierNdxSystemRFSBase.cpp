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
// DgHierNdxSystemRFSBase.cpp: DgHierNdxSystemRFSBase class definition.
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgHierNdxSystemRFSBase.h>
#include <dglib/DgHierNdxRF.h>
#include <dglib/DgDiscRFSGrids.h>
#include <dglib/DgHierNdx.h>

////////////////////////////////////////////////////////////////////////////////
const DgResAdd<DgHierNdx>
DgHierNdxSystemRFSBase::undefCoord(DgHierNdx::undefCoord, -1);

////////////////////////////////////////////////////////////////////////////////
DgHierNdxSystemRFSBase::DgHierNdxSystemRFSBase (const DgIDGGSBase& dggsIn, 
         bool outModeIntIn, const string& nameIn)
   : DgDiscRFS<DgHierNdx, DgQ2DICoord, long long int>
                         (dggsIn.network(), dggsIn, dggsIn.nRes(), nameIn),
     dggs_ (dggsIn), outModeInt_ (outModeIntIn)
{ 
   //grids_ = new vector<const DgHierNdxSystemRFBase*>(nRes_, nullptr);
}

////////////////////////////////////////////////////////////////////////////////
const DgResAdd<DgHierNdx>&
DgHierNdxSystemRFSBase::undefAddress (void) const
{
    return undefCoord;
}

////////////////////////////////////////////////////////////////////////////////
const DgHierNdxSystemRFBase&
DgHierNdxSystemRFSBase::sysRF (int res) const
{
    return *(static_cast<const DgHierNdxSystemRFBase*>(grids()[res]));
}

////////////////////////////////////////////////////////////////////////////////
const DgIDGGBase&
DgHierNdxSystemRFSBase::dgg (int res) const
{
    return dggs().idggBase(res);
}

////////////////////////////////////////////////////////////////////////////////
void
DgHierNdxSystemRFSBase::setNdxParent (int res, const DgLocation& loc,
                    DgLocation& parent) const
{
   parent.clearAddress();
   convert(&parent);

   if (res > 0 && res < nRes()) {
      DgLocation tmpLoc(loc);
      grids()[res]->convert(&tmpLoc);
      convert(&tmpLoc);
      setAddNdxParent(*(getAddress(tmpLoc)), parent);
   }

} // void DgHierNdxSystemRFSBase::setNdxParent

////////////////////////////////////////////////////////////////////////////////
void 
DgHierNdxSystemRFSBase::setNdxParent (const DgResAdd<DgHierNdx>& add, 
         const DgRFBase& rf, DgLocation& parent) const
{
  setNdxParent(add, parent);
  rf.convert(&parent);
}

////////////////////////////////////////////////////////////////////////////////
void 
DgHierNdxSystemRFSBase::setNdxParent (const DgResAdd<DgHierNdx>& add, 
                         DgLocation& parent) const
{
  //add.clearAddress();
  convert(&parent);
  if (add.res() > 0 && add.res() < nRes())
	         setAddNdxParent(add, parent);
}

////////////////////////////////////////////////////////////////////////////////
void
DgHierNdxSystemRFSBase::setNdxParent (const DgResAdd<DgQ2DICoord>& add,
                         DgLocation& parent) const
{
    DgResAdd<DgHierNdx> hierNdx = quantify(add);
    setNdxParent(hierNdx, parent);
}

////////////////////////////////////////////////////////////////////////////////
DgLocation* 
DgHierNdxSystemRFSBase::makeNdxParent (int res, const DgLocation& loc) const
{
  DgLocation* parent = new DgLocation(*this);
  setNdxParent(res, loc, *parent);

  return parent;
}

////////////////////////////////////////////////////////////////////////////////
DgLocation* 
DgHierNdxSystemRFSBase::makeNdxParent (const DgResAdd<DgHierNdx>& add) const
{
  DgLocation* parent = new DgLocation(*this);
  setNdxParent(add, *parent);
  return parent;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
DgHierNdxSystemRFSBase::setNdxChildren (int res, const DgLocation& loc,
                                   DgLocVector& children) const
{
   children.clearAddress();
   convert(children);

   if (res >= 0 && res < (nRes() - 1))
   {
      DgLocation tmpLoc(loc);
      grids()[res]->convert(&tmpLoc);
      convert(&tmpLoc);
      setAddNdxChildren(*(getAddress(tmpLoc)), children);
   }

} // void DgHierNdxSystemRFSBase::setNdxChildren

////////////////////////////////////////////////////////////////////////////////
void 
DgHierNdxSystemRFSBase::setNdxChildren (const DgResAdd<DgHierNdx>& add,
       const DgRFBase& rf, DgLocVector& chld) const
{
   setNdxChildren(add, chld);
   rf.convert(chld);
}

////////////////////////////////////////////////////////////////////////////////
void 
DgHierNdxSystemRFSBase::setNdxChildren (const DgResAdd<DgHierNdx>& add,
       DgLocVector& chld) const
{
  chld.clearAddress();
  convert(chld);
  if (add.res() >= 0 && add.res() < (nRes() - 1)) {
     setAddNdxChildren(add, chld);
  }
//KEVIN
    cout << add << ": " << chld << endl;
}

////////////////////////////////////////////////////////////////////////////////
void
DgHierNdxSystemRFSBase::setNdxChildren (const DgResAdd<DgQ2DICoord>& add,
       DgLocVector& chld) const
{
  DgResAdd<DgHierNdx> hierNdx = quantify(add);
  setNdxChildren(hierNdx, chld);
}

////////////////////////////////////////////////////////////////////////////////
DgLocVector* 
DgHierNdxSystemRFSBase::makeNdxChildren (int res,
     const DgLocation& loc) const
{
  DgLocVector* chld = new DgLocVector(*this);
  setNdxChildren(res, loc, *chld);

  return chld;
}

////////////////////////////////////////////////////////////////////////////////
DgLocVector* 
DgHierNdxSystemRFSBase::makeNdxChildren (const DgResAdd<DgHierNdx>& add) const
{
  DgLocVector* chld = new DgLocVector(*this);
  setNdxChildren(add, *chld);

  return chld;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
