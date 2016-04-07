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
  @file ExtensionsRegistry.hpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#ifndef __OPENFLUID_BUILDERAPP_EXTENSIONSREGISTRY_HPP__
#define __OPENFLUID_BUILDERAPP_EXTENSIONSREGISTRY_HPP__

#include <QWidget>

#include "ExtensionPluginsManager.hpp"
#include "ExtensionContainer.hpp"


class ExtensionsRegistry
{
  public:

    typedef std::map<openfluid::ware::WareID_t, ExtensionContainer*> ExtensionsByName_t;

    typedef std::map<openfluid::machine::UUID_t, ExtensionContainer*> ExtensionsByLinkUID_t;


  private:

    static ExtensionsRegistry* mp_Instance;

    bool m_IsRegistered;

    ExtensionsByName_t m_FeatureExtensions;

    ExtensionsByLinkUID_t m_ParameterizationExtensions;

    ExtensionsRegistry();


  public:

    static ExtensionsRegistry* instance();

    ~ExtensionsRegistry();


    void registerExtensions();

    ExtensionsByName_t* registeredFeatureExtensions()
    { return &m_FeatureExtensions; };

    openfluid::builderext::PluggableBuilderExtension* instanciateFeatureExtension(const openfluid::ware::WareID_t& ID);

    void releaseFeatureExtension(const openfluid::ware::WareID_t& ID);

    void releaseFeatureExtension(openfluid::builderext::PluggableBuilderExtension* Ext);

    void releaseAllFeatureExtensions();

    bool isFeatureExtensionRegistered(const openfluid::ware::WareID_t& ID);

    bool isFeatureExtensionActive(const openfluid::ware::WareID_t& ID)
    { return (isFeatureExtensionRegistered(ID) && m_FeatureExtensions[ID]->Active); }

    openfluid::builderext::PluggableBuilderExtension*
      instanciateParameterizationExtension(const openfluid::machine::UUID_t& UUID);

    bool isParameterizationExtensionRegistered(const openfluid::machine::UUID_t& UUID);

    openfluid::builderext::ExtensionMode getExtensionMode(const openfluid::ware::WareID_t& ID);
};



#endif /* __OPENFLUID_BUILDERAPP_EXTENSIONSREGISTRY_HPP__ */
