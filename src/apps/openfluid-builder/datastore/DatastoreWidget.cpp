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
  @file DatastoreWidget.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#include "ui_DatastoreWidget.h"
#include "DatastoreWidget.hpp"
#include "AddDatastoreItemDialog.hpp"

#include <openfluid/fluidx/AdvancedFluidXDescriptor.hpp>
#include <openfluid/base/RuntimeEnv.hpp>
#include <gdal.h>
#include <gdal_priv.h>
#include <ogr_api.h>
#include <openfluid/tools/QtHelpers.hpp>
#include "ogrsf_frmts.h"

#include <QFileInfo>
#include <QDir>
#include <QMessageBox>


DatastoreWidget::DatastoreWidget(QWidget* Parent, openfluid::fluidx::AdvancedFluidXDescriptor& AFXDesc):
  WorkspaceWidget(Parent,AFXDesc), ui(new Ui::DatastoreWidget), m_Datastore(AFXDesc.datastore())
{
  ui->setupUi(this);

  ui->AddButton->setIcon(QIcon(":/ui/common/icons/add.png"));
  ui->AddButton->setIconSize(QSize(20,20));

  ui->EditButton->setIcon(QIcon(":/ui/common/icons/modify.png"));
  ui->EditButton->setIconSize(QSize(20,20));

  ui->RemoveButton->setIcon(QIcon(":/ui/common/icons/remove.png"));
  ui->RemoveButton->setIconSize(QSize(20,20));

  ui->UpButton->setIcon(QIcon(":/ui/common/icons/go-up.png"));
  ui->UpButton->setIconSize(QSize(20,20));

  ui->DownButton->setIcon(QIcon(":/ui/common/icons/go-down.png"));
  ui->DownButton->setIconSize(QSize(20,20));

  // TODO to re-enable
  ui->EditButton->setVisible(false);

  connect(ui->AddButton,SIGNAL(clicked()),this,SLOT(addItem()));
  connect(ui->EditButton,SIGNAL(clicked()),this,SLOT(editItem()));
  connect(ui->RemoveButton,SIGNAL(clicked()),this,SLOT(removeItem()));

  connect(ui->UpButton,SIGNAL(clicked()),this,SLOT(moveItemUp()));
  connect(ui->DownButton,SIGNAL(clicked()),this,SLOT(moveItemDown()));

  connect(ui->DatastoreTableWidget->horizontalHeader(),SIGNAL(sectionResized(int, int, int)),
          ui->DatastoreTableWidget,SLOT(resizeRowsToContents()));

  refresh();
}


// =====================================================================
// =====================================================================


DatastoreWidget::~DatastoreWidget()
{
  delete ui;
}


// =====================================================================
// =====================================================================


void DatastoreWidget::refresh()
{
  ui->DatastoreTableWidget->setRowCount(0);

  const std::list<openfluid::fluidx::DatastoreItemDescriptor*>& Items = m_Datastore.items();

  ui->DatastoreTableWidget->setRowCount(Items.size());

  int CurrentRow = 0;

  for (std::list<openfluid::fluidx::DatastoreItemDescriptor*>::const_iterator it = Items.begin();
       it != Items.end(); ++it)
  {
    QTableWidgetItem *Item;

    Item = new QTableWidgetItem(QString::fromStdString((*it)->getID()));
    ui->DatastoreTableWidget->setItem(CurrentRow, 0, Item);

    Item = new QTableWidgetItem(QString::fromStdString(
        openfluid::core::UnstructuredValue::getStringFromValueType((*it)->getType())));
    ui->DatastoreTableWidget->setItem(CurrentRow, 1, Item);

    Item = new QTableWidgetItem(QString::fromStdString((*it)->getUnitsClass()));
    ui->DatastoreTableWidget->setItem(CurrentRow, 2, Item);

    Item = new QTableWidgetItem(QDir::toNativeSeparators(QString::fromStdString((*it)->getRelativePath())));
    ui->DatastoreTableWidget->setItem(CurrentRow, 3, Item);

    CurrentRow++;
  }

  ui->DatastoreTableWidget->resizeRowsToContents();
}


// =====================================================================
// =====================================================================


void DatastoreWidget::addItem()
{
  AddDatastoreItemDialog AddItemDlg(openfluid::tools::toQStringList(m_AdvFluidxDesc.spatialDomain().getClassNames()),
                                    openfluid::tools::toQStringList(m_Datastore.getItemsIDs()),
                                    this);

  if (AddItemDlg.exec() == QDialog::Accepted)
  {

    bool OKToCreateItem = true;

    // by default, the file associated to the item is in the inputdir path
    QString DSItemFile = AddItemDlg.getSourceFilePath();

    // copy of source file(s) if file is not in the inputdir path
    if (!AddItemDlg.isSourceFileInInputDataset())
    {
      // Prepare copy in dataset

      QString Subdir = QDir::fromNativeSeparators(AddItemDlg.getCopySubdir());
      QString SourceFilename = QDir::fromNativeSeparators(QFileInfo(AddItemDlg.getSourceFilePath()).fileName());

      QString DestFile =
          QDir::fromNativeSeparators(QString::fromStdString(
              openfluid::base::RuntimeEnvironment::instance()->getInputDir()) +
              "/"+Subdir+"/"+SourceFilename);

      if (AddItemDlg.getItemType() == openfluid::core::UnstructuredValue::GeoVectorValue)
      {
        // copy vector file
        OGRDataSource* SrcDS = OGRSFDriverRegistrar::Open(
            QDir::fromNativeSeparators(AddItemDlg.getSourceFilePath()).toStdString().c_str(),
            FALSE);

        if (SrcDS != NULL)
        {
          OGRSFDriver *CopyDriver = SrcDS->GetDriver();

          if (CopyDriver != NULL)
          {
            // creation of the destination directory if not exists
            QDir().mkpath(QFileInfo(DestFile).absolutePath());

            // temporary datasource to see if the file already exists
            OGRDataSource* DestAlreadyExists = CopyDriver->Open(DestFile.toStdString().c_str());

            if (DestAlreadyExists)
            {
              OGRDataSource::DestroyDataSource(DestAlreadyExists);

              OKToCreateItem =
                  (QMessageBox::question(QApplication::activeWindow(),
                                         "OpenFLUID-Builder",
                                         tr("文件%1将要被复制到空间数据集中,\n"
                                             "作为数据集的源文件。\n"
                                             "\n"
                                             "一个同名文件已存在于目录中\n"
                                             "他将被新的文件覆盖。\n"
                                             "\n"
                                             "确认要继续吗?").arg(SourceFilename),
                                             QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok);
              if (OKToCreateItem)
              {
                // if file already exists, it is deleted
                CopyDriver->DeleteDataSource(DestFile.toStdString().c_str());
              }
            }

            if (OKToCreateItem)
            {
              OGRDataSource* DestDS = CopyDriver->CopyDataSource(SrcDS,
                                                                 DestFile.toStdString().c_str());
              OGRDataSource::DestroyDataSource(DestDS);
            }

            OGRDataSource::DestroyDataSource(SrcDS);
          }
          else
          {
            QMessageBox::critical(QApplication::activeWindow(),
                                  "OpenFLUID-Builder",
                                  tr("复制geovector源文件时OGR驱动产生错误\n"
                                     "这项空间数据将不会被添加"),
                                  QMessageBox::Close);
            return;
          }
        }
        else
        {
          QMessageBox::critical(QApplication::activeWindow(),
                                "OpenFLUID-Builder",
                                tr("复制geovector源文件时OGR产生错误\n"
                                   "这项空间数据将不会被添加"),
                                QMessageBox::Close);
          return;
        }
      }
      else
      {
        // copy raster file

        GDALDataset* SrcDS = (GDALDataset *)GDALOpen(
            QDir::fromNativeSeparators(AddItemDlg.getSourceFilePath()).toStdString().c_str(),
            GA_ReadOnly);

        if (SrcDS != NULL)
        {
          GDALDriver *CopyDriver = SrcDS->GetDriver();

          if (CopyDriver != NULL)
          {
            QDir().mkpath(QFileInfo(DestFile).absolutePath());

            if (CopyDriver->CopyFiles(
                    DestFile.toStdString().c_str(),
                    QDir::fromNativeSeparators(AddItemDlg.getSourceFilePath()).toStdString().c_str()) == CE_Failure)
            {
              QMessageBox::critical(QApplication::activeWindow(),
                                    "OpenFLUID-Builder",
                                    tr("复制georaster源文件时GDAL产生错误\n"
                                       "这项空间数据将不会被添加"),
                                    QMessageBox::Close);
              return;
            }

          }
          else
          {
            QMessageBox::critical(QApplication::activeWindow(),
                                  "OpenFLUID-Builder",
                                  tr("复制georaster源文件时GDAL驱动产生错误\n"
                                     "这项空间数据将不会被添加"),
                                  QMessageBox::Close);
            return;
          }
        }
        else
        {
          QMessageBox::critical(QApplication::activeWindow(),
                                "OpenFLUID-Builder",
                                tr("打开georaster源文件时GDAL产生错误\n"
                                   "这项空间数据将不会被添加"),
                                QMessageBox::Close);
          return;
        }
        GDALClose(SrcDS);
      }

      // adjust the file associated to the item with the copied file
      DSItemFile = DestFile;
    }


    if (OKToCreateItem)
    {
      // creation of datastore item

      // build of the relative path for the file associated with the item
      QString RelativeDSItemFile =
          QDir(QString::fromStdString(openfluid::base::RuntimeEnvironment::instance()->getInputDir()))
          .relativeFilePath(DSItemFile);

      openfluid::fluidx::DatastoreItemDescriptor* DSItemDesc =
          new openfluid::fluidx::DatastoreItemDescriptor(AddItemDlg.getItemID().toStdString(),
                                                         openfluid::base::RuntimeEnvironment::instance()->getInputDir(),
                                                         QDir::fromNativeSeparators(RelativeDSItemFile).toStdString(),
                                                         AddItemDlg.getItemType());

      if (AddItemDlg.isUnitsClass())
        DSItemDesc->setUnitsClass(AddItemDlg.getUnitsClass().toStdString());

      m_Datastore.appendItem(DSItemDesc);

      emit changed(openfluid::builderext::FluidXUpdateFlags::FLUIDX_DATASTORE);

      refresh();
    }
  }
}


// =====================================================================
// =====================================================================


void DatastoreWidget::editItem()
{
  // TODO
  QMessageBox::critical(QApplication::activeWindow(),QString("not implemented"),
                        QString(__PRETTY_FUNCTION__),QMessageBox::Close);
}


// =====================================================================
// =====================================================================


void DatastoreWidget::removeItem()
{
  bool OK = true;

  int DSItemIndex = ui->DatastoreTableWidget->currentRow();

  if (DSItemIndex >=0)
  {
    QString DSItemID = ui->DatastoreTableWidget->item(DSItemIndex,0)->text();

    OK = (QMessageBox::question(QApplication::activeWindow(),
                                "OpenFLUID-Builder",
                                tr("您正从空间数据集中删除%1。\n"
                                   "这个项目将不再可用，但是数据依然会保存在磁盘中。\n"
                                   "\n"
                                   "确认要继续吗?").arg(DSItemID),
                                QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok);

    if (OK)
    {
      m_Datastore.removeItem(DSItemIndex);

      emit changed(openfluid::builderext::FluidXUpdateFlags::FLUIDX_DATASTORE);

      refresh();
    }
  }
}


// =====================================================================
// =====================================================================


void DatastoreWidget::moveItemUp()
{
  if (ui->DatastoreTableWidget->rowCount() < 2)
    return;

  int DSItemIndex = ui->DatastoreTableWidget->currentRow();

  if (DSItemIndex >=1)
  {
    int From = DSItemIndex;
    int To = DSItemIndex-1;

    m_Datastore.moveItem(From,To);

    emit changed(openfluid::builderext::FluidXUpdateFlags::FLUIDX_DATASTORE);

    refresh();

    ui->DatastoreTableWidget->selectRow(To);
  }
}


// =====================================================================
// =====================================================================


void DatastoreWidget::moveItemDown()
{
  int RowCount = ui->DatastoreTableWidget->rowCount();

  if (RowCount < 2)
    return;

  int DSItemIndex = ui->DatastoreTableWidget->currentRow();

  if (DSItemIndex>=0 && DSItemIndex<RowCount-1)
  {
    int From = DSItemIndex;
    int To = DSItemIndex+1;

    m_Datastore.moveItem(From,To);

    emit changed(openfluid::builderext::FluidXUpdateFlags::FLUIDX_DATASTORE);

    refresh();

    ui->DatastoreTableWidget->selectRow(To);
  }
}

