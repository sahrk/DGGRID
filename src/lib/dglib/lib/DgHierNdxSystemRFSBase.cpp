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
#include <dglib/DgZ3System.h>
#include <dglib/DgZ7System.h>
#include <dglib/DgZOrderSystem.h>
#include <dglib/DgH3System.h>

////////////////////////////////////////////////////////////////////////////////
DgHierNdxSystemRFSBase*
DgHierNdxSystemRFSBase::makeSystem (const DgIDGGSBase& dggsIn,
               DgHierNdxSysType sysType, DgHierNdxFormType form,
               const std::string& nameIn)
{
   DgHierNdxSystemRFSBase* sys = nullptr;

   if (sysType == InvalidHierNdxSysType) {
        ::report("DgHierNdxSystemRFSBase::makeSystem() invalid system type", DgBase::Fatal);
        return sys;
   }
   if (form != Int64 && form != DigitString) {
        ::report("DgHierNdxSystemRFSBase::makeSystem() " +
                 dgg::addtype::to_string(sysType) +
                 " requires INT64 or DIGIT_STRING form", DgBase::Fatal);
        return sys;
   }

   bool extModeInt = (form == Int64);
   switch (sysType) {
      case dgg::addtype::Z7:
         if (!dggsIn.isPure() || dggsIn.aperture() != 7) {
            ::report("DgHexIDGGS::makeRF() Z7 only available for pure aperture 7 grids", DgBase::Fatal);
         }
         sys = DgZ7System::makeSystem(dggsIn, extModeInt, "Z7");
         break;
       case dgg::addtype::ZOrder:
         if (!dggsIn.isPure() || (dggsIn.aperture() != 3 && dggsIn.aperture() != 4)) {
            ::report("DgHexIDGGS::makeRF() ZOrder only available for pure aperture 3 or 4 grids", DgBase::Fatal);
         }
         sys = DgZOrderSystem::makeSystem(dggsIn, extModeInt, "ZOrder");
         break;
       case dgg::addtype::Z3:
         if (!dggsIn.isPure() || dggsIn.aperture() != 3) {
            ::report("DgHexIDGGS::makeRF() Z3 only available for pure aperture 3 grids", DgBase::Fatal);
         }
         sys = DgZ3System::makeSystem(dggsIn, extModeInt, "Z3");
         break;
       case dgg::addtype::H3:
#ifdef USE_H3
         DgH3System::validateCompatibility(dggsIn, form);
         sys = DgH3System::makeSystem(dggsIn, extModeInt, "H3");
#else
         ::report("H3 hierarchical indexing requires DGGRID built with the H3 "
                  "library", DgBase::Fatal);
#endif
         break;
       case dgg::addtype::InvalidHierNdxSysType: // shouldn't be possible
         ::report("DgHexIDGGS::makeRF() invalid hierarchical system type", DgBase::Fatal);
         break;
   }

   return sys;
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxSystemRFSBase::DgHierNdxSystemRFSBase (const DgIDGGSBase& dggsIn,
         bool extModeIntIn, const std::string& nameIn,
         int firstNdxResIn, int lastNdxResIn)
   : DgDiscRFS<DgHierNdx, DgQ2DICoord, long long int>
                         (dggsIn.network(), dggsIn, dggsIn.nRes(), nameIn),
     dggs_ (dggsIn), extModeInt_ (extModeIntIn),
     firstNdxRes_ (firstNdxResIn), lastNdxRes_ (lastNdxResIn)
{
   if (lastNdxRes_ < 0 || lastNdxRes_ > dggsIn.nRes() - 1)
      lastNdxRes_ = dggsIn.nRes() - 1;

   //grids_ = new std::vector<const DgHierNdxSystemRFBase*>(nRes_, nullptr);
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
int
DgHierNdxSystemRFSBase::ndxParentRes (int res) const
{
   return (isNdxRes(res) && isNdxRes(res - 1)) ? res - 1 : -1;
}

////////////////////////////////////////////////////////////////////////////////
int
DgHierNdxSystemRFSBase::ndxChildRes (int res) const
{
   return (isNdxRes(res) && isNdxRes(res + 1)) ? res + 1 : -1;
}

////////////////////////////////////////////////////////////////////////////////
unsigned int
DgHierNdxSystemRFSBase::apertureAtResolution (int res) const
{
   if (res < 0 || res >= nRes()) {
      ::report("DgHierNdxSystemRFSBase::apertureAtResolution() resolution out of range",
               DgBase::Fatal);
   }
   return dgg(res).aperture();
}

////////////////////////////////////////////////////////////////////////////////
unsigned int
DgHierNdxSystemRFSBase::parentStepAperture (int res) const
{
   if (res <= 0 || res >= nRes()) {
      ::report("DgHierNdxSystemRFSBase::parentStepAperture() no parent step at this resolution",
               DgBase::Fatal);
   }
   return apertureAtResolution(res);
}

////////////////////////////////////////////////////////////////////////////////
unsigned int
DgHierNdxSystemRFSBase::childStepAperture (int res) const
{
   if (res < 0 || res >= nRes() - 1) {
      ::report("DgHierNdxSystemRFSBase::childStepAperture() no child step at this resolution",
               DgBase::Fatal);
   }
   return apertureAtResolution(res + 1);
}

////////////////////////////////////////////////////////////////////////////////
void
DgHierNdxSystemRFSBase::setNdxParent (int res, const DgLocation& loc,
                    DgLocation& parent) const
{
   parent.clearAddress();
   convert(&parent);

   if (ndxParentRes(res) >= 0) {
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
  if (ndxParentRes(add.res()) >= 0)
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

   if (ndxChildRes(res) >= 0)
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
  if (ndxChildRes(add.res()) >= 0) {
     setAddNdxChildren(add, chld);
  }
  //std::cout << add << ": " << chld << std::endl;
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
