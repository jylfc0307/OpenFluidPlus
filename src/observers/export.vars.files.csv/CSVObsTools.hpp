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
  @file CSVObsTools.hpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
*/



#ifndef __CSVOBSTOOLS_HPP__
#define __CSVOBSTOOLS_HPP__

#include <map>
#include <vector>

#include <openfluid/core/TypeDefs.hpp>
#include <openfluid/ware/WareParamsTree.hpp>


#define CSV_FILES_EXT "csv"


// =====================================================================
// =====================================================================


class CSVFormat
{
  public:

    enum HeaderType { None, Info, ColnamesAsData, ColnamesAsComment, Full };

    HeaderType Header;

    std::string ColSeparator;

    std::string DateFormat;

    std::string CommentChar;

    unsigned int Precision;

    bool IsTimeIndexDateFormat;

    CSVFormat();

};


// =====================================================================
// =====================================================================


typedef std::map<std::string, CSVFormat> FormatMap_t;


// =====================================================================
// =====================================================================


class CSVSet
{
  public:

    openfluid::core::UnitsClass_t UnitsClass;

    std::string UnitsIDsStr;

    bool isAllUnits;

    std::string VariablesStr;

    bool isAllVars;

    std::string FormatName;

    CSVSet();
};


// =====================================================================
// =====================================================================


typedef std::map<std::string, CSVSet> SetMap_t;


// =====================================================================
// =====================================================================


CSVFormat::HeaderType StrToHeaderType(const std::string& HeaderStr);


std::string HeaderTypeToStr(CSVFormat::HeaderType HType);


std::string buildHeader(const CSVFormat& Format, const std::string& FilePath,
                        const openfluid::core::UnitsClass_t& UClass, openfluid::core::UnitID_t UID,
                        const openfluid::core::VariableName_t& VarName);


std::string buildFilename(const std::string& OutputDir, const std::string& OutFileExt,
                          const std::string& SetName,
                          const openfluid::core::UnitsClass_t& UnitsClass,
                          const openfluid::core::UnitID_t& UnitID,
                          const openfluid::core::VariableName_t& Varname);


std::string StrToDateFormat(const std::string& FormatStr);


std::vector<std::string> parseFormatsFromParamsTree(const openfluid::ware::WareParamsTree& ParamsTree,
                                                    FormatMap_t& Formats);


std::vector<std::string> parseSetsFromParamsTree(const openfluid::ware::WareParamsTree& ParamsTree,
                                                 SetMap_t& Sets);


#endif /* __CSVOBSTOOLS_HPP__ */
