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
// DgOutGdalFile.h: DgOutGdalFile class definitions
//
// Version 7.0 - Elijah Anderson-Justis, 5/20/17
//
////////////////////////////////////////////////////////////////////////////////

// USE_GDAL is set in MakeIncludes
#ifdef USE_GDAL

#ifndef DGOUTGDALFILE_H
#define DGOUTGDALFILE_H

#include <dglib/DgOutLocTextFile.h>
#include <dglib/DgUtil.h>

#include <ogrsf_frmts.h>

class DgDVec2D;
class DgPolygon;
class DgGeoDegRF;
class DgIDGGBase;

class DgOutGdalFile : public DgOutLocFile
{
   public:

      // the collection insert below hides the base class inserts
      using DgOutLocFile::insert;

      static OGRLinearRing* createLinearRing (const DgPolygon& poly);
      static OGRPolygon*    createPolygon (const DgPolygon& poly);

      DgOutGdalFile(const DgGeoDegRF& rf, const std::string& filename = "",
               const std::string& gdalDriver = "GeoJSON",
               DgOutGdalFileMode mode = InvalidMode, int precision = 7,
               bool isPointFile = false, DgReportLevel failLevel = DgBase::Fatal);

      ~DgOutGdalFile();

      // direct the DgOutLocFile abstract methods to the DgOutputStream ones
      virtual bool open (const std::string& /* fileName */,
                       DgReportLevel /* failLevel = DgBase::Fatal */) {
         return true;
      }

      virtual void close (void) {
         GDALClose( _dataset );
      }

      // collection output
      virtual DgOutLocFile& insert (const DgCell& cell,
           bool outputPoint, bool outputRegion,
           const DgRFBase& outRF, const DgRFBase& chdOutRF,
           const DgRFBase* prtOutRF,
           const DgLocVector* neighbors, const DgLocVector* children,
           const DgLocation* ndxParent, const DgLocVector* ndxChildren);

      virtual void setFormatStr(void) { }

   protected:

      DgOutGdalFileMode _mode;

      virtual DgOutLocFile& writePoint (const DgLocation& loc,
                                const std::string* label,
                                const DgDataList* dataList);

      virtual DgOutLocFile& writePolyline (const DgLocVector& vec,
                                const std::string* label,
                                const DgLocation* cent,
                                const DgDataList* dataList);

      virtual DgOutLocFile& writePolygon (const DgPolygon& poly,
                                const std::string* label,
                                const DgLocation* cent,
                                const DgDataList* dataList);

      virtual DgOutLocFile& writeVec (const DgDVec2D& pt);

      OGRFeature* createFeature (const std::string& label) const;
      OGRPoint* createPoint (const DgLocation& loc) const;
      //OGRPolygon createPolygon (const DgPolygon& poly) const;
      OGRGeometryCollection* createCollection (const DgLocation& node,
                                               const DgPolygon& region) const;

      void createAddressProperty (OGRFeature* feature, const char* fieldName,
           const DgLocation& loc, const DgRFBase& outRF);

      void createAddressesProperty (OGRFeature* feature, const char* fieldName,
           const DgLocVector& vec, const DgRFBase& outRF);

      void addFeature (OGRFeature *feature);

   private:

      // Gdal helpers:
      std::string   _gdalDriver;
      GDALDriver   *_driver;
      GDALDataset  *_dataset;
      OGRLayer     *_oLayer;

      std::string fileNameOnly_;

      void init (bool outputPoint, bool outputRegion = false,
              bool outputNeighbors = false, bool outputChildren = false,
              bool outputNdxParent = false, bool outputNdxChildren = false,
              const DgDataList* dataList = nullptr);
};

#endif

#endif
