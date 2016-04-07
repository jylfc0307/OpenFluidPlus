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
  @file Attributes.hpp

  @author Aline LIBRES <libres@supagro.inra.fr>
  @author Jean-Christophe Fabre <jean-christophe.fabre@supagro.inra.fr>
 */


#ifndef __OPENFLUID_CORE_ATTRIBUTES_HPP__
#define __OPENFLUID_CORE_ATTRIBUTES_HPP__


#include <memory>

#include <openfluid/core/TypeDefs.hpp>
#include <openfluid/dllexport.hpp>
#include <openfluid/core/Value.hpp>
#include <openfluid/core/StringValue.hpp>
#include <openfluid/core/IntegerValue.hpp>


namespace openfluid { namespace core {


class OPENFLUID_API Attributes
{
  private:

    typedef std::map<AttributeName_t,std::shared_ptr<Value>> AttributesMap_t;

    AttributesMap_t m_Data;


  public:

    Attributes();

    ~Attributes();

    bool setValue(const AttributeName_t& aName, const Value& aValue);

    bool setValue(const AttributeName_t& aName, const std::string& aValue) OPENFLUID_DEPRECATED;

    bool setValueFromRawString(const AttributeName_t& aName, const std::string& aValue);

    bool getValue(const AttributeName_t& aName, openfluid::core::StringValue& aValue) const OPENFLUID_DEPRECATED;

    bool getValue(const AttributeName_t& aName, std::string& aValue) const OPENFLUID_DEPRECATED;

    const openfluid::core::Value* value(const AttributeName_t& aName) const;

    bool getValueAsDouble(const AttributeName_t& aName, double& aValue) const OPENFLUID_DEPRECATED;

    bool getValueAsLong(const AttributeName_t& aName, long& aValue) const OPENFLUID_DEPRECATED;

    bool isAttributeExist(const AttributeName_t& aName) const;

    std::vector<AttributeName_t> getAttributesNames() const;

    bool replaceValue(const AttributeName_t& aName, const StringValue& aValue);

    bool replaceValue(const AttributeName_t& aName, const std::string& aValue);

    bool removeAttribute(const AttributeName_t& aName);

    void clear();

};


} } // namespaces


#endif /* __OPENFLUID_CORE_ATTRIBUTES_HPP__ */
