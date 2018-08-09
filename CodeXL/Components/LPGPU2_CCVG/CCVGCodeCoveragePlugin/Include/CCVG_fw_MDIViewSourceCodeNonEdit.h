// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWMDIViewSourceCodeNonEdit interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_MDIVIEWSOURCECODENONEDIT_H_INCLUDE
#define LPGPU2_CCVG_FW_MDIVIEWSOURCECODENONEDIT_H_INCLUDE

// QScintilla:
#define QSCINTILLA_DLL
#include <Qsci/qsciscintilla.h>

// Infra:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>
#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGHighlight.h>

// Declarations:
class QWidget;

namespace ccvg {
namespace fw {

/// @brief    CCVG MDI View to display text source code. This content not 
///           editable.
/// @warning  None.
/// @see      MDIViewFactory, MDIViewMgr, MDIViewEnumTypes, 
///           fw::MDIViewRegisterAll().
/// @date     17/03/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWMDIViewSourceCodeNonEdit final
: public CCVGFWMDIViewBase
, public acSourceCodeView
{
// Statics:
public:
  // Required by the CCVGFWMDIViewFactory when registering *this view
  static CCVGFWMDIViewBase* CreateSelf();

// Methods:
public:
  CCVGFWMDIViewSourceCodeNonEdit();

// Overridden:
public:
  // From CCVGFWMDIViewBase
  virtual ~CCVGFWMDIViewSourceCodeNonEdit() override;
  virtual ::QWidget* GetWidget() override;
  virtual status ReloadContent() override;
  // From CCVGFWMDIViewMgr::IView
  virtual status Initialise() override;
  virtual status Shutdown() override;
  
  // Methods:
private:
  status ApplyHighlight(const CCVGFWCCVGHighlight &vrHighlight);
  status SetMonospace();



};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_MDIVIEWSOURCECODENONEDIT_H_INCLUDE