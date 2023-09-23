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
// DgApParamList.cpp: DgApParamList class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <algorithm>
#include <fstream>

#include <dgaplib/DgApParamList.h>

////////////////////////////////////////////////////////////////////////////////
DgApAssoc::~DgApAssoc (void)
{
   // does nothing

} // DgApAssoc::~DgApAssoc

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgApParamList::~DgApParamList (void)
{
   clearList();

} // DgApParamList::~DgApParamList

////////////////////////////////////////////////////////////////////////////////
void
DgApParamList::clearList (void)
{
   for (unsigned int i = 0; i < parameters.size(); i++) {
      delete parameters[i];
      parameters[i] = 0;
   }
   parameters.resize(0);

} // void DgApParamList::clearList

////////////////////////////////////////////////////////////////////////////////
void
DgApParamList::loadParams (const string& fileName, bool fail)
{
   ifstream metaFile;
   metaFile.open(fileName.c_str(), ios::in);
   if (!metaFile.good()) {
      report("DgApParamList::loadParams() unable to open file " + fileName,
             DgBase::Fatal);
   }

   const int maxLine = 10000;
   char nextLine[maxLine];

   while (!metaFile.eof()) {
      metaFile.getline(nextLine, maxLine);

      if (metaFile.eof()) break;

      if (strlen(nextLine) <= 1 || nextLine[0] == '#') continue;

      char* token = nextLine;
      while (isspace(*token)) token++;

      char* remainder = token;
      while (!isspace(*remainder)) remainder++;
      *remainder = '\0';

      remainder++;
      while (isspace(*remainder)) remainder++;

      setParam(token, remainder, fail);
   }

   metaFile.close();

} // void DgApParamList::loadParams

////////////////////////////////////////////////////////////////////////////////
void
DgApParamList::setParam (const string& nameIn, const string& strValIn, bool fail)
{
   if (toLower(strValIn) == string("invalid")) return;

   DgApAssoc* existing = getParam(nameIn, false);
   if (!existing) { // if not fail then just skip
      if (fail)
         report(string("DgApParamList::setParam() unknown parameter ")
             + nameIn, DgBase::Fatal);
   } else { // already exists

      existing->setValStr(strValIn);
      existing->setIsDefault(false);
      existing->setIsUserSet(true);
      if (!existing->validate()) {
         report(string("Invalid parameter data in parameter:\n") +
            existing->asString() + string("\n") +
            existing->validationErrMsg(), DgBase::Fatal);
      }
   }

} // void DgApParamList::setParam

////////////////////////////////////////////////////////////////////////////////
void
DgApParamList::setPresetParam (const string& nameIn, const string& strValIn)
{
   if (toLower(strValIn) == string("invalid")) return;

   DgApAssoc* existing = getParam(nameIn, false);
   if (!existing) {
      report(string("DgApParamList::setPresetParam() unknown parameter ")
             + nameIn, DgBase::Fatal);
   } else { // already exists
      // only change value if it was not set explicitly by the user
      if (!existing->isUserSet()) {
         existing->setValStr(strValIn);
         existing->setIsDefault(false);
         if (!existing->validate()) {
            report(string("Invalid parameter data in parameter:\n") +
               existing->asString() + string("\n") +
               existing->validationErrMsg(), DgBase::Fatal);
         }
      }
   }

} // void DgApParamList::setPresetParam

////////////////////////////////////////////////////////////////////////////////
void
DgApParamList::insertParam (DgApAssoc* param) // does not make a copy
{
   if (!param) {
      report("DgApParamList::insertParam() null parameter", DgBase::Fatal);
   }

   DgApAssoc* existing = getParam(param->name(), false);
   if (!existing) {
      parameters.push_back(param);
   } else { // already exists

      report(string("replacing parameter: ") + existing->asString()
        + string("\nwith parameter: ") + param->asString() +
        string("\n"), DgBase::Info);

      *existing = *param;
   }

} // void DgApParamList::insertParam

////////////////////////////////////////////////////////////////////////////////
DgApAssoc*
DgApParamList::getParam (const string& nameIn, bool setToIsApplicable) const
{
   string lower = toLower(nameIn);

   for (unsigned int i = 0; i < parameters.size(); i++) {
      if (parameters[i]->name() == lower) {
       if (setToIsApplicable)
          parameters[i]->setIsApplicable(true);
       return parameters[i];
      }
   }

   // if we're here we didn't find it
   return 0;

} // void DgApParamList::getParam

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
