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
// DgHierNdxSystemRFBase.cpp: DgHierNdxSystemRFBase class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgConverter.h>
#include <dglib/Dg2WayConverter.h>

#include <dglib/DgHierNdxSystemRFBase.h>
#include <dglib/DgHierNdx.h>
#include <dglib/DgHierNdxIntRF.h>
#include <dglib/DgHierNdxStringRF.h>

////////////////////////////////////////////////////////////////////////////////
DgHierNdxStringToIntConverter::DgHierNdxStringToIntConverter (
                  const DgHierNdxStringRF& from, const DgHierNdxIntRF& to)
         : DgConverter<DgHierNdxStringCoord, long long int, 
                    DgHierNdxIntCoord, long long int> (from, to),
           sys(from.system())
      { }

////////////////////////////////////////////////////////////////////////////////
DgHierNdxIntCoord 
DgHierNdxStringToIntConverter::convertTypedAddress
                                (const DgHierNdxStringCoord& addIn) const
{ 
   return sys.toIntCoord(addIn); 
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxIntToStringConverter:: 
      DgHierNdxIntToStringConverter (const DgHierNdxIntRF& from, 
            const DgHierNdxStringRF& to)
         : DgConverter<DgHierNdxIntCoord, long long int, 
                    DgHierNdxStringCoord, long long int> (from, to),
           sys(from.system())
      { }

////////////////////////////////////////////////////////////////////////////////
DgHierNdxStringCoord 
DgHierNdxIntToStringConverter::convertTypedAddress
                                (const DgHierNdxIntCoord& addIn) const
{ 
   return sys.toStringCoord(addIn); 
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdx2WayIntToStringConverter (const DgHierNdxSystemRF& sys)
   : Dg2WayConverter (*(new DgHierNdxStringToIntConverter(
                               *sys.strRF(), *sys.intRF())),
                      *(new DgHierNdxIntToStringConverter(
                               *sys.intRF(), *sys.strRF())))
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgHierNdxSystemRFBase::DgHierNdxSystemRFBase (
         const DgHierNdxSystemRFSBase& hierNdxRFSIn, int resIn,
         const string& nameIn = "HierNdxSysRF")
   : DgDiscRF<DgHierNdx, DgQ2DICoord, long long int>(hierNdxRFSIn.dggs().network,
              hierNdxRFSIn.dggs()[resIn], nameIn),
     hierNdxRFS_ (hierNdxRFSIn), dggs_ (hierNdxRFSIn.dggs()), res_ (resIn),
     aperture_ (hierNdxRFSIn.dggs().aperture()), pRes_ {nullptr, nullptr, nullptr},
     curRes_ {nullptr, nullptr, nullptr}, chRes_ {nullptr, nullptr, nullptr}
{
   // sub-classes need to assign appropriate RF's to curRes_
   // RFS has to call initialize to set up the parent and child systems 
   // after the grids_ are all initialized
}

////////////////////////////////////////////////////////////////////////////////
bool 
DgHierNdxSystemRFBase::outModeInt (void) 
{ 
   return hierNdxRFS().outModeInt(); 
}

////////////////////////////////////////////////////////////////////////////////
void 
DgHierNdxSystemRFBase::setIntFromStringCoord (DgHierNdx& hn) const
{ 
   hn.intNdx_ = this->toIntCoord(hn.strNdx_); 
}

////////////////////////////////////////////////////////////////////////////////
void 
DgHierNdxSystemRFBase::setStringFromIntCoord (DgHierNdx& hn) const
{ 
   hn.strNdx_ = this->toStringCoord(hn.intNdx_); 
}

////////////////////////////////////////////////////////////////////////////////
void
DgHierNdxSystemRFBase::initialize (void)
{
   // current res set in the sub-class constructor
   //setSystemSet(curRes_, res_);

   setSystemSet(pRes_, res_ - 1);
   setSystemSet(chRes_, res_ + 1);
}

////////////////////////////////////////////////////////////////////////////////
int 
DgHierNdxSystemRFBase::setSystemSet (DgSystemSet& set, int res)
{
   set.dgg_ = nullptr;
   set.intRF_ = nullptr;
   set.strRF_ = nullptr;
   if (res < 0 || res >= hierNdxRFS_.nRes())
      return 1;

   // if we're here res is valid
   set.dgg_ = hierNdxRFS_[res].dgg();
   set.intRF_ = hierNdxRFS_[res].intRF();
   set.strRF_ = hierNdxRFS_[res].strRF();
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdx
DgHierNdxSystemRFBase::quantify (const DgQ2DICoord& point) const
{
   DgHierNdx add;
   add.strNdx_ = strRF()->quantify(point);
   setIntFromStringCoord(add);
   return add;
}
   
////////////////////////////////////////////////////////////////////////////////
DgQ2DICoord
DgHierNdxSystemRFBase::invQuantify (const DgHierNdx& add) const
{
   DgQ2DICoord point = strRF()->invQuantify(add.strNdx_);
   return point;
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxIntCoord 
DgHierNdxSystemRFBase::toIntCoord (const DgHierNdxStringCoord& c)
{
    DgHierNdxIntCoord add;
    return add;
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxStringCoord 
DgHierNdxSystemRFBase::toStringCoord (const DgHierNdxIntCoord& c)
{
    DgHierNdxStringCoord add;
    return add;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////