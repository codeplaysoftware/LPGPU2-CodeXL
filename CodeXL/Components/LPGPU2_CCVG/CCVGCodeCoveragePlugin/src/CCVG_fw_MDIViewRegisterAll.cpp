// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewRegisterAll.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewSourceCodeNonEdit.h>

namespace ccvg {
namespace fw {

template<typename T> static ccvg::fnstatus::status MDIViewRegister();
template<typename T> static ccvg::fnstatus::status MDIViewRegisterView(const ccvg::fnstatus::status &vPrevRegisterStatus);

} // namespace fw
} // namespace ccvg

/// @brief         Global template function. CCVG MDI view to MDI view x
///                vregistration function.
/// @param         T A command type class.
/// @return status Success = command is registered, false = command failed to 
///                register.
/// @warning  None.
/// @see      ccvg::fw::MDIViewRegisterAll, ccvg::fw::MDIViewFactory.
/// @author   Illya Rudkin.
template<typename T>
static ccvg::fnstatus::status ccvg::fw::MDIViewRegister()
{
  static ccvg::fw::CCVGFWMDIViewFactory &rViewFactory = ccvg::fw::CCVGFWMDIViewFactory::Instance();
  const T view;
  const ccvg::fw::EnumCCVGMDIViewType eView = view.GetViewType();
  const gtString &rViewName(view.GetViewName());
  ccvg::fw::CCVGFWMDIViewFactory::ViewCreatorFnPtr fn = view.GetCreatorFn();
  return rViewFactory.ViewRegister(eView, fn, rViewName);
}

/// @brief Global template function MDIViewRegisterView.
///          
/// Macro like function to reduce code clutter.
///
/// @param     T A command type class.
/// @param[in] vPrevRegisterStatus Registration task status previously.
/// @param[in] vPrevRegisterStatus Registration task status previously.
/// @return    memmgr::fnstatus::status success or failure.
/// @warning   None.
/// @see       ccvg::fw::MDIViewRegister, ccvg::fw::MDIViewFactory.
/// @author    Illya Rudkin.
template<typename T>
static ccvg::fnstatus::status ccvg::fw::MDIViewRegisterView(const ccvg::fnstatus::status &vPrevRegisterStatus) {
  return ((vPrevRegisterStatus == ccvg::fnstatus::success) &&
    (ccvg::fnstatus::success == MDIViewRegister<T>())) ?
    ccvg::fnstatus::success : ccvg::fnstatus::failure;
}

/// @brief         CCVG Register MDI Views with the MDI view Factory.
/// @return status Success = All views are registered, failure = one of 
///                more view failed to register.
ccvg::fnstatus::status ccvg::fw::MDIViewRegisterAll()
{
  ccvg::fnstatus::status status = ccvg::fnstatus::success;
  status = MDIViewRegisterView<CCVGFWMDIViewSourceCodeNonEdit>(status);
  
  return status;
}

