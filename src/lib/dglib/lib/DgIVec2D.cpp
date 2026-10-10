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
// DgIVec2D.cpp: DgIVec2D class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <climits>
#include <string.h>

#include <dglib/DgBase.h>
#include <dglib/DgIVec2D.h>

////////////////////////////////////////////////////////////////////////////////

const DgIVec2D& DgIVec2D::undefDgIVec2D = DgIVec2D(INT_MAX, INT_MAX);

////////////////////////////////////////////////////////////////////////////////
const char*
DgIVec2D::fromString (const char* str, char delimiter)
{
   char delimStr[2];
   delimStr[0] = delimiter;
   delimStr[1] = '\0';

   char* tmpStr = new char[strlen(str) + 1];
   strcpy(tmpStr, str);

   // Get i and j:
   char* tok = nullptr;

   long long int	iIn(0),
	   		jIn(0);

   try
    {
   	tok = strtok(tmpStr, delimStr);
	if (!tok) throw 0;
   	iIn = dgg::util::from_string<long long int>(tok);
	
   	tok = strtok(NULL, delimStr);
	if (!tok) throw 0;
   	jIn = dgg::util::from_string<long long int>(tok);
    }
   catch(...)
    {
      const std::string value = tok ? std::string(tok) : std::string("\"\"");
      delete [] tmpStr;
      ::report("DgIVec2D::fromString() invalid value in string " + value,
               DgBase::Fatal);
      return nullptr;
    }

   setI(iIn);
   setJ(jIn);

   unsigned long long int offset = (tok - tmpStr) + strlen(tok) + 1;
   delete [] tmpStr;
   if (offset >= strlen(str))
    return 0;

   return &str[offset];

} // const char* DgIVec2D::fromString

////////////////////////////////////////////////////////////////////////////////
