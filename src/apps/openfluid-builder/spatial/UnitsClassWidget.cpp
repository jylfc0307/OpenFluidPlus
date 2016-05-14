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
  @file UnitsClassWidget.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#include "ui_UnitsClassWidget.h"
#include "UnitsClassWidget.hpp"
#include "AppTools.hpp"

#include <openfluid/base/ProjectManager.hpp>
#include <openfluid/base/PreferencesManager.hpp>
#include <openfluid/ui/common/UIHelpers.hpp>

#include <QColorDialog>
#include <QMessageBox>
#include <QDir>


QString UnitsClassWidget::m_ColorButtonStyleSheet =
    "QPushButton{ background-color : %1;}";
/*    "QPushButton{ background-color : %1; border-radius : 6px; border: 1px solid #777777;}"
    "QPushButton:pressed{ background-color : %1; border-radius : 6px; border: 1px solid #777777;}"
    "QPushButton:default{ background-color : %1; border-radius : 2px; border: 1px solid #777777;}";*/


UnitsClassWidget::UnitsClassWidget(const QString& ClassName,
                                   const std::list<openfluid::fluidx::DatastoreItemDescriptor*>& DSList,
                                   QWidget* Parent):
  QFrame(Parent),ui(new Ui::UnitsClassWidget),
  m_Selected(false), m_ClassName(ClassName), mp_LayerSource(NULL)
{
  setObjectName("UnitsClassFrame");

  ui->setupUi(this);

  setSelected(false);

  ui->UnitClassLabel->setText(ClassName);

  ui->UpButton->setIcon(QIcon(":/ui/common/icons/go-up.png"));
  ui->UpButton->setIconSize(QSize(16,16));

  ui->DownButton->setIcon(QIcon(":/ui/common/icons/go-down.png"));
  ui->DownButton->setIconSize(QSize(16,16));

  ui->RemoveButton->setIcon(QIcon(":/ui/common/icons/remove.png"));
  ui->RemoveButton->setIconSize(QSize(16,16));

  connect(ui->UpButton,SIGNAL(clicked()),this,SLOT(notifyUpClicked()));
  connect(ui->DownButton,SIGNAL(clicked()),this,SLOT(notifyDownClicked()));
  connect(ui->RemoveButton,SIGNAL(clicked()),this,SLOT(notifyRemoveClicked()));

  ui->StyleWidget->setVisible(false);
  ui->ShowHideStyleLabel->setText(tr("显示地图风格"));
  connect(ui->ShowHideStyleLabel,SIGNAL(clicked()),this,SLOT(toggleShowHideStyle()));

  linkToDatastoreItem(DSList);

  // initialize line width

  QVariant TmpLineWidth = openfluid::base::ProjectManager::instance()->getConfigValue("builder.spatial.unitsclasses",
                                                                                         m_ClassName+".linewidth");

  if (TmpLineWidth.type() == QVariant::String)
  {
    m_LineWidth = TmpLineWidth.toString().toInt();
  }
  else
  {
    m_LineWidth = 1;

    openfluid::base::ProjectManager::instance()->setConfigValue("builder.spatial.unitsclasses",
                                                                   m_ClassName+".linewidth",m_LineWidth);
  }


  // initialize line color

  QVariant TmpLineColor = openfluid::base::ProjectManager::instance()->getConfigValue("builder.spatial.unitsclasses",
                                                                                         m_ClassName+".linecolor");
  if (TmpLineColor.type() == QVariant::String)
  {
    m_LineColor = TmpLineColor.toString();
  }
  else
  {
    m_LineColor = openfluid::ui::common::getRandomColor();
    openfluid::base::ProjectManager::instance()->setConfigValue("builder.spatial.unitsclasses",
                                                                   m_ClassName+".linecolor",m_LineColor.name());
  }


  // initialize fill color

  QVariant TmpFillColor = openfluid::base::ProjectManager::instance()->getConfigValue("builder.spatial.unitsclasses",
                                                                                         m_ClassName+".fillcolor");
  if (TmpFillColor.type() == QVariant::String)
  {
    m_FillColor = TmpFillColor.toString();
  }
  else
  {
    m_FillColor = openfluid::ui::common::getRandomColor();
    openfluid::base::ProjectManager::instance()->setConfigValue("builder.spatial.unitsclasses",
                                                                   m_ClassName+".fillcolor",m_FillColor.name());
  }


  ui->LineColorButton->setStyleSheet(QString(m_ColorButtonStyleSheet).arg(m_LineColor.name()));
  ui->FillColorButton->setStyleSheet(QString(m_ColorButtonStyleSheet).arg(m_FillColor.name()));
  ui->LineWidthSpinBox->setValue(m_LineWidth);

  // TODO re-enable line width settings
  ui->LineWidthLabel->setVisible(false);
  ui->LineWidthSpinBox->setVisible(false);

  connect(ui->VisibleCheckBox,SIGNAL(toggled(bool)),this,SLOT(changeVisible()));
  connect(ui->LineColorButton,SIGNAL(clicked()),this,SLOT(changeLineColor()));
  connect(ui->FillColorButton,SIGNAL(clicked()),this,SLOT(changeFillColor()));
  connect(ui->LineWidthSpinBox,SIGNAL(valueChanged(int)),this,SLOT(changeLineWidth(int)));

}


// =====================================================================
// =====================================================================



UnitsClassWidget::~UnitsClassWidget()
{
  delete ui;
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::toggleShowHideStyle()
{
  if (ui->StyleWidget->isVisible())
  {
    ui->StyleWidget->setVisible(false);
    ui->ShowHideStyleLabel->setText(tr("显示地图风格"));
  }
  else
  {
    ui->StyleWidget->setVisible(true);
    ui->ShowHideStyleLabel->setText(tr("隐藏地图风格"));
  }
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::mousePressEvent(QMouseEvent* Event)
{
  if (!m_Selected)
    emit selectionRequested(m_ClassName);

  QWidget::mousePressEvent(Event);
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::setSelected(bool Selected)
{
  if (Selected)
  {
    m_Selected = true;
    setStyleSheet("QFrame {background-color : #FFFFFF;} QFrame#UnitsClassFrame {border : 1px solid #617487;}");
  }
  else
  {
    m_Selected = false;
    setStyleSheet("QFrame#UnitsClassFrame {border : 1px solid #CCCCCC;}");
  }
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::notifyUpClicked()
{
  emit upClicked(m_ClassName);
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::notifyDownClicked()
{
  emit downClicked(m_ClassName);
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::notifyRemoveClicked()
{
  bool OK = true;

  if (openfluid::base::PreferencesManager::instance()->isSpatialUnitsRemovalConfirm())
  {
    OK = (QMessageBox::question(QApplication::activeWindow(),
                                "OpenFLUID-Builder",
                                tr("您在删除单元类%1。\n"
                                   "所有的单元类%1和与之关联的变量将会丢失。\n\n"
                                   "确认要继续吗？").arg(m_ClassName),
                                QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok);
  }

  if (OK)
  {
    emit removeClicked(m_ClassName);
  }
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::setUpButtonEnabled(bool Enabled)
{
  ui->UpButton->setEnabled(Enabled);
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::setDownButtonEnabled(bool Enabled)
{
  ui->DownButton->setEnabled(Enabled);
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::changeLineColor()
{
  QColor TmpColor = QColorDialog::getColor(m_LineColor,this);

  if (TmpColor.isValid())
  {
    m_LineColor = TmpColor;
    ui->LineColorButton->setStyleSheet(QString(m_ColorButtonStyleSheet).arg(m_LineColor.name()));

    openfluid::base::ProjectManager::instance()->setConfigValue("builder.spatial.unitsclasses",
                                                                   m_ClassName+".linecolor",m_LineColor.name());
    emit styleChanged(m_ClassName);
  }
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::changeFillColor()
{
  QColor TmpColor = QColorDialog::getColor(m_FillColor,this);

  if (TmpColor.isValid())
  {
    m_FillColor = TmpColor;
    ui->FillColorButton->setStyleSheet(QString(m_ColorButtonStyleSheet).arg(m_FillColor.name()));

    openfluid::base::ProjectManager::instance()->setConfigValue("builder.spatial.unitsclasses",
                                                                   m_ClassName+".fillcolor",m_FillColor.name());

    emit styleChanged(m_ClassName);
  }
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::changeLineWidth(int Width)
{
  m_LineWidth = Width;

  openfluid::base::ProjectManager::instance()->setConfigValue("builder.spatial.unitsclasses",
                                                                 m_ClassName+".linewidth",m_LineWidth);

  //emit styleChanged(m_ClassName);
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::changeVisible()
{
  emit styleChanged(m_ClassName);
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::linkToDatastoreItem(const std::list<openfluid::fluidx::DatastoreItemDescriptor*>& DSList)
{
  mp_LayerSource = NULL;

  // disconnect signal to avoid multiple refresh of map,
  // due to changeVisible slot
  disconnect(ui->VisibleCheckBox,SIGNAL(toggled(bool)),this,SLOT(changeVisible()));

  if (!DSList.empty())
  {
    mp_LayerSource = DSList.front();
    ui->LayerSourceLabel->setText(QDir::toNativeSeparators(QString::fromStdString(mp_LayerSource->getRelativePath())));
    ui->StyleParamsWidget->setEnabled(true);
  }
  else
  {
    ui->LayerSourceLabel->setText(tr("(没有能显示的层)"));
    ui->StyleParamsWidget->setEnabled(false);
    ui->VisibleCheckBox->setChecked(false);
  }

  connect(ui->VisibleCheckBox,SIGNAL(toggled(bool)),this,SLOT(changeVisible()));
}


// =====================================================================
// =====================================================================


void UnitsClassWidget::setLayerVisible()
{
  if (mp_LayerSource != NULL)
    ui->VisibleCheckBox->setChecked(true);
}


// =====================================================================
// =====================================================================


bool UnitsClassWidget::isLayerVisible() const
{
  return ui->VisibleCheckBox->isChecked();
}
