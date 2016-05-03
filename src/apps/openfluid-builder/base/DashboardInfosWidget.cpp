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
  @file DashboardInfosWidget.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */

#include <openfluid/base/ProjectManager.hpp>

#include "ui_DashboardInfosWidget.h"
#include "DashboardInfosWidget.hpp"

#include <QDir>


DashboardInfosWidget::DashboardInfosWidget(const openfluid::fluidx::AdvancedFluidXDescriptor& Desc, QWidget* Parent):
  DashboardWidget(Parent), ui(new Ui::DashboardInfosWidget), m_AdvFluidxDesc(Desc)
{
  ui->setupUi(this);

  refreshProjectInfos();

  ui->ContentsFrame->setStyleSheet("#ContentsFrame "
                                       "{background-color: qlineargradient(spread:pad, x1:1, y1:1, x2:1, y2:0, "
                                           "stop:0 #314054, "
                                           "stop:1 #3F546E); "
                                           "border-radius: 6px;"
                                           "border : 1px solid #AAAAAA;}");

  ui->TitleLabel->setStyleSheet("color: white;");
  ui->PathLabel->setStyleSheet("color: white;");

  // Improve size of project title by 2 points
  QFont TmpFont = ui->TitleLabel->font();
  TmpFont.setPointSize(TmpFont.pointSize()+2);
  ui->TitleLabel->setFont(TmpFont);

  ui->ModelLabel->setStyleSheet("color: white;");
  ui->SpatialLabel->setStyleSheet("color: white;");
  ui->DatastoreLabel->setStyleSheet("color: white;");
  ui->MonitoringLabel->setStyleSheet("color: white;");
  ui->SimConfigLabel->setStyleSheet("color: white;");


  refresh();
}


// =====================================================================
// =====================================================================


DashboardInfosWidget::~DashboardInfosWidget()
{
  delete ui;
}


// =====================================================================
// =====================================================================


void DashboardInfosWidget::refresh()
{
  unsigned int SimCount, GenCount;
  m_AdvFluidxDesc.model().getItemsCountByType(SimCount,GenCount);

  unsigned int UnitsCount = m_AdvFluidxDesc.spatialDomain().getUnitsCount();
  int UnitsClassesCount = m_AdvFluidxDesc.spatialDomain().getClassNames().size();

  unsigned int DStoreCount = m_AdvFluidxDesc.datastoreDescriptor().items().size();

  unsigned int MonitoringCount = m_AdvFluidxDesc.monitoring().items().size();

  int DeltaT = m_AdvFluidxDesc.runDescriptor().getDeltaT();

  int Duration =
      m_AdvFluidxDesc.runDescriptor().getEndDate().diffInSeconds(m_AdvFluidxDesc.runDescriptor().getBeginDate());

  openfluid::base::SimulationStatus::SchedulingConstraint SchedConst =
      m_AdvFluidxDesc.runDescriptor().getSchedulingConstraint();
  QString SchedConstStr = tr("没有限制");

  if (SchedConst == openfluid::base::SimulationStatus::SCHED_DTCHECKED)
    SchedConstStr = tr("检验缺省ΔT值");
  if (SchedConst == openfluid::base::SimulationStatus::SCHED_DTFORCED)
    SchedConstStr = tr("强制缺省ΔT值");


  ui->ModelLabel->setText(tr("<u>耦合模型:</u> %1 个模拟器和 %2 个生成器").arg(SimCount).arg(GenCount));
  ui->SpatialLabel->setText(tr("<u>空间领域:</u> 在 %2 个空间类中有共计 %1 个空间单元")
                            .arg(UnitsCount).arg(UnitsClassesCount));
  ui->DatastoreLabel->setText(tr("<u>数据存储:</u> %1 个项目").arg(DStoreCount));
  ui->MonitoringLabel->setText(tr("<u>监控:</u> %1 监控器(s)").arg(MonitoringCount));
  ui->SimConfigLabel->setText(tr("<u>运行设置:</u> 总共持续 %1 秒， "
                                 "ΔT值为 %2 秒，%3")
                              .arg(Duration).arg(DeltaT).arg(SchedConstStr));
}


// =====================================================================
// =====================================================================


void DashboardInfosWidget::refreshProjectInfos()
{
  QString NativePath =
      QDir::toNativeSeparators(QString::fromStdString(openfluid::base::ProjectManager::instance()->getPath()));

  ui->TitleLabel->setText(QString::fromStdString(openfluid::base::ProjectManager::instance()->getName()));
  ui->PathLabel->setText(NativePath);
  ui->PathLabel->setToolTip(NativePath);
}
