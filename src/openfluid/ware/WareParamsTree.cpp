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
  @file WareParamsTree.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
*/




#include <openfluid/ware/WareParamsTree.hpp>
#include <openfluid/tools/DataHelpers.hpp>


namespace openfluid { namespace ware {




// =====================================================================
// =====================================================================


WareParamsTree::WareParamsTree(const WareParams_t& Params)
{
  setParams(Params);
}


// =====================================================================
// =====================================================================


void WareParamsTree::setParams(const WareParams_t& Params)
{
  m_Root.clear();

  for (auto& ParamItem : Params)
  {
    WareParamsTreeNode* CurrentNode = &m_Root;

    std::vector<std::string> SplittedKeys = openfluid::tools::splitString(ParamItem.first,".");

    for (std::string& Key : SplittedKeys)
    {
      if (!CurrentNode->hasChild(Key))
        CurrentNode = &(CurrentNode->addChild(Key));
      else
        CurrentNode = &(CurrentNode->child(Key));
    }
    CurrentNode->setValue(ParamItem.second);
  }
}


// =====================================================================
// =====================================================================


openfluid::core::StringValue WareParamsTree::getValueUsingFullKey(const std::string& FullName) const
{
  std::vector<std::string> SplittedKeys = openfluid::tools::splitString(FullName,".");

  const WareParamsTreeNode* CurrentNode = &m_Root;

  for (std::string& Key : SplittedKeys)
  {
    if (CurrentNode->hasChild(Key))
      CurrentNode = &(CurrentNode->child(Key));
    else
      throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,"Parameter " + FullName + " does not exist");
  }

  return CurrentNode->getValue();
}


// =====================================================================
// =====================================================================


openfluid::core::StringValue WareParamsTree::getValueUsingFullKey(const std::string& FullName,
                                                                  const openfluid::core::StringValue& DefaultVal) const
{
  try
  {
    return  getValueUsingFullKey(FullName);
  }
  catch (...)
  {
    return DefaultVal;
  }
}


} }  // namespaces
