// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGExecutionMode interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_EXECUTIONMODE_H_INCLUDE
#define LPGPU2_CCVG_EXECUTIONMODE_H_INCLUDE

// Framework:
#include <AMDTApplicationFramework/Include/afIExecutionMode.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
// LPGPU2 Project Code Coverage
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
#include <CCVGCodeCoveragePlugin/Include/CCVGProjectBuildDefinitions.h>
#else
// Linux build
#include <CCVGCodeCoveragePlugin/Include/CCVGProjectBuildDefinitions.h>
#endif // (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
#include <CCVGCodeCoveragePlugin/Include/CCVGDLLBuild.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_SingletonBase.h>

#if LPGPU2_CCVG_COMPILE_IN

/// @brief  CCVGExecutionMode is a concrete implementation of the interface
///         afIExecutionMode. Singleton follows CCVG Package pattern.
/// @warning  None.
/// @see      Interface afIExecutionMode.
/// @date     28/07/2016.
/// @author   Illya Rudkin.
// clang-format off
class LPGPU2_CCVG_API CCVGExecutionMode final
: public afIExecutionMode
, public CCVGClassCmnBase
, public ISingleton<CCVGExecutionMode>
{
  friend ISingleton<CCVGExecutionMode>;

// Overridden:
public:
  // From util::ISingleton
  virtual ccvg::fnstatus::status Initialise() override;
  virtual ccvg::fnstatus::status Shutdown() override;
  // From afIExecutionMode
  virtual ~CCVGExecutionMode() override;
  virtual gtString  modeName() override;
  virtual gtString  modeNameDisplayInGui() override;
  virtual bool      IsExecutionStatusRelevant() override;
  virtual gtString  modeActionString() override;
  virtual gtString  modeVerbString() override;
  virtual gtString  modeDescription() override;
  virtual bool      ExecuteStartupAction(afStartupAction vAction) override;
  virtual bool      IsStartupActionSupported(afStartupAction vAction) override;
  virtual void      execute(afExecutionCommandId vCommandId) override;
  virtual void      updateUI(afExecutionCommandId vCommandId, QAction *vpAction) override;
  virtual int       numberSessionTypes() override;
  virtual gtString  sessionTypeName(int vSessionTypeIndex) override;
  virtual QPixmap*  sessionTypeIcon(int vSessionTypeIndex) override;
  virtual int       indexForSessionType(const gtString &vSessionType) override;
  virtual afMainAppWindow::LayoutFormats layoutFormat() override;
  virtual gtString  ProjectSettingsPath() override;
  virtual bool      isModeEnabled() override;
  virtual gtString  HowToStartModeExecutionMessage() override;
  virtual void      GetToolbarStartButtonText(gtString &vButtonText, bool vbFullString = true) override;

// Methods:
private:
  // Singleton enforcement
  CCVGExecutionMode();
  CCVGExecutionMode(const CCVGExecutionMode&) = delete;
  CCVGExecutionMode(CCVGExecutionMode&&) = delete;
  CCVGExecutionMode& operator= (const CCVGExecutionMode&) = delete;
  CCVGExecutionMode& operator= (CCVGExecutionMode&&) = delete;
  //
  // Error handling helpers
  void LogError(const wchar_t *vpErrMsg, const wchar_t *vpErrCntxt);
};
// clang-format on

#endif // LPGPU2_CCVG_COMPILE_IN
#endif // LPGPU2_CCVG_EXECUTIONMODE_H_INCLUDE

