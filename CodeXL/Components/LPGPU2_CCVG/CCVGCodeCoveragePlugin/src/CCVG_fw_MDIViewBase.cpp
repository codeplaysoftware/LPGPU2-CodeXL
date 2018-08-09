// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWMDIViewBase implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace fw {

/// @brief  Class constructor. No work is done here by this class.
CCVGFWMDIViewBase::CCVGFWMDIViewBase()
: m_pSelfCreatorFn(nullptr)
, m_bBeenInitialised(false)
, m_bBeenShutdown(false)
, m_viewId(L"<no CCVGItemId+FileType>")
, m_viewName(L"<no name>")
, m_refCount(0)
, m_eViewType(kCCVGMDIViewType_invalid)
, m_pWgtParent(nullptr)
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGFWMDIViewBase::~CCVGFWMDIViewBase()
{
}

/// @brief  Retrieve *this view's creation function.
/// @return CCVGFWMDIViewFactory::ViewCreatorFnPtr Function pointer.
CCVGFWMDIViewFactory::ViewCreatorFnPtr CCVGFWMDIViewBase::GetCreatorFn() const
{
  return m_pSelfCreatorFn;
}

/// @brief    Set *this view's Qt parent widget. 
/// @param[in] vpParent The parent widget to *this widget.
void CCVGFWMDIViewBase::SetParent(QWidget *vpParent)
{
  m_pWgtParent = vpParent;
}

/// @brief  Retreive *this view's parent widget.
/// @return QWidget* Parent Qt widget object pointer of NULL pointer.
QWidget* CCVGFWMDIViewBase::GetParent() const
{
  return m_pWgtParent;
}

/// @brief Assign to *this instance a unique ID. The ID is in the format:
///        "<CCVGItemId>,FIle type". This ID is used elsewhere where it is
///        parsed and split to determine operations that can take place.
/// @param[in] vId Text formatted ID
/// @return status Success = ID assigned, failure = invalid ID, see error
///         description.
status CCVGFWMDIViewBase::SetViewId(const gtString &vId)
{
  if(vId.isEmpty())
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_fw_MDIView_err_invalidViewId), vId.asCharArray()));
  }

  m_viewId = vId;
  return success;
}

/// @brief  Overridden. Retrieve the view's CCVGItemId+FileType identifier.
/// @return gtString& Text identifier.
const gtString& CCVGFWMDIViewBase::GetViewId() const
{
  return m_viewId;
}

/// @brief  Overridden. Retrieve the view's name.
/// @return gtString Text identifier.
gtString CCVGFWMDIViewBase::GetViewName() const
{
  return m_viewName;
}

/// @brief  Overridden. Retrieve the view's view type identifier.
/// @return EnumCCVGMDIViewType View type enumeration.
EnumCCVGMDIViewType CCVGFWMDIViewBase::GetViewType() const
{
  return m_eViewType;
}

/// @brief Overridden. Retrieve the Qt QWidget pointer for *this view.
/// @return QWidget NULL, over ride to implement.
::QWidget* CCVGFWMDIViewBase::GetWidget()
{
  // Override to implement
  return nullptr;
}

/// @brief  Overridden. Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewBase::Initialise()
{
  // Override to implement
  return failure;
}

/// @brief  Overridden. Class shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewBase::Shutdown()
{
  // Override to implement
  return failure;
}


} // namespace fw 
} // namespace ccvg 
