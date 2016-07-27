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
  @file OpenExampleProjectDialog.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */

#include <QPushButton>
#include <QDir>

#include <openfluid/config.hpp>
#include <openfluid/base/RuntimeEnv.hpp>
#include <openfluid/base/ProjectManager.hpp>
#include <openfluid/buddies/ExamplesBuddy.hpp>

#include "ui_OpenExampleProjectDialog.h"
#include "OpenExampleProjectDialog.hpp"

#include "AppTools.hpp"
#include <openfluid/ui/config.hpp>


OpenExampleProjectDialog::OpenExampleProjectDialog(QWidget *Parent):
  openfluid::ui::common::OpenFLUIDDialog(Parent), ui(new Ui::OpenExampleProjectDialog)
{
  ui->setupUi(this);


  // installation of missing examples
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  openfluid::buddies::ExamplesBuddy(NULL).run();
  QApplication::restoreOverrideCursor();

  m_ProjectsRootPath = QString(openfluid::base::RuntimeEnvironment::instance()->getUserExamplesDir().c_str()) +
                       QString("/") +
                       QString(openfluid::config::PROJECTS_SUBDIR.c_str());

  QStringList ExamplesDirs;
  ExamplesDirs = QDir(m_ProjectsRootPath).entryList(QStringList("*"),QDir::Dirs | QDir::NoDotAndDotDot);

  ExamplesDirs.sort();

  for (int i=0;i<ExamplesDirs.size();++i)
  {
    QString CurrentFullDir(m_ProjectsRootPath+"/"+ExamplesDirs[i]);

    if (openfluid::base::ProjectManager::isProject(CurrentFullDir.toStdString()))
      ui->ProjectsListWidget->addItem(ExamplesDirs[i]);
  }

  ui->ButtonBox->button(QDialogButtonBox::Ok)->setText("确定");
  ui->ButtonBox->button(QDialogButtonBox::Cancel)->setText("取消");

  updateProjectInfo();

  connect(ui->ProjectsListWidget,
          SIGNAL(itemSelectionChanged()),
          this, SLOT(updateProjectInfo()));

  connect(ui->ButtonBox,SIGNAL(accepted()),this,SLOT(accept()));
  connect(ui->ButtonBox,SIGNAL(rejected()),this,SLOT(reject()));
}


// =====================================================================
// =====================================================================


OpenExampleProjectDialog::~OpenExampleProjectDialog()
{
  delete ui;
}


// =====================================================================
// =====================================================================


void OpenExampleProjectDialog::setMessage(const QString& Msg)
{
  if (Msg.isEmpty())
  {
    ui->MessageFrame->setStyleSheet(QString("background-color: %1;")
                                    .arg(openfluid::ui::config::DIALOGBANNER_BGCOLOR));
    ui->MessageLabel->setText(tr("打开一个项目"));
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


void OpenExampleProjectDialog::updateProjectInfo()
{
  if (ui->ProjectsListWidget->currentRow()<0)
  {
    setMessage(tr("没有选择项目"));
    ui->ProjectInfosLabel->setText("(没有项目信息)");
    ui->ProjectInfosLabel->setStyleSheet("font:italic;");
  }
  else
  {
    QString InfosStr = getProjectInfosAsHTML(m_ProjectsRootPath+"/"+ui->ProjectsListWidget->currentItem()->text());

    if (!InfosStr.isEmpty())
    {
      setMessage();
      ui->ProjectInfosLabel->setStyleSheet("");
      ui->ProjectInfosLabel->setText(InfosStr);
    }
    else
    {
      setMessage(tr("错误的项目格式"));
      ui->ProjectInfosLabel->setText("(没有项目信息)");
      ui->ProjectInfosLabel->setStyleSheet("font:italic;");
    }
  }
}


// =====================================================================
// =====================================================================


QString OpenExampleProjectDialog::getSelectedProjectPath() const
{
  if (ui->ProjectsListWidget->currentRow()<0)
    return "";
  else
    return QString(m_ProjectsRootPath+"/"+ui->ProjectsListWidget->currentItem()->text());

}
