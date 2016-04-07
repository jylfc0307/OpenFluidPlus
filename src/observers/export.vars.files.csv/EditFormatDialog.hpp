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
  @file EditFormatDialog.hpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
*/



#ifndef __EDITFORMATDIALOG_HPP__
#define __EDITFORMATDIALOG_HPP__


#include <QDialog>

#include "CSVObsTools.hpp"
#include <openfluid/core/DateTime.hpp>

namespace Ui
{
  class EditFormatDialog;
}


class EditFormatDialog: public QDialog
{
  Q_OBJECT;


  private slots:

    void checkGlobal();

    void updatePreview();


  private:

    Ui::EditFormatDialog* ui;

    CSVFormat m_Format;

    QStringList m_ExistingFormatsNames;

    QStringList m_HeaderLabels;
    QList<CSVFormat::HeaderType> m_HeaderCodes;

    QStringList m_DateLabels;
    QList<std::string> m_DateCodes;

    QList<openfluid::core::DateTime> m_PreviewDateTimes;
    QList<double> m_PreviewValues;


  public:

    EditFormatDialog(const QStringList& ExistingFormats,QWidget* Parent = NULL);

    ~EditFormatDialog();

    void initialize(const QString& Name, const QString& Header, const QString& ColSep,
                    const QString& Date, const QString& Precision, const QString& CommentChar);

    openfluid::ware::WareParams_t getFormatParams();
};


#endif /* __EDITFORMATDIALOG_HPP__ */
