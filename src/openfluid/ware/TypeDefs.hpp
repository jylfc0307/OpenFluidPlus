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
  @file ware/TypeDefs.hpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
*/



#ifndef __OPENFLUID_WARE_TYPEDEFS_HPP__
#define __OPENFLUID_WARE_TYPEDEFS_HPP__


#include <openfluid/core/TypeDefs.hpp>


namespace openfluid { namespace ware {


typedef std::string WareID_t;

typedef std::string WareName_t;

typedef std::string WareVersion_t;



// =====================================================================
// =====================================================================



/**
  Ware status
*/
enum WareStatus_t
{
  /**
    Experimental status, for testing only
  */
  EXPERIMENTAL,

  /**
    Beta status, on the road to a stable status
  */
  BETA,

  /**
    Stable status
  */
  STABLE
};


// =====================================================================
// =====================================================================


typedef std::string WareParamKey_t;

typedef openfluid::core::StringValue WareParamValue_t;

typedef std::map<WareParamKey_t, WareParamValue_t> WareParams_t;


} } // namespaces


#endif /* __OPENFLUID_WARE_TYPEDEFS_HPP__ */
