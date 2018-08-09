// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGMenuActionsExecutor implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MenuActionsExecutor.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_CommandIDs.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Params.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdProjectSettingsDlgOpen.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadAll.h>

// Declarations:
namespace ccvg { namespace cmd { extern status ExecuteCmdFilesReadAll(); } }

namespace ccvg {
namespace fw {

/// @brief    Class constructor. No work is done here by this class.
CCVGMenuActionsExecutor::CCVGMenuActionsExecutor()
: m_rCmdInvoker(ccvg::cmd::CCVGCmdInvoker::Instance())
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGMenuActionsExecutor::~CCVGMenuActionsExecutor()
{
}

/// @brief      Handle the action when it is triggered. Fire of a command 
///             perhaps. Called by:
///             appMainAppWindow::triggerGlobalAction()
///             appMainAppWindow::triggerViewAction().
/// @param[in]  vActionIndex Index of the vector of supported command ids by 
///             *this executor.
void CCVGMenuActionsExecutor::handleTrigger(int vActionIndex)
{
  const int commandId = actionIndexToCommandId(vActionIndex);
  switch (commandId)
  {
    case kMainMenuItemCommandID_CCVGMode:
    {
      // First check if the CCVG part of the current project is enabled for the 
      // current loaded project. If not get the user to confirm current settings
      // suitable for thic current project.
      const CCVGFWTheCurrentStateProjectSettings &rProjSettings = CCVGFWTheCurrentState::Instance().GetProjectSetting();
      const bool bSameUserOrPC = rProjSettings.IsUserAndPCTheSameAsPreviousUseOfTheCodeXLProject();
      const bool bProjEnabled = rProjSettings.GetSettingsWorking().GetCcvgProjectEnabledFlag();
      const bool bEnabled = bProjEnabled && bSameUserOrPC;
      if (bEnabled)
      {
        ExecuteCmdSwitchToCCVGExecutionMode();
      }
      else
      {
        ExecuteCmdOpenProjectSettingsDlg();
      }
    }
    break;
    case kMainMenuItemCommandID_ProjectSettings:
    {
      ExecuteCmdOpenProjectSettingsDlg();
    }
    break;
    case kMainMenuItemCommandID_RefreshCCVGResultsFiles:
    {
      ExecuteCmdFilesReadAll();
    }
    break;
    default:
      GT_ASSERT_EX(false, CCVGRSRC(kIDS_CCVG_STR_fw_action_err_unknown_ID));
      break;
  }
}

/// @brief Setup and execute command to scan the code coverage source 
///        directories and read in files.
void CCVGMenuActionsExecutor::ExecuteCmdSwitchToCCVGExecutionMode()
{
  fnstatus::status status = m_rCmdInvoker.CmdInit(cmd::CmdID::kSwitchCCVGMode);
  const gtString &rCmdName(m_rCmdInvoker.CmdGetNameAboutToExecute());
  if (status == fnstatus::success)
  {
    status = m_rCmdInvoker.CmdExecute();
  }
  m_rCmdInvoker.CmdHandleError(status, false, rCmdName);
}

/// @brief Setup and execute command to scan the code coverage source 
///        directories and read in files.
void CCVGMenuActionsExecutor::ExecuteCmdFilesReadAll()
{
  cmd::ExecuteCmdFilesReadAll();
}

/// @brief Setup and execute command open project settings dialog.
void CCVGMenuActionsExecutor::ExecuteCmdOpenProjectSettingsDlg()
{
  using namespace ccvg::cmd;
  const CCVGuint paramIDUseDlg(CCVGCmdProjectSettingDlgOpen::kParamID_UseDlgFlag);
  CCVGCmdParams *pCmdParams = nullptr;
  fnstatus::status status = m_rCmdInvoker.CmdGetParams(CmdID::kProjectSettingDlgOpen, pCmdParams);
  const gtString &rCmdName(m_rCmdInvoker.CmdGetNameAboutToExecute());
  bool bCaughtException = false;
  try
  {
    CCVGCmdParameter *pParamUseDlg = pCmdParams->GetParam(paramIDUseDlg);
    bool bOk = pParamUseDlg->param.Set<bool>(true);
    bOk = bOk && (m_rCmdInvoker.CmdExecute() == success);
    status = bOk;
  }
  catch (...)
  {
    bCaughtException = true;
    status = failure;
  }
  m_rCmdInvoker.CmdHandleError(status, bCaughtException, rCmdName);
}

/// @brief     Handle UI updates of the command or menu action items. Change 
///            handling or look of the QAction. To get the associated QAction
///            use function QAction* action(vActionIndex).
///            Called by:
///               appMainAppWindow::updateViewAction()
///               appMainAppWindow::updateGlobalAction()
/// @param[in] vActionIndex Index of the vector of supported command ids by 
///            *this executor.
void CCVGMenuActionsExecutor::handleUiUpdate(int vActionIndex)
{
  bool bIsActionEnabled = false;
  bool bIsActionChecked = false;
  bool bIsActionCheckable = false;
  QString updatedActionText;

  const int commandId = actionIndexToCommandId(vActionIndex);
  switch (commandId)
  {
  case kMainMenuItemCommandID_CCVGMode:
  {
    // Enable if the active mode
    CCVGFWTheCurrentState &rCurrState(CCVGFWTheCurrentState::Instance());
    const bool bIsModeActive = rCurrState.IsInCCVGExecutionMode();
    const bool bIsBusy = rCurrState.IsCCVGBusy();
    const bool bIsReady = rCurrState.IsCCVGReadyToWork();
    const bool bEnableAcrossAllProjs = rCurrState.GetSettingsWorking().GetCcvgAllProjectsEnabledFlag();
    bIsActionEnabled = !bIsModeActive && bIsReady && !bIsBusy && bEnableAcrossAllProjs;
    bIsActionChecked = bIsModeActive;
    bIsActionCheckable = true;
    updatedActionText = bIsModeActive ? acGTStringToQString(CCVGRSRC(kIDS_CCVG_STR_execmode_name_gui)) :
                                        acGTStringToQString(CCVGRSRC(kIDS_CCVG_STR_execmode_fw_switchToAnalyzeMode));
  }
  break;
  case kMainMenuItemCommandID_ProjectSettings:
  {
    CCVGFWTheCurrentState &rCurrState(CCVGFWTheCurrentState::Instance());
    const bool bIsModeActive = rCurrState.IsInCCVGExecutionMode();
    const bool bEnableAcrossAllProjs = rCurrState.GetSettingsWorking().GetCcvgAllProjectsEnabledFlag();
    bIsActionEnabled = bEnableAcrossAllProjs && bIsModeActive;
  }
  break;
  case kMainMenuItemCommandID_RefreshCCVGResultsFiles:
  {
    CCVGFWTheCurrentState &rCurrState(CCVGFWTheCurrentState::Instance());
    const bool bIsCCVGMode = rCurrState.IsInCCVGExecutionMode();
    const bool bIsBusy = rCurrState.IsCCVGBusy();
    const bool bIsReady = rCurrState.IsCCVGReadyToWork();
    const bool bEnableAcrossAllProjs = rCurrState.GetSettingsWorking().GetCcvgAllProjectsEnabledFlag();
    const bool bEnableThisCurrProj = rCurrState.GetProjectSetting().GetSettingsWorking().GetCcvgProjectEnabledFlag();
    const bool bProjectOpen = rCurrState.IsProjectOpen();
    bIsActionEnabled = bIsCCVGMode && bIsReady && !bIsBusy && bEnableThisCurrProj && bEnableAcrossAllProjs && bProjectOpen;

    const CCVGFWTheCurrentStateProjectData &rProjData = rCurrState.GetProjectData();
    const bool bLoadedAlreadyCCVGFiles = !rProjData.ItemsEmpty();
    const wchar_t *pTxt = bLoadedAlreadyCCVGFiles ? CCVGRSRC(kIDS_CCVG_STR_project_refreshResultsFiles) : CCVGRSRC(kIDS_CCVG_STR_project_loadResultsFiles);
    updatedActionText = acGTStringToQString(pTxt);
  }
  break;
  default:
    GT_ASSERT_EX(false, CCVGRSRC(kIDS_CCVG_STR_fw_action_err_unknown_ID));
    break;
  }

  // Get the QT action:
  QAction *pAction = action(vActionIndex);
  GT_IF_WITH_ASSERT(pAction != nullptr)
  {
    // Set the action enable / disable:
    pAction->setEnabled(bIsActionEnabled);

    // Set the action checkable state:
    pAction->setCheckable(bIsActionCheckable);

    // Set the action check state:
    pAction->setChecked(bIsActionChecked);

    // Update the text if needed:
    if (!updatedActionText.isEmpty())
    {
      pAction->setText(updatedActionText);
    }
  }

}

/// @brief:     Build the menu. Menu position. Where to insert a menu action
///             item to build up the various application menus.
///             appMainAppWindow::setNewActionProperties() calls this function 
///             to build the application's main menus.
///    
/// The string is composed of menu levels separated by '/'. For example if a 
/// command. For example if a command 'start' in 'Debug' menu then the return
/// value should be text "Debug". However if the command is in a sub-menu
/// "Actions" under "Debug" then return would be "Debug/Actions".
/// Each hierarchy on the, emu include name/priority.
/// If separator is needed after the item then 's' after the priority is needed
/// in case of a sub menu if one item is marked with an 's' it is enough to 
/// mark a separator after it. Called by functions:
///    setNewActionProperties()
///    createSingleViewAction()
///    createSingleAction()
///
/// @param[in]   vActionIndex Index of the vector of supported command ids by 
///              *this executor.
/// @param[out]  vrPositionData Defines additional control of item position 
///              within its parent menu: afCommandPosition, start/middle/end 
///              block in the menu. afCommandSeperatorType if the command 
///              requires a separator. Should the command come before another
///              action, use additional parameters; m_beforeActionMenuPosition
///              the menu position of the command, m_beforeActionText the 
///              command text.
/// @return      gtString The user facing menu item's text describing menu action.
gtString CCVGMenuActionsExecutor::menuPosition(int vActionIndex, afActionPositionData &vrPositionData)
{
  gtString retVal;

  vrPositionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;
  const int commandId = actionIndexToCommandId(vActionIndex);
  switch (commandId)
  {
    case kMainMenuItemCommandID_CCVGMode:
    case kMainMenuItemCommandID_ProjectSettings:
    case kMainMenuItemCommandID_RefreshCCVGResultsFiles:
    {
      retVal = AF_STR_CodeCoverageString;
      vrPositionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
    }
    break;
    default:
      GT_ASSERT(false);
      break;
  };

  return retVal;
}
 
/// @brief Get the name of the toolbar name that the action ID belongs to. If
///        no string is supplied (empty string) then the command will not be 
///        added to the toolbar. Create toolbar item and its position. 
///        Called by functions:
///           setNewActionProperties()
///           createSingleViewAction()
///           createSingleAction()
/// @param[in]   vActionIndex Index of the vector of supported command ids by 
///              *this executor.
/// return gtString Toolbar ID.
gtString CCVGMenuActionsExecutor::toolbarPosition(int vActionIndex)
{
  GT_UNREFERENCED_PARAMETER(vActionIndex);
  // Not implemented
  return L"";
}

/// @brief If a toolbar separator needs to be added before or after the toolbar
///        command then overwrite this function.
/// @param[in]  vActionIndex Index of the vector of supported command ids by 
///             *this executor.
/// @return int -1 = set before, 1 = separator after.
// int separatorPosition(int vActionIndex)

/// @brief      If the requested action should be a part of a group then 
///             override this function. If the requested action should be a 
///             part of a group, create the action group and add the relevant 
///             actions to the group.Grouped actions are actions that only
///             one can be active at a time. Create the action group and add
///             the relevant actions to the group. Get the group command ids 
///             for the action. Called by:
///               appMainAppWindow::setNewActionProperties().
/// @param[in]  vActionIndex Index of the vector of supported command ids by 
///             *this executor.
void CCVGMenuActionsExecutor::groupAction(int vActionIndex)
{
  GT_UNREFERENCED_PARAMETER(vActionIndex);
  // Not implemented
}

/// @brief  Create a vector of command Ids that are supported by *this executor
///         actions. Each derived class must populate the vector of supported 
///         command IDs. Populate vector m_supportedCommandIds. The index into
///         the container is the action index. Action index is not the same as
///         the command ID. Use actionIndexToCommandId() for converstion. 
///         Called by
///           afActionCreatorAbstract::initializeCreator().
void CCVGMenuActionsExecutor::populateSupportedCommandIds()
{
  m_supportedCommandIds.push_back(kMainMenuItemCommandID_CCVGMode);
  m_supportedCommandIds.push_back(kMainMenuItemCommandID_ProjectSettings);
  m_supportedCommandIds.push_back(kMainMenuItemCommandID_RefreshCCVGResultsFiles);
}

/// @brief  Specify which if any actions (application commands) have icons. 
///         Create the icons for the application commands.
///         Called by afActionCreatorAbstract::initializeCreator().
void CCVGMenuActionsExecutor::initActionIcons()
{
  // No command icons
  // Not implemented
}

/// @brief      Get the caption, tooltip and accelerator for a specified action 
///             item. The common edit actions 
///             that are shared by most action creators are covered by this 
///             function. A derived class should over-ride this function if it
///             implements actions that do not appear in this list. Called by
///             functions:
///               setNewActionProperties()
///               createSingleViewAction()
///               createSingleAction()
/// @param[in]  vActionIndex Index of the vector of supported command ids by 
///             *this executor.
/// param[out]  vrActionText Action's user facing text description of the 
///             option. The caption
/// param[out]  vrTooltip Action's tooltip assignment.
/// param[out]  vrKeyboardShortcut Action short cut.
/// return bool True = success, false = failure.
bool CCVGMenuActionsExecutor::actionText(int vActionIndex, gtString &vrActionText, gtString &vrTooltip, gtString &vrKeyboardShortcut)
{
  GT_UNREFERENCED_PARAMETER(vrKeyboardShortcut);

  bool retVal = true;

  const int commandId = actionIndexToCommandId(vActionIndex);
  switch (commandId)
  {

  case kMainMenuItemCommandID_CCVGMode:
    vrActionText = CCVGRSRC(kIDS_CCVG_STR_execmode_menu);
    vrTooltip = CCVGRSRC(kIDS_CCVG_STR_execmode_fw_statusBarString);
    break;
  case kMainMenuItemCommandID_ProjectSettings:
    vrActionText = CCVGRSRC(kIDS_CCVG_STR_project_settings_menu); 
    vrTooltip = CCVGRSRC(kIDS_CCVG_STR_project_settings_fw_statusBarString);
    break;
  case kMainMenuItemCommandID_RefreshCCVGResultsFiles:
    {
      const CCVGFWTheCurrentStateProjectData &rProjData = CCVGFWTheCurrentState::Instance().GetProjectData();
      const bool bLoadedAlreadyCCVGFiles = !rProjData.ItemsEmpty();
      const wchar_t *pTxt = bLoadedAlreadyCCVGFiles ? CCVGRSRC(kIDS_CCVG_STR_project_refreshResultsFiles) : CCVGRSRC(kIDS_CCVG_STR_project_loadResultsFiles);
      const wchar_t *pTip = bLoadedAlreadyCCVGFiles ? CCVGRSRC(kIDS_CCVG_STR_project_fw_statusBarString_refreshResultsFiles) : CCVGRSRC(kIDS_CCVG_STR_project_fw_statusBarString_loadResultsFiles);
      vrActionText = pTxt;
      vrTooltip = pTip;
    }
    break;
  default:
    GT_ASSERT(false);
    retVal = false;
    break;
  };

  return retVal;
}

} // namespace fw
} // namespace ccvg
