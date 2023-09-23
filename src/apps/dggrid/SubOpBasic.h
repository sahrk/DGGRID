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
// SubOpBasic.h: this file defines a sub operation associated with an OpBasic
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SUBOPBASIC_H
#define SUBOPBASIC_H

#include <dgaplib/DgApSubOpPList.h>

struct OpBasic;

////////////////////////////////////////////////////////////////////////////////
struct SubOpBasic : public DgApSubOpPList {

   SubOpBasic (OpBasic& _op, bool activate = true);

   OpBasic& op;
};

////////////////////////////////////////////////////////////////////////////////

#endif
