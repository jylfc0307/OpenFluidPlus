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

/*
<sim2doc>
This is a text for testing \LaTeX doc
</sim2doc>
*/

/**
  @file CMakeSim.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#include <openfluid/ware/PluggableSimulator.hpp>


// =====================================================================
// =====================================================================


BEGIN_SIMULATOR_SIGNATURE("tests.sim.cmake")

  DECLARE_NAME("test simulator built using the OpenFLUID cmake module");
  DECLARE_DESCRIPTION("");

  DECLARE_VERSION("1.0");
  DECLARE_STATUS(openfluid::ware::EXPERIMENTAL);

  DECLARE_DOMAIN("");
  DECLARE_PROCESS("");
  DECLARE_METHOD("");

END_SIMULATOR_SIGNATURE

/**

*/
class CMakeSimulator : public openfluid::ware::PluggableSimulator
{
  private:

  public:


  CMakeSimulator() : PluggableSimulator()
  {


  }


  // =====================================================================
  // =====================================================================


  ~CMakeSimulator()
  {


  }


  // =====================================================================
  // =====================================================================


  void initParams(const openfluid::ware::WareParams_t& /*Params*/) { }


  // =====================================================================
  // =====================================================================


  void prepareData() { }


  // =====================================================================
  // =====================================================================


  void checkConsistency() { }


  // =====================================================================
  // =====================================================================


  openfluid::base::SchedulingRequest initializeRun()
  {
    std::cout << "\nCMake module built simulator" << std::endl;

    return DefaultDeltaT();
  }


  // =====================================================================
  // =====================================================================


  openfluid::base::SchedulingRequest runStep()
  {

    return DefaultDeltaT();
  }

  // =====================================================================
  // =====================================================================


  void finalizeRun()
  {

  }

};

// =====================================================================
// =====================================================================

DEFINE_SIMULATOR_CLASS(CMakeSimulator)
