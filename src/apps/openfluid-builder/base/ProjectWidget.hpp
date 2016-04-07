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
  @file ProjectWidget.hpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#ifndef __OPENFLUID_BUILDERAPP_PROJECTWIDGET_HPP__
#define __OPENFLUID_BUILDERAPP_PROJECTWIDGET_HPP__

#include <QWidget>
#include <QTabWidget>

#include "AbstractMainWidget.hpp"


class WorkspaceTabWidget : public QTabWidget
{
  Q_OBJECT;

  private slots:

    void closeTab(int Index);

  public:

    WorkspaceTabWidget(QWidget* Parent);

    void addWorkspaceTab(QWidget* Tab, const QString& Label, bool Closable = false);

};


// =====================================================================
// =====================================================================


namespace Ui
{
  class ProjectWidget;
}


namespace openfluid { namespace ui { namespace waresdev {
class WareSrcWidgetCollection;
} } }


class ProjectWidget : public AbstractMainWidget
{
  Q_OBJECT;

  private:

    Ui::ProjectWidget* ui;

    WorkspaceTabWidget* mp_WorkspaceTabWidget;

    openfluid::ui::waresdev::WareSrcWidgetCollection* mp_WareSrcCollection;


  public:

    ProjectWidget(QWidget* Parent);

    ~ProjectWidget();

    void addWorkspaceTab(QWidget* Tab, const QString& Label);

    void addWorkspaceExtensionTab(QWidget* Tab, const QString& Label);

    void addWorkspaceWareSrcTab(const QString& Path);

    void openSimulatorSrc();

    void openObserverSrc();

    void newSimulator();

    void newSimulatorFromGhost(const openfluid::ware::SimulatorSignature& Signature);

    void newObserver();

  public slots :

    void updateWareSrcEditorsSettings();
};



#endif /* __OPENFLUID_BUILDERAPP_PROJECTWIDGET_HPP__ */
