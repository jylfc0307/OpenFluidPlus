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
  @file ModelWidget.hpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#ifndef __OPENFLUID_BUILDERAPP_MODELWIDGET_HPP__
#define __OPENFLUID_BUILDERAPP_MODELWIDGET_HPP__


#include <QWidget>

#include <openfluid/ui/common/ActionLabel.hpp>

#include "WorkspaceWidget.hpp"
#include "WaresManagementWidget.hpp"
#include "ModelScene.hpp"


namespace Ui
{
  class ModelWidget;
}


class ModelWidget : public WorkspaceWidget
{
  Q_OBJECT

  private:

    Ui::ModelWidget* ui;

    openfluid::ui::common::ActionLabel* mp_ShowHideGlobalParamsLabel;

    WaresManagementWidget* mp_WaresManWidget;

    ModelScene* mp_ModelScene;

    openfluid::fluidx::AdvancedModelDescriptor& m_Model;

    void updateGlobalParams();

    void updateCoupledModel();


  private slots:

    void updateShowHideGlobalParams();

    void addGlobalParam();

    void removeGlobalParam(const QString& Name);

    void updateGlobalParamValue(const QString& Name, const QString& Value);

    void addSimulator();

    void addGenerator();

    void moveModelItemUp(const QString& ID, int CurrentIndex);

    void moveModelItemDown(const QString& ID, int CurrentIndex);

    void removeModelItem(const QString& ID, int CurrentIndex);

    void dispatchChangesFromChildren();

    void notifySrcEditAsked(const QString& ID, bool Ghost);

    void notifySrcGenerateAsked(const QString& ID);


  signals:

    void srcEditAsked(const QString& ID,openfluid::ware::PluggableWare::WareType WType,bool Ghost);

    void srcGenerateAsked(const QString& ID);


  public slots:

    void refresh();


  public:

    ModelWidget(QWidget* Parent, openfluid::fluidx::AdvancedFluidXDescriptor& AFXDesc);

    virtual ~ModelWidget();

    void prepareWaresUpdate();

    void updateWares();
};



#endif /* __OPENFLUID_BUILDERAPP_MODELWIDGET_HPP__ */
