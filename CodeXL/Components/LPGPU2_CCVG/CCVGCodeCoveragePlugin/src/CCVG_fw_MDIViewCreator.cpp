// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGMDIViewCreator implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Qt:
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewCreator.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIGeneralActionsCreator.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewMgr.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_MsgDlg.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesIcon.h>

namespace ccvg {
namespace fw {

// Instantiations:
gtString CCVGMDIViewCreator::ms_constThisCreatorId = L"CCVG_MDIViews";

/// @brief    Class constructor. No work is done here by this class.
CCVGMDIViewCreator::CCVGMDIViewCreator()
: m_debugThisObIdStr(L"CCVGMDIViewCreator")
, m_bBeenInitialised(false)
, m_bBeenShutdown(false)
, m_pViewActionCreator(nullptr)
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGMDIViewCreator::~CCVGMDIViewCreator()
{
}

/// @brief  Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGMDIViewCreator::Initialise()
{
  if (m_bBeenInitialised)
  {
    return success;
  }

  // Create the view actions creator:
  m_pViewActionCreator = new (std::nothrow) CCVGMDIGeneralActionsCreator();
  GT_IF_WITH_ASSERT(m_pViewActionCreator == nullptr)
  {
    LogError(CCVGRSRC(KIDS_CCVG_STR_gui_MDIViewCreator_err_fail_init), L"");
    return failure;
  }
  m_pViewActionCreator->initializeCreator();

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Class shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGMDIViewCreator::Shutdown()
{
  if (m_bBeenShutdown)
  {
    return m_bBeenShutdown;
  }

  // Tidy up
  if (m_pViewActionCreator != nullptr)
  {
    delete m_pViewActionCreator;
    m_pViewActionCreator = nullptr;
  }

  m_bBeenShutdown = true;

  return m_bBeenShutdown;
}

/// @brief Static function. Retrieve the ID for *this MDI view creator. 
///        It must be unique to *this creator.
/// @return gtString& Text ID.
const gtString& CCVGMDIViewCreator::GetId()
{
  return ms_constThisCreatorId;
}

/// @brief Determine if a CCVG view with specific ID exists, the view object
///        exists.
/// @param[in] The unique ID for the view.
/// @return bool True = Exists, false = does not exist.
bool CCVGMDIViewCreator::ViewIsExist(const gtString &vrViewId)
{
  return CCVGFWMDIViewMgr::Instance().ViewIsExist(vrViewId);
}

/// @brief Delete the CCVG view with specific ID exists.
/// @param[in] The unique ID for the view.
/// @return status success = task ok, false = fail to find or delete the
///         view object.
status CCVGMDIViewCreator::ViewDelete(const gtString &vrViewId)
{
  return CCVGFWMDIViewMgr::Instance().ViewDelete(vrViewId);
}

/// @brief Remove all records of the view objects. The views are not
///        closed or deleted by this function.
void CCVGMDIViewCreator::ViewDeleteAll()
{
  CCVGFWMDIViewMgr::Instance().ViewDeleteAll();
}

/// @brief Retrieve the QWidget sub-class pointer to the derived view object.
///        The view object has multiple inheritance. Use the MDIViewMgr to 
///        retrieve the derived view object. This pointer is used by the CodeXL
///        framework.
/// @param[in] vrViewId The unique ID for the view.
/// @return QWidget* Pointer to the view, NULL = not found.
::QWidget* CCVGMDIViewCreator::ViewGet(const gtString &vrViewId)
{
  return CCVGFWMDIViewMgr::Instance().ViewGetWidget(vrViewId);
}

/// @brief Close the CCVG view with specific ID.
/// @param[in] The unique ID for the view.
/// @return status success = task ok, false = fail to find or close the
///         view.
status CCVGMDIViewCreator::ViewClose(const gtString &vrViewId)
{
  return CCVGFWMDIViewMgr::Instance().ViewClose(vrViewId);
}

/// @brief      Overidden. Get the title or command title associated with *this
///             view.
/// @param[in]  vViewIndex View unique ID associated with a view. Not used.
/// @param[out] vViewTitle User facing view title text.
/// @param[out] vViewMenuCommand User facing menu item action text.
void CCVGMDIViewCreator::titleString(int vViewIndex, gtString &vViewTitle, gtString &vViewMenuCommand)
{
  GT_UNREFERENCED_PARAMETER(vViewIndex);

  const apMDIViewCreateEvent *pCurrentEvent = static_cast<apMDIViewCreateEvent*>(_pCreationEvent);
  const gtString &rViewId(pCurrentEvent->viewTitle());
  vViewTitle = rViewId;
  vViewMenuCommand = rViewId;
}

/// @brief Get the associated toolbar ID string for the view. When the view is 
///        is visible this toolbar will be displayed.
/// @param[in] vViewIndex View unique ID associated with a view.
/// @return gtString Name of the toolbar, empty string = no toolbar.
gtString CCVGMDIViewCreator::associatedToolbar(int vViewIndex)
{
  GT_UNREFERENCED_PARAMETER(vViewIndex);
  return L"";
}

/// @brief Get view type. The type defines whether the view is a MDI or dock
///        type view. See function dockArea() if the view is a dock view.
/// @param[in] vViewIndex View unique ID associated with a view.
/// @return    afViewType Enumeration view type.
afViewCreatorAbstract::afViewType CCVGMDIViewCreator::type(int vViewIndex)
{
  GT_UNREFERENCED_PARAMETER(vViewIndex);
  return afViewCreatorAbstract::AF_VIEW_mdi;
}

/// @brief Get the docking area. Return the initial docking area of the view 
///        based on enumeration afDockAreaFlag. This function is not called
///        as the view is a MDI type. See function type().
/// @param[in] vViewIndex View unique ID associated with a view.
/// int afDockingAreaFlag enumeration, 0 = not defined/ignore.
int CCVGMDIViewCreator::dockArea(int vViewIndex)
{
  GT_UNREFERENCED_PARAMETER(vViewIndex);
  return 0;
}

///@brief Retrieve the dockable widget's features for the specified view. It  
///       is recommended to return QDockWidget::AllDockWidgetFeatures.
///       This function is not called as the view is a MDI type. See 
///       function type().
/// @param[in] vViewIndex View unique ID associated with a view. 
/// @return DockWidgetFeatures enumeration, 0 = no dock features.
QDockWidget::DockWidgetFeatures CCVGMDIViewCreator::dockWidgetFeatures(int vViewIndex)
{
  GT_UNREFERENCED_PARAMETER(vViewIndex);
  return 0;
}

/// @brief If the dock view is a dockable view and we want to dock it with a
///        another view then return the view ID it is to dock with. If the 
///        ID contains a prefix of '-' the view will open below the indicated
///        view.
// gtString dockWith(int vViewIndex)

/// @brief Retrieve whether *this view creator creates only MDI type views. The 
///        creator only creates views during run time.
/// @return bool True = only MDI views no dock type views, false = can mix.
bool CCVGMDIViewCreator::isDynamic()
{ 
  return true; 
}

/// @brief Retrieve the MDI ID to match the (the event type)
///        apMDIViewCreateEvent::CreatedMDIType. When opening an MDI view it 
///        will know it belongs to *this creator. Re-implement this function to
///        match apMDIViewCreateEvent::CreatedMDIType.
/// @return gtString Text ID.
const gtString CCVGMDIViewCreator::CreatedMDIType() const
{ 
  return L"CCVG_MDIViews";
}

/// @brief Retrieve the initial size for dockable views only. However if the 
///        controls require a larger size the size will be changed externally.
///        This function is not called as the view is a MDI type. See 
///        function type().
/// @param[in] vViewIndex View unique ID associated with a view. 
/// @return QSize Initial dimensions.
QSize CCVGMDIViewCreator::initialSize(int vViewIndex)
{
  GT_UNREFERENCED_PARAMETER(vViewIndex);
  return QSize();
}

/// @brief Retrieve the initial visibility flag.
/// @param[in] vViewIndex View unique ID associated with a view. 
/// @return bool True = visible, false = not visible.
bool CCVGMDIViewCreator::visibility(int vViewIndex)
{
  GT_UNREFERENCED_PARAMETER(vViewIndex);
  return true;
}

/// @brief Get number of views that can be created by this creator.
/// @return int 1 to n, 0 = no views.
int CCVGMDIViewCreator::amountOfViewTypes()
{
  return kCCVGMDIViewTypeType_count - 1;
}

/// @brief Handle a menu item action to likely fire off a command.
/// @param[in] vViewIndex View unique ID associated with a view. 
/// @param[in] vActionIndex Action ID related to a commmand. 
void CCVGMDIViewCreator::handleTrigger(int vViewIndex, int vActionIndex)
{
  GT_UNREFERENCED_PARAMETER(vViewIndex);
  GT_UNREFERENCED_PARAMETER(vActionIndex);
}

/// @brief Handle UI update for a specified view.
/// @param[in] vViewIndex View unique ID associated with a view. 
/// @param[in] vActionIndex Action ID related to a commmand. 
void CCVGMDIViewCreator::handleUiUpdate(int vViewIndex, int vActionIndex)
{
  GT_UNREFERENCED_PARAMETER(vViewIndex);
  GT_UNREFERENCED_PARAMETER(vActionIndex);
}

/// @brief Overridden. Create the context for the view. This is the core of
///        the creation of *this creator where QWidget itself is created. 
///        Create the inner view. 
///        This function is normally reach by an event. If error occurs
///        display error message dialog here.
/// @param[in]  vViewIndex View unique ID associated with a view (not used).
/// @param[out] A new widget object, the view.
/// @param[in] The parent of the new widget.
/// @return bool True = success, false = failure.
bool CCVGMDIViewCreator::createViewContent(int vViewIndex, QWidget *&vpContentQWidget, QWidget *vpQParent)
{
  GT_UNREFERENCED_PARAMETER(vViewIndex);
  
  vpContentQWidget = nullptr;

  apMDIViewCreateEvent *pCCVGMDIEvent = nullptr;
  bool bOk = (_pCreationEvent != nullptr);
  bOk = bOk && (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT);
  bOk = bOk && ((pCCVGMDIEvent = dynamic_cast<apMDIViewCreateEvent *>(_pCreationEvent)) != nullptr);
  if (!bOk)
  {
    const wchar_t *pErrMsg = CCVGRSRC(KIDS_CCVG_STR_fw_MDIViewCreator_err_failCreateViewCodeLogic);
    ErrorSet(pErrMsg);
    CCVGUtilMsgDlg::Show(pErrMsg);
    LogError(pErrMsg, L"");
    return false;
  }

  const gtString &rViewId(pCCVGMDIEvent->viewTitle());
  QWidget *pContentQWidget = nullptr;
  CCVGFWMDIViewBase *pView = nullptr;
  const EnumCCVGMDIViewType eView = static_cast<EnumCCVGMDIViewType>(pCCVGMDIEvent->viewIndex());
  status status = CCVGFWMDIViewMgr::Instance().ViewCreate(rViewId, eView, vpQParent, pContentQWidget, pView);
  if (status == success)
  {
    vpContentQWidget = pContentQWidget;
  }
  else
  {
    const gtString &rErrMsg(CCVGFWMDIViewMgr::Instance().ErrorGetDescription());
    ErrorSet(rErrMsg);
    CCVGUtilMsgDlg::Show(rErrMsg);
    LogError(rErrMsg.asCharArray(), L"");
  }

  // View created and with valid viewId, now safe to load its content:
  auto pViewSource = dynamic_cast<CCVGFWMDIViewBase*>(ViewGet(rViewId));
  status = (pViewSource == nullptr) ? failure : pViewSource->ReloadContent();
  
  if (status == failure)
  {
    const gtString &rErrMsg(pViewSource->ErrorGetDescription());
    ErrorSet(rErrMsg);
    CCVGUtilMsgDlg::Show(rErrMsg);
    LogError(rErrMsg.asCharArray(), L"");
  }
  
  return bOk && ((status == success) ? true : false);
}

/// @brief  Log error message.
/// @param[in] wchar_t* General error description text. 
/// @param[in] wchar_t* Context error description text. 
void CCVGMDIViewCreator::LogError(const wchar_t *vpErrMsg, const wchar_t *vpErrCntxt)
{
  gtString msg;
  msg.appendFormattedString(L"CCVG %ls. %ls", vpErrMsg, vpErrCntxt);
  OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_ERROR);
}

/// @brief Check with *this creator if a specific MDI view exists already or
///        that a view needs to be created.
/// @param[in] CodeXL event information object.
/// @return bool True = exists, false = does not exist.
bool CCVGMDIViewCreator::displayExistingView(const apMDIViewCreateEvent &vMdiViewEvent)
{
  const gtString &rViewId(vMdiViewEvent.viewTitle()); // View title changed purpose
  return ViewIsExist(rViewId);
}

/// @brief Retrieve the ID, the file path is used as ID, for a specific view. 
///        The ID is used extensively by the framework when searching for a 
///        MDI sub window widget.
/// @param[out] vFilePath A file path or unique text string.
/// @return bool True = success, false = failure.
bool CCVGMDIViewCreator::getCurrentlyDisplayedFilePath(osFilePath &vFilePath)
{
  const apMDIViewCreateEvent *pCurrentEvent = static_cast<apMDIViewCreateEvent*>(_pCreationEvent);
  const gtString &rViewId(pCurrentEvent->viewTitle());
  vFilePath.setFullPathFromString(rViewId);
  return true;
}

/// @brief Handle sub-window close. If the window belongs to *this creator then
///        remove record of it and release associated resources. Normally 
///        visited by an event. 
/// @return bool Return false (other code does this too)
bool CCVGMDIViewCreator::onMDISubWindowClose(afQMdiSubWindow *vpMDISubWindow) 
{
  // Sanity check:
  GT_IF_WITH_ASSERT(vpMDISubWindow != nullptr)
  {
    // Get the sub window widget
    QWidget *pWidget = vpMDISubWindow->widget();
    if(pWidget != nullptr)
    {
      CCVGFWMDIViewMgr &rMgr(CCVGFWMDIViewMgr::Instance());
      const CCVGFWMDIViewBase *pView = rMgr.ViewGetSubClass(*pWidget);
      if (pView == nullptr)
      {
        // View does not belong to CCVG
        return false;
      }

      const gtString &rCCVGItemId(pView->GetViewId());
      rMgr.ViewDelete(rCCVGItemId);
    }
  }

  return false;
}

/// @brief     Get the icon that represents the a specified view.
/// @param[in] vViewIndex Unique view's ID.
/// @return QPixmap* Pointer to an an icon image.
QPixmap* CCVGMDIViewCreator::iconAsPixmap(int vViewIndex)
{
  GT_UNREFERENCED_PARAMETER(vViewIndex);

  return CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeCCVGItem);
}

/// @brief Get the execution mode of the view. The views show or hide menu 
///        items that are grouped together by the 'execution' mode such as 
///        'Debug' or 'Profile'. If no mode is defined the the view menu item
///         appears directly under the "View" menu. This function returns which
///        group *this view should be added to. See also function 
///        modeMenuPosition().
/// @param[in] vViewIndex  Unique view's ID.
/// @return gtString The group item's name or empty string if no mode/group. 
// gtString executionMode(int vViewIndex)

/// @brief Get the menu position in the menu view. Should an 'execution' mode 
///        be supplied (see function executionMode()) a sub-menu can also be 
///        provided for the execution mode menu. 
/// @param[in]  vViewIndex Unique view's ID.
/// @param[out] vPositionData Not use for future use.
/// @return gtString Menu item's name to be positioned by. 
// gtString modeMenuPosition(int vViewIndex, afActionPositionData &vPositionData) 

/// @brief For future use.
/// @return bool Must return true.
// bool createQTWrapping(int vViewIndex, QMainWindow *vpWnd)

/// @brief If there is a need to create a wrapping QWidget for a MDI view then
///        use this function. It can be used if the creator creates different
///        QWidgets that need to be accessed by a single known wrapper QWidget.
/// @param[in]  vViewIndex Unique view's ID.
/// @return QWidget* A pointer to a wrapper QWidget.
// QWidget* createQtWidgetWrapper(int vViewIndex)

/// @brief Make sure the specified view is active (raised and visible in case
///        it has overlapping with sibling views.
/// @param[in] vViewIndex View unique ID associated with a view. 
/// @return bool True = active, false = not active.
// bool initiallyActive(int vViewIndex);

} // namespace fw
} // namespace ccvg
