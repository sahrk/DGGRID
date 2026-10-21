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
// DgOutChildrenFile.cpp: DgOutChildrenFile class implementation
//
// Version 7.0 - Kevin Sahr, 12/14/14
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgOutChildrenFile.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgIDGG.h>
#include <dglib/DgIDGGS.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutChildrenFile::DgOutChildrenFile (const std::string& fileName,
                    const DgRFBase& outRF, const DgRFBase& chdOutRF,
                    const std::string& suffix, DgReportLevel failLevel)
   : DgOutputStream (fileName, suffix, failLevel),
                     outRF_ (outRF), chdOutRF_ (chdOutRF)
{

} // DgOutChildrenFile::DgOutChildrenFile

////////////////////////////////////////////////////////////////////////////////
DgOutChildrenFile&
DgOutChildrenFile::insert (const DgLocation& center, const DgLocVector& vec)
{
   *this << outRF_.addressText(center, ' ');
   for (int i = 0; i < vec.size(); i++)
      *this << " " << chdOutRF_.addressText(vec[i], ' ');

   *this << std::endl;

   return *this;

} // DgOutChildrenFile::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
