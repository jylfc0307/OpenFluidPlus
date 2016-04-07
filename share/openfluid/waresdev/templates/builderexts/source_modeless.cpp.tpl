/**
  @file %%ROOTCPPFILENAME%%

*/


#include "%%ROOTHPPFILENAME%%"


// =====================================================================
// =====================================================================


BEGIN_BUILDEREXT_SIGNATURE("%%WAREID%%", openfluid::builderext::MODE_MODELESS)

  DECLARE_CATEGORY(%%BUILDEREXTCATEGORY%%)
  DECLARE_MENUTEXT("%%BUILDEREXTMENUTEXT%%")

END_BUILDEREXT_SIGNATURE


// =====================================================================
// =====================================================================


%%CLASSNAME%%::%%CLASSNAME%%() :
  openfluid::builderext::PluggableModelessExtension()
{

}


// =====================================================================
// =====================================================================


%%CLASSNAME%%::~%%CLASSNAME%%()
{

}


// =====================================================================
// =====================================================================


bool %%CLASSNAME%%::initialize()
{
  return true;
}


// =====================================================================
// =====================================================================


void %%CLASSNAME%%::update(openfluid::builderext::FluidXUpdateFlags::Flags /*UpdateFlags*/)
{

}


// =====================================================================
// =====================================================================


void %%CLASSNAME%%::manageSimulationStart()
{

}


// =====================================================================
// =====================================================================


void %%CLASSNAME%%::manageSimulationFinish()
{

}


// =====================================================================
// =====================================================================


DEFINE_BUILDEREXT_CLASS(%%CLASSNAME%%)


