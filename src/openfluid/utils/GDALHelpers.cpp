/*

  This file is part of OpenFLUID software
  Copyright(c) 2007, INRA - Montpellier SupAgro


 == GNU General Public License Usage ==

  OpenFLUID is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OpenFLUID is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OpenFLUID. If not, see <http://www.gnu.org/licenses/>.


 == Other Usage ==

  Other Usage means a use of OpenFLUID that is inconsistent with the GPL
  license, and requires a written agreement between You and INRA.
  Licensees for Other Usage of OpenFLUID may use this file in accordance
  with the terms contained in the written agreement between You and INRA.
  
*/


/**
  @file GDALHelpers.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */

#include <ogrsf_frmts.h>
#include <ogr_api.h>
#include <gdal.h>
#include <openfluid/utils/GDALHelpers.hpp>

#include <QStringList>


namespace openfluid { namespace utils {


const GDALDriversFilesExts_t getOGRFilesDriversForOpenFLUID()
{
  GDALDriversFilesExts_t Drivers;

  if (OGRGetDriverByName("ESRI Shapefile"))
  {
    Drivers["ESRI Shapefile"].Label = "ESRI Shapefile";
    Drivers["ESRI Shapefile"].FilesExts.push_back("shp");
  }

  if (OGRGetDriverByName("GeoJSON"))
  {
    Drivers["GeoJSON"].Label = "GeoJSON";
    Drivers["GeoJSON"].FilesExts.push_back("json");
    Drivers["GeoJSON"].FilesExts.push_back("geojson");
  }

  if (OGRGetDriverByName("GML"))
  {
    Drivers["GML"].Label = "GML";
    Drivers["GML"].FilesExts.push_back("gml");
  }

  if (OGRGetDriverByName("MapInfo File"))
  {
    Drivers["MapInfo File"].Label = "MapInfo";
    Drivers["MapInfo File"].FilesExts.push_back("mif");
    Drivers["MapInfo File"].FilesExts.push_back("tab");
  }

  return Drivers;
}


// =====================================================================
// =====================================================================


const GDALDriversFilesExts_t getGDALFilesDriversForOpenFLUID()
{
  GDALDriversFilesExts_t Drivers;

  if (GDALGetDriverByName("GTiff"))
  {
    Drivers["GTiff"].Label = "TIFF / GeoTIFF";
    Drivers["GTiff"].FilesExts.push_back("tif");
    Drivers["GTiff"].FilesExts.push_back("tiff");
    Drivers["GTiff"].FilesExts.push_back("Gtiff");
  }

  if (GDALGetDriverByName("JPEG"))
  {
    Drivers["JPEG"].Label = "JPEG";
    Drivers["JPEG"].FilesExts.push_back("jpg");
    Drivers["JPEG"].FilesExts.push_back("jpeg");
  }

  if (GDALGetDriverByName("AAIGrid"))
  {
    Drivers["AAIGrid"].Label = "Arc/Info ASCII Grid";
    Drivers["AAIGrid"].FilesExts.push_back("asc");
  }

  if (GDALGetDriverByName("AIG"))
  {
    Drivers["AIG"].Label = "Arc/Info Binary Grid";
    Drivers["AIG"].FilesExts.push_back("adf");
  }

  if (GDALGetDriverByName("HFA"))
  {
    Drivers["HFA"].Label = "ERDAS Imagine";
    Drivers["HFA"].FilesExts.push_back("img");
  }

  return Drivers;
}


// =====================================================================
// =====================================================================


const std::set<std::string> getOGRFilesExtensionsForOpenFLUID()
{
  std::set<std::string> ExtsList;

  // getting files drivers list
  GDALDriversFilesExts_t Drivers = getOGRFilesDriversForOpenFLUID();
  GDALDriversFilesExts_t::const_iterator DriversIt;

  // adding extensions in set
  for (DriversIt = Drivers.begin(); DriversIt != Drivers.end(); ++DriversIt)
  {
    std::vector<std::string>::const_iterator ExtsIt;

    for (ExtsIt = DriversIt->second.FilesExts.begin(); ExtsIt != DriversIt->second.FilesExts.end(); ++ExtsIt)
    {
      ExtsList.insert(*ExtsIt);
    }
  }

  return ExtsList;
}


// =====================================================================
// =====================================================================


QString getOGRGDALFormatsForQFileDialogs(const GDALDriversFilesExts_t& Drivers,
                                         const QString& AllFormatsLabel)
{
  QStringList AllFormats;
  QStringList DetailedFormats;

  GDALDriversFilesExts_t::const_iterator it;
  GDALDriversFilesExts_t::const_iterator bit = Drivers.begin();
  GDALDriversFilesExts_t::const_iterator eit = Drivers.end();

  for (it=bit; it!= eit; ++it)
  {
    QString CurrentFormatsStr;

    for (unsigned int i =0; i< (*it).second.FilesExts.size(); i++)
    {
      if (i!=0) CurrentFormatsStr += " ";
      CurrentFormatsStr += "*." + QString::fromStdString((*it).second.FilesExts.at(i));
    }

    DetailedFormats.append(QString::fromStdString((*it).second.Label) + "(" + CurrentFormatsStr + ")");
    AllFormats.append(CurrentFormatsStr);
  }

  if (!AllFormats.isEmpty())
  {
    QString AllFormatsStr;

    AllFormatsStr = AllFormatsLabel +" (" + AllFormats.join(" ")+")";
    DetailedFormats.prepend(AllFormatsStr);
  }

  return DetailedFormats.join(";;");
}


} } // namespaces
