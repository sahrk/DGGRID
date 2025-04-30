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
// DgOutNdxChildrenFile.cpp: DgOutNdxChildrenFile class implementation
//
// Version 7.0 - Kevin Sahr, 12/14/14
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgOutNdxChildrenFile.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgIDGG.h>
#include <dglib/DgIDGGS.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutNdxChildrenFile::DgOutNdxChildrenFile (const string& fileName,
                    const DgIDGGBase& dgg, const DgIDGGBase& chdDgg,
                    const DgRFBase* outRF, const DgRFBase* ndxChdOutRF,
                    const string& suffix, DgReportLevel failLevel)
   : DgOutputStream (fileName, suffix, failLevel), dgg_ (dgg), chdDgg_ (chdDgg),
                     outRF_ (outRF), ndxChdOutRF_ (ndxChdOutRF)
{

} // DgOutNdxChildrenFile::DgOutNdxChildrenFile

////////////////////////////////////////////////////////////////////////////////
DgOutNdxChildrenFile&
DgOutNdxChildrenFile::insert (const DgLocation& center, DgLocVector& vec)
{
//cout << "@@@@@ DgOutNdxChildrenFile::insert:" << endl;
//cout << " dgg: " << dgg << endl;
//cout << " center: " << center << endl;
//cout << " vec: " << vec << endl;

   if (!outRF_) { // indicates seqnum output
      unsigned long long int sn = dgg_.bndRF().seqNum(center);
      *this << sn;
      for (int i = 0; i < vec.size(); i++) {
         DgLocation tmpLoc(vec[i]);
         chdDgg_.convert(&tmpLoc);
         *this << " " << chdDgg_.bndRF().seqNum(tmpLoc);
      }
   } else {
      DgLocation tmpLoc(center);
      outRF_->convert(&tmpLoc);
      *this << tmpLoc.asString(' ');
      ndxChdOutRF_->convert(&vec);
       for (int i = 0; i < vec.size(); i++) {
           *this << " " << vec[i].asString(' ');
           cout << "KEVIN: " << i << " " << vec[i].asString(' ') << endl;
       }
   }

   *this << endl;

   return *this;

} // DgOutNdxChildrenFile::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
