// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// Global utility or helper functions.
/// CCVGUtilMsgDlg implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infrastructure:
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acQMessageDialog.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_MsgDlg.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

/// @brief Static function. Display a CodeXL modal dialog box with a message.
/// @param[in] vMsg The message to display to the user.
void CCVGUtilMsgDlg::Show(const gtString &vMsg)
{
  QWidget *pMainWindow = afMainAppWindow::instance();
  const wchar_t *pDlgTitle = CCVGRSRC(kIDS_CCVG_STR_AppNameLong);
  const QString dlgTitle(acGTStringToQString(pDlgTitle));
  const QSize messageSize(acScalePixelSizeToDisplayDPI(ms_msgBxWidth), acScalePixelSizeToDisplayDPI(ms_msgBxHeight));
  acQMessageDialog messageBox(afGlobalVariablesManager::ProductNameA(), dlgTitle, acGTStringToQString(vMsg.asCharArray()), pMainWindow, messageSize);
  messageBox.exec();
}
