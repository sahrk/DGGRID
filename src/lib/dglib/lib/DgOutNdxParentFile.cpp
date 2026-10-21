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
// DgOutNdxParentFile.cpp: DgOutNdxParentFile class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgOutNdxParentFile.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgIDGG.h>
#include <dglib/DgIDGGS.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///
DgOutNdxParentFile::DgOutNdxParentFile (const std::string& fileName,
                        const DgRFBase& outRF, const DgRFBase& ndxPrtOutRF,
                        const std::string& suffix, DgReportLevel failLevel)
   : DgOutputStream (fileName, suffix, failLevel),
     outRF_ (outRF), ndxPrtOutRF_ (ndxPrtOutRF)
{

} // DgOutNdxParentFile::DgOutNdxParentFile

////////////////////////////////////////////////////////////////////////////////
DgOutNdxParentFile&
DgOutNdxParentFile::insert (const DgLocation& center, const DgLocation& parent)
{
   *this << outRF_.addressText(center, ' ');
   *this << " " << ndxPrtOutRF_.addressText(parent, ' ');

   *this << std::endl;

   return *this;

} // DgOutNdxParentFile::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
