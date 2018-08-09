// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdProjDataSetNoDriverFilesReq implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdProjDataSetNoDriverFilesReq.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>

namespace ccvg {
namespace cmd {

/// @brief  Retrieve a new instance of *this class.
/// @return CCVGCmdBase* Pointer to new object, NULL on allocation failure.
CCVGCmdBase* CCVGCmdProjDataSetNoDriverFilesReq::CreateSelf()
{
  return new (std::nothrow) CCVGCmdProjDataSetNoDriverFilesReq();
}

/// @brief Class constructor. Initialise *this command's mandatory attributes.
CCVGCmdProjDataSetNoDriverFilesReq::CCVGCmdProjDataSetNoDriverFilesReq()
: m_paramFlagDriversRequired(kParamID_FlagDriversRequired, true, L"bool", L"Set Drivers required", L"Iterate CCVGItems setting drivers not required for set of file to be complete and ok")
, m_bPreviousSetDriversRequired(true)
{
  m_cmdID = CmdID::kProjDataSetNoDriverFilesReq;
  m_cmdName = L"CmdProjDataSetNoDriverFilesReq";
  m_pSelfCreatorFn = &CCVGCmdProjDataSetNoDriverFilesReq::CreateSelf;

  m_listCmdParameters.push_back(&m_paramFlagDriversRequired);
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdProjDataSetNoDriverFilesReq::~CCVGCmdProjDataSetNoDriverFilesReq()
{
  Shutdown();
}

/// @brief  Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdProjDataSetNoDriverFilesReq::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Class shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdProjDataSetNoDriverFilesReq::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  m_bBeenShutdown = true;

  return success;
}

/// @brief  Retrieve *this command's creation function.
/// @return CCVGCmdFactory::CmdCreatorFnPtr Function pointer.
CCVGCmdFactory::CmdCreatorFnPtr CCVGCmdProjDataSetNoDriverFilesReq::GetCmdCreatorFn() const
{
  return m_pSelfCreatorFn;
}

/// @brief  The Execute() is the command work function to carry out work. Part 
///         of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdProjDataSetNoDriverFilesReq::Execute()
{
  m_bFinishedTask = false;
  
  const fw::CCVGFWTheCurrentStateProjectSettings &rProjSettings = fw::CCVGFWTheCurrentState::Instance().GetProjectSetting();
  const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rCurrentSettings(rProjSettings.GetSettingsWorking());
  m_bPreviousSetDriversRequired = rCurrentSettings.GetCcvgDriverFilesMandatoryFlag();
  const bool bNewSetDriversFlag = m_paramFlagDriversRequired.param.Get<bool>();
  if (m_bPreviousSetDriversRequired == bNewSetDriversFlag)
  {
    m_bFinishedTask = true;
    return success;
  }
      
  fw::CCVGFWTheCurrentStateProjectData &rProjData = fw::CCVGFWTheCurrentState::Instance().GetProjectData();
  rProjData.ItemsSetCcvgdRequired(bNewSetDriversFlag);

  m_bFinishedTask = true;
  return success;
}

/// @brief  The opposite function to Execute(). Part of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdProjDataSetNoDriverFilesReq::Undo()
{
  m_bFinishedTask = false;
  
  if (!GetIsUndoableFlag())
  {
    m_bFinishedTask = true;
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_err_called_undo_when_not_undoable));
  }

  const fw::CCVGFWTheCurrentStateProjectSettings &rProjSettings = fw::CCVGFWTheCurrentState::Instance().GetProjectSetting();
  const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rCurrentSettings(rProjSettings.GetSettingsWorking());
  const bool bCurrSetDriversFlag = rCurrentSettings.GetCcvgDriverFilesMandatoryFlag();
  if (m_bPreviousSetDriversRequired == bCurrSetDriversFlag)
  {
    m_bFinishedTask = true;
    return success;
  }

  fw::CCVGFWTheCurrentStateProjectData &rProjData = fw::CCVGFWTheCurrentState::Instance().GetProjectData();
  rProjData.ItemsSetCcvgdRequired(m_bPreviousSetDriversRequired);
  
  m_bFinishedTask = true;
  return success;
}

/// @brief  Ask the command if it should be undoable, it is able to reverse
///         the task carried out in its Execute() function. It may be the has
///         indicate is cannot undo a previous action because it for example
///         performing a GUI operation i.e. open a dialog.
/// @return bool True = can undo, false = cannot undo.
bool CCVGCmdProjDataSetNoDriverFilesReq::GetIsUndoableFlag() const
{
  return true;
}

} // namespace ccvg 
} // namespace cmd 