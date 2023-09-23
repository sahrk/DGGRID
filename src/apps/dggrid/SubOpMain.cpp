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
// SubOpMain.cpp: SubOpMain class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgConstants.h>
#include <dglib/DgConverterBase.h>

#include "SubOpMain.h"

////////////////////////////////////////////////////////////////////////////////
SubOpMain::SubOpMain (OpBasic& op, bool _activate)
   : SubOpBasic (op, _activate),
     operation (""), precision (DEFAULT_PRECISION), verbosity (0),
     megaVerbose (false), pauseOnStart (false), pauseBeforeExit (false),
     useMother(false), updateFreq (100000)
{
}

////////////////////////////////////////////////////////////////////////////////
int
SubOpMain::initializeOp (void)
{
   vector<string*> choices;

   // dggrid_operation <GENERATE_GRID | GENERATE_GRID_FROM_POINTS |
   //     BIN_POINT_VALS | BIN_POINT_PRESENCE | TRANSFORM_POINTS | OUTPUT_STATS>
   choices.push_back(new string("GENERATE_GRID"));
   choices.push_back(new string("GENERATE_GRID_FROM_POINTS"));
   choices.push_back(new string("BIN_POINT_VALS"));
   choices.push_back(new string("BIN_POINT_PRESENCE"));
   choices.push_back(new string("TRANSFORM_POINTS"));
   choices.push_back(new string("OUTPUT_STATS"));
   pList().insertParam(new DgStringChoiceParam("dggrid_operation", "GENERATE_GRID",
               &choices));
   dgg::util::release(choices);

   // rng_type <RAND | MOTHER>
   choices.push_back(new string("RAND"));
   choices.push_back(new string("MOTHER"));
   pList().insertParam(new DgStringChoiceParam("rng_type", "RAND", &choices));
   dgg::util::release(choices);

   // precision <int> (0 <= v <= 30)
   pList().insertParam(new DgIntParam("precision", DEFAULT_PRECISION, 0, INT_MAX));

   //  verbosity <int> (0 <= v <= 3)
   pList().insertParam(new DgIntParam("verbosity", 0, 0, 3));

   // pause_on_startup <TRUE | FALSE>
   pList().insertParam(new DgBoolParam("pause_on_startup", false));

   // pause_before_exit <TRUE | FALSE>
   pList().insertParam(new DgBoolParam("pause_before_exit", false));

   //  update_frequency <int> (v >= 0)
   pList().insertParam(new DgULIntParam("update_frequency", 100000, 0, ULONG_MAX, true));

   return 0;

} // int SubOpMain::initializeOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpMain::setupOp (void)
{
   /////// fill state variables from the parameter list //////////

   getParamValue(pList(), "dggrid_operation", operation, false);

   string dummy;
   getParamValue(pList(), "rng_type", dummy, false);

   if (dummy == "MOTHER") useMother = true;
   else useMother = false;

   getParamValue(pList(), "precision", precision, false);

   getParamValue(pList(), "verbosity", verbosity, false);
   if (verbosity > 2) {
      megaVerbose = true;
      DgConverterBase::setTraceOn(true);
   }

   getParamValue(pList(), "pause_on_startup", pauseOnStart, false);
   getParamValue(pList(), "pause_before_exit", pauseBeforeExit, false);

   getParamValue(pList(), "update_frequency", updateFreq, false);

   return 0;

} // SubOpMain::setupOp

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

