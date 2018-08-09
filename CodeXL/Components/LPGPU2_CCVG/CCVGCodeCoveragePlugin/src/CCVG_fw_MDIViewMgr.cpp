// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWMDIViewMgr implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Std:
#include <memory> // For std::unique_ptr

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewMgr.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewFactory.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewBase.h>

namespace ccvg {
namespace fw {

/// @brief Class constructor. Copy the CCVG default operational parameter values
///        to the user entered equivalent parameters.
CCVGFWMDIViewMgr::CCVGFWMDIViewMgr()
: m_pMDIViewFactory(nullptr)
{
}

/// @brief Class destructor. Tidy up release resources used by *this instance.
CCVGFWMDIViewMgr::~CCVGFWMDIViewMgr()
{
  Shutdown();
}

/// @brief  Package initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewMgr::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  m_mapMapCCVGItemIdToViewWgt = new MapCCVGItemIdToViewWgt_t;
  m_mapMapCCVGItemIdToView = new MapCCVGItemIdToView_t;
  m_mapMapViewWgtToView = new MapViewWgtToView_t;

  const status status = CCVGFWMDIViewFactory::Instance().Initialise();
  if (status == failure)
  {
    const wchar_t *pErr = CCVGFWMDIViewFactory::Instance().ErrorGetDescriptionW();
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewMgr_string_err_fail_init), pErr));
  }
  m_pMDIViewFactory = &CCVGFWMDIViewFactory::Instance();

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Package shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewMgr::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  // Tidy up
  ViewDeleteAll();
  if (m_mapMapCCVGItemIdToView != nullptr)
  {
    delete m_mapMapCCVGItemIdToView;
    m_mapMapCCVGItemIdToView = nullptr;
  }
  if (m_mapMapCCVGItemIdToViewWgt != nullptr)
  {
    delete m_mapMapCCVGItemIdToViewWgt;
    m_mapMapCCVGItemIdToViewWgt = nullptr;
  }
  if (m_mapMapViewWgtToView)
  {
    delete m_mapMapViewWgtToView;
    m_mapMapViewWgtToView = nullptr;
  }
  
  status status = success;
  if (m_pMDIViewFactory != nullptr)
  {
    status = m_pMDIViewFactory->Shutdown();
    if (status == failure)
    {
      const wchar_t *pErr = m_pMDIViewFactory->ErrorGetDescriptionW();
      ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewMgr_string_err_fail_shtdwn), pErr));
    }
    m_pMDIViewFactory = nullptr;
  }

  m_bBeenShutdown = true;

  return status;
}

/// @brief Determine if a CCVG view with specific ID exists, the view object
///        exists.
/// @param[in] The unique ID for the view.
/// @return bool True = Exists, false = does not exist.
bool CCVGFWMDIViewMgr::ViewIsExist(const gtString &vrViewId)
{
  const MapCCVGItemIdToView_t::const_iterator it = m_mapMapCCVGItemIdToView->find(vrViewId);
  return (it != m_mapMapCCVGItemIdToView->cend());
}

/// @brief Delete the CCVG view with specific ID exists.
/// @param[in] The unique ID for the view.
/// @return status success = task ok, false = fail to find or delete the
///         view object.
status CCVGFWMDIViewMgr::ViewDelete(const gtString &vrViewId)
{
  if (ViewIsExist(vrViewId))
  {
    ViewClose(vrViewId);

    // Actual QWidget/CCVGFWMDIViewBase object deletion is handled by the 
    // framework 

    const MapCCVGItemIdToViewWgt_t::const_iterator it = m_mapMapCCVGItemIdToViewWgt->find(vrViewId);
    m_mapMapViewWgtToView->erase((*it).second);
    m_mapMapCCVGItemIdToView->erase(vrViewId);
    m_mapMapCCVGItemIdToViewWgt->erase(vrViewId);
    return success;
  }

  return failure;
}

/// @brief Retrieve the CCVGFWMDIViewBase sub-class pointer to the derived 
///        view object.
///        The view object has multiple inheritance. Use the MDIViewMgr to 
///        retrieve the derived view object. This pointer is used by the CCVG 
///        MDI handling.
/// @param[in] vrForThisWgt The other sub-class to the view class.
/// @return CCVGFWMDIViewBase* Pointer to the view, NULL = not found.
CCVGFWMDIViewBase* CCVGFWMDIViewMgr::ViewGetSubClass(QWidget &vrForThisWgt) const
{
  const MapViewWgtToView_t::const_iterator it = m_mapMapViewWgtToView->find(&vrForThisWgt);
  if (it != m_mapMapViewWgtToView->cend())
  {
    CCVGFWMDIViewBase *pView = (*it).second;
    return pView;
  }

  return nullptr;
}

/// @brief Remove all records of the view objects. The views are not
///        closed or deleted by this function.
void CCVGFWMDIViewMgr::ViewDeleteAll()
{
  // Actual QWidget/CCVGFWMDIViewBase object deletion is handled by the 
  // framework 
  if (m_mapMapCCVGItemIdToView != nullptr)
  {
    m_mapMapCCVGItemIdToView->clear();
  }
  if (m_mapMapCCVGItemIdToViewWgt != nullptr)
  {
    m_mapMapCCVGItemIdToViewWgt->clear();
  }
  if (m_mapMapViewWgtToView)
  {
    m_mapMapViewWgtToView->clear();
  }
}

/// @brief Add the derived view's sub-class pointers to *this manager's records
///        to register a view with *this view manager and for quick retrival. 
///        The view object has multiple inheritance. Use the MDIViewMgr to 
///        retrieve the derived view object. This pointer is used by the CCVG 
///        MDI handling.
/// @param[in] vrViewId The unique ID for the view.
/// @param[in] vNewViewWgt The QWidget sub-class to the view class.
/// @param[in] CCVGFWMDIViewBase The CCVGFWMDIViewBase sub-class to the view class.
/// @return status success = recorded, false = invalid ID or ID exists already.
status CCVGFWMDIViewMgr::ViewAdd(const gtString &vrViewId, QWidget &vNewViewWgt, CCVGFWMDIViewBase &vNewView)
{
  if (vrViewId.isEmpty())
  {
    return failure;
  }
  if (!ViewIsExist(vrViewId))
  {
    const MapPairCCVGItemIdToViewWgt_t pair(vrViewId, &vNewViewWgt);
    m_mapMapCCVGItemIdToViewWgt->insert(pair);
    const MapPairCCVGItemIdToView_t pair2(vrViewId, &vNewView);
    m_mapMapCCVGItemIdToView->insert(pair2);
    const MapPairViewWgtToView_t pair3(&vNewViewWgt, &vNewView);
    m_mapMapViewWgtToView->insert(pair3);
    return success;
  }
  return success;
}

/// @brief Retrieve the QWidget sub-class pointer to the derived view object.
///        The view object has multiple inheritance. Use the MDIViewMgr to 
///        retrieve the derived view object. This pointer is used by the CodeXL
///        framework.
/// @param[in] vrViewId The unique ID for the view.
/// @return QWidget* Pointer to the view, NULL = not found.
::QWidget* CCVGFWMDIViewMgr::ViewGetWidget(const gtString &vrViewId)
{
  const MapCCVGItemIdToViewWgt_t::const_iterator it = m_mapMapCCVGItemIdToViewWgt->find(vrViewId);
  if (it != m_mapMapCCVGItemIdToViewWgt->cend())
  {
    ::QWidget *pWgt = (*it).second;
    return pWgt;
  }

  return nullptr;
}

/// @brief Retrieve the CCVGFWMDIViewBase sub-class pointer to the derived 
///        view object.
///        The view object has multiple inheritance. Use the MDIViewMgr to 
///        retrieve the derived view object. This pointer is used by the CCVG
///        MDI view code.
/// @param[in] vrViewId The unique ID for the view.
/// @return QWidget* Pointer to the view, NULL = not found.
CCVGFWMDIViewBase* CCVGFWMDIViewMgr::ViewGet(const gtString &vrViewId)
{
  const MapCCVGItemIdToView_t::const_iterator it = m_mapMapCCVGItemIdToView->find(vrViewId);
  if (it != m_mapMapCCVGItemIdToView->cend())
  {
    CCVGFWMDIViewBase *pView = (*it).second;
    return pView;
  }

  return nullptr;
}

/// @brief Close the CCVG view with specific ID.
/// @param[in] The unique ID for the view.
/// @return status success = task ok, false = fail to find or close the
///         view.
status CCVGFWMDIViewMgr::ViewClose(const gtString &vrViewId)
{
  if (ViewIsExist(vrViewId))
  {
    // Todo: close the view
    return success;
  }

  return failure;
}

/// @brief  Create a view and fill it with content.
/// @param[in] vkViewId View's unique ID formatted as 
///            "<CVGItemId>,<File type>".
/// @param[in] vkViewType View enumeration.
/// @param[out] vpViewWgt Pointer to a new Qt derived widget object. vpViewWgt
///             is the same object pointed by vpView.
/// @param[out] vpView Pointer to a new CCVG MDI view object. vpViewWgt
///             is the same object pointed by vpView.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewMgr::ViewCreate(const gtString &rViewId, EnumCCVGMDIViewType veViewType, QWidget *vpQParent, ::QWidget *&vpViewWgt, CCVGFWMDIViewBase *&vpView)
{
  vpViewWgt = nullptr;
  vpView = nullptr;

  QWidget *pNewViewWgt = nullptr;
  CCVGFWMDIViewBase *pNewView = nullptr;
  status status = m_pMDIViewFactory->ViewCreate(rViewId, veViewType, pNewViewWgt, pNewView);
  if (status == failure)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewMgr_err);
    gtString errMsg;
    errMsg.appendFormattedString(pErrMsg, m_pMDIViewFactory->ErrorGetDescription().asCharArray());
    return ErrorSet(errMsg);
  }
  std::unique_ptr<CCVGFWMDIViewBase> pObj(pNewView);
  
  if (status == success)
  {
    pNewView->SetParent(vpQParent);
    pNewView->SetDoNotDeleteThisFlag(true); // CodeXL MDI framework handles deletion 
    status = pNewView->SetViewId(rViewId);
    if(status != success)
    {
      const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewMgr_err);
      gtString errMsg;
      errMsg.appendFormattedString(pErrMsg, pNewView->ErrorGetDescription().asCharArray());
      return ErrorSet(errMsg);
    }
    status = pNewView->Initialise(); // Add data to view
    if (status != success)
    {
      const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_pkg_fw_MDIViewMgr_err);
      gtString errMsg;
      errMsg.appendFormattedString(pErrMsg, pNewView->ErrorGetDescription().asCharArray());
      return ErrorSet(errMsg);
    }
    status = ViewAdd(rViewId, *pNewViewWgt, *pNewView);
    if (status == success)
    {
      pObj.release();
      vpViewWgt = pNewViewWgt;
      vpView = pNewView;
    }
  }

  return status;
}

/// @brief Validate the MDI view's ID.
/// @param[in] vViewId Text formatted ID formatted as 
///            "<CVGItemId>,<File type>".
/// @return bool True = valid, false = not valid.
bool CCVGFWMDIViewMgr::IsValidViewId(const gtString &vViewId)
{
  if (vViewId.isEmpty())
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_fw_MDIView_err_invalidViewId), vViewId.asCharArray()));
  }

  return true;
}

} // namespace fw
} // namespace ccvg
