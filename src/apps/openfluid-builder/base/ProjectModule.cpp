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
  @file ProjectModule.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */

#include <QApplication>
#include <QMessageBox>
#include <QMainWindow>

#include <openfluid/base/RuntimeEnv.hpp>
#include <openfluid/base/PreferencesManager.hpp>
#include <openfluid/base/ProjectManager.hpp>

#include <openfluid/builderext/PluggableModalExtension.hpp>
#include <openfluid/builderext/PluggableModelessExtension.hpp>
#include <openfluid/builderext/PluggableWorkspaceExtension.hpp>

#include <openfluid/machine/SimulatorPluginsManager.hpp>
#include <openfluid/machine/SimulatorSignatureRegistry.hpp>
#include <openfluid/machine/ObserverSignatureRegistry.hpp>
#include <openfluid/machine/GhostSimulatorFileIO.hpp>

#include <openfluid/ui/common/EditSignatureDialog.hpp>
#include <openfluid/ui/common/PreferencesDialog.hpp>

#include <openfluid/waresdev/WareSrcManager.hpp>

#include <openfluid/tools/QtHelpers.hpp>
#include <openfluid/tools/Filesystem.hpp>

#include "AppTools.hpp"
#include "ProjectCentral.hpp"
#include "ProjectModule.hpp"
#include "SaveAsDialog.hpp"
#include "EditProjectPropertiesDialog.hpp"

#include "ExtensionsRegistry.hpp"
#include "WaresTranslationsRegistry.hpp"

#include "DashboardFrame.hpp"

#include "ProjectWidget.hpp"
#include "ModelWidget.hpp"
#include "SpatialDomainWidget.hpp"
#include "MonitoringWidget.hpp"
#include "DatastoreWidget.hpp"
#include "RunConfigurationWidget.hpp"
#include "OutputsWidget.hpp"


#define BUILDER_WARE_WATCHERS_DELAY 2000
#define BUILDER_INPUTDIR_WATCHERS_DELAY 2000


// =====================================================================
// =====================================================================


ProjectModule::ProjectModule(const QString& ProjectPath):
  AbstractModule(),
  mp_MainWidget(NULL), mp_DashboardFrame(NULL),
  m_ProjectPath(ProjectPath), mp_ProjectCentral(NULL)
{
  mp_ProjectCentral = ProjectCentral::initInstance(ProjectPath);


  // watcher for simulators, with delay for ui update using timer
  mp_SimulatorsPlugsWatcher = new QFileSystemWatcher(this);

  mp_SimulatorsPlugsUpdateTimer = new QTimer(this);
  mp_SimulatorsPlugsUpdateTimer->setInterval(BUILDER_WARE_WATCHERS_DELAY);
  mp_SimulatorsPlugsUpdateTimer->setSingleShot(true);

  connect(mp_SimulatorsPlugsWatcher,SIGNAL(directoryChanged(const QString&)),
          mp_SimulatorsPlugsUpdateTimer,SLOT(start()));
  connect(mp_SimulatorsPlugsUpdateTimer,SIGNAL(timeout()),SLOT(updateSimulatorsWares()));

  // watcher for observers, with delay for ui update using timer
  mp_ObserversPlugsWatcher = new QFileSystemWatcher(this);

  mp_ObserversPlugsUpdateTimer = new QTimer(this);
  mp_ObserversPlugsUpdateTimer->setInterval(BUILDER_WARE_WATCHERS_DELAY);
  mp_ObserversPlugsUpdateTimer->setSingleShot(true);

  connect(mp_ObserversPlugsWatcher,SIGNAL(directoryChanged(const QString&)),
          mp_ObserversPlugsUpdateTimer,SLOT(start()));
  connect(mp_ObserversPlugsUpdateTimer,SIGNAL(timeout()),SLOT(updateObserversWares()));


  updateWaresWatchersPaths();

  // watcher for input directory
  mp_InputDirWatcher = new QFileSystemWatcher(this);

  mp_InputDirUpdateTimer = new QTimer(this);
  mp_InputDirUpdateTimer->setInterval(BUILDER_INPUTDIR_WATCHERS_DELAY);
  mp_InputDirUpdateTimer->setSingleShot(true);

  connect(mp_InputDirWatcher,SIGNAL(directoryChanged(const QString&)),mp_InputDirUpdateTimer,SLOT(start()));
  connect(mp_InputDirUpdateTimer,SIGNAL(timeout()),SLOT(checkInputDir()));

  resetInputDirWatcher();
}


// =====================================================================
// =====================================================================


ProjectModule::~ProjectModule()
{
  ExtensionsRegistry::instance()->releaseAllFeatureExtensions();

  ProjectCentral::resetInstance();
  mp_SimulatorsPlugsWatcher->deleteLater();
  mp_ObserversPlugsWatcher->deleteLater();
  mp_InputDirWatcher->deleteLater();
}


// =====================================================================
// =====================================================================


void ProjectModule::updateWaresWatchersPaths()
{
  QStringList Paths;

  // simulators

  if (!mp_SimulatorsPlugsWatcher->directories().isEmpty())
    mp_SimulatorsPlugsWatcher->removePaths(mp_SimulatorsPlugsWatcher->directories());

  if (openfluid::base::PreferencesManager::instance()->isWaresWatchersActive())
  {
    Paths << openfluid::tools::toQStringList(openfluid::base::RuntimeEnvironment::instance()
                                                 ->getSimulatorsPluginsPaths())
          << openfluid::tools::toQStringList(openfluid::base::RuntimeEnvironment::instance()
                                                 ->getExtraSimulatorsPluginsPaths());

    Paths.removeDuplicates();

    foreach (QString P,Paths)
    {
      if (QDir(P).exists())
        mp_SimulatorsPlugsWatcher->addPath(P);
    }
  }

  // observers

  Paths.clear();

  if (!mp_ObserversPlugsWatcher->directories().isEmpty())
    mp_ObserversPlugsWatcher->removePaths(mp_ObserversPlugsWatcher->directories());

  if (openfluid::base::PreferencesManager::instance()->isWaresWatchersActive())
  {

    Paths << openfluid::tools::toQStringList(openfluid::base::RuntimeEnvironment::instance()
                                                 ->getObserversPluginsPaths())
          << openfluid::tools::toQStringList(openfluid::base::RuntimeEnvironment::instance()
                                                 ->getExtraObserversPluginsPaths());

    Paths.removeDuplicates();

    foreach (QString P,Paths)
    {
      if (QDir(P).exists())
        mp_ObserversPlugsWatcher->addPath(P);
    }
  }
}


// =====================================================================
// =====================================================================


void ProjectModule::disableInputDirWatcher()
{
  mp_InputDirWatcher->removePaths(mp_InputDirWatcher->directories());
}


// =====================================================================
// =====================================================================


void ProjectModule::resetInputDirWatcher()
{
  mp_InputDirWatcher->addPath(m_ProjectPath+"/IN");
}


// =====================================================================
// =====================================================================


AbstractMainWidget* ProjectModule::mainWidgetRebuilt(QWidget* Parent)
{
  if (mp_MainWidget != NULL)
  {
    delete mp_MainWidget;
    mp_MainWidget = NULL;
  }

  mp_MainWidget = new ProjectWidget(Parent);


  mp_ModelTab = new ModelWidget(NULL,mp_ProjectCentral->advancedDescriptors());
  connect(mp_ModelTab,SIGNAL(changed(openfluid::builderext::FluidXUpdateFlags::Flags)),
          this,SLOT(dispatchChanges(openfluid::builderext::FluidXUpdateFlags::Flags)));
  connect(mp_ModelTab,SIGNAL(srcEditAsked(const QString&,openfluid::ware::PluggableWare::WareType,bool)),
          this,SLOT(whenSrcEditAsked(const QString&,openfluid::ware::PluggableWare::WareType,bool)));
  connect(mp_ModelTab,SIGNAL(srcGenerateAsked(const QString&)),this,SLOT(whenSrcGenerateAsked(const QString&)));

  mp_SpatialTab = new SpatialDomainWidget(NULL,mp_ProjectCentral->advancedDescriptors());
  connect(mp_SpatialTab,SIGNAL(changed(openfluid::builderext::FluidXUpdateFlags::Flags)),
          this,SLOT(dispatchChanges(openfluid::builderext::FluidXUpdateFlags::Flags)));

  mp_DatastoreTab = new DatastoreWidget(NULL,mp_ProjectCentral->advancedDescriptors());
  connect(mp_DatastoreTab,SIGNAL(changed(openfluid::builderext::FluidXUpdateFlags::Flags)),
          this,SLOT(dispatchChanges(openfluid::builderext::FluidXUpdateFlags::Flags)));

  mp_MonitoringTab = new MonitoringWidget(NULL,mp_ProjectCentral->advancedDescriptors());
  connect(mp_MonitoringTab,SIGNAL(changed(openfluid::builderext::FluidXUpdateFlags::Flags)),
          this,SLOT(dispatchChanges(openfluid::builderext::FluidXUpdateFlags::Flags)));
  connect(mp_MonitoringTab,SIGNAL(srcEditAsked(const QString&,openfluid::ware::PluggableWare::WareType,bool)),
          this,SLOT(whenSrcEditAsked(const QString&,openfluid::ware::PluggableWare::WareType,bool)));

  mp_RunConfigTab = new RunConfigurationWidget(NULL,mp_ProjectCentral->advancedDescriptors());
  connect(mp_RunConfigTab,SIGNAL(changed(openfluid::builderext::FluidXUpdateFlags::Flags)),
          this,SLOT(dispatchChanges(openfluid::builderext::FluidXUpdateFlags::Flags)));

  mp_OutputsTab = new OutputsWidget(NULL,mp_ProjectCentral->advancedDescriptors());


  mp_MainWidget->addWorkspaceTab(mp_ModelTab,tr("模型"));
  mp_MainWidget->addWorkspaceTab(mp_SpatialTab,tr("空间单元联系"));
  mp_MainWidget->addWorkspaceTab(mp_DatastoreTab,tr("空间数据集"));
  mp_MainWidget->addWorkspaceTab(mp_MonitoringTab,tr("监听器"));
  mp_MainWidget->addWorkspaceTab(mp_RunConfigTab,tr("模拟配置"));
  mp_MainWidget->addWorkspaceTab(mp_OutputsTab,tr("输出浏览"));

  return mp_MainWidget;
}


// =====================================================================
// =====================================================================


QWidget* ProjectModule::dockWidgetRebuilt(QWidget* Parent)
{
  if (mp_DashboardFrame != NULL)
  {
    delete mp_DashboardFrame;
    mp_DashboardFrame = NULL;
  }

  mp_DashboardFrame = new DashboardFrame(mp_ProjectCentral,Parent);

  return mp_DashboardFrame;
}


// =====================================================================
// =====================================================================


bool ProjectModule::whenQuitAsked()
{
  return true;
}


// =====================================================================
// =====================================================================


bool ProjectModule::whenNewAsked()
{
  return true;
}

// =====================================================================
// =====================================================================


bool ProjectModule::whenOpenAsked()
{
  return true;
}


// =====================================================================
// =====================================================================


bool ProjectModule::whenReloadAsked()
{
  if (QMessageBox::question(QApplication::activeWindow(),tr("重新载入项目"),
                              tr("重新载入项目将会撤销一切未保存的项目")+
                              "\n\n"+
                              tr("继续吗?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
  {
    emit savePerformed();

    return true;
  }

  return false;
}


// =====================================================================
// =====================================================================


void ProjectModule::whenSaveAsked()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  disableInputDirWatcher();
  if (mp_ProjectCentral->save())
  {
    emit savePerformed();
  }
  resetInputDirWatcher();
  QApplication::restoreOverrideCursor();
}


// =====================================================================
// =====================================================================


bool ProjectModule::whenSaveAsAsked()
{
  SaveAsDialog SaveAsDlg(QApplication::activeWindow());

  if (SaveAsDlg.exec() == QDialog::Accepted)
  {
    disableInputDirWatcher();
    if (mp_ProjectCentral->saveAs(SaveAsDlg.getProjectName(),
                                  SaveAsDlg.getProjectFullPath()))
    {
      m_ProjectPath = SaveAsDlg.getProjectFullPath();
      mp_DashboardFrame->refreshProjectInfos();
      mp_OutputsTab->refreshOutputDir();
      resetInputDirWatcher();
      emit savePerformed();
      return true;
    }
    resetInputDirWatcher();
  }
  return false;
}


// =====================================================================
// =====================================================================


void ProjectModule::whenPropertiesAsked()
{
  EditProjectPropertiesDialog EditDlg(QApplication::activeWindow());

  if (EditDlg.exec() == QDialog::Accepted)
  {
    openfluid::base::ProjectManager::instance()->setDescription(EditDlg.getDescription().toStdString());
    openfluid::base::ProjectManager::instance()->setAuthors(EditDlg.getAuthors().toStdString());
    openfluid::base::ProjectManager::instance()->save();
  }

}


// =====================================================================
// =====================================================================


bool ProjectModule::whenCloseAsked()
{
  return true;
}


// =====================================================================
// =====================================================================


void ProjectModule::whenPreferencesAsked()
{
  bool WaresWatchingUpdated = false;

  openfluid::ui::common::PreferencesDialog PrefsDlg(
      QApplication::activeWindow(),
      openfluid::ui::common::PreferencesDialog::MODE_BUILDER,
      openfluid::tools::toQStringList(ExtensionPluginsManager::instance()->getPluginsStandardSearchPaths()));

  connect(&PrefsDlg, SIGNAL(applyTextEditorSettingsAsked()), mp_MainWidget, SLOT(updateWareSrcEditorsSettings()));

  PrefsDlg.exec();


  openfluid::base::PreferencesManager* PrefsMgr =
    openfluid::base::PreferencesManager::instance();

  if (PrefsDlg.isSimPathsChanged())
  {
    QStringList ExtraPaths = PrefsMgr->getExtraSimulatorsPaths();

    openfluid::base::RuntimeEnvironment::instance()->resetExtraSimulatorsPluginsPaths();
    for (int i=0;i<ExtraPaths.size(); i++)
      openfluid::base::RuntimeEnvironment::instance()->addExtraSimulatorsPluginsPaths(ExtraPaths[i].toStdString());

    updateWaresWatchersPaths();
    WaresWatchingUpdated = true;

    updateSimulatorsWares();
  }

  if (PrefsDlg.isObsPathsChanged())
  {
    QStringList ExtraPaths = PrefsMgr->getExtraObserversPaths();

    openfluid::base::RuntimeEnvironment::instance()->resetExtraObserversPluginsPaths();
    for (int i=0;i<ExtraPaths.size(); i++)
      openfluid::base::RuntimeEnvironment::instance()->addExtraObserversPluginsPaths(ExtraPaths[i].toStdString());

    updateWaresWatchersPaths();
    WaresWatchingUpdated = true;

    updateObserversWares();
  }

  if (PrefsDlg.isWaresWatchingChanged() && !WaresWatchingUpdated)
  {
    updateWaresWatchersPaths();

    if (PrefsMgr->isWaresWatchersActive())
    {
      // update wares when re-enabling watching
      updateSimulatorsWares();
      updateObserversWares();
    }
  }

  emit refreshWaresEnabled(!PrefsMgr->isWaresWatchersActive());

  if(PrefsDlg.isTextEditorSettingsChanged())
    mp_MainWidget->updateWareSrcEditorsSettings();
}


// =====================================================================
// =====================================================================


void ProjectModule::whenRecentProjectsActionsChanged()
{

}


// =====================================================================
// =====================================================================


void ProjectModule::whenRunAsked()
{
  if (openfluid::base::PreferencesManager::instance()->isAutomaticSaveBeforeRun())
    whenSaveAsked();

  emit simulationStarted();

  mp_ProjectCentral->run();

  // following two lines are added in conjunction with removal of equivalent lines in ProjectCentral::run()
  updateSimulatorsWares();
  updateObserversWares();

  emit simulationFinished();
}


// =====================================================================
// =====================================================================


void ProjectModule::whenExtensionAsked(const QString& ID)
{
  // TODO provide simulators and observers registries to extensions?
  // for model and monitoring handling for example

  openfluid::ware::WareID_t WareID = ID.toStdString();

  ExtensionsRegistry* ExtReg = ExtensionsRegistry::instance();

  if (ExtReg->isFeatureExtensionRegistered(WareID))
  {
    if (!ExtReg->isFeatureExtensionActive(WareID))
    {
      if (ExtReg->getExtensionMode(WareID) == openfluid::builderext::MODE_MODAL)
      {
        openfluid::builderext::PluggableModalExtension* ExtModal =
            (openfluid::builderext::PluggableModalExtension*)(ExtReg->instanciateFeatureExtension(WareID));
        ExtModal->setParent(QApplication::activeWindow(),Qt::Dialog);
        ExtModal->setModal(true);

        // TODO set correct extension configuration
        ExtModal->setConfiguration(openfluid::ware::WareParams_t());
        ExtModal->setFluidXDescriptor(&(mp_ProjectCentral->advancedDescriptors()));

        connect(ExtModal,SIGNAL(fluidxChanged(openfluid::builderext::FluidXUpdateFlags::Flags)),
                this,SLOT(dispatchChangesFromExtension(openfluid::builderext::FluidXUpdateFlags::Flags)));
        connect(this,SIGNAL(fluidxChanged(openfluid::builderext::FluidXUpdateFlags::Flags)),
                ExtModal,SLOT(update(openfluid::builderext::FluidXUpdateFlags::Flags)));
        connect(this,SIGNAL(simulationStarted()),ExtModal,SLOT(manageSimulationStart()));
        connect(this,SIGNAL(simulationFinished()),ExtModal,SLOT(manageSimulationFinish()));

        if (ExtModal->initialize())
        {
          ExtModal->update(openfluid::builderext::FluidXUpdateFlags::FLUIDX_ALL);
          ExtModal->exec();
        }

        ExtReg->releaseFeatureExtension(ExtModal);
        ExtModal->deleteLater();
      }
      else if (ExtReg->getExtensionMode(WareID) == openfluid::builderext::MODE_MODELESS)
      {
        openfluid::builderext::PluggableModelessExtension* ExtModeless =
            (openfluid::builderext::PluggableModelessExtension*)(ExtReg->instanciateFeatureExtension(WareID));
        ExtModeless->setParent(QApplication::activeWindow(),Qt::Dialog);
        ExtModeless->setModal(false);

        // TODO set correct extension configuration
        ExtModeless->setConfiguration(openfluid::ware::WareParams_t());
        ExtModeless->setFluidXDescriptor(&(mp_ProjectCentral->advancedDescriptors()));

        connect(ExtModeless,SIGNAL(finished(int)),this, SLOT(releaseModelessExtension()));
        connect(ExtModeless,SIGNAL(fluidxChanged(openfluid::builderext::FluidXUpdateFlags::Flags)),
                this,SLOT(dispatchChangesFromExtension(openfluid::builderext::FluidXUpdateFlags::Flags)));
        connect(this,SIGNAL(fluidxChanged(openfluid::builderext::FluidXUpdateFlags::Flags)),
                ExtModeless,SLOT(update(openfluid::builderext::FluidXUpdateFlags::Flags)));
        connect(this,SIGNAL(simulationStarted()),ExtModeless,SLOT(manageSimulationStart()));

        connect(this,SIGNAL(simulationFinished()),ExtModeless,SLOT(manageSimulationFinish()));

        if (ExtModeless->initialize())
        {
          ExtModeless->update(openfluid::builderext::FluidXUpdateFlags::FLUIDX_ALL);
          ExtModeless->show();
        }
        else
          releaseModelessExtension(ExtModeless);
      }
      else if (ExtReg->getExtensionMode(WareID) == openfluid::builderext::MODE_WORKSPACE)
      {
        openfluid::builderext::PluggableWorkspaceExtension* ExtWork =
            (openfluid::builderext::PluggableWorkspaceExtension*)(ExtReg->instanciateFeatureExtension(WareID));
        ExtWork->setProperty("ID",QString::fromStdString(WareID));

        // TODO set correct extension configuration

        ExtWork->setConfiguration(openfluid::ware::WareParams_t());
        ExtWork->setFluidXDescriptor(&(mp_ProjectCentral->advancedDescriptors()));

        connect(ExtWork,SIGNAL(fluidxChanged(openfluid::builderext::FluidXUpdateFlags::Flags)),
                this,SLOT(dispatchChangesFromExtension(openfluid::builderext::FluidXUpdateFlags::Flags)));
        connect(this,SIGNAL(fluidxChanged(openfluid::builderext::FluidXUpdateFlags::Flags)),
                ExtWork,SLOT(update(openfluid::builderext::FluidXUpdateFlags::Flags)));
        connect(this,SIGNAL(simulationStarted()),ExtWork,SLOT(manageSimulationStart()));
        connect(this,SIGNAL(simulationFinished()),ExtWork,SLOT(manageSimulationFinish()));

        if (ExtWork->initialize())
        {
          ExtensionContainer* ExtCon = ExtReg->registeredFeatureExtensions()->at(WareID);

          QString TabText = WaresTranslationsRegistry::instance()
            ->tryTranslate(QString::fromStdString(ExtCon->FileFullPath),
                           "signature",ExtCon->Signature->MenuText);

          // Replace empty menu text by extension ID
          TabText = QString::fromStdString(openfluid::tools::replaceEmptyString(TabText.toStdString(),WareID));

          ExtWork->update(openfluid::builderext::FluidXUpdateFlags::FLUIDX_ALL);
          mp_MainWidget->addWorkspaceExtensionTab(ExtWork,TabText);
        }
        else
        {
          // destruction of the extension
          ExtensionsRegistry::instance()->releaseFeatureExtension(ExtWork);
          ExtWork->deleteLater();
        }
      }
      else
      {
        QMessageBox::critical(QApplication::activeWindow(),
                              tr("拓展组件错误"),
                              tr("未知的拓展类型。\n无法实例化拓展组件。"),
                              QMessageBox::Close);
      }
    }
    else
    {
      QMessageBox::warning(QApplication::activeWindow(),
                           tr("拓展组件错误"),
                           tr("拓展组件已激活"),
                           QMessageBox::Close);

    }
  }
  else
  {
    QMessageBox::critical(QApplication::activeWindow(),
                          tr("拓展组件错误"),
                          tr("拓展组件未注册\n无法实例化拓展组件。"),
                          QMessageBox::Close);
  }
}


// =====================================================================
// =====================================================================


void ProjectModule::whenMarketAsked()
{

}


// =====================================================================
// =====================================================================


void ProjectModule::whenWaresRefreshAsked()
{
  updateSimulatorsWares();
  updateObserversWares();
}


// =====================================================================
// =====================================================================


bool ProjectModule::whenOpenExampleAsked()
{
  return true;
}


// =====================================================================
// =====================================================================


bool ProjectModule::findGhostSignature(const QString& ID,
                                       openfluid::ware::SimulatorSignature& Signature, std::string& FileFullPath)
{
  std::vector<openfluid::machine::ModelItemSignatureInstance*> Signatures =
    openfluid::machine::SimulatorPluginsManager::instance()->getAvailableGhostsSignatures();

  openfluid::machine::ModelItemSignatureInstance* GhostSignatureInstance = nullptr;

  unsigned int i = 0;
  while (!GhostSignatureInstance && i < Signatures.size())
  {
    if (Signatures[i]->Signature->ID == ID.toStdString())
    {
      GhostSignatureInstance = Signatures[i];
      Signature = *(GhostSignatureInstance->Signature);
      FileFullPath = GhostSignatureInstance->FileFullPath;
    }
    else
      i++;
  }

  return (GhostSignatureInstance != nullptr);
}


// =====================================================================
// =====================================================================


void ProjectModule::whenSrcEditAsked(const QString& ID,openfluid::ware::PluggableWare::WareType WType,bool Ghost)
{
  if (Ghost)
  {
    openfluid::ware::SimulatorSignature Signature;
    std::string FileFullPath;

    if (findGhostSignature(ID,Signature,FileFullPath))
    {
      openfluid::ui::common::EditSignatureDialog Dlg(mp_MainWidget);
      Dlg.initialize(Signature);
      if (Dlg.exec() == QDialog::Accepted)
      {
        Signature = Dlg.getSignature();
        openfluid::machine::GhostSimulatorFileIO::saveToFile(
                Signature,
                openfluid::tools::Filesystem::dirname(FileFullPath));
        updateSimulatorsWares();
      }
    }
    else
      QMessageBox::critical(QApplication::activeWindow(),
                            tr("虚拟模拟器版本错误"),
                            tr("无法找到虚拟模拟器"),
                            QMessageBox::Close);

  }
  else
  {
    QString ErrMsg;

    QString Path = openfluid::waresdev::WareSrcManager::instance()->getWarePath(ID,WType,ErrMsg);

    if(!Path.isEmpty())
      mp_MainWidget->addWorkspaceWareSrcTab(Path);
    else
      QMessageBox::critical(QApplication::activeWindow(),
                            tr("源代码版本错误"),
                            ErrMsg,
                            QMessageBox::Close);
  }
}


// =====================================================================
// =====================================================================


void ProjectModule::whenSrcGenerateAsked(const QString& ID)
{
  openfluid::ware::SimulatorSignature Signature;
  std::string FileFullPath;

  if (findGhostSignature(ID,Signature,FileFullPath))
    mp_MainWidget->newSimulatorFromGhost(Signature);
  else
    QMessageBox::critical(QApplication::activeWindow(),
                          tr("源代码生成错误"),
                          tr("无法找到虚拟模拟器"),
                          QMessageBox::Close);

}


// =====================================================================
// =====================================================================


void ProjectModule::whenNewSimulatorSrcAsked()
{
  mp_MainWidget->newSimulator();
}


// =====================================================================
// =====================================================================


void ProjectModule::whenNewGhostSimulatorAsked()
{
  QStringList ExistingIDs;

  for (auto Sign : openfluid::machine::SimulatorPluginsManager::instance()->getAvailableGhostsSignatures())
    ExistingIDs.append(QString::fromStdString(Sign->Signature->ID));

  openfluid::ui::common::EditSignatureDialog Dlg(mp_MainWidget);
  Dlg.initialize(openfluid::ware::SimulatorSignature(),ExistingIDs);

  if (Dlg.exec() == QDialog::Accepted)
  {
    openfluid::ware::SimulatorSignature Signature = Dlg.getSignature();
    openfluid::machine::GhostSimulatorFileIO::saveToFile(
        Signature,
        openfluid::base::RuntimeEnvironment::instance()->getDefaultSimulatorsPluginsPaths().front());
    updateSimulatorsWares();
  }
}


// =====================================================================
// =====================================================================


void ProjectModule::whenOpenSimulatorSrcAsked()
{
  mp_MainWidget->openSimulatorSrc();
}


// =====================================================================
// =====================================================================


void ProjectModule::whenNewObserverSrcAsked()
{
  mp_MainWidget->newObserver();
}


// =====================================================================
// =====================================================================


void ProjectModule::whenOpenObserverSrcAsked()
{
  mp_MainWidget->openObserverSrc();
}


// =====================================================================
// =====================================================================


void ProjectModule::whenLaunchDevStudioAsked()
{
  launchDevStudio();
}


// =====================================================================
// =====================================================================


void ProjectModule::dispatchChanges(openfluid::builderext::FluidXUpdateFlags::Flags UpdateFlags)
{
  if (UpdateFlags.testFlag(openfluid::builderext::FluidXUpdateFlags::FLUIDX_ALL))
  {
    mp_SpatialTab->refresh();
    mp_DatastoreTab->refresh();
  }
  else
  {
    if (UpdateFlags.testFlag(openfluid::builderext::FluidXUpdateFlags::FLUIDX_DATASTORE))
    {
      mp_SpatialTab->refreshMap();
    }
  }

  emit fluidxChanged(UpdateFlags);

  doCheck();
}


// =====================================================================
// =====================================================================


void ProjectModule::dispatchChangesFromExtension(openfluid::builderext::FluidXUpdateFlags::Flags UpdateFlags)
{

  if (UpdateFlags.testFlag(openfluid::builderext::FluidXUpdateFlags::FLUIDX_SPATIALATTRS) ||
      UpdateFlags.testFlag(openfluid::builderext::FluidXUpdateFlags::FLUIDX_SPATIALSTRUCT))
  {
    mp_SpatialTab->refresh();
  }


  if (UpdateFlags.testFlag(openfluid::builderext::FluidXUpdateFlags::FLUIDX_DATASTORE))
  {
    mp_DatastoreTab->refresh();
  }


  dispatchChanges(UpdateFlags);
}


// =====================================================================
// =====================================================================


void ProjectModule::releaseModelessExtension(openfluid::builderext::PluggableModelessExtension* Sender)
{
  if (Sender == NULL)
    Sender = (openfluid::builderext::PluggableModelessExtension*)(QObject::sender());

  if (Sender)
  {
    ExtensionsRegistry::instance()->releaseFeatureExtension(Sender);
    Sender->deleteLater();
  }
}


// =====================================================================
// =====================================================================


void ProjectModule::updateSimulatorsWares()
{
  mp_ModelTab->prepareWaresUpdate();
  openfluid::machine::SimulatorSignatureRegistry::instance()->update();
  mp_ModelTab->updateWares();

  doCheck();
}


// =====================================================================
// =====================================================================


void ProjectModule::updateObserversWares()
{
  mp_MonitoringTab->prepareWaresUpdate();
  openfluid::machine::ObserverSignatureRegistry::instance()->update();
  mp_MonitoringTab->updateWares();

  doCheck();
}


// =====================================================================
// =====================================================================


void ProjectModule::checkInputDir()
{
  doCheck();
}


// =====================================================================
// =====================================================================


bool ProjectModule::isOkForSimulation() const
{
  return mp_ProjectCentral->checkInfos()->isOKForSimulation();
}


// =====================================================================
// =====================================================================


void ProjectModule::doCheck()
{
  mp_ProjectCentral->check();
  mp_DashboardFrame->refresh();
  emit runEnabled(mp_ProjectCentral->checkInfos()->isOKForSimulation());
}
