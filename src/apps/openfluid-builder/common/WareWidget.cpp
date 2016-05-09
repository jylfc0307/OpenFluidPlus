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
  @file WareWidget.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */

#include <openfluid/base/PreferencesManager.hpp>

#include "builderconfig.hpp"

#include "ui_WareWidget.h"
#include "WareWidget.hpp"
#include "ParameterWidget.hpp"


#include <QMessageBox>


WareWidget::WareWidget(QWidget* Parent, const openfluid::ware::WareID_t& ID,
                       bool Enabled, const QString& BGColor, int Index):
  QWidget(Parent),ui(new Ui::WareWidget), m_ID(ID), m_EnabledBGColor(BGColor),
  m_Available(false),m_Ghost(false),m_Enabled(Enabled), m_CurrentIndex(Index),
  m_ParamsExpanded(false), mp_ParamsWidget(NULL)
{
  ui->setupUi(this);
  ui->IDLabel->setText(QString::fromStdString(m_ID));
  ui->NameLabel->setElideMode(Qt::ElideRight);

  ui->UpButton->setText("");
  ui->UpButton->setIcon(QIcon(":/ui/common/icons/go-up.png"));
  ui->UpButton->setIconSize(QSize(16,16));

  ui->DownButton->setText("");
  ui->DownButton->setIcon(QIcon(":/ui/common/icons/go-down.png"));
  ui->DownButton->setIconSize(QSize(16,16));

  ui->RemoveButton->setText("");
  ui->RemoveButton->setIcon(QIcon(":/ui/common/icons/remove.png"));
  ui->RemoveButton->setIconSize(QSize(16,16));

  ui->AddParamButton->setIcon(QIcon(":/ui/common/icons/add.png"));
  ui->AddParamButton->setIconSize(QSize(16,16));


  setEnabledWare(m_Enabled);
  if (m_Enabled) ui->EnabledCheckBox->setCheckState(Qt::Checked);
  else ui->EnabledCheckBox->setCheckState(Qt::Unchecked);

  ui->ShowHideParamsLabel->setText(tr("显示变量和信息"));


  QList<int> SplitSizes;
  SplitSizes << 300 << 300;
  ui->ParamInfoSplitter->setSizes(SplitSizes);

  connect(ui->ShowHideParamsLabel,SIGNAL(clicked()),this,SLOT(updateShowHideParams()));
  connect(ui->EnabledCheckBox,SIGNAL(toggled(bool)),this,SLOT(setEnabledWare(bool)));

  connect(ui->UpButton,SIGNAL(clicked()),this,SLOT(notifyUpClicked()));
  connect(ui->DownButton,SIGNAL(clicked()),this,SLOT(notifyDownClicked()));
  connect(ui->RemoveButton,SIGNAL(clicked()),this,SLOT(notifyRemoveClicked()));

  connect(ui->ParameterizationSwitchLabel,SIGNAL(clicked()),this,SLOT(switchParameterizationMode()));

  ui->ParamInfoWidget->setVisible(false);
  ui->GenerateSrcButton->setVisible(false);
}


// =====================================================================
// =====================================================================


WareWidget::~WareWidget()
{
  delete ui;
}


// =====================================================================
// =====================================================================


void WareWidget::setAvailableWare(bool Available)
{
  m_Available = Available;
  ui->NameLabel->setVisible(Available);
  ui->ParamInfoTitleWidget->setVisible(Available);
  ui->ShowHideParamsLabel->setVisible(Available);
  ui->ParamInfoWidget->setVisible(Available && m_ParamsExpanded);

  updateWidgetBackground();
}


// =====================================================================
// =====================================================================


void WareWidget::setEnabledWare(bool Enabled)
{
  m_Enabled = Enabled;

  updateWidgetBackground();
}


// =====================================================================
// =====================================================================


void WareWidget::updateWidgetBackground()
{
  QString BGColor = m_EnabledBGColor;
  QString BGPattern = "";

  if (m_Ghost)
    BGColor = BUILDER_GHOST_BGCOLOR;

  if (!m_Enabled)
    BGColor = BUILDER_DISABLEDWARE_BGCOLOR;

  if (!m_Available)
    BGPattern = " background-image:url(:/images/warn-pattern-lightgray.png);";

  setStyleSheet("#WareFrame { background-color:"+BGColor+"; "+BGPattern+"}");
}


// =====================================================================
// =====================================================================


void WareWidget::updateShowHideParams()
{
  setExpanded(!m_ParamsExpanded);
  displayParams();
}


// =====================================================================
// =====================================================================


void WareWidget::setExpanded(bool Expand)
{
  if (m_Available)
  {
    m_ParamsExpanded = Expand;
  }
  else
    m_ParamsExpanded = false;
}


// =====================================================================
// =====================================================================


void WareWidget::displayParams()
{
  ui->ParamInfoWidget->setVisible(m_ParamsExpanded);
  if (m_ParamsExpanded)
    ui->ShowHideParamsLabel->setText(tr("隐藏变量和信息"));
  else
    ui->ShowHideParamsLabel->setText(tr("显示变量和信息"));
}


// =====================================================================
// =====================================================================


void WareWidget::setUpButtonEnabled(bool Enabled)
{
  ui->UpButton->setEnabled(Enabled);
}


// =====================================================================
// =====================================================================


void WareWidget::setDownButtonEnabled(bool Enabled)
{
  ui->DownButton->setEnabled(Enabled);
}


// =====================================================================
// =====================================================================


void WareWidget::setCurrentIndex(int Index)
{
  m_CurrentIndex = Index;
}


// =====================================================================
// =====================================================================


void WareWidget::notifyUpClicked()
{
  emit upClicked(QString::fromStdString(m_ID),m_CurrentIndex);
}


// =====================================================================
// =====================================================================


void WareWidget::notifyDownClicked()
{
  emit downClicked(QString::fromStdString(m_ID),m_CurrentIndex);
}


// =====================================================================
// =====================================================================


void WareWidget::notifyRemoveClicked()
{
  bool OK = true;

  if (openfluid::base::PreferencesManager::instance()->isItemRemovalConfirm())
  {
    QString TypeStr = tr("the generator");

    if (getType() == openfluid::fluidx::WareDescriptor::PluggedSimulator) TypeStr = tr("模拟器");
    else if (getType() == openfluid::fluidx::WareDescriptor::PluggedObserver) TypeStr = tr("观察者");


    OK = (QMessageBox::question(QApplication::activeWindow(),
                                "OpenFLUID-Builder",
                                tr("你正在移除 %1 %2.\n"
                                   "所有的变量都将会丢失.\n\n确定要继续吗?")
                                .arg(QString::fromStdString(getID())).arg(TypeStr),
                                QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok);
  }

  if (OK)
    emit removeClicked(QString::fromStdString(m_ID),m_CurrentIndex);
}


// =====================================================================
// =====================================================================


void WareWidget::updateParameterValue(const QString& /*Name*/, const QString& /*Value*/)
{

}


// =====================================================================
// =====================================================================


void WareWidget::removeParameterFromList(const QString& /*Name*/)
{

}


// =====================================================================
// =====================================================================


bool WareWidget::addParameterWidget(const QString& Name, const QString& Value)
{
  ParameterWidget* ParamWidget = new ParameterWidget(this,
                                                     Name,Value,
                                                     QString::fromStdString(""),false,true);

  connect(ParamWidget,SIGNAL(valueChanged(const QString&, const QString&)),
          this, SLOT(updateParameterValue(const QString&,const QString&)));
  connect(ParamWidget,SIGNAL(removeClicked(const QString&)),
          this, SLOT(removeParameterFromList(const QString&)));

  ((QBoxLayout*)(ui->ParamsListZoneWidget->layout()))->addWidget(ParamWidget);

  return true;
}


// =====================================================================
// =====================================================================


bool WareWidget::removeParameterWidget(const QString& Name)
{
  int LastIndex = ui->ParamsListZoneWidget->layout()->count()-1;

  for (int i=0;i<=LastIndex;i++)
  {
    ParameterWidget* W = static_cast<ParameterWidget*>(ui->ParamsListZoneWidget->layout()->itemAt(i)->widget());
    if (W != 0 && W->getName() == Name)
    {
      W = static_cast<ParameterWidget*>(ui->ParamsListZoneWidget->layout()->takeAt(i)->widget());
      W->deleteLater();
      return true;
    }
  }

  return false;
}


// =====================================================================
// =====================================================================


void WareWidget::prepareWareUpdate()
{

}


// =====================================================================
// =====================================================================


void WareWidget::updateWare()
{

}


// =====================================================================
// =====================================================================


void WareWidget::clearParameterWidgets()
{
  QBoxLayout* Layout = ((QBoxLayout*)(ui->ParamsListZoneWidget->layout()));
  QLayoutItem* Item;

  while ((Item = Layout->takeAt(0)) != NULL)
  {
    if (Item->widget() != NULL)
      Item->widget()->deleteLater();

    delete Item;
  }
}


// =====================================================================
// =====================================================================


void WareWidget::updateParametersList()
{

}


// =====================================================================
// =====================================================================


void WareWidget::updateParameterizationSwitch()
{
  ui->ParameterizationSwitchLabel->setVisible(mp_ParamsWidget);

  if (mp_ParamsWidget)
  {
    if (ui->ParameterizationStackWidget->currentIndex() == 0)
      ui->ParameterizationSwitchLabel->setText(tr("切换到帮助"));
    else
      ui->ParameterizationSwitchLabel->setText(tr("切换到列表"));
  }
}


// =====================================================================
// =====================================================================


void WareWidget::switchParameterizationMode()
{
  if (mp_ParamsWidget && ui->ParameterizationStackWidget->count() > 1)
  {

    if (ui->ParameterizationStackWidget->currentWidget() == mp_ParamsWidget)
    {
      updateParametersList();
      ui->ParameterizationStackWidget->setCurrentWidget(ui->ParamsListPage);
    }
    else
    {
      mp_ParamsWidget->update();
      ui->ParameterizationStackWidget->setCurrentWidget(mp_ParamsWidget);
    }

    updateParameterizationSwitch();
  }
}


// =====================================================================
// =====================================================================


void WareWidget::notifyChangedFromParameterizationWidget()
{
  emit changed();
}
