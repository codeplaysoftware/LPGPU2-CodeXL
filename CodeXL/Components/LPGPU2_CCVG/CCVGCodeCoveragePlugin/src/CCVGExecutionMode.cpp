// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGExecutionMode implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGAppWrapper.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGExecutionMode.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesIcon.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGProjectBuildDefinitions.h>

#if LPGPU2_CCVG_COMPILE_IN

/// @brief  Class constructor. No work is done here. See Initialise().
CCVGExecutionMode::CCVGExecutionMode()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance
///         see Shutdown().
CCVGExecutionMode::~CCVGExecutionMode()
{
  Shutdown();
}

/// @brief  Package initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGExecutionMode::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  // Do nothing here

  m_bBeenInitialised = true;
  return success;
}

/// @brief  Package shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGExecutionMode::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  m_bBeenShutdown = true;
  return success;
}

/// @brief  Log error message.
/// @param[in] wchar_t* General error description text. 
/// @param[in] wchar_t* Context error description text. 
void CCVGExecutionMode::LogError(const wchar_t *vpErrMsg, const wchar_t *vpErrCntxt)
{
  const wchar_t *subj(CCVGRSRC(kIDS_CCVG_STR_AppNameShort));
  gtString msg(gtString().appendFormattedString(vpErrMsg, subj));
  msg.appendFormattedString(L"%ls. %ls", msg.asCharArray(), vpErrCntxt);
  OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_ERROR);
}

/// @brief  Execution mode for identification. Get the mode's name including
///         the word 'mode'. Used through out the CodeXL's framework to do
///         determine which execution mode it is in. 
/// @return gtString wide char text.
gtString CCVGExecutionMode::modeName()
{
  const wchar_t *pTxt(CCVGRSRC(kIDS_CCVG_STR_execmode_name));
  return pTxt; 
}

/// @brief  Get the mode's name including the word 'mode'. This text is
///         for display in CodeXL's GUI.
/// @return gtString wide char text.
gtString CCVGExecutionMode::modeNameDisplayInGui()
{
  // Also see function CCVGGUIAppTreeHandler::GetCCVGExecModeEnabled()for 
  // similar functionality
  gtString txt;
  ccvg::fw::CCVGFWTheCurrentState &rCurrState(ccvg::fw::CCVGFWTheCurrentState::Instance());
  const ccvg::fw::CCVGFWTheCurrentState::Settings &currSetting(rCurrState.GetSettingsWorking());
  ccvg::fw::CCVGFWTheCurrentStateProjectData &rProjData(rCurrState.GetProjectData());
  const ccvg::fw::CCVGFWTheCurrentStateProjectSettings &rProjSettings = rCurrState.GetProjectSetting();
  const ccvg::fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSetting(rProjSettings.GetSettingsWorking());
  const bool bAppWideCcvgEnabled = currSetting.GetCcvgAllProjectsEnabledFlag();
  const bool bCurrentProjCcvgEnabled = rSetting.GetCcvgProjectEnabledFlag();
  const bool bCCVGExecEnabled = bAppWideCcvgEnabled && bCurrentProjCcvgEnabled;
  const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_execmode_name_gui_ccvgDisabled);
  if (bCCVGExecEnabled)
  {
    txt.appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_execmode_name_gui_ccvgEnabled), rProjData.ItemsCount());
    pTxt = txt.asCharArray();
  }
                                       
  return pTxt;
}

/// @brief  Execution status relevance.
/// @return bool true = relevant, false = not relevant.
bool CCVGExecutionMode::IsExecutionStatusRelevant()
{ 
  return false; 
}

/// @brief  Execution mode for identification.
/// @return gtString wide char text.
gtString CCVGExecutionMode::modeActionString()
{
  const wchar_t *pTxt(CCVGRSRC(kIDS_CCVG_STR_execmode_action));
  return pTxt;
}

/// @brief  The action verb the mode encompasses (e.g. "debug", "profile", 
///         "analyze", etc.).
/// @return gtString wide char text.
gtString CCVGExecutionMode::modeVerbString()
{
  const wchar_t *pTxt(CCVGRSRC(kIDS_CCVG_STR_execmode_verb));
  return pTxt;
}

/// @brief  Mode description for tooltips.
/// @return gtString wide char text.
gtString CCVGExecutionMode::modeDescription()
{
  const wchar_t *pTxt(CCVGRSRC(kIDS_CCVG_STR_execmode_description));
  return pTxt;
}

/// @brief     Execute a mode command. For a selection of execution
///            commands see aflExecutionMode.h.
/// @param[in] vCommandId Unique ID specific command.
void CCVGExecutionMode::execute(afExecutionCommandId vCommandId)
{
  // ToDo: investigate and implement functionality
  switch (vCommandId)
  {
      case AF_EXECUTION_ID_BUILD:
      case AF_EXECUTION_ID_CANCEL_BUILD:
      case AF_EXECUTION_ID_START:
      case AF_EXECUTION_ID_BREAK:
      case AF_EXECUTION_ID_STOP:
      case AF_EXECUTION_ID_API_STEP:
      case AF_EXECUTION_ID_DRAW_STEP:
      case AF_EXECUTION_ID_FRAME_STEP:
      case AF_EXECUTION_ID_STEP_IN:
      case AF_EXECUTION_ID_STEP_OVER:
      case AF_EXECUTION_ID_STEP_OUT:
      case AF_EXECUTION_ID_CAPTURE:
      case AF_EXECUTION_ID_CAPTURE_CPU:
      case AF_EXECUTION_ID_CAPTURE_GPU:
      {
          break;
      }
      default: break;
  }
}

/// @brief     Handle the command UI update. This includes enable disable, hide
///            show etc for the QAction object.
/// @param[in] vCommandId Unique ID specific command.
/// @param[in] vpAction Pointer to a Qt action object.
void CCVGExecutionMode::updateUI(afExecutionCommandId vCommandId, QAction *vpAction)
{
  // ToDo: investigate and implement functionality

  bool bIsActionEnabled = true;
  bool bIsActionVisible = true;

  GT_IF_WITH_ASSERT(nullptr != vpAction)
  {
      switch (vCommandId)
      {
          case AF_EXECUTION_ID_BUILD:
          case AF_EXECUTION_ID_CANCEL_BUILD:
          case AF_EXECUTION_ID_START:
          case AF_EXECUTION_ID_BREAK:
          case AF_EXECUTION_ID_STOP:
          case AF_EXECUTION_ID_API_STEP:
          case AF_EXECUTION_ID_DRAW_STEP:
          case AF_EXECUTION_ID_FRAME_STEP:
          case AF_EXECUTION_ID_STEP_IN:
          case AF_EXECUTION_ID_STEP_OVER:
          case AF_EXECUTION_ID_STEP_OUT:
          case AF_EXECUTION_ID_CAPTURE:
          case AF_EXECUTION_ID_CAPTURE_CPU:
          case AF_EXECUTION_ID_CAPTURE_GPU:
          {
              bIsActionVisible = false;
              bIsActionEnabled = false;
              break;
          }

          default: break;
      }

      vpAction->setEnabled(bIsActionEnabled);
      vpAction->setVisible(bIsActionVisible);

      // Use the start action different function:
      if (vCommandId == AF_EXECUTION_ID_START)
      {
        afExecutionModeManager::instance().UpdateStartActionVisibility(bIsActionVisible, bIsActionEnabled);
      }
  }
}

/// @brief  Get the number of execution mode's session types.
/// @return int 0 or greater.
int CCVGExecutionMode::numberSessionTypes()
{ 
  return 1; 
}

/// @brief     Get the session type at a particular index. CCVG does not
///            implement the notion of sessions.
/// @param[in] vSessionTypeIndex Only 0.
/// @return    gtString wide char text.
gtString CCVGExecutionMode::sessionTypeName(int vSessionTypeIndex)
{
    gtString sessionName;

    GT_IF_WITH_ASSERT((vSessionTypeIndex >= 0) && (vSessionTypeIndex < 1))
    {
      // If session name not empty then appears in Application Tree View root
      // node's label appending in '()'.
      sessionName = L""; // CCVGRSRC(kIDS_CCVG_STR_execmode_session_type);
    }

    return sessionName;
}

/// @brief     Get the icon of each session type for a specific index.
/// @param[in] vSessionTypeIndex Only 0.
/// @return    QPixmap* Pointer to a new object, NULL == failure.
QPixmap* CCVGExecutionMode::sessionTypeIcon(int vSessionTypeIndex)
{
  QPixmap *pPixmap = nullptr;
  GT_IF_WITH_ASSERT((vSessionTypeIndex >= 0) && (vSessionTypeIndex < 1))
  {
    pPixmap = CCVGRSRCICON(kIDS_CCVG_ICON_CCVGExecutionMode);
  }

  return pPixmap;
}

/// @brief     Return the index for the requested session type name.
/// @param[in] vSessionType Session identifier wide char text.
/// @return    QPixmap* Pointer to a new object.
int CCVGExecutionMode::indexForSessionType(const gtString &vSessionType)
{ 
  GT_UNREFERENCED_PARAMETER(vSessionType);
  return 0; 
};

/// @brief     Return the layout to be used for this mode at specific time. Each
///            mode has a specific layout of widgets (windows) it uses. Note:
///            CodeXL authors state that at this time new modes for plugins
///            cannot be added so a layout of those available  must be used.
/// @return    afMainAppWindow::LayoutFormats Layout functions enumeration.
afMainAppWindow::LayoutFormats CCVGExecutionMode::layoutFormat()
{ 
  // ToDo: IOR: new app window layout 
  return afMainAppWindow::LayoutKernelAnalyzer; 
};

/// @brief     Return the CCVG project settings path used for this mode.
///            Project setting extension display name in the Project Settings
///            dialog. Identifies the settings for this mode in the XML. It 
///            must unique.
/// @return    gtString wide char text.
gtString CCVGExecutionMode::ProjectSettingsPath()
{ 
  return CCVGRSRC(kIDS_CCVG_STR_project_settings_extn_display_name);
};

/// @brief     Perform a startup action. For the possible startup actions see
///            file aflExecutionMode.h.
/// @return    bool True = supports requested action, false = does not.
bool CCVGExecutionMode::ExecuteStartupAction(afStartupAction vAction)
{
  bool retVal = false;
  
  if (vAction == AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_LPGPU2_CCVG)
  {
    afApplicationCommands::instance()->OnFileNewProject();
    retVal = true;
  }

  return retVal;
}

/// @brief Perform a startup action. Called from 
///        afWebPage::javaScriptConsoleMessage()
/// @return bool True = supports requested action, false = does not.
bool CCVGExecutionMode::IsStartupActionSupported(afStartupAction vAction)
{
  return (vAction == AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_LPGPU2_CCVG);
}

/// @brief     Is *this excution mode enabled and ready for use. 
/// @return    bool True = met, false = not met.
bool CCVGExecutionMode::isModeEnabled()
{
    return CCVGAppWrapper::GetIsLoadEnabled();
}

/// @brief     Get the properties view message to start the execution of the 
///            mode. 
/// @return    gtString wide char text.
gtString CCVGExecutionMode::HowToStartModeExecutionMessage()
{
  const wchar_t *pMsg = CCVGRSRC(kIDS_CCVG_STR_execmode_user_msg_stand_alone);

  if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
  {
    pMsg = CCVGRSRC(kIDS_CCVG_STR_execmode_user_msg_vstudio);
  }

  return pMsg;
}

/// @brief      Get the toolbar start button text. 
/// @param[out] vButtonText gtString wide char text.
/// @param[in]  vbFullString True = yes full text, false = summary text.
void CCVGExecutionMode::GetToolbarStartButtonText(gtString &vButtonText, bool vbFullString /*= true*/)
{
    GT_UNREFERENCED_PARAMETER(vbFullString);

    vButtonText = CCVGRSRC(kIDS_CCVG_STR_execmode_start_button);
}

/// @brief Enquire if the execution mode supports the requested action if at 
///        the time a project is not loaded.
/// bool True = Yes supported, false = no.
// bool IsStartupActionSupportedWithNoProject(afExecutionCommandId vCommandId)

/// @brief An execution mode can have more than one session type. For example
///        Profiling mode has session types for CPU profiling and more for GPU
///        profiling. Retrieve the current selected session type.
/// @return gtString Name of the session.
// gtString selectedSessionTypeName()

#endif // LPGPU2_CCVG_COMPILE_IN
