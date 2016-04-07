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
  @file ExtensionsRegistry.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#include "ExtensionsRegistry.hpp"
#include <openfluid/builderext/PluggableModelessExtension.hpp>


ExtensionsRegistry* ExtensionsRegistry::mp_Instance = NULL;


ExtensionsRegistry::ExtensionsRegistry():
  m_IsRegistered(false)
{

}


// =====================================================================
// =====================================================================


ExtensionsRegistry* ExtensionsRegistry::instance()
{
  if (!mp_Instance)
    mp_Instance = new ExtensionsRegistry();

  return mp_Instance;
}


// =====================================================================
// =====================================================================


ExtensionsRegistry::~ExtensionsRegistry()
{
  for(auto ExtCont : m_FeatureExtensions)
  {
    if (ExtCont.second)
    {
      delete ExtCont.second;
      ExtCont.second = nullptr;
    }
  }

  for(auto ExtCont : m_ParameterizationExtensions)
  {
    if (ExtCont.second)
    {
      delete ExtCont.second;
      ExtCont.second = nullptr;
    }
  }
}


// =====================================================================
// =====================================================================


void ExtensionsRegistry::registerExtensions()
{
  if (m_IsRegistered) return;

  std::vector<ExtensionContainer*> ExtVector =
      ExtensionPluginsManager::instance()->getAvailableWaresSignatures().AvailablePlugins;

  for (unsigned int i=0; i<ExtVector.size(); i++)
  {
    if (ExtVector[i]->Signature->Type == openfluid::builderext::TYPE_FEATURE)
      m_FeatureExtensions[ExtVector[i]->Signature->ID] = ExtVector[i];
    else if (ExtVector[i]->Signature->Type == openfluid::builderext::TYPE_PARAMETERIZATION)
      m_ParameterizationExtensions[ExtVector[i]->LinkUID] = ExtVector[i];
  }

  m_IsRegistered = true;
}


// =====================================================================
// =====================================================================


openfluid::builderext::PluggableBuilderExtension*
    ExtensionsRegistry::instanciateFeatureExtension(const openfluid::ware::WareID_t& ID)
{
  if (isFeatureExtensionRegistered(ID) && !m_FeatureExtensions[ID]->Active)
  {
    ExtensionPluginsManager::instance()->completeSignatureWithWareBody(m_FeatureExtensions[ID]);
    m_FeatureExtensions[ID]->Body
      ->linkToRunEnvironment(openfluid::base::RuntimeEnvironment::instance()->wareEnvironment());
    m_FeatureExtensions[ID]->Body->initializeWare(ID);
    m_FeatureExtensions[ID]->Active = true;
    return m_FeatureExtensions[ID]->Body;
  }

  return nullptr;
}


// =====================================================================
// =====================================================================


void ExtensionsRegistry::releaseFeatureExtension(const openfluid::ware::WareID_t& ID)
{
  if (isFeatureExtensionRegistered(ID))
  {
    m_FeatureExtensions[ID]->Active = false;
  }
}


// =====================================================================
// =====================================================================


void ExtensionsRegistry::releaseFeatureExtension(openfluid::builderext::PluggableBuilderExtension* Ext)
{
  releaseFeatureExtension(Ext->getID());
}


// =====================================================================
// =====================================================================


void ExtensionsRegistry::releaseAllFeatureExtensions()
{
  ExtensionsByName_t::iterator it;
  ExtensionsByName_t::iterator itb = m_FeatureExtensions.begin();
  ExtensionsByName_t::iterator ite = m_FeatureExtensions.end();

  for (it=itb;it!=ite;++it)
  {
    if ((*it).second->Active)
      (*it).second->Active = false;

    if ((*it).second->Body != NULL && (*it).second->Signature != NULL &&
        (*it).second->Signature->Mode == openfluid::builderext::MODE_MODELESS)
    {
      dynamic_cast<openfluid::builderext::PluggableModelessExtension*>((*it).second->Body)->close();
      dynamic_cast<openfluid::builderext::PluggableModelessExtension*>((*it).second->Body)->deleteLater();
    }
  }

}


// =====================================================================
// =====================================================================


bool ExtensionsRegistry::isFeatureExtensionRegistered(const openfluid::ware::WareID_t& ID)
{
  return (m_FeatureExtensions.find(ID) != m_FeatureExtensions.end());
}


// =====================================================================
// =====================================================================

openfluid::builderext::PluggableBuilderExtension*
  ExtensionsRegistry::instanciateParameterizationExtension(const openfluid::machine::UUID_t& UUID)
{
  if (isParameterizationExtensionRegistered(UUID))
  {
    ExtensionPluginsManager::instance()->completeSignatureWithWareBody(m_ParameterizationExtensions[UUID]);
    m_ParameterizationExtensions[UUID]->Body
      ->linkToRunEnvironment(openfluid::base::RuntimeEnvironment::instance()->wareEnvironment());
    m_ParameterizationExtensions[UUID]->Body->initializeWare(m_ParameterizationExtensions[UUID]->Signature->ID);
    m_ParameterizationExtensions[UUID]->Active = true;
    return m_ParameterizationExtensions[UUID]->Body;
  }

  return nullptr;
}


// =====================================================================
// =====================================================================


bool ExtensionsRegistry::isParameterizationExtensionRegistered(const openfluid::machine::UUID_t& UUID)
{
  return (m_ParameterizationExtensions.find(UUID) != m_ParameterizationExtensions.end());
}


// =====================================================================
// =====================================================================


openfluid::builderext::ExtensionMode ExtensionsRegistry::getExtensionMode(const openfluid::ware::WareID_t& ID)
{
  if (isFeatureExtensionRegistered(ID))
    return m_FeatureExtensions[ID]->Signature->Mode;

  return openfluid::builderext::MODE_UNKNOWN;
}


