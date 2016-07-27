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
  @file AddParamDialog.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#include <openfluid/ui/config.hpp>

#include "ui_AddParamDialog.h"
#include "AddParamDialog.hpp"

#include <QPushButton>


AddParamDialog::AddParamDialog(const QStringList& ExistingParams,const QStringList& CompletingParams,
                               QWidget* Parent):
  openfluid::ui::common::OpenFLUIDDialog(Parent),ui(new(Ui::AddParamDialog)),
  mp_Completer(NULL),
  m_ExistingParams(ExistingParams), m_CompletingParams(CompletingParams)
{
  ui->setupUi(this);

  ui->MessageLabel->setText(tr("Add parameter"));

  if (!CompletingParams.isEmpty())
  {
    mp_Completer = new QCompleter(CompletingParams, this);
    mp_Completer->setCaseSensitivity(Qt::CaseInsensitive);
    mp_Completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    ui->NameEdit->setCompleter(mp_Completer);

    connect(mp_Completer,SIGNAL(activated(const QString&)),this,SLOT(checkGlobal()));
  }

  connect(ui->NameEdit,SIGNAL(textEdited(const QString&)),this,SLOT(checkGlobal()));

  ui->NameEdit->setPlaceholderText(openfluid::ui::config::PLACEHOLDER_REQUIRED);

  ui->ButtonBox->button(QDialogButtonBox::Ok)->setText("确定");
  ui->ButtonBox->button(QDialogButtonBox::Cancel)->setText("取消");

  connect(ui->ButtonBox,SIGNAL(accepted()),this,SLOT(accept()));
  connect(ui->ButtonBox,SIGNAL(rejected()),this,SLOT(reject()));

  checkGlobal();
}


// =====================================================================
// =====================================================================


AddParamDialog::~AddParamDialog()
{
  delete ui;
}


// =====================================================================
// =====================================================================


void AddParamDialog::checkGlobal()
{
  if (ui->NameEdit->text().isEmpty())
    setMessage(tr("参数不能为空"));
  else if (m_ExistingParams.contains(ui->NameEdit->text()))
    setMessage(tr("参数已存在"));
  else
    setMessage();
}


// =====================================================================
// =====================================================================


void AddParamDialog::setMessage(const QString& Msg)
{
  if (Msg.isEmpty())
  {
    ui->MessageFrame->setStyleSheet(QString("background-color: %1;")
                                    .arg(openfluid::ui::config::DIALOGBANNER_BGCOLOR));
    ui->MessageLabel->setText(tr("添加参数"));
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


QString AddParamDialog::getParamName() const
{
  return ui->NameEdit->text();
}


// =====================================================================
// =====================================================================


QString AddParamDialog::getParamValue() const
{
  return ui->ValueEdit->text();
}
