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
  @file LogSim.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#include <openfluid/ware/PluggableSimulator.hpp>


// =====================================================================
// =====================================================================


BEGIN_SIMULATOR_SIGNATURE("tests.logger")

  DECLARE_NAME("test simulator for logger");
  DECLARE_DESCRIPTION("");

  DECLARE_VERSION("1.0");
  DECLARE_SDKVERSION;
  DECLARE_STATUS(openfluid::ware::EXPERIMENTAL);

  DECLARE_DOMAIN("");
  DECLARE_PROCESS("");
  DECLARE_METHOD("");
  DECLARE_AUTHOR("","");


END_SIMULATOR_SIGNATURE

/**

*/
class LoggerSimulator : public openfluid::ware::PluggableSimulator
{
  private:

  public:


  LoggerSimulator() : PluggableSimulator()
  {


  }


  // =====================================================================
  // =====================================================================


  ~LoggerSimulator()
  {


  }


  // =====================================================================
  // =====================================================================


  void initParams(const openfluid::ware::WareParams_t& /*Params*/)
  {
    OPENFLUID_LogAndDisplayInfo("This is a message to both file and console");
    OPENFLUID_LogInfo("This is a message to file only");
    OPENFLUID_DisplayInfo("This is a message to console only");

    OPENFLUID_LogAndDisplayDebug("This is a message to both file and console");
    OPENFLUID_LogDebug("This is a message to file only");
    OPENFLUID_DisplayDebug("This is a message to console only");

    OPENFLUID_LogAndDisplayWarning("This is a message to both file and console");
    OPENFLUID_LogWarning("This is a message to file only");
    OPENFLUID_DisplayWarning("This is a message to console only");
  }

  // =====================================================================
  // =====================================================================


  void prepareData()
  { }


  // =====================================================================
  // =====================================================================


  void checkConsistency()
  { }


  // =====================================================================
  // =====================================================================


  openfluid::base::SchedulingRequest initializeRun()
  {
    OPENFLUID_LogInfo("Time step duration" << OPENFLUID_GetDefaultDeltaT());

    return DefaultDeltaT();
  }

  // =====================================================================
  // =====================================================================


  openfluid::base::SchedulingRequest runStep()
  {
    openfluid::core::SpatialUnit* TU;

    OPENFLUID_UNITS_ORDERED_LOOP("TestUnits",TU)
    {
      OPENFLUID_LogInfo("TestUnits #" << TU->getID());
      OPENFLUID_DisplayInfo("TestUnits #" << TU->getID());

      OPENFLUID_LogWarning("This is a warning message for " << "TestUnits #" << TU->getID());
    }


    return DefaultDeltaT();
  }

  // =====================================================================
  // =====================================================================


  void finalizeRun()
  {
    OPENFLUID_LogAndDisplayWarning("This is almost the end");
    OPENFLUID_LogAndDisplayDebug("Are you sure?");
    OPENFLUID_LogAndDisplayInfo("This is the end");
  }

};

// =====================================================================
// =====================================================================

DEFINE_SIMULATOR_CLASS(LoggerSimulator)

