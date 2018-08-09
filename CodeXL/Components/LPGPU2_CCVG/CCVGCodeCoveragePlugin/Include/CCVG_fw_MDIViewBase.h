// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWMDIViewBase interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_MDIVIEWBASE_H_INCLUDE
#define LPGPU2_CCVG_FW_MDIVIEWBASE_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewMgr.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewFactory.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewEnumTypes.h>

// Declarations:
class QWidget;

namespace ccvg {
namespace fw {
  /// @brief    CCVG MDI Views' base class.
///
/// @warning  None.
/// @see      MDIViewFactory, MDIViewMgr, MDIViewEnumTypes, 
///           fw::MDIViewRegisterAll().
/// @date     16/03/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWMDIViewBase
: public CCVGFWMDIViewFactory::IFactory
, public CCVGFWMDIViewMgr::IView
, public CCVGClassCmnBase
{
// Methods:
public:
  CCVGFWMDIViewBase();
  //
  void        SetParent(::QWidget *vpParent);
  ::QWidget*  GetParent() const;
  status      SetViewId(const gtString &vId);
  //
  virtual status  ReloadContent() = 0;
  
// Overridden:
public:
  // From CCVGClassCmnBase
  virtual ~CCVGFWMDIViewBase() override;
  // From CCVGFWMDIViewFactory::IFactory
  virtual const gtString&                        GetViewId() const override;
  virtual gtString                               GetViewName() const override;
  virtual EnumCCVGMDIViewType                    GetViewType() const override;
  virtual CCVGFWMDIViewFactory::ViewCreatorFnPtr GetCreatorFn() const override;
  virtual ::QWidget*                             GetWidget() override;
  // From CCVGFWMDIViewMgr::IView
  virtual status Initialise() override;
  virtual status Shutdown() override;

// Attributes:
protected:
  // *this object's state data
  CCVGuint  m_refCount;
  bool      m_bBeenInitialised;  // True = yes this has been initialised, false = not yet
  bool      m_bBeenShutdown;     // True = yes this has been shutdown already, false = not yet
  //
  gtString                                m_viewId;
  gtString                                m_viewName;
  EnumCCVGMDIViewType                     m_eViewType;
  CCVGFWMDIViewFactory::ViewCreatorFnPtr  m_pSelfCreatorFn;
  ::QWidget                                *m_pWgtParent;     // Some commands can be called by other commands
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_MDIVIEWBASE_H_INCLUDE