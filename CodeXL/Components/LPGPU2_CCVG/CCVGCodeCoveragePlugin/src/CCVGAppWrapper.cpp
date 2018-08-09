// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGAppWrapper implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Qt library:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Application Framework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
//#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGDLLBuild.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGAppWrapper.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGExecutionMode.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesIcon.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Invoker.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_ToolbarMainWnd.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MenuActionsExecutor.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewCreator.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_EventObserver.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_TabPageSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_ProjectSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_AppTreeHandler.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewMgr.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGProjectBuildDefinitions.h>
#if LPGPU2_CCVG_COMPILE_IN

// Instantiations:
bool                                CCVGAppWrapper::m_sbLoadEnabled = true;
CCVGAppWrapper*                     CCVGAppWrapper::m_spMySingleInstance = nullptr;
CCVGExecutionMode*                  CCVGAppWrapper::m_spCCVGExecutionMode = nullptr;
ccvg::gui::CCVGToolBarMainWnd*      CCVGAppWrapper::m_psToolbarMainWnd = nullptr;
ccvg::fw::CCVGMenuActionsExecutor*  CCVGAppWrapper::m_psMenuActionsExecutor = nullptr;
ccvg::fw::CCVGMDIViewCreator*       CCVGAppWrapper::m_psMDIViewCreator = nullptr;
ccvg::gui::CCVGGUITabPageSettings*  CCVGAppWrapper::m_psTabPageSettings = nullptr;
ccvg::gui::CCVGGUIProjectSettings*  CCVGAppWrapper::m_psProjectSettings = nullptr;
ccvg::fw::CCVGFWEventObserver*      CCVGAppWrapper::m_psAppEventObserver = nullptr;

/// @brief  Class constructor. No work is done here.
CCVGAppWrapper::CCVGAppWrapper()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGAppWrapper::~CCVGAppWrapper()
{
  // Tide up resources
  const ccvg::fnstatus::status status = Shutdown();
  GT_UNREFERENCED_PARAMETER(status);
}

/// @brief  Create the singleton instance of *this class.
/// @return CCVGAppWrapper& Reference to *this.
CCVGAppWrapper& CCVGAppWrapper::Instance()
{
  static CCVGAppWrapper instance;
  return instance;
}

/// @brief       Check the validity of the plug in. Check it has the resources
///              available to operate, the prerequisities are met. Called 
///              before CCVGAppWrapper::Initialize() it checks for system or 
///              external resources required by this plugin to operate.
/// @param[out]  vErrString Error description text. Wide char text.
/// @return      int 0 == all ok success, !0 == an error condition, check 
///              vErrString.
int CCVGAppWrapper::CheckValidity(gtString &vErrString)
{
  GT_UNREFERENCED_PARAMETER(vErrString);
  constexpr int retVal = 0;
  m_sbLoadEnabled = (0 == retVal);
  return retVal;
}

/// @brief Entry point for initialisation of a plugin. Create objects that 
///        interact with the CodeXL framework. Typical order of initialisation:
///        - Create and register the plugin's specific main menu actions
///          creator.
///        - Initialise backend modules/libraries/DLL specific to this plugin.
///        - Create and register the plugin's specific XML node in the global
///          settings page.
///        - Create and register the plugin's specific XML node in the project
///          settings page.
///        - Create and register the plugin's specific execution mode.
///        - Create event observers and register them with framework managers
///        - Create, initialise and register the plugin's specific views 
///          creator.
void CCVGAppWrapper::Initialize()
{
  ccvg::fw::CCVGFWTheCurrentStateSetIsBusy CCVGIsBusy;

  // Package resource string should be the first object to be initialised
  ccvg::fnstatus::status status = ccvg::fnstatus::success;
  status = CCVGResourcesString::Instance().Initialise();
  if (status == failure)
  {
    const wchar_t *err = CCVGResourcesString::Instance().ErrorGetDescriptionW();
    LogError(CCVGRSRC(kIDS_CCVG_STR_pkg_resource_string_err_fail_init), err);
    return;
  }

  status = CCVGResourcesIcon::Instance().Initialise();
  if (status == failure)
  {
    const wchar_t *err = CCVGResourcesIcon::Instance().ErrorGetDescriptionW();
    LogError(CCVGRSRC(kIDS_CCVG_STR_pkg_resource_icon_err_fail_init), err);
    return;
  }

  status = ccvg::cmd::CCVGCmdInvoker::Instance().Initialise();
  if (status == failure)
  {
    const wchar_t *err = ccvg::cmd::CCVGCmdInvoker::Instance().ErrorGetDescriptionW();
    LogError(err, L"");
    return;
  }

  status = ccvg::fw::CCVGFWTheCurrentState::Instance().Initialise();
  if (status == failure)
  {
    const wchar_t *err = ccvg::fw::CCVGFWTheCurrentState::Instance().ErrorGetDescriptionW();
    LogError(err, L"");
    return;
  }

  m_psAppEventObserver = new (std::nothrow) ccvg::fw::CCVGFWEventObserver;
  GT_IF_WITH_ASSERT(m_psAppEventObserver != nullptr)
  {
    status = m_psAppEventObserver->Initialise();
    if (status == failure)
    {
      const wchar_t *err = m_psAppEventObserver->ErrorGetDescriptionW();
      LogError(CCVGRSRC(kIDS_CCVG_STR_err_plugin_failed_init), err);
      delete m_psAppEventObserver;
      m_psAppEventObserver = nullptr;
      return;
    }
    afPluginConnectionManager::instance().registerRunModeManager(m_psAppEventObserver);
  }

  m_psMDIViewCreator = new (std::nothrow) ccvg::fw::CCVGMDIViewCreator;
  GT_IF_WITH_ASSERT(m_psMDIViewCreator != nullptr)
  {
    status = m_psMDIViewCreator->Initialise();
    if (status == failure)
    {
      const wchar_t *err = m_psMDIViewCreator->ErrorGetDescriptionW();
      LogError(CCVGRSRC(kIDS_CCVG_STR_err_plugin_failed_init), err);
      delete m_psMDIViewCreator;
      m_psMDIViewCreator = nullptr;
      return;
    }
    afQtCreatorsManager::instance().registerViewCreator(m_psMDIViewCreator);
  }

  status = ccvg::fw::CCVGFWMDIViewMgr::Instance().Initialise();
  if (status == failure)
  {
    const wchar_t *err = ccvg::fw::CCVGFWMDIViewMgr::Instance().ErrorGetDescriptionW();
    LogError(err, L"");
    return;
  }

    m_psMenuActionsExecutor = new (std::nothrow) ccvg::fw::CCVGMenuActionsExecutor;
  GT_IF_WITH_ASSERT(m_psMenuActionsExecutor != nullptr)
  {
    afQtCreatorsManager::instance().registerActionExecutor(m_psMenuActionsExecutor);
  }

  // This not common to all execution modes so may not be needed here either
  // The execution modes needs to register before the gwEventObserver
  status = CCVGExecutionMode::Instance().Initialise();
  if (status == failure)
  {
    const wchar_t *err = CCVGExecutionMode::Instance().ErrorGetDescriptionW();
    LogError(CCVGRSRC(kIDS_CCVG_STR_err_plugin_failed_init), err);
    return;
  }
  m_spCCVGExecutionMode = &CCVGExecutionMode::Instance();
  afExecutionModeManager::instance().registerExecutionMode(m_spCCVGExecutionMode);

  // Create and register the global settings page:
  m_psTabPageSettings = new (std::nothrow) ccvg::gui::CCVGGUITabPageSettings;
  GT_IF_WITH_ASSERT(m_psTabPageSettings != nullptr)
  {
    status = m_psTabPageSettings->Initialise();
    if (status == failure)
    {
      const wchar_t *err = m_psTabPageSettings->ErrorGetDescriptionW();
      LogError(CCVGRSRC(kIDS_CCVG_STR_gui_TabPageSettings_err_fail_init), err);
      delete m_psTabPageSettings;
      m_psTabPageSettings = nullptr;
      return;
    }
    afGlobalVariablesManager::instance().registerGlobalSettingsPage(m_psTabPageSettings);
  }

  // Create and register the project settings extension object
  m_psProjectSettings = new ccvg::gui::CCVGGUIProjectSettings;
  GT_IF_WITH_ASSERT(m_psProjectSettings != nullptr)
  {
    status = m_psProjectSettings->Initialise();
    if (status == failure)
    {
      const wchar_t *err = m_psProjectSettings->ErrorGetDescriptionW();
      LogError(CCVGRSRC(kIDS_CCVG_STR_gui_ProjectSettings_err_fail_init), err);
      delete m_psProjectSettings;
      m_psProjectSettings = nullptr;
      return;
    }
    afProjectManager::instance().registerProjectSettingsExtension(m_psProjectSettings);
    m_psProjectSettings->SetDoNotDeleteThisFlag(true);
  }

  status = ccvg::gui::CCVGGUIAppTreeHandler::Instance().Initialise();
  if (status == failure)
  {
    const wchar_t *err = ccvg::gui::CCVGGUIAppTreeHandler::Instance().ErrorGetDescriptionW();
    LogError(err, L"");
    return;
  }
}

/// @brief  Release resources used by this plugin. Generally the other of
///         shutdown in the the reverse order of packages initialised.
/// @return status success = all resources released ok, 
///         failure = failed to release.
ccvg::fnstatus::status CCVGAppWrapper::Shutdown()
{
  ccvg::fnstatus::status status = ccvg::fnstatus::success;

  ccvg::fw::CCVGFWTheCurrentStateSetIsBusy CCVGIsBusy;

  status = ccvg::gui::CCVGGUIAppTreeHandler::Instance().Shutdown();
  if (status == failure)
  {
    const wchar_t *err = ccvg::gui::CCVGGUIAppTreeHandler::Instance().ErrorGetDescriptionW();
    LogError(err, L"");
    return status;
  }
  
  if (m_psProjectSettings != nullptr)
  {
    status = m_psProjectSettings->Shutdown();
    if (status == ccvg::fnstatus::failure)
    {
      const wchar_t *err = m_psProjectSettings->ErrorGetDescriptionW();
      LogError(CCVGRSRC(kIDS_CCVG_STR_gui_ProjectSettings_err_fail_init), err);
    }
    if (!m_psProjectSettings->GetDoNotDeleteThisFlag())
    {
      delete m_psProjectSettings;
    }
    m_psProjectSettings = nullptr;
  }

  if (m_psTabPageSettings != nullptr)
  {
    status = m_psTabPageSettings->Shutdown();
    if (status == ccvg::fnstatus::failure)
    {
      const wchar_t *err = m_psTabPageSettings->ErrorGetDescriptionW();
      LogError(CCVGRSRC(kIDS_CCVG_STR_gui_TabPageSettings_err_fail_shtdwn), err);
    }
    delete m_psTabPageSettings;
    m_psTabPageSettings = nullptr;
  }

  if (m_spCCVGExecutionMode != nullptr)
  {
    status = m_spCCVGExecutionMode->Shutdown();
    if (status == ccvg::fnstatus::failure)
    {
      const wchar_t *err = m_spCCVGExecutionMode->ErrorGetDescriptionW();
      LogError(CCVGRSRC(kIDS_CCVG_STR_err_plugin_failed_shutdown), err);
    }
    // delete m_spCCVGExecutionMode - a singleton
    m_spCCVGExecutionMode = nullptr;
  }

  if (m_psMenuActionsExecutor != nullptr)
  {
    delete m_psMenuActionsExecutor;
    m_psMenuActionsExecutor = nullptr;
  }
  
    if (m_psMDIViewCreator != nullptr)
  {
    status = m_psMDIViewCreator->Shutdown();
    if (status == ccvg::fnstatus::failure)
    {
      const wchar_t *err = m_psMDIViewCreator->ErrorGetDescriptionW();
      LogError(CCVGRSRC(kIDS_CCVG_STR_err_plugin_failed_shutdown), err);
    }
    delete m_psMDIViewCreator;
    m_psMDIViewCreator = nullptr;
  }

  if (m_psAppEventObserver != nullptr)
  {
    status = m_psAppEventObserver->Shutdown();
    if (status == ccvg::fnstatus::failure)
    {
      const wchar_t *err = m_psAppEventObserver->ErrorGetDescriptionW();
      LogError(CCVGRSRC(kIDS_CCVG_STR_err_plugin_failed_shutdown), err);
    }
    delete m_psAppEventObserver;
    m_psAppEventObserver = nullptr;
  }

  // IOR: Todo: to remove this
  if(m_psToolbarMainWnd != nullptr)
  {
    status = m_psToolbarMainWnd->Shutdown();
    if (status == ccvg::fnstatus::failure)
    {
      const wchar_t *err = m_psToolbarMainWnd->ErrorGetDescriptionW();
      LogError(CCVGRSRC(kIDS_CCVG_STR_err_plugin_failed_shutdown), err);
    }
    delete m_psToolbarMainWnd;
    m_psToolbarMainWnd = nullptr;
  }
  
  status = ccvg::fw::CCVGFWMDIViewMgr::Instance().Shutdown();
  if (status == failure)
  {
    const wchar_t *err = ccvg::fw::CCVGFWMDIViewMgr::Instance().ErrorGetDescriptionW();
    LogError(err, L"");
    GT_IF_WITH_ASSERT(false) {}
  }

  status = ccvg::fw::CCVGFWTheCurrentState::Instance().Shutdown();
  if (status == failure)
  {
    const wchar_t *err = ccvg::fw::CCVGFWTheCurrentState::Instance().ErrorGetDescriptionW();
    LogError(err, L"");
    GT_IF_WITH_ASSERT(false) {}
  }

  status = ccvg::cmd::CCVGCmdInvoker::Instance().Shutdown();
  if (status == failure)
  {
    const wchar_t *err = ccvg::cmd::CCVGCmdInvoker::Instance().ErrorGetDescriptionW();
    LogError(err, L"");
    GT_IF_WITH_ASSERT(false) {}
  }

  status = CCVGResourcesIcon::Instance().Shutdown();
  if (status == ccvg::fnstatus::failure)
  {
    const wchar_t *err = CCVGResourcesIcon::Instance().ErrorGetDescriptionW();
    LogError(CCVGRSRC(kIDS_CCVG_STR_pkg_resource_icon_err_fail_shtdwn), err);
    GT_IF_WITH_ASSERT(false) {}
  }

  status = CCVGResourcesString::Instance().Shutdown();
  if (status == ccvg::fnstatus::failure)
  {
    const wchar_t *err = CCVGResourcesString::Instance().ErrorGetDescriptionW();
    LogError(CCVGRSRC(kIDS_CCVG_STR_pkg_resource_string_err_fail_shtdwn), err);
    GT_IF_WITH_ASSERT(false) {}
  }

  return status;
}

/// @brief  This function initialises all the widget items that are not
///         registered with the creators mechanism. These widgets are responsible for
///         its own callbacks and string.
void CCVGAppWrapper::InitializeIndependentWidgets()
{
  // Get the main application window
  afMainAppWindow *pAppMainWindow = afMainAppWindow::instance();
  GT_IF_WITH_ASSERT(pAppMainWindow != nullptr)
  {
    // IOR: Todo: to remove this
    m_psToolbarMainWnd = new (std::nothrow) ccvg::gui::CCVGToolBarMainWnd(pAppMainWindow);
    GT_IF_WITH_ASSERT(m_psToolbarMainWnd != nullptr)
    {
      const status status = m_psToolbarMainWnd->Initialise();
      if (status == ccvg::fnstatus::failure)
      {
        const wchar_t *err = m_psToolbarMainWnd->ErrorGetDescriptionW();
        LogError(CCVGRSRC(kIDS_CCVG_STR_gui_toolbar_mainWnd_err_fail_init), err);
        GT_ASSERT(true);
        return;
      }

      m_psToolbarMainWnd->setVisible(true);

      // Get the image and buffers toolbar and insert CCVG toolbar
      const gtString toolbarName = AF_STR_ImagesAndBuffersToolbar;
      QString toolbarNameQstr = QString::fromWCharArray(toolbarName.asCharArray());
      toolbarNameQstr.append(AF_STR_toolbarPostfix);
      QToolBar *pImagesAndBuffersToolbar = pAppMainWindow->findChild<QToolBar *>(toolbarNameQstr);
      pAppMainWindow->addToolbar(m_psToolbarMainWnd, static_cast<acToolBar *>(pImagesAndBuffersToolbar));
    }
  }
}

/// @brief  Indicates this plugin can be loaded as all it's prerequisites were
///         met. Note: only in stand along (SA) apparently (not verified) when 
///         the DLL can be dynamically loaded could the flag m_sbLoadEnabled 
///         be false. (IOR: From code documented previous elsewhere)
/// @return bool True = yes can be loaded, false = cannot be loaded.
bool CCVGAppWrapper::GetIsLoadEnabled()
{
  return m_sbLoadEnabled;
}

/// @brief  Log error message.
/// @param[in] wchar_t* General error description text. 
/// @param[in] wchar_t* Context error description text. 
void CCVGAppWrapper::LogError(const wchar_t *vpErrMsg, const wchar_t *vpErrCntxt)
{
  const wchar_t *subj(CCVGRSRC(kIDS_CCVG_STR_AppNameShort));
  gtString msg(gtString().appendFormattedString(vpErrMsg, subj));
  msg.appendFormattedString(L"%ls. %ls", msg.asCharArray(), vpErrCntxt);
  OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_ERROR);
}

/// @brief Retrieve *this module's GUI project settings object (created by 
///        *this classe). Caller takes ownership.
/// @return CCVGGUIProjectSettings& object.
/// @warning Caller takes ownership of the resource.
ccvg::gui::CCVGGUIProjectSettings& CCVGAppWrapper::GetGuiProjSettings()
{
  auto& rReturn = *m_psProjectSettings;
  m_psProjectSettings = nullptr;
  return rReturn;
}

// These 'C' functions are called by the function 
// appApplicationStart::appMain()'s call to DynamicallyCall() to install 
// DLL plugins which form CodeXL execution mode.
extern "C"
{
  /// @brief       Check the validity of the plug in. Check it has the resources
  ///              available to operate.
  /// @param[out]  vErrString Error description text. Wide char text.
  /// @return      int 0 == all ok success,  !0 == an error condition, check 
  ///              vErrString.
  int CheckValidity(gtString &vErrString)
  {
    return CCVGAppWrapper::Instance().CheckValidity(vErrString);
  }

  /// @brief Entry point for initialization called on application 
  ///        startup (appApplicationStart::appMain()). 
  void initialize()
  {
      CCVGAppWrapper::Instance().Initialize();
  }

  /// @brief Initialize other GUI items that can be done only after main 
  ///        window is alive.
  void LPGPU2_CCVG_API initializeIndependentWidgets()
  {
      CCVGAppWrapper::Instance().InitializeIndependentWidgets();
  }

}; // extern "C"

#endif // LPGPU2_CCVG_COMPILE_IN
