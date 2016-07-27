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
  @file EditAttributeNameDialog.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */

#include <openfluid/ui/config.hpp>

#include "ui_EditAttributeNameDialog.h"
#include "EditAttributeNameDialog.hpp"

#include <QPushButton>


EditAttributeNameDialog::EditAttributeNameDialog(EditMode Mode,
                                                 const QStringList& AttrsList,
                                                 QWidget* Parent):
  openfluid::ui::common::OpenFLUIDDialog(Parent),ui(new Ui::EditAttributeNameDialog),
  m_Mode(Mode), m_AttrsNames(AttrsList)
{
  ui->setupUi(this);

  if (m_Mode == EDIT_ADD)
  {
    ui->NewNameLabel->setText(tr("属性名称:"));
    ui->DefaultValueLabel->setText(tr("默认值:"));
    m_DefaultMsg = tr("新建属性");
    connect(ui->NewNameEdit,SIGNAL(textEdited(const QString&)),this,SLOT(checkGlobal()));
    connect(ui->DefaultValueEdit,SIGNAL(textEdited(const QString&)),this,SLOT(checkGlobal()));
  }
  else if (m_Mode == EDIT_RENAME)
  {
    ui->OriginalNameLabel->setText(tr("属性:"));
    ui->OriginalNameComboBox->addItems(AttrsList);
    ui->NewNameLabel->setText(tr("新名称:"));
    m_DefaultMsg = tr("属性重命名");
    connect(ui->OriginalNameComboBox,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(checkGlobal()));
    connect(ui->NewNameEdit,SIGNAL(textEdited(const QString&)),this,SLOT(checkGlobal()));
  }
  else if (m_Mode == EDIT_REMOVE)
  {
    ui->OriginalNameLabel->setText(tr("待删除属性:"));
    ui->OriginalNameComboBox->addItems(AttrsList);
    m_DefaultMsg = tr("删除属性");
  }

  ui->NewNameEdit->setPlaceholderText(openfluid::ui::config::PLACEHOLDER_REQUIRED);
  ui->DefaultValueEdit->setPlaceholderText(openfluid::ui::config::PLACEHOLDER_REQUIRED);

  ui->OriginalNameWidget->setVisible(m_Mode == EDIT_REMOVE ||
                                     m_Mode == EDIT_RENAME);

  ui->NewNameWidget->setVisible(m_Mode == EDIT_RENAME ||
                                m_Mode == EDIT_ADD);

  ui->DefaultValueWidget->setVisible(m_Mode == EDIT_ADD);

  ui->ButtonBox->button(QDialogButtonBox::Ok)->setText("确定");
  ui->ButtonBox->button(QDialogButtonBox::Cancel)->setText("取消");

  connect(ui->ButtonBox,SIGNAL(accepted()),this,SLOT(accept()));
  connect(ui->ButtonBox,SIGNAL(rejected()),this,SLOT(reject()));

  adjustSize();

  checkGlobal();
}


// =====================================================================
// =====================================================================


EditAttributeNameDialog::~EditAttributeNameDialog()
{
  delete ui;
}


// =====================================================================
// =====================================================================


void EditAttributeNameDialog::checkGlobal()
{
  if (m_Mode == EDIT_ADD)
  {
    if (m_AttrsNames.contains(ui->NewNameEdit->text()))
      setMessage(tr("新属性名称已存在"));
    else if (ui->NewNameEdit->text().isEmpty())
      setMessage(tr("新属性名称为空"));
    else if (ui->DefaultValueEdit->text().isEmpty())
      setMessage(tr("默认属性值为空"));
    else
      setMessage();
  }
  else if (m_Mode == EDIT_RENAME)
  {
    if (m_AttrsNames.contains(ui->NewNameEdit->text()))
      setMessage(tr("新属性名称已存在"));
    else if (ui->NewNameEdit->text().isEmpty())
      setMessage(tr("新属性名称为空"));
    else
      setMessage();
  }
  else if (m_Mode == EDIT_REMOVE)
    setMessage();
}


// =====================================================================
// =====================================================================


void EditAttributeNameDialog::setMessage(const QString& Msg)
{
  if (Msg.isEmpty())
  {
    ui->MessageFrame->setStyleSheet(QString("background-color: %1;")
                                    .arg(openfluid::ui::config::DIALOGBANNER_BGCOLOR));
    ui->MessageLabel->setText(m_DefaultMsg);
    ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  }
  else
  {
    ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->MessageFrame->setStyleSheet(QString("background-color: %1;")
                                    .arg(openfluid::ui::config::DIALOGBANNER_WARNBGCOLOR));
    ui->MessageLabel->setText(Msg);
  }
}


// =====================================================================
// =====================================================================


QString EditAttributeNameDialog::getSelectedOriginalName() const
{
  return ui->OriginalNameComboBox->currentText();
}


// =====================================================================
// =====================================================================


QString EditAttributeNameDialog::getNewName() const
{
  return ui->NewNameEdit->text();
}


// =====================================================================
// =====================================================================


QString EditAttributeNameDialog::getDefaultValue() const
{
  return ui->DefaultValueEdit->text();
}
