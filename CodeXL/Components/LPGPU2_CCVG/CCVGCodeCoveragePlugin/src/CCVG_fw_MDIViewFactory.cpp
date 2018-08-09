// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWMDIViewFactory implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewFactory.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewRegisterAll.h>

namespace ccvg {
namespace fw {

/// @brief Class constructor. Copy the CCVG default operational parameter values
///        to the user entered equivalent parameters.
CCVGFWMDIViewFactory::CCVGFWMDIViewFactory()
{
}

/// @brief Class destructor. Tidy up release resources used by *this instance.
CCVGFWMDIViewFactory::~CCVGFWMDIViewFactory()
{
  Shutdown();
}

/// @brief  Package initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewFactory::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  m_mapViewTypeToViewCreatorFn = new (std::nothrow) MapViewTypeToViewCreatorFn_t;

  const status status = MDIViewRegisterAll();
  if (status == failure)
  {
    const wchar_t *pErr = ErrorGetDescriptionW();
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewFactory_string_err_fail_init_register_views), pErr));
  }

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Package shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewFactory::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  // Tidy up
  if (m_mapViewTypeToViewCreatorFn != nullptr)
  {
    m_mapViewTypeToViewCreatorFn->clear();
    delete m_mapViewTypeToViewCreatorFn;
    m_mapViewTypeToViewCreatorFn = nullptr;
  }
  
  m_bBeenShutdown = true;

  return success;
}

/// @brief  Register a view with the MDI View factory so that it is able to
///         create and instance of the view on demand.
/// @param[in] vkViewType View's unique type ID.
/// @param[in] vViewCreateFn The view's how to create itself function.
/// @param[in] vrViewName The view's name.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewFactory::ViewRegister(EnumCCVGMDIViewType vkViewType, ViewCreatorFnPtr vViewCreateFn, const gtString &vrViewName)
{
  if (vrViewName.isEmpty())
  {
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewFactory_err_view_name_invalid));
  }

  if (!IsValidViewType(vkViewType))
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewFactory_viewType_err_invalid), vrViewName.asCharArray()));
  }

  if (HaveViewTypeAlreadyRegistered(vkViewType))
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewFactory_viewType_err_alreadyRegistered), vrViewName.asCharArray()));
  }

  if (vViewCreateFn == nullptr)
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewFactory_err_view_selfFnPtr_invalid), vrViewName.asCharArray()));
  }

  MapPairViewTypeToViewCreatorFn_t pair(vkViewType, vViewCreateFn);
  m_mapViewTypeToViewCreatorFn->insert(pair);

  return success;
}

/// @brief  Create a view with the view factory so that it is able to
///         create and instance of the view on demand.
/// @param[in] vkViewId View's unique ID formatted as 
///            "<CVGItemId>,<File type>".
/// @param[in] vkViewType View enumeration.
/// @param[out] vpView Pointer to a new Qt derived widget object.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewFactory::ViewCreate(const gtString &vViewid, EnumCCVGMDIViewType vkViewType, ::QWidget *&vpViewWgt, CCVGFWMDIViewBase *&vpView)
{
  vpView = nullptr;

  if (!IsValidViewId(vViewid))
  {
    return false;
  }

  if (!IsValidViewType(vkViewType))
  {
    return false;
  }

  if (!HaveViewTypeAlreadyRegistered(vkViewType))
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewFactory_viewType_err_notRegisteredType), vkViewType));
  }

  CCVGFWMDIViewBase *pView = nullptr;
  const MapViewTypeToViewCreatorFn_t::const_iterator it = m_mapViewTypeToViewCreatorFn->find(vkViewType);
  if (it != m_mapViewTypeToViewCreatorFn->cend())
  {
    ViewCreatorFnPtr fn = (*it).second;
    pView = fn();
  }
  if (pView == nullptr)
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewFactory_err_create_failed), vkViewType));
  }

  vpViewWgt = pView->GetWidget();
  vpView = pView;

  return success;
}

/// @brief     Check the given view type enumeration is a valid one.
/// @param[in] vkViewType View enumeration.
/// @return    bool True = yes valid,  false = not valid.
bool CCVGFWMDIViewFactory::IsValidViewType(EnumCCVGMDIViewType vkViewType) 
{
  const int type = static_cast<int>(vkViewType);
  if ((type < 1) || (type >= static_cast<int>(EnumCCVGMDIViewType::kCCVGMDIViewTypeType_count)))
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewFactory_viewType_err_invalid), vkViewType));
  }

  return true;
}

/// @brief     Check the given view type is already registered with *this factory.
/// @param[in] vkViewType View enumeration.
/// @return    bool True = yes registered,  false = not registered.
bool CCVGFWMDIViewFactory::HaveViewTypeAlreadyRegistered(EnumCCVGMDIViewType vkViewType) const
{
  const MapViewTypeToViewCreatorFn_t::const_iterator it = m_mapViewTypeToViewCreatorFn->find(vkViewType);
  return (it != m_mapViewTypeToViewCreatorFn->cend() ? true : false);
}

/// @brief Validate the MDI view's ID.
/// @param[in] vViewId Text formatted ID formatted as 
///            "<CVGItemId>,<File type>".
/// @return bool True = valid, false = not valid.
bool CCVGFWMDIViewFactory::IsValidViewId(const gtString &vViewId) 
{
  if(vViewId.isEmpty())
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_fw_MDIView_err_invalidViewId), vViewId.asCharArray()));
  }

  return true;
}

} // namespace fw
} // namespace ccvg
