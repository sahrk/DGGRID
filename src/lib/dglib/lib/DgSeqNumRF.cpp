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
// DgSeqNumRF.cpp: DgSeqNumRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstring>

#include <dglib/DgSeqNumRF.h>

const unsigned long long int DgSeqNumCoord::undefValue = ULLONG_MAX;

const DgSeqNumCoord DgSeqNumRF::undefCoord(DgSeqNumCoord::undefValue);

////////////////////////////////////////////////////////////////////////////////
const char*
DgSeqNumRF::str2add (DgSeqNumCoord* c, const char* str, char delimiter) const
{
   char delimStr[2];
   delimStr[0] = delimiter;
   delimStr[1] = '\0';

   char* tmpStr = new char[strlen(str) + 1];
   strcpy(tmpStr, str);
   char* tok = strtok(tmpStr, delimStr);

   char* end = nullptr;
   unsigned long long int val = 0;
   if (tok)
      val = strtoull(tok, &end, 10);
   if (!tok || end == tok)
      report("DgSeqNumRF::str2add(): invalid sequence number", DgBase::Fatal);

   if (c) c->setValue(val);

   unsigned long offset = strlen(tok) + 1;
   delete[] tmpStr;
   if (offset >= strlen(str)) return 0;
   else return &str[offset];

} // const char* DgSeqNumRF::str2add
