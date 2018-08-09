// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGMenuActionsExecutor interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_MENUACTIONSEXECUTOR_H_INCLUDE
#define LPGPU2_CCVG_FW_MENUACTIONSEXECUTOR_H_INCLUDE

// Infra:
#include <AMDTApplicationFramework/Include/afActionExecutorAbstract.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Invoker.h>

// Declarations:
class afApplicationTree;

namespace ccvg {
namespace fw {

/// @brief    CCVGToolBarMainWnd derived from afActionExecutorAbstract. 
///           Concrete implementation for the action executor. This class
///           represents an action creator that both trigger actions and 
///           handles UI update.
///
/// Commands are added to main menu. There are two types of commmand both of 
/// which appear in the main menu in the same way. They are implemented through
/// the same class interface. 
/// Global commmands: Commands that are executed regardless of any views open, 
/// the current active view or views that are close. If a command is enabled and
/// executed it will carried out directly not going through the current actice
/// view or the current active view creator.
///
/// @warning  None.
/// @see      None.
/// @date     12/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGMenuActionsExecutor final
: public afActionExecutorAbstract
{
// Methods:
public:
  CCVGMenuActionsExecutor();

// Overridden:
public:
  // From afActionExecutorAbstract
  ~CCVGMenuActionsExecutor();
  virtual void handleTrigger(int vActionIndex) override;
  virtual void handleUiUpdate(int vActionIndex) override;
  // From afActionCreatorAbstract
  virtual gtString  menuPosition(int vActionIndex, afActionPositionData &vrPositionData) override;
  virtual gtString  toolbarPosition(int vActionIndex) override;
  virtual void      groupAction(int vActionIndex) override;
  virtual void      initActionIcons() override;
  virtual bool      actionText(int vActionIndex, gtString &vrActionText, gtString &vrTooltip, gtString &vrKeyboardShortcut) override;

// Methods:
private:
  void   ExecuteCmdOpenProjectSettingsDlg();
  void   ExecuteCmdFilesReadAll();
  void   ExecuteCmdSwitchToCCVGExecutionMode();
  status ExecuteCmdHandleError(status vFnStatus, bool vbExceptionCaught, const gtString &vrCmdName);

// Overridden:
private:
  // From afActionCreatorAbstract
  virtual void populateSupportedCommandIds() override;

// Attributes:
private:
  ccvg::cmd::CCVGCmdInvoker &m_rCmdInvoker;
};
// clang-format on

} //  namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_MENUACTIONSEXECUTOR_H_INCLUDE