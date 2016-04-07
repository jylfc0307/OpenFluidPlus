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
  @file RunSimulationWorker.hpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#ifndef __OPENFLUID_UICOMMON_RUNSIMULATIONWORKER_HPP__
#define __OPENFLUID_UICOMMON_RUNSIMULATIONWORKER_HPP__

#include <QApplication>
#include <QtDeclarative>

#include <openfluid/machine/MachineListener.hpp>
#include <openfluid/base/Exception.hpp>


#include <openfluid/dllexport.hpp>


namespace openfluid {

namespace fluidx {
  class FluidXDescriptor;
}


namespace ui { namespace common {


class OPENFLUID_API RunSimulationListener : public QObject, public openfluid::machine::MachineListener
{
  Q_OBJECT;

  public:

    enum Stage { RUNW_BEFORE, RUNW_PRESIM, RUNW_INIT, RUNW_RUN, RUNW_FINAL, RUNW_AFTER };

  private:
    unsigned int m_CurrentIndex;
    std::string m_CurrentIndexStr;
    unsigned int m_CurrentSimulator;

    unsigned int m_TotalTime;
    unsigned int m_TotalSimulators;

    unsigned int m_CurrentPreSim;
    unsigned int m_CurrentInit;
    unsigned int m_CurrentFinal;
    unsigned int m_TotalTotal;

    bool m_Completed;

    openfluid::core::Duration_t m_SimDuration;

    bool m_PausedByUser;
    bool m_ConfirmedPauseByUser;

    bool m_AbortedByUser;


  signals:

    void stageChanged(openfluid::ui::common::RunSimulationListener::Stage S);

    void progressValueChanged(int Index);

    void progressMaxChanged(int Index);

    void pauseConfirmed();


  public slots:

    void requestAbort();

    void requestSuspendResume();


  public:

    RunSimulationListener();

    ~RunSimulationListener();


    void setInfos(const unsigned int& TotalSimulators, const unsigned int& TotalTime);

    bool isPausedByUser();


    void onInitParams();

    void onInitializeRun();

    void onBeforeRunSteps();

    void onRunStep(const openfluid::base::SimulationStatus* SimStatus);

    void onFinalizeRun();

    void onFinalizeRunDone(const openfluid::base::Listener::Status& /*Status*/);


    void onSimulatorInitParams(const std::string& /*SimulatorID*/);

    void onSimulatorPrepareData(const std::string& /*SimulatorID*/);

    void onSimulatorCheckConsistency(const std::string& /*SimulatorID*/);

    void onSimulatorInitializeRun(const std::string& /*SimulatorID*/);

    void onSimulatorRunStep(const std::string& /*SimulatorID*/);

    void onSimulatorFinalizeRun(const std::string& /*SimulatorID*/);

};


// =====================================================================
// =====================================================================


class RunSimulationWorker : public QObject
{

  Q_OBJECT;

  private:

    openfluid::fluidx::FluidXDescriptor* mp_FXDesc;

    RunSimulationListener* mp_Listener;


  signals:

    void periodChanged(QString Begin, QString End, int Duration);

    void warningsChanged(unsigned int Count);

    void finished();

    void error(QString Error, openfluid::base::ExceptionContext Context);

    void userAbort();


  public slots:

    void run();


  public:

    RunSimulationWorker(openfluid::fluidx::FluidXDescriptor* FXDesc, RunSimulationListener* Listener);

    ~RunSimulationWorker();


};

} } } //namespaces


Q_DECLARE_METATYPE(openfluid::ui::common::RunSimulationListener::Stage);


#endif /* __OPENFLUID_UICOMMON_RUNSIMULATIONWORKER_HPP__ */
