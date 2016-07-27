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
  @file GNUplotObs.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */



#include <openfluid/utils/ExternalProgram.hpp>
#include <openfluid/ware/PluggableObserver.hpp>
#include <openfluid/tools/DataHelpers.hpp>
#include <openfluid/ware/WareParamsTree.hpp>

#include <QDir>
#include <QProcess>


// =====================================================================
// =====================================================================


BEGIN_OBSERVER_SIGNATURE("export.vars.plot.gnuplot")

  DECLARE_NAME("用GNUplot绘制模拟变量");
  DECLARE_DESCRIPTION("这个观察者用GNUplot formalism and tools绘制模拟变量\n"
      "他使用可以被合并为一个或多个图形的值的序列 (模拟出的或者已经存在的)。\n"
      "只能绘制标量参数 (布尔值, 整数, 实数)\n"
      "用于声明序列的参数可以有:\n"
      "  serie.<序列名称>.var : 需要绘制的参数名称 (例 var.foo)\n"
      "  serie.<序列名称>.unitsclass : 单元类 (例 TU)\n"
      "  serie.<序列名称>.unitid : 单元类 (例 17)\n"
      "  serie.<序列名称>.sourcefile : 需要绘制的文件 (例 measured_data.dat), "
        "如果这个序列不是基于模拟变量\n"
      "  serie.<序列名称>.style : 绘制此序列所用的GNUplot风格 (例 linespoint)\n"
      "  serie.<序列名称>.label : 使用指定的标签 (例 foo) 替换自动的标签\n"
      "用于声明图形的参数可以为:\n"
      "  graph.<图形名称>.series : 用分号隔开的 <序列名称> (例 serie1;serie2)\n"
      "  graph.<图形名称>.title : 使用指定的标题 (例 my graph) 替换自动的标题\n"
      "  graph.<图形名称>.key : 显示主键的模式\n"
      "  graph.<图形名称>.ylabel : 图形y坐标标签\n"
      "全局参数有:\n"
      "  terminal : 用于替换默认wxt模式的终端模式 (例 pdfcairo size 11.7,8.3)\n"
      "  output : 输出文件名 (例 mygraph.pdf)\n"
      "  tryopengnuplot : 置1则在模拟结束后运行GNUplot\n"
      "  persistent : 置1则在模拟结束后也保持GNUplot运行\n"
      );

  DECLARE_VERSION(openfluid::config::FULL_VERSION);
  DECLARE_STATUS(openfluid::ware::STABLE);

END_OBSERVER_SIGNATURE


// =====================================================================
// =====================================================================

class SerieInfo
{
  public:

    enum SerieType {SERIE_UNKNOWN, SERIE_VAR, SERIE_FILE};

    SerieType Type;

    std::string SourceFile;

    std::ofstream* FileHandle;

    openfluid::core::VariableName_t VarName;

    openfluid::core::UnitsClass_t UnitsClass;

    openfluid::core::UnitID_t UnitID;

    openfluid::core::SpatialUnit* Unit;

    std::string Label;

    std::string Style;

    std::string Color;

    SerieInfo() : Type(SERIE_UNKNOWN),
        SourceFile(""), FileHandle(NULL), VarName(""), UnitsClass(""), UnitID(1), Unit(NULL),
        Label(""), Style("line"), Color("")
    { }

};


class GraphInfo
{
  public:

    std::string Title;

    std::string Key;

    std::string YLabel;

    std::list<SerieInfo*> Series;

    GraphInfo() : Title(""), Key("default"), YLabel("")
    { }

};


// =====================================================================
// =====================================================================


class GNUplotObserver : public openfluid::ware::PluggableObserver
{
  private:

    std::map<std::string, SerieInfo> m_Series;

    std::map<std::string,GraphInfo> m_Graphs;

    std::string m_OutputDir;
    std::string m_InputDir;

    bool m_TryOpenGNUplot;

    bool m_Persistent;

    std::string m_Terminal;
    std::string m_Output;


  public:

    GNUplotObserver() : PluggableObserver(), m_OutputDir(""),
      m_TryOpenGNUplot(false), m_Persistent(false), m_Terminal(""), m_Output("")
    {

    }

    // =====================================================================
    // =====================================================================


    ~GNUplotObserver()
    {
      CloseFiles();
    }


    // =====================================================================
    // =====================================================================


    void initParams(const openfluid::ware::WareParams_t& Params)
    {
      openfluid::ware::WareParamsTree ParamsTree;

      try
      {
        ParamsTree.setParams(Params);

        ParamsTree.root().getChildValue("tryopengnuplot",m_TryOpenGNUplot).toBoolean(m_TryOpenGNUplot);
        ParamsTree.root().getChildValue("persistent",m_Persistent).toBoolean(m_Persistent);
        m_Terminal = ParamsTree.root().getChildValue("terminal",m_Terminal);
        m_Output = ParamsTree.root().getChildValue("output",m_Output);

      }
      catch (openfluid::base::FrameworkException& E)
      {
        OPENFLUID_RaiseError(E.getMessage());
      }

      if (!ParamsTree.root().hasChild("serie"))
        OPENFLUID_RaiseError("No serie defined");

      if (!ParamsTree.root().hasChild("graph"))
        OPENFLUID_RaiseError("No graph defined");


      for (auto& Serie : ParamsTree.root().child("serie"))
      {
        std::string SerieID = Serie.first;

        SerieInfo SInfo;
        SInfo.VarName = Serie.second.getChildValue("var","");
        SInfo.UnitsClass = Serie.second.getChildValue("unitsclass","");
        if (SInfo.UnitsClass == "")
        {
          // search for deprecated "unitclass" parameter
          SInfo.UnitsClass = Serie.second.getChildValue("unitclass","");
        }
        std::string UnitIDStr = Serie.second.getChildValue("unitID","");
        SInfo.SourceFile = Serie.second.getChildValue("sourcefile","");
        SInfo.Label = Serie.second.getChildValue("label","");
        SInfo.Style = Serie.second.getChildValue("style","line");
        SInfo.Color = Serie.second.getChildValue("color","");

        if (!SInfo.VarName.empty() &&
            !SInfo.UnitsClass.empty() &&
            openfluid::tools::convertString(UnitIDStr,&SInfo.UnitID))
        {
          openfluid::core::SpatialUnit* TmpU;
          TmpU = mp_SpatialData->spatialUnit(SInfo.UnitsClass,SInfo.UnitID);
          if (TmpU != NULL)
          {
            SInfo.Type = SerieInfo::SERIE_VAR;
            SInfo.Unit = TmpU;
            SInfo.SourceFile = SerieID + "_data.gnuplot";
          }
        }
        else if (!SInfo.SourceFile.empty())
          SInfo.Type = SerieInfo::SERIE_FILE;

        if (SInfo.Type!=SerieInfo::SERIE_UNKNOWN) m_Series[SerieID] = SInfo;
        else OPENFLUID_LogWarning("Serie " + SerieID + "ignored");

      }


      for (auto& Graph : ParamsTree.root().child("graph"))
      {
        std::string GraphID = Graph.first;

         GraphInfo GInfo;

         GInfo.Title = Graph.second.getChildValue("title",GraphID);
         GInfo.Key = Graph.second.getChildValue("key","default");
         GInfo.YLabel = Graph.second.getChildValue("ylabel","");

         std::vector<std::string> SeriesStr =
             openfluid::tools::splitString(Graph.second.getChildValue("series",""),
                                           ";",false);

         for (std::vector<std::string>::const_iterator it = SeriesStr.begin();it != SeriesStr.end();++it)
         {
           if (m_Series.find(*it) != m_Series.end())
             GInfo.Series.push_back(&(m_Series[*it]));
         }

         if (!GInfo.Series.empty())
           m_Graphs[GraphID] = GInfo;
         else
           OPENFLUID_LogWarning("Graph " + GraphID + "ignored");
      }
    }


    // =====================================================================
    // =====================================================================


    void WriteDataToFiles()
    {
      std::map<std::string, SerieInfo>::iterator Sit;
      std::map<std::string, SerieInfo>::iterator Sitb = m_Series.begin();
      std::map<std::string, SerieInfo>::iterator Site = m_Series.end();

      for (Sit = Sitb; Sit != Site; ++Sit)
      {
        if ((*Sit).second.Type == SerieInfo::SERIE_VAR)
        {
          openfluid::core::Value* Val =
              (*Sit).second.Unit->variables()->currentValueIfIndex((*Sit).second.VarName,
                                                                   OPENFLUID_GetCurrentTimeIndex());

          if (Val!= NULL)
          {
            *((*Sit).second.FileHandle) << OPENFLUID_GetCurrentDate().getAsString("%Y-%m-%dT%H:%M:%S");
            *((*Sit).second.FileHandle) << " ";
            if (Val->isSimple() && !Val->isStringValue())
              Val->writeToStream(*((*Sit).second.FileHandle));
            else
              *((*Sit).second.FileHandle) << "NaN";
            *((*Sit).second.FileHandle) << "\n";
          }
        }
      }
    }

    // =====================================================================
    // =====================================================================


    void CloseFiles()
    {
      std::map<std::string, SerieInfo>::iterator Sit;
      std::map<std::string, SerieInfo>::iterator Sitb = m_Series.begin();
      std::map<std::string, SerieInfo>::iterator Site = m_Series.end();

      for (Sit = Sitb; Sit != Site; ++Sit)
      {
        if ((*Sit).second.Type == SerieInfo::SERIE_VAR)
        {
          if ((*Sit).second.FileHandle!=NULL && (*Sit).second.FileHandle->is_open())
            (*Sit).second.FileHandle->close();
        }
      }

    }


    // =====================================================================
    // =====================================================================


    void onPrepared()
    {
      OPENFLUID_GetRunEnvironment("dir.output",m_OutputDir);
      OPENFLUID_GetRunEnvironment("dir.input",m_InputDir);

      std::map<std::string, SerieInfo>::iterator Sit;
      std::map<std::string, SerieInfo>::iterator Sitb = m_Series.begin();
      std::map<std::string, SerieInfo>::iterator Site = m_Series.end();

      for (Sit = Sitb; Sit != Site; ++Sit)
      {
        if ((*Sit).second.Type == SerieInfo::SERIE_VAR)
        {
          (*Sit).second.FileHandle = new std::ofstream(std::string(m_OutputDir+"/"+(*Sit).second.SourceFile).c_str());
        }
      }

    }


    // =====================================================================
    // =====================================================================


    void onInitializedRun()
    {
      WriteDataToFiles();
    }


    // =====================================================================
    // =====================================================================


    void onStepCompleted()
    {
      WriteDataToFiles();
    }


    // =====================================================================
    // =====================================================================


    void tryOpenGNUplot()
    {
      if (m_TryOpenGNUplot)
      {
        openfluid::utils::ExternalProgram GNUPlotProgram =
            openfluid::utils::ExternalProgram::getRegisteredProgram(openfluid::utils::ExternalProgram::GnuplotProgram);

        if (GNUPlotProgram.isFound())
        {
          QString PersistOption = " ";
          if (m_Persistent) PersistOption = " -persist ";

          QString GNUPlotCommand = QString("\"%1\"%2\"%3\"").arg(GNUPlotProgram.getFullProgramPath())
                                                    .arg(PersistOption)
                                                    .arg(QDir(QString::fromStdString(m_OutputDir))
                                                         .absoluteFilePath("script.gnuplot"));

          QProcess::execute(GNUPlotCommand);
        }
        else
        {
          OPENFLUID_LogWarning("Cannot find GNUPlot");
        }
      }
    }


    // =====================================================================
    // =====================================================================


    void onFinalizedRun()
    {
      CloseFiles();

      std::ofstream ScriptFile(std::string(m_OutputDir+"/script.gnuplot").c_str());

      if (!m_Terminal.empty())
      {
        ScriptFile << "set terminal " << m_Terminal << "\n";
        if (!m_Output.empty())
          ScriptFile << "set output \"" << m_OutputDir << "/" << m_Output << "\"\n";
      }

      ScriptFile << "set xtics rotate font \",5\"\n";
      ScriptFile << "set ytics font \",7\"\n";
      ScriptFile << "set xdata time\n";
      ScriptFile << "set timefmt \"%Y-%m-%dT%H:%M:%S\"\n";
      ScriptFile << "set datafile separator \" \"\n";
      ScriptFile << "set datafile commentschars \"#\"\n";
      ScriptFile << "set format x \"%Y-%m-%d\\n%H:%M:%S\"\n";
      ScriptFile << "set datafile missing \"NaN\"\n";
      ScriptFile << "set xlabel \"Time\"\n";

      unsigned int Columns = 1;
      unsigned int Rows = 1;

      if (m_Graphs.size() > 1)
      {
        Columns = (unsigned int)(std::ceil(std::sqrt(m_Graphs.size())));
        Rows = (unsigned int)(std::ceil(m_Graphs.size() / Columns));

        if (Columns*Rows < m_Graphs.size()) Rows++;
      }

      ScriptFile << "set multiplot layout " << Rows << "," << Columns << " rowsfirst scale 1,1\n";

      std::map<std::string,GraphInfo>::iterator Git;
      std::map<std::string,GraphInfo>::iterator Gitb = m_Graphs.begin();
      std::map<std::string,GraphInfo>::iterator Gite = m_Graphs.end();


      for (Git = Gitb; Git != Gite; ++Git)
      {

        ScriptFile << "set title \"" << (*Git).second.Title << "\" font \",10\"\n";
        ScriptFile << "set key " << (*Git).second.Key << "\n";

        if ((*Git).second.YLabel.empty())
          ScriptFile << "unset ylabel\n";
        else
          ScriptFile << "set ylabel \"" << (*Git).second.YLabel << "\"\n";

        ScriptFile << "plot ";

        std::list<SerieInfo*>::iterator Sit;
        std::list<SerieInfo*>::iterator Sitb = (*Git).second.Series.begin();
        std::list<SerieInfo*>::iterator Site = (*Git).second.Series.end();

        for (Sit = Sitb; Sit != Site; ++Sit)
        {
          std::string SourceDir = m_OutputDir;
          if ((*Sit)->Type == SerieInfo::SERIE_FILE) SourceDir = m_InputDir;

          std::string Label = (*Sit)->Label;
          if (Label.empty())
          {
            if ((*Sit)->Type == SerieInfo::SERIE_FILE) Label = (*Sit)->SourceFile;
            else
            {
              std::string UnitIDStr;
              openfluid::tools::convertValue((*Sit)->UnitID,&UnitIDStr);
              Label= (*Sit)->VarName + " (" + (*Sit)->UnitsClass + "#" + UnitIDStr + ")";
            }
          }


          if (Sit != Sitb) ScriptFile << ", ";

          ScriptFile << "\"" << SourceDir << "/" << (*Sit)->SourceFile
                     << "\" using 1:2 with " << (*Sit)->Style << " title \"" << Label << "\"";
        }

        ScriptFile << "\n";

      }

      ScriptFile << "unset multiplot\n";

      ScriptFile.close();

      tryOpenGNUplot();
    }


};


// =====================================================================
// =====================================================================


DEFINE_OBSERVER_CLASS(GNUplotObserver)

