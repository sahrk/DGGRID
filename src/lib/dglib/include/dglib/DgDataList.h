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
// DgDataList.h: DgDataList class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGDATALIST_H
#define DGDATALIST_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include <dglib/DgDataFieldBase.h>

////////////////////////////////////////////////////////////////////////////////
class DgDataList {

   public:

      DgDataList (void) {}

      virtual ~DgDataList (void);

      vector<DgDataFieldBase*>& list (void) { return list_; }

      void clearList (void);

      void addField (DgDataFieldBase* field) {
             if (field) {
                list_.push_back(field);
                map_[field->name()] = field;
             }
      }

      DgDataFieldBase* getFieldByName (const string& fldName) {
                auto it = map_.find(fldName);
                return ((it == map_.end()) ? nullptr : it->second);
      }

#ifdef USE_GDAL
      virtual void createFields (OGRLayer* oLayer) const;
      virtual void setFields (OGRFeature* feature) const;
#endif

   protected:

      virtual ostream& writeTo (ostream& stream) const {

         bool first = true;
         stream << "{";
         for (auto& field : list_) {
            if (!first)
               stream << ",";
            else
               first = false;
            stream << field->name() << ":";
            field->writeTo(stream);
         }
         stream << "}";

         return stream;
      }

   private:

      vector<DgDataFieldBase*> list_;
      unordered_map<string, DgDataFieldBase*> map_;

   friend ostream& operator<< (ostream& stream, const DgDataList& dataField);
};

////////////////////////////////////////////////////////////////////////////////
inline ostream& operator<< (ostream& stream, const DgDataList& dataList)
{
   return dataList.writeTo(stream);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif
