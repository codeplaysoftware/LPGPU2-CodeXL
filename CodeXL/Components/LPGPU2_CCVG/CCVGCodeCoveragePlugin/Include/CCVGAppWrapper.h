// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGAppWrapper interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_APPWRAPPER_H_INCLUDE
#define LPGPU2_CCVG_APPWRAPPER_H_INCLUDE

// Local:
// LPGPU2 Project Code Coverage
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
#include <CCVGCodeCoveragePlugin/Include/CCVGProjectBuildDefinitions.h>
#else
// Linux build
#include <CCVGCodeCoveragePlugin/Include/CCVGProjectBuildDefinitions.h>
#endif // (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

#include <CCVGCodeCoveragePlugin/Include/CCVGDLLBuild.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGFnStatus.h>

#if LPGPU2_CCVG_COMPILE_IN

// Declarations:
class CCVGExecutionMode;
namespace ccvg { namespace gui { class CCVGGUITabPageSettings; } }
namespace ccvg { namespace gui { class CCVGToolBarMainWnd; } }
namespace ccvg { namespace fw { class CCVGMenuActionsExecutor; } }
namespace ccvg { namespace fw { class CCVGMDIViewCreator; } }
namespace ccvg { namespace gui { class CCVGGUIProjectSettings; } }
namespace ccvg { namespace fw { class CCVGFWEventObserver; } }

/// @brief    CCVGAppWrapper is class wrapper for providing information to
///           the application on startup and how to instantiate itself, hook
///           itself in the application framework.
///           Singleton object.
/// @warning  None.
/// @see      None.
/// @date     28/07/2016.
/// @author   Illya Rudkin.
// clang-format off
class LPGPU2_CCVG_API CCVGAppWrapper final
{
// Statics:
public:
  static CCVGAppWrapper&                    Instance();
  static bool                               GetIsLoadEnabled();
  static ccvg::gui::CCVGGUIProjectSettings& GetGuiProjSettings();

// Methods:
public:
        ~CCVGAppWrapper();
  int   CheckValidity(gtString &vErrString);
  void  Initialize();
  void  InitializeIndependentWidgets();

// Methods:
private:
  // Singleton enforcement
  CCVGAppWrapper();
  CCVGAppWrapper(const CCVGAppWrapper&) = delete;
  CCVGAppWrapper(CCVGAppWrapper&&) = delete;
  CCVGAppWrapper& operator= (const CCVGAppWrapper&) = delete;
  CCVGAppWrapper& operator= (CCVGAppWrapper&&) = delete;
  //
  // Code Coverage resource handling
  ccvg::fnstatus::status Shutdown();
  //
  // Error handling helpers
  void LogError(const wchar_t *vpErrMsg, const wchar_t *vpErrCntxt);

// Attributes:
private:
  static CCVGAppWrapper *                     m_spMySingleInstance;
  static bool                                 m_sbLoadEnabled;        // True = able to use plugin, false = disabled
  static CCVGExecutionMode *                  m_spCCVGExecutionMode;
  static ccvg::gui::CCVGToolBarMainWnd *      m_psToolbarMainWnd;
  static ccvg::fw::CCVGMenuActionsExecutor *  m_psMenuActionsExecutor;
  static ccvg::fw::CCVGMDIViewCreator *       m_psMDIViewCreator;
  static ccvg::gui::CCVGGUITabPageSettings *  m_psTabPageSettings; // Global CCVG setting, menu tool-->options
  static ccvg::gui::CCVGGUIProjectSettings *  m_psProjectSettings;
  static ccvg::fw::CCVGFWEventObserver *      m_psAppEventObserver;
};
// clang-format on

// DLL library exported functions - common for all plugins
extern "C"
{
    // Check validity of the plugin:
    int LPGPU2_CCVG_API CheckValidity(gtString& errString);

    // Initialize function for this wrapper:
    void LPGPU2_CCVG_API initialize();

    // Initialize other items after main window creation:
    void LPGPU2_CCVG_API initializeIndependentWidgets();
};

#endif // LPGPU2_CCVG_COMPILE_IN
#endif // LPGPU2_CCVG_APPWRAPPER_H_INCLUDE

