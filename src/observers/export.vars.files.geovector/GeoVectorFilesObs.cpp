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
  @file GeoVectorFilesObs.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#include <QStringList>
#include <QDir>
#include <ogrsf_frmts.h>

#include <openfluid/utils/GDALHelpers.hpp>
#include <openfluid/ware/PluggableObserver.hpp>
#include <openfluid/ware/WareParamsTree.hpp>


// =====================================================================
// =====================================================================


BEGIN_OBSERVER_SIGNATURE("export.vars.files.geovector")
  DECLARE_NAME("导出模拟变量到空间地理向量文件");
  DECLARE_DESCRIPTION("这个观察者将模拟变量导出到空间地理向量文件\n"
      "接受如下参数\n"
      "  format : 输出文件的GDAL格式 (必填)"
      "  outsubdir : 输出文件的子目录, 相对于输出目录 (可选)\n"
      "  geoserie.<序列名>.sourcefile : 序列的几何数据源 (必填)\n"
      "  geoserie.<序列名>.unitsclass : 序列的单元类 (必填)\n"
      "  geoserie.<序列名>.vars : 序列的参数列表 (必填)."
      "变量的field名称可以通过varname=>fieldname来准确指定\n"
      "  geoserie.<序列名>.when : 输出文件采用的时间模式 (可选). "
      "为geoserie.<序列名>.when提供的值为init则在初始化阶段, "
        "提供的值为final则在最后的时候, continuous则是连续. "
      "在两个连续输出可以被配置为一个最小的延迟"
        "(例如 continuous;7200 可以设置为一个为2小时的最小延迟)."
      );

  DECLARE_VERSION(openfluid::config::FULL_VERSION);
  DECLARE_STATUS(openfluid::ware::EXPERIMENTAL);

END_OBSERVER_SIGNATURE


// =====================================================================
// =====================================================================


class GeoVectorSerie
{
  public:

    enum WhenModeCases {WHENINIT, WHENCONTINUOUS, WHENFINAL};

    typedef std::map<openfluid::core::VariableName_t,std::string> VariablesSet_t;


    std::string SerieName;

    std::string GeoSourceFilePath;

    openfluid::core::UnitsClass_t UnitsClass;

    VariablesSet_t VariablesSet;

    WhenModeCases WhenMode;

    openfluid::core::Duration_t WhenContinuousDelay;

    openfluid::core::TimeIndex_t LatestContinuousIndex;

    OGRDataSource* GeoSource;

    OGRLayer* GeoLayer;

    std::string OutfilePattern;

    int OFLDIDFieldIndex;


    GeoVectorSerie(const std::string& SName,
                   const std::string& SrcFilePath,
                   const openfluid::core::UnitsClass_t& UClass,
                   const VariablesSet_t& VarsSet,
                   const std::string& OutfileExt,
                   const WhenModeCases& Mode = WHENCONTINUOUS,
                   const openfluid::core::Duration_t& ContModeDelay = 1):
      SerieName(SName),
      GeoSourceFilePath(SrcFilePath),
      UnitsClass(UClass),
      VariablesSet(VarsSet),
      WhenMode(Mode), WhenContinuousDelay(ContModeDelay), LatestContinuousIndex(0),
      GeoSource(NULL), GeoLayer(NULL),
      OutfilePattern(SName+"_"+"%1"+"."+OutfileExt),
      OFLDIDFieldIndex(-1)
    {

    }


    // =====================================================================
    // =====================================================================


    ~GeoVectorSerie()
    {
      OGRDataSource::DestroyDataSource(GeoSource);
    }

};


// =====================================================================
// =====================================================================


class GeoVectorFilesObserver : public openfluid::ware::PluggableObserver
{
  private:

    std::string m_GDALFormat;

    std::string m_InputPath;

    std::string m_OutputPath;

    std::vector<GeoVectorSerie> m_Series;


    // =====================================================================
    // =====================================================================


    bool static isFieldExist(const std::string& FName, const GeoVectorSerie::VariablesSet_t& VarsSet)
    {
      GeoVectorSerie::VariablesSet_t::const_iterator itV;
      GeoVectorSerie::VariablesSet_t::const_iterator itVb = VarsSet.begin();
      GeoVectorSerie::VariablesSet_t::const_iterator itVe = VarsSet.end();

      for (itV = itVb; itV != itVe; ++itV)
      {
        if ((*itV).second == FName)
          return true;
      }

      return false;
    }


    // =====================================================================
    // =====================================================================


    void updateFieldNamesUsingFormat(GeoVectorSerie::VariablesSet_t& VarsSet)
    {
      if (m_GDALFormat == "ESRI Shapefile")
      {
        // limit field name to 10 characters,
        // and generate a new name avoiding duplicate names

        GeoVectorSerie::VariablesSet_t::iterator itV;
        GeoVectorSerie::VariablesSet_t::iterator itVb = VarsSet.begin();
        GeoVectorSerie::VariablesSet_t::iterator itVe = VarsSet.end();

        for (itV = itVb; itV != itVe; ++itV)
        {

          if ((*itV).second.size()>10)
          {

            QString BaseName = QString::fromStdString((*itV).second).left(8);
            int Nbr = 1;

            QString NewFieldName = QString(BaseName+"%1").arg(Nbr);

            while (isFieldExist(NewFieldName.toStdString(),VarsSet) && Nbr < 100)
              Nbr++;

            (*itV).second = NewFieldName.toStdString();

          }
        }
      }
    }



    // =====================================================================
    // =====================================================================


    static GeoVectorSerie::VariablesSet_t convertParamToVariableSet(const std::string& VarsStr)
    {
      GeoVectorSerie::VariablesSet_t VarsSet;

      QStringList VarsList = QString::fromStdString(VarsStr).split(";");

      for (int i=0; i<VarsList.size();i++)
      {
        QStringList VarsNameAlias = VarsList[i].split("=>");

        if (VarsNameAlias.size() == 1)
          VarsSet[VarsNameAlias[0].toStdString()] = VarsNameAlias[0].toStdString();
        else if (VarsNameAlias.size() == 2)
          VarsSet[VarsNameAlias[0].toStdString()] = VarsNameAlias[1].toStdString();
        else
          // error in variables list format, so returning an empty variables set
          return GeoVectorSerie::VariablesSet_t();
      }

      return VarsSet;
    }


    // =====================================================================
    // =====================================================================


    static bool setWhenModeFromParam(const std::string& WhenStr,
                                     GeoVectorSerie::WhenModeCases& Mode, openfluid::core::Duration_t& ContinuousDelay)
    {
      if (WhenStr == "init")
      {
        Mode = GeoVectorSerie::WHENINIT;
        ContinuousDelay = 1;
        return true;
      }

      if (WhenStr == "final")
      {
        Mode = GeoVectorSerie::WHENFINAL;
        ContinuousDelay = 1;
        return true;
      }

      if (WhenStr.empty() || WhenStr == "continuous")
      {
        Mode = GeoVectorSerie::WHENCONTINUOUS;
        ContinuousDelay = 1;
        return true;
      }


      QString WhenQStr = QString::fromStdString(WhenStr);

      if (WhenQStr.startsWith("continuous"))
      {
        QStringList WhenQParts = WhenQStr.split(";");

        if (WhenQParts.size() == 2)
        {
          bool IsConverted;
          ContinuousDelay = WhenQParts[1].toULongLong(&IsConverted);

          if (IsConverted)
          {
            Mode = GeoVectorSerie::WHENCONTINUOUS;
            return true;
          }
          return false;
        }
        return false;
      }
      return false;
    }


    // =====================================================================
    // =====================================================================


    void prepareSerie(GeoVectorSerie& Serie)
    {
      // opening and checking of source files
      Serie.GeoSource = OGRSFDriverRegistrar::Open(Serie.GeoSourceFilePath.c_str(),FALSE);


      if (Serie.GeoSource)
      {
        Serie.GeoLayer = Serie.GeoSource->GetLayer(0);

        if (Serie.GeoLayer)
        {
          // checking of OFLD_ID
          Serie.OFLDIDFieldIndex = Serie.GeoLayer->GetLayerDefn()->GetFieldIndex("OFLD_ID");

        }
      }
    }


    // =====================================================================
    // =====================================================================


    void processSerie(GeoVectorSerie& Serie)
    {
      QString IndexStr = "init";

      openfluid::base::SimulationStatus::SimulationStage CurrentStage =
          OPENFLUID_GetCurrentStage();

      bool OKToWrite = false;

      if (CurrentStage == openfluid::base::SimulationStatus::INITIALIZERUN)
      {
        OKToWrite = Serie.WhenMode == GeoVectorSerie::WHENINIT ||
                    Serie.WhenMode == GeoVectorSerie::WHENCONTINUOUS;

      }
      else if (CurrentStage == openfluid::base::SimulationStatus::RUNSTEP)
      {
        if (Serie.WhenMode == GeoVectorSerie::WHENCONTINUOUS)
        {
          openfluid::core::TimeIndex_t CurrentIndex = OPENFLUID_GetCurrentTimeIndex();
          IndexStr = QString("%1").arg(CurrentIndex);

          if (Serie.LatestContinuousIndex + Serie.WhenContinuousDelay < CurrentIndex)
          {
            OKToWrite = true;
            Serie.LatestContinuousIndex = CurrentIndex;
          }
        }
      }
      else if (CurrentStage == openfluid::base::SimulationStatus::FINALIZERUN)
      {
        IndexStr = "final";
        OKToWrite = Serie.WhenMode == GeoVectorSerie::WHENCONTINUOUS ||
                    Serie.WhenMode == GeoVectorSerie::WHENFINAL;
      }
      else
      {
        OPENFLUID_LogWarning("Internal stage error when processing geographic series");
        return;
      }



      if (OKToWrite)
      {
        std::string FullFilePath =
            m_OutputPath + "/" + QString(QString::fromStdString(Serie.OutfilePattern).arg(IndexStr)).toStdString();


        OGRSFDriver* Driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(m_GDALFormat.c_str());
        OGRDataSource* CreatedFile = Driver->CreateDataSource(FullFilePath.c_str());

        std::string CreatedLayerName = QFileInfo(QString::fromStdString(FullFilePath)).completeBaseName().toStdString();

        OGRLayer* CreatedLayer = CreatedFile->CreateLayer(CreatedLayerName.c_str(),NULL,
                                                          Serie.GeoLayer->GetLayerDefn()->GetGeomType(),
                                                          NULL);

        OGRFieldDefn IDField("OFLD_ID",OFTInteger);
        CreatedLayer->CreateField(&IDField);


        GeoVectorSerie::VariablesSet_t::const_iterator itV;
        GeoVectorSerie::VariablesSet_t::const_iterator itVb = Serie.VariablesSet.begin();
        GeoVectorSerie::VariablesSet_t::const_iterator itVe = Serie.VariablesSet.end();

        for (itV = itVb; itV != itVe; ++itV)
        {
          std::string FieldName = (*itV).second;

          OGRFieldDefn VarField(FieldName.c_str(),OFTReal);
          VarField.SetWidth(24);
          VarField.SetPrecision(15);

          CreatedLayer->CreateField(&VarField);
        }


        OGRFeature* SourceFeature;
        openfluid::core::SpatialUnit* UU;

        Serie.GeoLayer->ResetReading();
        while ((SourceFeature = Serie.GeoLayer->GetNextFeature()) != NULL)
        {
          int SourceID = SourceFeature->GetFieldAsInteger(Serie.OFLDIDFieldIndex);
          openfluid::core::DoubleValue CreatedValue = 0.0;

          UU = OPENFLUID_GetUnit(Serie.UnitsClass,SourceID);

          if (UU)
          {
            CreatedLayer->GetLayerDefn();

            OGRFeature* CreatedFeature = OGRFeature::CreateFeature(CreatedLayer->GetLayerDefn());

            CreatedFeature->SetGeometry(SourceFeature->GetGeometryRef()->clone());
            CreatedFeature->SetField("OFLD_ID",SourceID);



            for (itV = itVb; itV != itVe; ++itV)
            {
              std::string VarName = (*itV).first;
              std::string FieldName = (*itV).second;

              if (FieldName.empty())
                FieldName = VarName;

              openfluid::core::IndexedValue VarValue;

              if (OPENFLUID_IsVariableExist(UU,VarName))
                OPENFLUID_GetLatestVariable(UU,VarName,VarValue);
              else
              {
                QString Msg("Variable %1 does not exist on unit %2#%3");
                Msg.arg(VarName.c_str()).arg(UU->getClass().c_str()).arg(UU->getID());
                OPENFLUID_LogWarning(Msg.toStdString());
              }

              if (VarValue.value()->isDoubleValue())
                CreatedValue = VarValue.value()->asDoubleValue();
              else
              {
                QString Msg("Variable %1 on unit %2#%3 is not a double. Only double are currently supported");
                Msg.arg(VarName.c_str()).arg(UU->getClass().c_str()).arg(UU->getID());
                OPENFLUID_LogWarning(Msg.toStdString());
              }

              CreatedFeature->SetField(FieldName.c_str(),VarValue.value()->asDoubleValue());
            }

            CreatedLayer->CreateFeature(CreatedFeature);

            OGRFeature::DestroyFeature(CreatedFeature);

          }
        }
        OGRDataSource::DestroyDataSource(CreatedFile);
      }
    }


    // =====================================================================
    // =====================================================================


    void closeSerie(GeoVectorSerie& Serie)
    {
      if (Serie.GeoSource)
      {
        OGRDataSource::DestroyDataSource(Serie.GeoSource);
        Serie.GeoSource = NULL;
      }
    }


    // =====================================================================
    // =====================================================================


  public:

    GeoVectorFilesObserver() : PluggableObserver()
    {
      OGRRegisterAll();
    }


    // =====================================================================
    // =====================================================================


    ~GeoVectorFilesObserver()
    {
      std::vector<GeoVectorSerie>::iterator it;

      for (it = m_Series.begin();it!=m_Series.end(); ++it)
        closeSerie(*it);
    }


    // =====================================================================
    // =====================================================================


    void initParams(const openfluid::ware::WareParams_t& Params)
    {
      openfluid::ware::WareParamsTree ParamsTree;

      try
      {
        ParamsTree.setParams(Params);
      }
      catch (openfluid::base::FrameworkException& E)
      {
        OPENFLUID_RaiseError(E.getMessage());
      }

      // checking of mandatory parameters
      if (!ParamsTree.root().hasChild("format"))
      {
        OPENFLUID_LogWarning("Missing GDAL format for output files");
        return;
      }

      if (!ParamsTree.root().hasChild("format"))
      {
        OPENFLUID_LogWarning("No serie defined");
        return;
      }


      // process of format parameter
      m_GDALFormat = ParamsTree.root().getChildValue("format","");

      openfluid::utils::GDALDriversFilesExts_t ValidVectorDrivers =
          openfluid::utils::getOGRFilesDriversForOpenFLUID();

      if (ValidVectorDrivers.find(m_GDALFormat) == ValidVectorDrivers.end())
      {
        OPENFLUID_LogWarning("Unsupported GDAL format for output files");
        return;
      }

      std::string OutfileExt = ValidVectorDrivers[m_GDALFormat].FilesExts.front();


      // process of parameter for optional output subdirectory
      std::string OutSeriesSubdir = ParamsTree.root().getChildValue("outsubdir","");


      // get paths for input dataset and output directory
      OPENFLUID_GetRunEnvironment("dir.input",m_InputPath);
      OPENFLUID_GetRunEnvironment("dir.output",m_OutputPath);

      if (!OutSeriesSubdir.empty())
        m_OutputPath += "/"+OutSeriesSubdir;


      for (auto& Serie : ParamsTree.root().child("geoserie"))
      {
        GeoVectorSerie::WhenModeCases Mode = GeoVectorSerie::WHENCONTINUOUS;
        openfluid::core::Duration_t ContinuousDelay = 1;

        std::string SerieName = Serie.first;

        std::string GeoSourceFilename = Serie.second.getChildValue("sourcefile","");
        std::string VarsString = Serie.second.getChildValue("vars","");
        openfluid::core::UnitsClass_t UnitsClass = Serie.second.getChildValue("unitsclass","");
        std::string WhenModeString = Serie.second.getChildValue("when","");


        if (GeoSourceFilename.empty())
          OPENFLUID_LogWarning("Missing geographic source filename for serie "+SerieName);
        else if (UnitsClass.empty())
          OPENFLUID_LogWarning("Missing units class for serie "+SerieName);
        else if (VarsString.empty())
          OPENFLUID_LogWarning("Missing variables list for serie "+SerieName);
        else
        {
          GeoVectorSerie::VariablesSet_t VarsSet = convertParamToVariableSet(VarsString);

          if (VarsSet.empty())
            OPENFLUID_LogWarning("Format error in variables list for serie "+SerieName);
          else
          {
            if (setWhenModeFromParam(WhenModeString,Mode,ContinuousDelay))
            {
              // everything's OK, add the serie to the active series set
              m_Series.push_back(GeoVectorSerie(SerieName,
                                                m_InputPath + "/" + GeoSourceFilename,
                                                UnitsClass,VarsSet,
                                                OutfileExt,
                                                Mode,ContinuousDelay));
            }
            else
              OPENFLUID_LogWarning("Format error in whenmode for serie "+SerieName);
          }
        }
      }
    }


    // =====================================================================
    // =====================================================================


    void onPrepared()
    {
      if (m_Series.empty()) return;

      std::vector<GeoVectorSerie>::iterator it;


      // preparation of output directory for series (if any)
      QDir().mkpath(QString::fromStdString(m_OutputPath));


      // preparation of series
      for (it = m_Series.begin();it!=m_Series.end(); ++it)
        prepareSerie(*it);


      // removal of wrong formatted source geovector file
      for (it=m_Series.begin();it!=m_Series.end(); )
      {
        if ((*it).GeoSource == NULL)
        {
          OPENFLUID_LogWarning("Cannot open geographic source for serie "+(*it).SerieName);
          closeSerie(*it);
          it = m_Series.erase(it);
        }
        else if ((*it).GeoLayer == NULL)
        {
          OPENFLUID_LogWarning("Cannot open layer in geographic source for serie "+(*it).SerieName);
          closeSerie(*it);
          it = m_Series.erase(it);
        }
        else if ((*it).OFLDIDFieldIndex < 0)
        {
          OPENFLUID_LogWarning("File not found or wrong file format for geographic source for serie "+
                                 (*it).SerieName);
          closeSerie(*it);
          it = m_Series.erase(it);
        }
        else if ((*it).VariablesSet.empty())
        {
          OPENFLUID_LogWarning("No correct variable name in serie "+(*it).SerieName);
          closeSerie(*it);
          it = m_Series.erase(it);
        }
        else
          ++it;
      }


      // Update field names if necessary
      for (it=m_Series.begin();it!=m_Series.end(); ++it)
         updateFieldNamesUsingFormat((*it).VariablesSet);

    }


    // =====================================================================
    // =====================================================================


    void onInitializedRun()
    {
      if (m_Series.empty()) return;

      std::vector<GeoVectorSerie>::iterator it;

      for (it = m_Series.begin();it!=m_Series.end(); ++it)
        processSerie(*it);

    }


    // =====================================================================
    // =====================================================================


    void onStepCompleted()
    {
      if (m_Series.empty()) return;

      std::vector<GeoVectorSerie>::iterator it;

      for (it = m_Series.begin();it!=m_Series.end(); ++it)
        processSerie(*it);

    }


    // =====================================================================
    // =====================================================================


    void onFinalizedRun()
    {
      if (m_Series.empty()) return;

      std::vector<GeoVectorSerie>::iterator it;

      for (it = m_Series.begin();it!=m_Series.end(); ++it)
        processSerie(*it);

      for (it = m_Series.begin();it!=m_Series.end(); ++it)
        closeSerie(*it);
    }

};


// =====================================================================
// =====================================================================


DEFINE_OBSERVER_CLASS(GeoVectorFilesObserver)

