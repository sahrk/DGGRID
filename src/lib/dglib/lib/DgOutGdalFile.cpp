/*******************************************************************************
    Copyright (C) 2021 Kevin Sahr

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
// DgOutGdalFile.cpp: DgOutGdalFile class implementation
//
// Version 7.0 - Elijah Anderson-Justis, 5/20/17
//
////////////////////////////////////////////////////////////////////////////////

// USE_GDAL is set in MakeIncludes
#ifdef USE_GDAL

#include <sstream>
#include <iostream>

#include <dglib/DgOutGdalFile.h>
#include <dglib/DgLocList.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgLocation.h>
#include <dglib/DgCell.h>
#include <dglib/DgGeoSphRF.h>

////////////////////////////////////////////////////////////////////////////////
DgOutGdalFile::DgOutGdalFile (const DgGeoSphDegRF& rf,
                    const std::string& filename, const std::string& gdalDriver, 
                    DgOutGdalFileMode mode, int precision, bool isPointFile, 
                    DgReportLevel failLevel)
    : DgOutLocFile (filename, rf, isPointFile, failLevel), _mode (mode),
         _gdalDriver(""), _driver(NULL), _dataset(NULL), _oLayer(NULL), _oField(NULL), 
         fileNameOnly_("")
{
   // test for override of vecAddress
   DgAddressBase* dummy = rf.vecAddress(DgDVec2D(M_ZERO, M_ZERO));
   if (!dummy)
        ::report("DgOutGdalFile::DgOutGdalFile(): RF " + rf.name() +
             " must override the vecAddress() method", DgBase::Fatal);
   delete dummy;

   _gdalDriver = gdalDriver;
}

////////////////////////////////////////////////////////////////////////////////
DgOutGdalFile::~DgOutGdalFile()
{
   delete _oField;
   _oField = NULL;
   delete _dataset;
   _dataset = NULL;
   delete _oField;
   _oField = NULL;
   close();
}

////////////////////////////////////////////////////////////////////////////////
void 
DgOutGdalFile::init (bool outputPoint, bool outputRegion)
{
   fileNameOnly_ = DgOutLocFile::fileName();

   GDALAllRegister();

   delete _driver;
   _driver = GetGDALDriverManager()->GetDriverByName(_gdalDriver.c_str());
   if (_driver == NULL)
        ::report( _gdalDriver + " driver not available.",  DgBase::Fatal);

   delete _dataset;
   _dataset = _driver->Create( fileNameOnly_.c_str(), 0, 0, 0, GDT_Unknown, NULL );
   if (_dataset == NULL)
      ::report( "Creation of output file failed.", DgBase::Fatal );
   
   delete _oLayer;
   _oLayer = NULL;
   delete _oField;
   _oField = NULL;

   // create the layer that we will be using
   OGRwkbGeometryType geomType = wkbUnknown;
   switch (_mode) {
      case Polygon:
         geomType = wkbPolygon;
         break;
      case Point:
         geomType = wkbPoint;
         break;
      case Collection:
         if (outputPoint) {
            if (outputRegion) 
               geomType = wkbGeometryCollection;
            else // just points
               geomType = wkbPoint;
         } else // just regions
            geomType = wkbPolygon;
         break;
      default:
         ::report( "Invalid GDAL file mode.", DgBase::Fatal );
   }
   _oLayer = _dataset->CreateLayer( fileNameOnly_.c_str(), NULL, geomType, NULL );
   if (_oLayer == NULL)
      ::report( "Layer creation failed.", DgBase::Fatal );

   // create the name field; other fields may be added if this is a collection
   _oField = new OGRFieldDefn( "name", OFTString );
   _oField->SetWidth(32);
   if (_oLayer->CreateField(_oField) != OGRERR_NONE)
      ::report("Creating name field failed.", DgBase::Fatal );
}

////////////////////////////////////////////////////////////////////////////////
OGRFeature*
DgOutGdalFile::createFeature (const string& label) const
{
   OGRFeature *feature = OGRFeature::CreateFeature(_oLayer->GetLayerDefn());
   if (!feature)
      ::report("GDAL feature creation failed.", DgBase::Fatal );
   feature->SetField("name", label.c_str());
   return feature;
}

////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutGdalFile::insert(const DgDVec2D& pt)
{
   //Probably isn't needed but keep in for safety
   return *this;
}

////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutGdalFile::insert (const DgIDGGBase& dgg, const DgCell& cell,
           bool outputPoint, bool outputRegion,
           const DgLocVector* neighbors, const DgLocVector* children)
{
   if (_mode != Collection)
      ::report("invalid GDAL output file mode encountered.", DgBase::Fatal);

   if (!_oLayer)
      init(outputPoint, outputRegion);

   // create the named feature
   OGRFeature *feature = createFeature(cell.label());
   
   // determine the geometry

/*
   // first check for multi
   if (outputPoint && outputRegion) {
cout << "both" << endl;
   } else if (outputPoint) {
*/

      OGRPoint oPt = createPoint(cell.node());
      feature->SetGeometry(&oPt);
/*

   } else if (outputRegion) {

      OGRPolygon poly = createPolygon(cell.region());
      feature->SetGeometry(&poly);
   } else
      ::report( "No geometry specified for GDAL collection feature.", DgBase::Fatal );
*/

   addFeature(feature);

   return *this;
/*
   //Make sure no errors occur with binding the feature to the layer
   if (_oLayer->CreateFeature( feature ) != OGRERR_NONE)
      ::report( "Failed to create feature in file", DgBase::Fatal );
   
   //Clean up the feature and ready for the next one    
   OGRFeature::DestroyFeature( feature );
*/
/*
//// children
   const DgIDGGSBase& dggs = *(dgg.dggs());
   const DgIDGGBase& dggr = dggs.idggBase(dgg.res() + 1);

   unsigned long long int sn = dgg.bndRF().seqNum(center);
   *this << sn;
   for (int i = 0; i < vec.size(); i++)
   {
      DgLocation tmpLoc(vec[i]);
      dggr.convert(&tmpLoc);
      *this << " " << dggr.bndRF().seqNum(tmpLoc);
   }

   *this << endl;
*/
   //// end  children
}

////////////////////////////////////////////////////////////////////////////////
OGRPoint
DgOutGdalFile::createPoint (const DgLocation& loc) const
{
   DgDVec2D pt = rf().getVecLocation(loc);
   OGRPoint oPt;

   oPt.setX(pt.x());
   oPt.setY(pt.y());

   return oPt;
}

////////////////////////////////////////////////////////////////////////////////
OGRPolygon
DgOutGdalFile::createPolygon (const DgPolygon& poly) const
{
   // first create a linearRing
   OGRLinearRing *linearRing;
   linearRing = (OGRLinearRing*) OGRGeometryFactory::createGeometry(wkbLinearRing);

   // fill linearRing with points
   const vector<DgAddressBase *>& v = poly.addressVec();
   for (vector<DgAddressBase *>::const_iterator i = v.begin(); v.end() != i; ++i) {
     DgDVec2D pt = rf().getVecAddress(*(*i));
     linearRing->addPoint(pt.x(), pt.y());
   }

   // add the first point to the end
   DgDVec2D pt = rf().getVecAddress(*v[0]);
   linearRing->addPoint(pt.x(), pt.y());
   
   // create an OGRPolygon and attach ring to it
   OGRPolygon polygon;
   polygon.addRingDirectly(linearRing);

   return polygon;
}

////////////////////////////////////////////////////////////////////////////////
void
DgOutGdalFile::addFeature (OGRFeature *feature) {

   // make sure no errors occure with binding the feature to the layer
   if (_oLayer->CreateFeature( feature ) != OGRERR_NONE)
        ::report( "Failed to create feature in file", DgBase::Fatal );
 
   // clean up the feature and ready for the next one    
   OGRFeature::DestroyFeature( feature );
}

////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutGdalFile::insert (DgLocation& loc, const string* label)
{
   if (_mode != Point)
      ::report( "invalid GDAL output file mode encountered.", DgBase::Fatal );

   if (!_oLayer)
      init(true, false);

   // create the feature
   OGRFeature *feature = createFeature(*label);
   
   OGRPoint oPt = createPoint(loc);

   feature->SetGeometry(&oPt);

   addFeature(feature);
/*
   //Make sure no errors occur with binding the feature to the layer
   if (_oLayer->CreateFeature( feature ) != OGRERR_NONE)
      ::report( "Failed to create feature in file", DgBase::Fatal );
   
   //Clean up the feature and ready for the next one    
   OGRFeature::DestroyFeature( feature );
*/
   
   return *this;
}

////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutGdalFile::insert (DgLocVector& vec, const string* label,
                          const DgLocation* cent)
{
   ::report( "polyline output not supported for GDAL file output", DgBase::Fatal );
   return *this;
}

////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutGdalFile::insert (DgPolygon& poly, const string* label,
                          const DgLocation* cent)
{
   if (_mode != Polygon)
      ::report( "invalid GDAL output file mode encountered.", DgBase::Fatal );

   if (!_oLayer)
      init(false, true);

   OGRPolygon polygon = createPolygon(poly);

   OGRFeature *feature = createFeature(*label);
   feature->SetGeometry(&polygon);

   addFeature(feature);
/*
   // make sure no errors occure with binding the feature to the layer
   if (_oLayer->CreateFeature( feature ) != OGRERR_NONE)
        ::report( "Failed to create feature in file", DgBase::Fatal );
 
   // clean up the feature and ready for the next one    
   OGRFeature::DestroyFeature( feature );
*/
   return *this;
}

#endif

