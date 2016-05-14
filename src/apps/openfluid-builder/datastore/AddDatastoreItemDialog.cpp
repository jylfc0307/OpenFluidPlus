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
  @file AddDatastoreItemDialog.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#include "ui_EditDatastoreItemDialog.h"
#include "AddDatastoreItemDialog.hpp"

#include <openfluid/ui/config.hpp>
#include <openfluid/ui/common/UIHelpers.hpp>
#include <openfluid/base/RuntimeEnv.hpp>
#include <openfluid/utils/GDALHelpers.hpp>

#include <gdal.h>
#include <ogr_api.h>
#include <openfluid/tools/DataHelpers.hpp>

#include <QPushButton>
#include <QFileDialog>



AddDatastoreItemDialog::AddDatastoreItemDialog(const QStringList& ExistingUnitsClass,
  const QStringList& ExistingIDs, QWidget* Parent):
  openfluid::ui::common::OpenFLUIDDialog(Parent),ui(new(Ui::EditDatastoreItemDialog)),
  m_ExistingIDs(ExistingIDs)
{
  ui->setupUi(this);

  GDALAllRegister();
  OGRRegisterAll();

  m_OGRFormatsStr =
      openfluid::utils::getOGRGDALFormatsForQFileDialogs(openfluid::utils::getOGRFilesDriversForOpenFLUID(),
                                                         tr("All vector files"));

  m_GDALFormatsStr =
      openfluid::utils::getOGRGDALFormatsForQFileDialogs(openfluid::utils::getGDALFilesDriversForOpenFLUID(),
                                                         tr("All raster files"));

  setMessage();

  ui->StaticIDLabel->setVisible(false);
  ui->GeovectorRadioButton->setChecked(true);
  ui->CopyRadioButton->setChecked(true);

  connect(ui->IDEdit,SIGNAL(textEdited(const QString&)),this,SLOT(checkGlobal()));
  connect(ui->UnitsClassCheckBox,SIGNAL(toggled(bool)),this,SLOT(checkGlobal()));
  connect(ui->UnitsClassEdit,SIGNAL(textEdited(const QString&)),this,SLOT(checkGlobal()));
  connect(ui->GeovectorRadioButton,SIGNAL(toggled(bool)),this,SLOT(checkGlobal()));
  connect(ui->GeorasterRadioButton,SIGNAL(toggled(bool)),this,SLOT(checkGlobal()));
  connect(ui->CopyRadioButton,SIGNAL(toggled(bool)),this,SLOT(checkGlobal()));
  connect(ui->CopySubdirRadioButton,SIGNAL(toggled(bool)),this,SLOT(checkGlobal()));
  connect(ui->CopySubdirEdit,SIGNAL(textEdited(const QString&)),this,SLOT(checkGlobal()));

  connect(ui->GeovectorBrowseButton,SIGNAL(clicked()),this,SLOT(selectVectorFile()));
  connect(ui->GeorasterBrowseButton,SIGNAL(clicked()),this,SLOT(selectRasterFile()));

  // completer for units classes
  if (!ExistingUnitsClass.isEmpty())
  {
    mp_Completer = new QCompleter(ExistingUnitsClass, this);
    mp_Completer->setCaseSensitivity(Qt::CaseInsensitive);
    mp_Completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    ui->UnitsClassEdit->setCompleter(mp_Completer);

    connect(mp_Completer,SIGNAL(activated(const QString&)),this,SLOT(checkGlobal()));
  }

  ui->IDEdit->setPlaceholderText(openfluid::ui::config::PLACEHOLDER_REQUIRED);

  connect(ui->ButtonBox,SIGNAL(accepted()),this,SLOT(accept()));
  connect(ui->ButtonBox,SIGNAL(rejected()),this,SLOT(reject()));

  checkGlobal();
}


// =====================================================================
// =====================================================================


AddDatastoreItemDialog::~AddDatastoreItemDialog()
{
  delete ui;
}


// =====================================================================
// =====================================================================


void AddDatastoreItemDialog::checkGlobal()
{
  openfluid::ui::common::fixLineEdit(ui->IDEdit);

  ui->UnitsClassEdit->setEnabled(ui->UnitsClassCheckBox->isChecked());
  if (ui->UnitsClassEdit->isEnabled())
    ui->UnitsClassEdit->setPlaceholderText(openfluid::ui::config::PLACEHOLDER_REQUIRED);
  else
    ui->UnitsClassEdit->setPlaceholderText("");

  ui->GeovectorEdit->setEnabled(ui->GeovectorRadioButton->isChecked());
  ui->GeovectorBrowseButton->setEnabled(ui->GeovectorRadioButton->isChecked());
  if (ui->GeovectorEdit->isEnabled())
    ui->GeovectorEdit->setPlaceholderText(openfluid::ui::config::PLACEHOLDER_REQUIRED);
  else
    ui->GeovectorEdit->setPlaceholderText("");

  ui->GeorasterEdit->setEnabled(ui->GeorasterRadioButton->isChecked());
  ui->GeorasterBrowseButton->setEnabled(ui->GeorasterRadioButton->isChecked());
  if (ui->GeorasterEdit->isEnabled())
    ui->GeorasterEdit->setPlaceholderText(openfluid::ui::config::PLACEHOLDER_REQUIRED);
  else
    ui->GeorasterEdit->setPlaceholderText("");

  ui->CopySubdirEdit->setEnabled(ui->CopySubdirRadioButton->isChecked());
  if (ui->CopySubdirEdit->isEnabled())
    ui->CopySubdirEdit->setPlaceholderText(openfluid::ui::config::PLACEHOLDER_REQUIRED);
  else
    ui->CopySubdirEdit->setPlaceholderText("");

  ui->DataDestGroupBox->setEnabled(!isSourceFileInInputDataset());


  if (ui->IDEdit->text().isEmpty())
    setMessage(tr("项目ID不能为空"));
  else if (m_ExistingIDs.contains(ui->IDEdit->text()))
    setMessage(tr("项目ID已存在"));
  else if (ui->UnitsClassCheckBox->isChecked() && ui->UnitsClassEdit->text().isEmpty())
    setMessage(tr("单元类不能为空"));
  else if (ui->GeovectorRadioButton->isChecked() && ui->GeovectorEdit->text().isEmpty())
    setMessage(tr("未选择空间向量源文件"));
  else if (ui->GeorasterRadioButton->isChecked() && ui->GeorasterEdit->text().isEmpty())
    setMessage(tr("未选择空间向量源文件"));
  else if (ui->CopySubdirRadioButton->isChecked() && ui->CopySubdirEdit->text().isEmpty())
    setMessage(tr("目标文件夹的子目录为空"));
  else if (ui->CopySubdirRadioButton->isChecked() &&
           (ui->CopySubdirEdit->text().startsWith("/") || ui->CopySubdirEdit->text().startsWith("\\")))
    setMessage(tr("目标文件夹的子目录名错误"));
  else
    setMessage();
}


// =====================================================================
// =====================================================================


void AddDatastoreItemDialog::setMessage(const QString& Msg)
{
  if (Msg.isEmpty())
  {
    ui->MessageFrame->setStyleSheet(QString("background-color: %1;")
                                    .arg(openfluid::ui::config::DIALOGBANNER_BGCOLOR));
    ui->MessageLabel->setText(tr("添加一个存储单元"));
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


void AddDatastoreItemDialog::selectVectorFile()
{
  QString SelectedFile =
      QFileDialog::getOpenFileName(this,
                                   tr("选择文件"),
                                   "",
                                   m_OGRFormatsStr);

  if (!SelectedFile.isEmpty())
    ui->GeovectorEdit->setText(SelectedFile);

  checkGlobal();
}


// =====================================================================
// =====================================================================


void AddDatastoreItemDialog::selectRasterFile()
{

  QString SelectedFile =
      QFileDialog::getOpenFileName(this,
                                   tr("选择文件"),
                                   "",
                                   m_GDALFormatsStr);

  if (!SelectedFile.isEmpty())
    ui->GeorasterEdit->setText(SelectedFile);

  checkGlobal();
}



// =====================================================================
// =====================================================================


QString AddDatastoreItemDialog::getItemID() const
{
  return ui->IDEdit->text();
}


// =====================================================================
// =====================================================================


bool AddDatastoreItemDialog::isUnitsClass() const
{
  return ui->UnitsClassCheckBox->isChecked();
}


// =====================================================================
// =====================================================================


QString AddDatastoreItemDialog::getUnitsClass() const
{
  return ui->UnitsClassEdit->text();
}


// =====================================================================
// =====================================================================


openfluid::core::UnstructuredValue::UnstructuredType AddDatastoreItemDialog::getItemType() const
{
  if (ui->GeovectorRadioButton->isChecked())
    return openfluid::core::UnstructuredValue::GeoVectorValue;
  else
    return openfluid::core::UnstructuredValue::GeoRasterValue;
}


// =====================================================================
// =====================================================================


QString AddDatastoreItemDialog::getSourceFilePath() const
{
  if (ui->GeovectorRadioButton->isChecked())
    return ui->GeovectorEdit->text();
  else
    return ui->GeorasterEdit->text();
}


// =====================================================================
// =====================================================================


bool AddDatastoreItemDialog::isSourceFileInInputDataset() const
{
  QString SourceFile(getSourceFilePath());

  if (SourceFile.isEmpty())
    return false;

  QString InputPath = QString::fromStdString(openfluid::base::RuntimeEnvironment::instance()->getInputDir());

  if (QDir::fromNativeSeparators(SourceFile).startsWith(QDir::fromNativeSeparators(InputPath)))
    return true;

  return false;
}


// =====================================================================
// =====================================================================


QString AddDatastoreItemDialog::getCopySubdir() const
{
  if (ui->CopyRadioButton->isChecked())
    return "";
  else
    return ui->CopySubdirEdit->text();
}

