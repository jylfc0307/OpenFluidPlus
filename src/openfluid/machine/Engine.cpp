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
  @file Engine.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
*/


#include <openfluid/machine/Engine.hpp>

#include <iostream>
#include <iomanip>
#include <set>
#include <cmath>

#include <openfluid/config.hpp>
#include <openfluid/base/RuntimeEnv.hpp>
#include <openfluid/machine/MachineListener.hpp>
#include <openfluid/machine/ModelInstance.hpp>
#include <openfluid/machine/ModelItemInstance.hpp>
#include <openfluid/machine/MonitoringInstance.hpp>
#include <openfluid/machine/SimulationBlob.hpp>
#include <openfluid/tools/FileHelpers.hpp>
#include <openfluid/tools/Filesystem.hpp>


namespace openfluid { namespace machine {


// =====================================================================
// =====================================================================


Engine::Engine(SimulationBlob& SimBlob,
               ModelInstance& MInstance, MonitoringInstance& OLInstance,
               openfluid::machine::MachineListener* MachineListener)
       : m_SimulationBlob(SimBlob), m_ModelInstance(MInstance), m_MonitoringInstance(OLInstance), mp_SimLogger(NULL)
{

  mp_RunEnv = openfluid::base::RuntimeEnvironment::instance();

  mp_MachineListener = MachineListener;
  if (mp_MachineListener == NULL) mp_MachineListener = new openfluid::machine::MachineListener();

  mp_SimStatus = &(m_SimulationBlob.simulationStatus());

  prepareOutputDir();

  mp_SimLogger =
    new openfluid::base::SimulationLogger(mp_RunEnv->getOutputFullPath(openfluid::config::MESSAGES_LOG_FILE));

  std::chrono::system_clock::time_point TimePoint = std::chrono::system_clock::now();
  std::time_t Time = std::chrono::system_clock::to_time_t(TimePoint);

  mp_SimLogger->addInfo(openfluid::base::FrameworkException::computeContext().toString(),
                        "Date: " + std::string(std::ctime(&Time)));
  mp_SimLogger->addInfo(openfluid::base::FrameworkException::computeContext().toString(),
                        "Computer: " + mp_RunEnv->getHostName());
  mp_SimLogger->addInfo(openfluid::base::FrameworkException::computeContext().toString(),
                        "User: " + mp_RunEnv->getUserID());
  mp_SimLogger->addInfo(openfluid::base::FrameworkException::computeContext().toString(),
                        "Input directory: " + mp_RunEnv->getInputDir());
  mp_SimLogger->addInfo(openfluid::base::FrameworkException::computeContext().toString(),
                        "Output directory: " + mp_RunEnv->getOutputDir());
}


// =====================================================================
// =====================================================================


Engine::~Engine()
{
  if (mp_SimLogger != NULL) delete mp_SimLogger;
}


// =====================================================================
// =====================================================================


void Engine::checkExistingVariable(const openfluid::core::VariableName_t& VarName,
                                   const openfluid::core::Value::Type& VarType,
                                   const openfluid::core::UnitsClass_t& ClassName,
                                   const std::string& SimulatorID)
{
  openfluid::core::UnitsList_t::const_iterator UnitIter;
  openfluid::core::UnitsList_t* UnitList;

  UnitList = NULL;
  if (m_SimulationBlob.spatialGraph().isUnitsClassExist(ClassName))
    UnitList = m_SimulationBlob.spatialGraph().spatialUnits(ClassName)->list();
  else
    throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,
                                              "Unit class " + ClassName + " does not exist for " +
                                              VarName + " variable required by " + SimulatorID);

  bool Status = true;

  UnitIter = UnitList->begin();
  while (UnitIter != UnitList->end())
  {
    if(VarType == openfluid::core::Value::NONE)
      Status = (*UnitIter).variables()->isVariableExist(VarName);
    else
      Status = (*UnitIter).variables()->isTypedVariableExist(VarName,VarType);

    if (!Status)
      throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,
                                                VarName + " variable on " + ClassName +
                                                " required by " + SimulatorID + " does not exist");

    ++UnitIter;
  }
}


// =====================================================================
// =====================================================================


void Engine::createVariable(const openfluid::core::VariableName_t& VarName,
                            const openfluid::core::Value::Type& VarType,
                            const openfluid::core::UnitsClass_t& ClassName,
                            bool UpdateMode,
                            const std::string& SimulatorID)
{
  openfluid::core::UnitsList_t::iterator UnitIter;
  openfluid::core::UnitsList_t* UnitList;

  UnitList = NULL;
  if (m_SimulationBlob.spatialGraph().isUnitsClassExist(ClassName))
    UnitList = m_SimulationBlob.spatialGraph().spatialUnits(ClassName)->list();
  else
    throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,"Unit class " + ClassName +
                                              " does not exist for " + VarName +
                                              " variable produced by " + SimulatorID);

  bool Status = true;

  // if not update mode, variables must not exist before creation
  if (!UpdateMode)
  {
    UnitIter = UnitList->begin();
    while (UnitIter != UnitList->end())
    {
       Status = !((*UnitIter).variables()->isVariableExist(VarName));

      if (!Status)
        throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,
                                                  VarName + " variable on " + ClassName +
                                                  " produced by " + SimulatorID +
                                                  " cannot be created because it is already created");

      ++UnitIter;
    }
  }

  for(UnitIter = UnitList->begin(); UnitIter != UnitList->end(); ++UnitIter )
  {
    (*UnitIter).variables()->createVariable(VarName,VarType);
  }
}


// =====================================================================
// =====================================================================


void Engine::checkExistingAttribute(openfluid::core::AttributeName_t AttrName,
                                    openfluid::core::UnitsClass_t ClassName,
                                    const std::string& SimulatorID)
{
  openfluid::core::UnitsList_t::const_iterator UnitIter;
  openfluid::core::UnitsList_t* UnitList;

  UnitList = NULL;
  if (m_SimulationBlob.spatialGraph().isUnitsClassExist(ClassName))
    UnitList = m_SimulationBlob.spatialGraph().spatialUnits(ClassName)->list();
  else
    throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,
                                              "Unit " + ClassName + " class does not exist for " +
                                              AttrName + " attribute required by " + SimulatorID);

  bool Status = true;

  UnitIter = UnitList->begin();
  while (Status && (UnitIter != UnitList->end()))
  {
    Status = (*UnitIter).attributes()->isAttributeExist(AttrName);
    if (!Status)
      throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,
                                                AttrName + " attribute on " + ClassName +
                                                " required by " + SimulatorID + " is not available");

    ++UnitIter;
  }
}


// =====================================================================
// =====================================================================


void Engine::createAttribute(openfluid::core::AttributeName_t AttrName,
                             openfluid::core::UnitsClass_t ClassName,
                             const std::string& SimulatorID)
{
  openfluid::core::UnitsList_t::iterator UnitIter;
  openfluid::core::UnitsList_t* UnitList;

  UnitList = NULL;
  if (m_SimulationBlob.spatialGraph().isUnitsClassExist(ClassName))
    UnitList = m_SimulationBlob.spatialGraph().spatialUnits(ClassName)->list();
  else throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,
                                                 "Unit class " + ClassName + " does not exist for " +
                                                 AttrName + " attribute produced by " + SimulatorID);


  for(UnitIter = UnitList->begin(); UnitIter != UnitList->end(); ++UnitIter )
  {
    (*UnitIter).attributes()->setValue(AttrName,openfluid::core::NullValue());
  }
}


// =====================================================================
// =====================================================================


void Engine::checkSimulationVarsProduction(int ExpectedVarsCount)
{
  openfluid::core::UnitsListByClassMap_t::const_iterator UnitsClassesIter;
  openfluid::core::UnitsList_t::const_iterator UnitsIter;
  const openfluid::core::UnitsListByClassMap_t* AllUnits;
  const openfluid::core::UnitsList_t* UnitsList;


  AllUnits = m_SimulationBlob.spatialGraph().allSpatialUnitsByClass();

  for (UnitsClassesIter = AllUnits->begin(); UnitsClassesIter != AllUnits->end();++UnitsClassesIter)
  {
    UnitsList = UnitsClassesIter->second.list();

    for (UnitsIter = UnitsList->begin();UnitsIter != UnitsList->end();++UnitsIter)
    {
      if (!((*UnitsIter).variables()->isAllVariablesCount(ExpectedVarsCount)))
        throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,"Variable production error");
    }
  }
}


// =====================================================================
// =====================================================================


void Engine::checkParametersConsistency()
{
  for (ModelItemInstance* IInstance : m_ModelInstance.items())
  {
    for (openfluid::ware::SignatureDataItem Param : IInstance->Signature->HandledData.RequiredParams)
    {
      bool FoundParam = false;
      bool FilledParam = false;

      // searching for parameter in local parameters
      auto it = IInstance->Params.find(Param.DataName);
      if (it != IInstance->Params.end())
      {
        FoundParam = true;
        FilledParam = (*it).second.size() > 0;
      }
      else
      {
        // searching for parameter in global parameters
        auto git = m_ModelInstance.globalParameters().find(Param.DataName);
        if (git != m_ModelInstance.globalParameters().end())
        {
          FoundParam = true;
          FilledParam = (*git).second.size() > 0;
        }
      }


      if (!FoundParam)
        throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,
                                                  "Cannot find parameter " + Param.DataName +
                                                  " required by " + IInstance->Signature->ID);
      else if (!FilledParam)
        throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,
                                                  "Parameter " + Param.DataName +
                                                  " required by " + IInstance->Signature->ID + " is empty");
    }
  }
}


// =====================================================================
// =====================================================================


void Engine::checkModelConsistency()
{
  std::list<ModelItemInstance*>::const_iterator SimIter;
  openfluid::ware::SignatureHandledData HData;
  ModelItemInstance* CurrentSimulator;
  unsigned int i;


  /* Variables processing order is important
     A) first pass
        1) produced vars
        2) updated vars
     B) second pass
        3) required vars
  */

  SimIter = m_ModelInstance.items().begin();

  while (SimIter != m_ModelInstance.items().end())
  {
    CurrentSimulator = (*SimIter);
    HData = CurrentSimulator->Signature->HandledData;

    // checking variables to create (produced)
    for (i=0;i< HData.ProducedVars.size();i++)
      createVariable(HData.ProducedVars[i].DataName,
                     HData.ProducedVars[i].DataType,
                     HData.ProducedVars[i].UnitsClass,
                     false,
                     CurrentSimulator->Signature->ID);


    // checking variables to update
    for (i=0;i<HData.UpdatedVars.size();i++)
      createVariable(HData.UpdatedVars[i].DataName,
                     HData.UpdatedVars[i].DataType,
                     HData.UpdatedVars[i].UnitsClass,
                     true,
                     CurrentSimulator->Signature->ID);

    ++SimIter;
  }


  SimIter = m_ModelInstance.items().begin();

  while (SimIter != m_ModelInstance.items().end())
  {
    CurrentSimulator = (*SimIter);
    HData = CurrentSimulator->Signature->HandledData;

    // checking required variables
    for (i=0;i< HData.RequiredVars.size();i++)
      checkExistingVariable(HData.RequiredVars[i].DataName,
                            HData.RequiredVars[i].DataType,
                            HData.RequiredVars[i].UnitsClass,
                            CurrentSimulator->Signature->ID);

    ++SimIter;
  }

}


// =====================================================================
// =====================================================================


void Engine::checkAttributesConsistency()
{
  std::list<ModelItemInstance*>::const_iterator SimIter;
  openfluid::ware::SignatureHandledData HData;
  ModelItemInstance* CurrentSimulator;
  unsigned int i;


  SimIter = m_ModelInstance.items().begin();

  while (SimIter != m_ModelInstance.items().end())
  {
    CurrentSimulator = (*SimIter);
    HData = CurrentSimulator->Signature->HandledData;

    // checking required attribute
    for(i=0; i < HData.RequiredAttribute.size();i++)
      checkExistingAttribute(HData.RequiredAttribute[i].DataName,
          HData.RequiredAttribute[i].UnitsClass,
          CurrentSimulator->Signature->ID);

    // checking produced attribute
    for(i=0; i < HData.ProducedAttribute.size();i++)
      createAttribute(HData.ProducedAttribute[i].DataName,
          HData.ProducedAttribute[i].UnitsClass,
          CurrentSimulator->Signature->ID);

    ++SimIter;
  }

}


// =====================================================================
// =====================================================================


void Engine::checkExtraFilesConsistency()
{
  std::list<ModelItemInstance*>::const_iterator SimIter;
  openfluid::ware::SignatureHandledData HData;
  ModelItemInstance* CurrentSimulator;


  // on each simulator
  for (SimIter = m_ModelInstance.items().begin(); SimIter != m_ModelInstance.items().end(); ++SimIter)
  {
    CurrentSimulator = *SimIter;

    HData = CurrentSimulator->Signature->HandledData;

    for (unsigned int i=0;i<HData.RequiredExtraFiles.size();i++)
    {
      if (!openfluid::tools::Filesystem::isFile(mp_RunEnv->getInputFullPath(HData.RequiredExtraFiles[i])))
        throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,
                                                  "File " + HData.RequiredExtraFiles[i] +
                                                  " required by " + CurrentSimulator->Signature->ID + " not found");
    }
  }
}


// =====================================================================
// =====================================================================


void Engine::prepareOutputDir()
{
  if (!openfluid::tools::Filesystem::isDirectory(mp_RunEnv->getOutputDir()))
  {
    openfluid::tools::Filesystem::makeDirectory(mp_RunEnv->getOutputDir());
    if (!openfluid::tools::Filesystem::isDirectory(mp_RunEnv->getOutputDir()))
      throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,"Error creating output directory");
  }
  else
  {
    if (mp_RunEnv->isClearOutputDir())
    {
      openfluid::tools::emptyDirectoryRecursively(mp_RunEnv->getOutputDir().c_str());
    }
  }
}


// =====================================================================
// =====================================================================


void Engine::initialize()
{
  m_ModelInstance.initialize(mp_SimLogger);
  m_MonitoringInstance.initialize(mp_SimLogger);

  if (mp_RunEnv->isUserValuesBufferSize())
  {
    openfluid::core::ValuesBufferProperties::setBufferSize(mp_RunEnv->getValuesBufferSize());
  }
  else
  {
    openfluid::core::ValuesBufferProperties::setBufferSize(
        (mp_SimStatus->getSimulationDuration()/mp_SimStatus->getDefaultDeltaT())+2);
  }



}


// =====================================================================
// =====================================================================


void Engine::initParams()
{
  mp_MachineListener->onInitParams();

  try
  {
    mp_SimStatus->setCurrentStage(openfluid::base::SimulationStatus::INITPARAMS);
    checkParametersConsistency();
    m_ModelInstance.call_initParams();
    m_MonitoringInstance.call_initParams();
  }
  catch (openfluid::base::FrameworkException& E)
  {
    mp_MachineListener->onInitParamsDone(openfluid::machine::MachineListener::LISTEN_ERROR);
    throw;
  }

  if (mp_SimLogger->isCurrentWarningFlag())
    mp_MachineListener->onInitParamsDone(openfluid::machine::MachineListener::LISTEN_WARNING);
  else
    mp_MachineListener->onInitParamsDone(openfluid::machine::MachineListener::LISTEN_OK);

  mp_SimLogger->resetCurrentWarningFlag();

}


// =====================================================================
// =====================================================================


void Engine::prepareData()
{
  try
  {
    mp_SimStatus->setCurrentStage(openfluid::base::SimulationStatus::PREPAREDATA);
    m_ModelInstance.call_prepareData();
  }
  catch (openfluid::base::FrameworkException& E)
  {
    mp_MachineListener->onPrepareDataDone(openfluid::machine::MachineListener::LISTEN_ERROR);
    throw;
  }

  if (mp_SimLogger->isCurrentWarningFlag())
    mp_MachineListener->onPrepareDataDone(openfluid::machine::MachineListener::LISTEN_WARNING);
  else
    mp_MachineListener->onPrepareDataDone(openfluid::machine::MachineListener::LISTEN_OK);


  mp_SimLogger->resetCurrentWarningFlag();

}


// =====================================================================
// =====================================================================


void Engine::checkConsistency()
{
  // check simulators count

  try
  {
    if (m_ModelInstance.getItemsCount() == 0)
      throw openfluid::base::FrameworkException(OPENFLUID_CODE_LOCATION,"No simulator in model");

    checkExtraFilesConsistency();

    checkModelConsistency();

    checkAttributesConsistency();
  }
  catch (openfluid::base::FrameworkException& E)
  {
    mp_MachineListener->onCheckConsistencyDone(openfluid::machine::MachineListener::LISTEN_ERROR);
    throw;
  }


  try
  {
    mp_SimStatus->setCurrentStage(openfluid::base::SimulationStatus::CHECKCONSISTENCY);
    m_ModelInstance.call_checkConsistency();
    m_MonitoringInstance.call_onPrepared();
  }
  catch (openfluid::base::FrameworkException& E)
  {
    mp_MachineListener->onCheckConsistencyDone(openfluid::machine::MachineListener::LISTEN_ERROR);
    throw;
  }

  if (mp_SimLogger->isCurrentWarningFlag())
    mp_MachineListener->onCheckConsistencyDone(openfluid::machine::MachineListener::LISTEN_WARNING);
  else
    mp_MachineListener->onCheckConsistencyDone(openfluid::machine::MachineListener::LISTEN_OK);

  mp_SimLogger->resetCurrentWarningFlag();

}


// =====================================================================
// =====================================================================


void Engine::run()
{
  // Check for simulation vars production before init
  checkSimulationVarsProduction(0);

  // ============= initializeRun() =============

  mp_MachineListener->onInitializeRun();

  try
  {
    mp_SimStatus->setCurrentStage(openfluid::base::SimulationStatus::INITIALIZERUN);
    m_ModelInstance.call_initializeRun();
    m_MonitoringInstance.call_onInitializedRun();
  }
  catch (openfluid::base::FrameworkException& E)
  {
    mp_MachineListener->onInitializeRunDone(openfluid::machine::MachineListener::LISTEN_ERROR);
    throw;
  }

  if (mp_SimLogger->isCurrentWarningFlag())
    mp_MachineListener->onInitializeRunDone(openfluid::machine::MachineListener::LISTEN_WARNING);
  else
    mp_MachineListener->onInitializeRunDone(openfluid::machine::MachineListener::LISTEN_OK);

  mp_SimLogger->resetCurrentWarningFlag();

  // check simulation vars production after init
  checkSimulationVarsProduction(1);


  // ============= runStep() =============



  mp_MachineListener->onBeforeRunSteps();
  mp_SimStatus->setCurrentStage(openfluid::base::SimulationStatus::RUNSTEP);


  while (m_ModelInstance.hasTimePointToProcess())
  {

    mp_SimLogger->resetCurrentWarningFlag();

    try
    {
      m_ModelInstance.processNextTimePoint();
      m_MonitoringInstance.call_onStepCompleted(mp_SimStatus->getCurrentTimeIndex());

      // TODO to remove? check simulation vars production at each time step
      //checkSimulationVarsProduction(mp_SimStatus->getCurrentStep()+1);
    }
    catch (openfluid::base::FrameworkException& E)
    {
      mp_MachineListener->onRunStepDone(openfluid::machine::MachineListener::LISTEN_ERROR);
      throw;
    }
  }

  mp_MachineListener->onAfterRunSteps();

  mp_SimLogger->resetCurrentWarningFlag();


  // ============= finalizeRun() =============

  mp_MachineListener->onFinalizeRun();

  try
  {
    mp_SimStatus->setCurrentStage(openfluid::base::SimulationStatus::FINALIZERUN);
    m_ModelInstance.call_finalizeRun();
    m_MonitoringInstance.call_onFinalizedRun();
  }
  catch (openfluid::base::FrameworkException& E)
  {
    mp_MachineListener->onFinalizeRunDone(openfluid::machine::MachineListener::LISTEN_ERROR);
    throw;
  }


  if (mp_SimLogger->isCurrentWarningFlag())
    mp_MachineListener->onFinalizeRunDone(openfluid::machine::MachineListener::LISTEN_WARNING);
  else
    mp_MachineListener->onFinalizeRunDone(openfluid::machine::MachineListener::LISTEN_OK);

  mp_SimLogger->resetCurrentWarningFlag();

  // check simulation vars production after finalize
  //checkSimulationVarsProduction(mp_SimStatus->getCurrentStep()+1);

  mp_SimStatus->setCurrentStage(openfluid::base::SimulationStatus::POST);
}


// =====================================================================
// =====================================================================


void Engine::finalize()
{
  m_ModelInstance.finalize();
}


} } //namespaces



