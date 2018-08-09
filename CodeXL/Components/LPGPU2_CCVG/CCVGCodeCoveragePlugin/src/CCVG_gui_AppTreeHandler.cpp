// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGGUIAppTreeHandler implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Std:
#include <memory> // For std::unique_ptr

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Framework:
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_AppTreeHandler.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesIcon.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_AppTreeDataExtn.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGAppWrapper.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadAll.h>

// Declarations:
namespace ccvg { namespace cmd { extern status ExecuteCmdFilesReadAll(); } }
namespace ccvg { namespace cmd { extern status ExecuteCmdWndSrcOpenDriverFile(const gtString &vrFileName); } }
namespace ccvg { namespace cmd { extern status ExecuteCmdWndSrcOpenSourceFile(const gtString &vrFileName); } }
namespace ccvg { namespace cmd { extern status ExecuteCmdWndSrcOpenResultFile(const gtString &vrFileName); } }
namespace ccvg { namespace cmd { extern status ExecuteCmdWndSrcOpenExecFile(const gtString &vrFileName); } }


namespace ccvg {
namespace gui {

/// @brief    Class constructor. No work is done here by this class.
CCVGGUIAppTreeHandler::CCVGGUIAppTreeHandler()
: m_pApplicationTree(nullptr)
, m_bRegisterAppTreeHandlerViaSigSlot(false)
, m_pProgramsRootNode(nullptr)
, m_pCCVGItemsRootNodeGood(nullptr)
, m_pCCVGItemsRootNodeBad(nullptr)
, m_bCCVGItemsSortedInTree(false)
, m_pActionCCVGItemsRefreshAll(nullptr)
, m_pActionCCVGItemViewCoverageResults(nullptr)
, m_pActionCCVGItemDriverSrcCodeView(nullptr)
, m_pActionCCVGItemResultFileView(nullptr)
, m_pActionCCVGItemExecFileView(nullptr)
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGGUIAppTreeHandler::~CCVGGUIAppTreeHandler()
{
  Shutdown();
}

/// @brief  Package initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGGUIAppTreeHandler::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  afApplicationCommands *pApplicationCommands = afApplicationCommands::instance();
  GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
  {
    m_pApplicationTree = pApplicationCommands->applicationTree();
    if (m_pApplicationTree != nullptr)
    {
      // Should add a signal to main window that the tree was created, and 
      // register all tree handler when handling this signal
      OnApplicationTreeCreated();
    }
    else
    {
      // The tree has not been created yet, connect to the signal so know when 
      // it was created and register then
      m_bRegisterAppTreeHandlerViaSigSlot = true;
      const bool rc = connect(&afQtCreatorsManager::instance(), SIGNAL(ApplicationTreeCreated()), this, SLOT(OnApplicationTreeCreated()));
      GT_ASSERT(rc);
    }
  }

  // Register as an events observer
  apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_MANAGERS_EVENTS_HANDLING_PRIORITY);

  // Register for CCVG internal events
  bool bOk = true;
  fw::CCVGFWTheCurrentState &rCurrState(fw::CCVGFWTheCurrentState::Instance());
  bOk = (success == rCurrState.RegisterForNotificationOnSettingChangedCcvgAllProjectsEnabledFlag(*this));
  fw::CCVGFWTheCurrentStateProjectData &rProjData(rCurrState.GetProjectData());
  bOk = bOk && (success && rProjData.RegisterForNotificationOnProjectDataChanged(*this));
  // TODO: This #if check prevents a build failure when the CCVG plugin is not set to be compiled in.
  // This is a hack. The root cause should be investigated and fixed.
#if LPGPU2_CCVG_COMPILE_IN
  CCVGGUIProjectSettings &rProjSetting(CCVGAppWrapper::Instance().GetGuiProjSettings());
  bOk = bOk && (success && rProjSetting.RegisterForNotificationOnProjectClose(*this));
#endif

  // Create context menu actions and link to do functions
  bOk = bOk && (success && ContextMenuCreateActions());

  m_bBeenInitialised = bOk;

  return bOk ? success : failure;
}

/// @brief Create context menu QAction objects for the CCVG tree view. They
///        form the entries in a context menu. 
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGGUIAppTreeHandler::ContextMenuCreateActions()
{
  bool bOk = true;

  std::unique_ptr<QAction> pActionRefresh(new (std::nothrow) QAction("dummy", m_pApplicationTree));
  if (pActionRefresh == nullptr)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_err_creatingActionObjs);
    return ErrorSet(pErrMsg);
  }
  const QString labelSrcCode(acGTStringToQString(CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewSrcCode)));
  std::unique_ptr<QAction> pActionCCVGItemViewCoverageResults(new (std::nothrow) QAction(labelSrcCode, m_pApplicationTree));
  if (pActionCCVGItemViewCoverageResults == nullptr)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_err_creatingActionObjs);
    return ErrorSet(pErrMsg);
  }
  const QString labelDrvCode(acGTStringToQString(CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewDriverCode)));
  std::unique_ptr<QAction> pActionCCVGItemDriverSrcCodeView(new (std::nothrow) QAction(labelDrvCode, m_pApplicationTree));
  if (pActionCCVGItemDriverSrcCodeView == nullptr)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_err_creatingActionObjs);
    return ErrorSet(pErrMsg);
  }
  const QString labelResFile(acGTStringToQString(CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewResultFile)));
  std::unique_ptr<QAction> pActionCCVGItemResultFileView(new (std::nothrow) QAction(labelResFile, m_pApplicationTree));
  if (pActionCCVGItemResultFileView == nullptr)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_err_creatingActionObjs);
    return ErrorSet(pErrMsg);
  }
  const QString labelExeFile(acGTStringToQString(CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewExecFile)));
  std::unique_ptr<QAction> pActionCCVGItemExecFileView(new (std::nothrow) QAction(labelExeFile, m_pApplicationTree));
  if (pActionCCVGItemExecFileView == nullptr)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_err_creatingActionObjs);
    return ErrorSet(pErrMsg);
  }
  
  bOk = bOk && connect(pActionRefresh.get(), SIGNAL(triggered()), this, SLOT(ActionDoRefreshCCVGItemsReadFiles()));
  bOk = bOk && connect(pActionCCVGItemViewCoverageResults.get(), SIGNAL(triggered()), this, SLOT(ActionDoCCVGItemViewCoverageResults()));
  bOk = bOk && connect(pActionCCVGItemDriverSrcCodeView.get(), SIGNAL(triggered()), this, SLOT(ActionDoCCVGItemDriverSrcCodeView()));
  bOk = bOk && connect(pActionCCVGItemResultFileView.get(), SIGNAL(triggered()), this, SLOT(ActionDoCCVGItemResultFileView()));
  bOk = bOk && connect(pActionCCVGItemExecFileView.get(), SIGNAL(triggered()), this, SLOT(ActionDoCCVGItemExecFileView()));
  if (bOk)
  {
    m_pActionCCVGItemsRefreshAll = pActionRefresh.release();
    m_pActionCCVGItemViewCoverageResults = pActionCCVGItemViewCoverageResults.release();
    m_pActionCCVGItemDriverSrcCodeView = pActionCCVGItemDriverSrcCodeView.release();
    m_pActionCCVGItemResultFileView = pActionCCVGItemResultFileView.release();
    m_pActionCCVGItemExecFileView = pActionCCVGItemExecFileView.release();
  }

  return success;
}

/// @brief  Package shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGGUIAppTreeHandler::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  status ok = TreeClear();
  
  if (m_bRegisterAppTreeHandlerViaSigSlot)
  {
    const bool rc = disconnect(&afQtCreatorsManager::instance(), SIGNAL(ApplicationTreeCreated()), this, SLOT(OnApplicationTreeCreated()));
    GT_ASSERT(rc);
  }

  m_pProgramsRootNode = nullptr;
  m_pApplicationTree = nullptr;
  m_pCCVGItemsRootNodeGood = nullptr;
  m_pCCVGItemsRootNodeBad = nullptr;
  m_bBeenShutdown = true;

  return ok;
}


/// @brief Overridden. Build a context menu for a requested item or set of
///        selected items.
/// @param[in]  vContextMenuItemsList List selected tree nodes.
/// @param[out] vrwMenu The new or udated context menu object.
/// @return bool True = There is a context menu for the listed items, false =
///         no list. bool - Success / failure.
bool CCVGGUIAppTreeHandler::BuildContextMenuForItems(const gtList<const afApplicationTreeItemData *> vContextMenuItemsList, QMenu &vrwMenu)
{
  bool bOk = false;

  if (!GetCCVGExecModeEnabled() || !GetCCVGExecModeCurrent() || 
      !GetCCVGIsProjectOpen() || vContextMenuItemsList.empty())
  {
    return false;
  }

  if (vContextMenuItemsList.size() == 1)
  {
    // Build the context menu item for a single item
    const afApplicationTreeItemData *pItemData = vContextMenuItemsList.front();
    bOk = ContextMenuBuildForSingleItem(pItemData, vrwMenu);
  }
  else
  {
    bOk = false;
  }

  return true;
}

/// @brief Build a context menu for a single item.
/// @param[in]  vpItemData Data extension object attached to a tree item.
/// @param[out] vrwMenu The new or updated context menu object.
/// @return bool True = success, false = failure.
bool CCVGGUIAppTreeHandler::ContextMenuBuildForSingleItem(const afApplicationTreeItemData *vpItemData, QMenu &vrwMenu)
{
  bool bOk = false;

  GT_IF_WITH_ASSERT(nullptr != vpItemData)
  {
    switch (vpItemData->m_itemType)
    {
      case AF_TREE_ITEM_APP_ROOT:
      {
        return ContextBuildMenuRootNode(vrwMenu);
      }
      case AF_TREE_ITEM_CCVG_ROOTNODE_SETFILESGOOD:
      {
        return ContextBuildMenuCCVGRootNodeSetFilesGood(vrwMenu);
      }
      case AF_TREE_ITEM_CCVG_CCVGITEM_RESULTS:
      {
        return ContextBuildMenuCCVGItemResult(vrwMenu);
      }
      case AF_TREE_ITEM_CCVG_CCVGITEM:
      case AF_TREE_ITEM_CCVG_CCVGITEM_SOURCE:
      {
        return ContextBuildMenuCCVGItem(vrwMenu);
      }
      case AF_TREE_ITEM_CCVG_CCVGITEM_EXECUTEABLE:
      {
        return ContextBuildMenuCCVGItemExec(vrwMenu);
      }
      case AF_TREE_ITEM_CCVG_CCVGITEM_DRIVERS:
      {
        return ContextBuildMenuCCVGItemDrivers(vrwMenu);
      }
      default:
        return false;
    }
  }

  return bOk;
}

/// @brief Build a context menu for root node of the application tree control.
///        It may be shared with other menu items external to the CCVG plugin.
/// @param[out] vrwMenu The new or updated context menu object.
/// @return bool True = success, false = failure.
bool CCVGGUIAppTreeHandler::ContextBuildMenuRootNode(QMenu &vrwMenu)
{
  // Need to add this menu command before any other commands in the menu and
  // then add a separator, this is done a bit differently then just adding 
  // an action to the end of the menu
  QAction *pSeparator = new (std::nothrow) QAction(nullptr);
  if (pSeparator == nullptr) 
  {
    GT_IF_WITH_ASSERT(false) { ; }
    return false;
  }
  pSeparator->setSeparator(true);
  QList<QAction *> menuActions = vrwMenu.actions();
  QAction *pFirstAction = (menuActions.count() != 0) ? pFirstAction = menuActions[0] : nullptr;
  vrwMenu.insertAction(pFirstAction, pSeparator);
  vrwMenu.insertAction(pSeparator, m_pActionCCVGItemsRefreshAll);

  // Change update menu text
  const wchar_t *pTxt = GetCCVGIsProjectEmpty() ? CCVGRSRC(kIDS_CCVG_STR_project_loadResultsFiles) 
                                                : CCVGRSRC(kIDS_CCVG_STR_project_refreshResultsFiles);
  const QString txt = acGTStringToQString(gtString(pTxt));
  m_pActionCCVGItemsRefreshAll->setText(txt);
  
  return true;
}

/// @brief Build a context menu for CCVG root node Set Files all Good.
/// @param[out] vrwMenu The new or updated context menu object.
/// @return bool True = success, false = failure.
bool CCVGGUIAppTreeHandler::ContextBuildMenuCCVGRootNodeSetFilesGood(QMenu &vrwMenu)
{
  GT_UNREFERENCED_PARAMETER(vrwMenu);

  // Do nothing
  return true;
}

/// @brief Build a context menu for CCVGItem (root) node.
/// @param[out] vrwMenu The new or updated context menu object.
/// @return bool True = success, false = failure.
bool CCVGGUIAppTreeHandler::ContextBuildMenuCCVGItem(QMenu &vrwMenu)
{
  vrwMenu.addAction(m_pActionCCVGItemViewCoverageResults);
  return true;
}

/// @brief Build a context menu for CCVGItem Driver node.
/// @param[out] vrwMenu The new or updated context menu object.
/// @return bool True = success, false = failure.
bool CCVGGUIAppTreeHandler::ContextBuildMenuCCVGItemDrivers(QMenu &vrwMenu)
{
  const fw::CCVGFWTheCurrentStateProjectData::CCVGItem *pCCVGItem = GetSelectedItemDataCCVGItem();
  if (pCCVGItem != nullptr)
  {
    const bool bCCVGDriverFileValid = pCCVGItem->IsPresentCcvgd();
    m_pActionCCVGItemDriverSrcCodeView->setEnabled(bCCVGDriverFileValid);
    vrwMenu.addAction(m_pActionCCVGItemDriverSrcCodeView);
  }
  return true;
}

/// @brief Build a context menu for CCVGItem Results node.
/// @param[out] vrwMenu The new or updated context menu object.
/// @return bool True = success, false = failure.
bool CCVGGUIAppTreeHandler::ContextBuildMenuCCVGItemResult(QMenu &vrwMenu)
{
  const fw::CCVGFWTheCurrentStateProjectData::CCVGItem *pCCVGItem = GetSelectedItemDataCCVGItem();
  if (pCCVGItem != nullptr)
  {
    const bool bCCVGResultFileValid = pCCVGItem->IsPresentCcvgr();
    m_pActionCCVGItemResultFileView->setEnabled(bCCVGResultFileValid);
    vrwMenu.addAction(m_pActionCCVGItemResultFileView);
  }
  return true;
}

/// @brief Build a context menu for CCVGItem Executeable node.
/// @param[out] vrwMenu The new or updated context menu object.
/// @return bool True = success, false = failure.
bool CCVGGUIAppTreeHandler::ContextBuildMenuCCVGItemExec(QMenu &vrwMenu)
{
  const fw::CCVGFWTheCurrentStateProjectData::CCVGItem *pCCVGItem = GetSelectedItemDataCCVGItem();
  if (pCCVGItem != nullptr)
  {
    const bool bCCVGResultFileValid = pCCVGItem->IsPresentCcvge();
    m_pActionCCVGItemResultFileView->setEnabled(bCCVGResultFileValid);
    vrwMenu.addAction(m_pActionCCVGItemExecFileView);
  }
  return true;
}


/// @brief Retrieve the CCVG CCVGItem data object bound to the one or first 
///        selected tree view item in the tree control.
/// @return CCVGItem* CCVGItem data object pointer,
///         NULL = data object not present for the selected item.
const fw::CCVGFWTheCurrentStateProjectData::CCVGItem* CCVGGUIAppTreeHandler::GetSelectedItemDataCCVGItem()
{
  QTreeWidgetItem *pTreeItem = GetSelectedItemFirst();
  if (pTreeItem != nullptr)
  {
    afApplicationTreeItemData *pRetVal = m_pApplicationTree->getTreeItemData(pTreeItem);
    if (pRetVal != nullptr)
    {
      switch (pRetVal->m_itemType)
      {
      case AF_TREE_ITEM_CCVG_CCVGITEM:
        break;
      case AF_TREE_ITEM_CCVG_CCVGITEM_RESULTS:
      case AF_TREE_ITEM_CCVG_CCVGITEM_SOURCE:
      case AF_TREE_ITEM_CCVG_CCVGITEM_DRIVERS:
      case AF_TREE_ITEM_CCVG_CCVGITEM_EXECUTEABLE:
      {
        // Only the CCVGItem (parent node holds the CCVGItem data object, so not
        // to duplicate unncessarily
        pTreeItem = pRetVal->m_pTreeWidgetItem->parent();
      }
      break;
      default:
        // Do nothing
        return nullptr;
      }

      return NodeGetDataExtn(*pTreeItem);
    }
  }

  return nullptr;
}

/// @brief Retrieve the CodeXL framework data object bound to the one or first 
///        selected tree view item in the tree control. From this object a
///        CCVGItem object can be obtained if present.
/// @return afApplicationTreeItemData* Custom tree item data object pointer,
///         NULL = tree control not valid at the time or data object not
///         present for the selected item.
QTreeWidgetItem* CCVGGUIAppTreeHandler::GetSelectedItemFirst()
{
  // This function can be called when the toolbar is created. Sometimes, m_pApplicationTree is not initialized.
  // Do not assert at this point, this is a false assertion.
  if (m_pApplicationTree != nullptr)
  {
    if (m_pApplicationTree->treeControl()->selectedItems().size() > 0)
    {
      QList<QTreeWidgetItem *> listSelected = m_pApplicationTree->treeControl()->selectedItems();
      return listSelected.at(0);
    }
  }

  return nullptr;
}

/// @brief Find an existing matching tree item for the supplied tree item ID.
/// @param[in] vDisplayedItemId A tree item data with only the identification
///            information for the object.
/// @return afApplicationTreeItemData* The tree item, NULL = not found.
afApplicationTreeItemData* CCVGGUIAppTreeHandler::FindMatchingTreeItem(const afApplicationTreeItemData &vDisplayedItemId) 
{
  GT_UNREFERENCED_PARAMETER(vDisplayedItemId);
  return nullptr;
}

/// @brief For the supplied display  item build HTML content.
/// @param[in] vDisplayedItemId The item ID.
/// @param[out] vHtmlContent A new HTML content written to the object.
/// @return bool True = HTML context is supplied, false = there is no content.
bool CCVGGUIAppTreeHandler::BuildItemHTMLPropeties(const afApplicationTreeItemData &vDisplayedItemId, afHTMLContent &vHtmlContent)
{
  GT_UNREFERENCED_PARAMETER(vDisplayedItemId);
  GT_UNREFERENCED_PARAMETER(vHtmlContent);
  return true;
}

/// @brief Overridden. Sets the items' visibility in the tree view. Passing through all or
///        some of the items to hide or show items based on their active mode 
///        or other criteria.
void CCVGGUIAppTreeHandler::SetItemsVisibility()
{
  fw::CCVGFWTheCurrentState &rCurrState(fw::CCVGFWTheCurrentState::Instance());
  const bool bProjOpen = rCurrState.IsProjectOpen();
  const bool bCCVGExMode = rCurrState.IsInCCVGExecutionMode();
  const bool bCCVGExecModeEnabled = GetCCVGExecModeEnabled();
  const bool bVisibleCCVGItems = bProjOpen && bCCVGExMode && bCCVGExecModeEnabled;
  SetItemsVisibility(bVisibleCCVGItems);
}

/// @brief Sets the CCVG items' visibility in the tree view. 
/// @param[in] vbYesVisible True = make visible, false = hide.
void CCVGGUIAppTreeHandler::SetItemsVisibility(bool vbYesVisible)
{
  GT_IF_WITH_ASSERT(m_pProgramsRootNode != nullptr)
  {
    const int nChild = m_pProgramsRootNode->childCount();
    for (int i = 0; i < nChild; i++)
    {
      QTreeWidgetItem *pWgt = m_pProgramsRootNode->child(i);
      afApplicationTreeItemData *pChildData = m_pApplicationTree->getTreeItemData(pWgt);
      if (pChildData != nullptr)
      {
        if (IsItemCCVGType(pChildData->m_itemType))
        {
          // Hide show Root Item Good/Bad, children takes on parents' visibility
          if (pChildData->m_pTreeWidgetItem != nullptr)
          {
            pChildData->m_pTreeWidgetItem->setHidden(!vbYesVisible);
          }
        }
      }
    }
  }
}

/// @brief     Called when an application wide or execution mode event occurs.
///            The function gets visited for all events that can occur. Any 
///            events not handled must be allowed to pass on to other 
///            observers. An observer can veto or block an event being passed
///            on to later observers if desired.
/// @param[in] vEvent An event object.
/// @param[out] vbVetoEvent True = do not pass on, false = allow other 
///             handlers.
void CCVGGUIAppTreeHandler::onEvent(const apEvent &vEvent, bool &vbVetoEvent)
{
  GT_UNREFERENCED_PARAMETER(vbVetoEvent);

  const apEvent::EventType eEventType = vEvent.eventType();
  if ((eEventType == apEvent::AP_EXECUTION_MODE_CHANGED_EVENT) && GetCCVGExecModeEnabled())
  {
    const apExecutionModeChangedEvent &rExecChangedEvent = static_cast<const apExecutionModeChangedEvent &>(vEvent);
    // If this is a mode change and not a session type change
    if (!rExecChangedEvent.onlySessionTypeIndex())
    {
      const gtString &rMode = rExecChangedEvent.modeType();
      const bool bIsCCVGMode = (rMode == CCVGRSRC(kIDS_CCVG_STR_execmode_name));
      TreeEnableSorting(bIsCCVGMode);
      return;
    }
  }

  switch (eEventType)
  {
    case apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT:
    {
      // User double clicked a tree item
      // Event initiated from afApplicationTree::onObjectTreeActivation()
      const apMonitoredObjectsTreeSelectedEvent &rEvent(static_cast<const apMonitoredObjectsTreeSelectedEvent &>(vEvent));
      const void *pData = rEvent.selectedItemData();
      if (pData != nullptr)
      {
        const afApplicationTreeItemData *pItemData = reinterpret_cast<const afApplicationTreeItemData *>(pData);
        if (IsItemCCVGType(pItemData->m_itemType))
        {
          switch (pItemData->m_itemType)
          {
          case AF_TREE_ITEM_CCVG_CCVGITEM:
          case AF_TREE_ITEM_CCVG_CCVGITEM_RESULTS:
            ActionDoCCVGItemResultFileView();
            break;
          case AF_TREE_ITEM_CCVG_CCVGITEM_SOURCE:
            ActionDoCCVGItemViewCoverageResults();
            break;
          case AF_TREE_ITEM_CCVG_CCVGITEM_EXECUTEABLE:
            ActionDoCCVGItemExecFileView();
            break;
          case AF_TREE_ITEM_CCVG_CCVGITEM_DRIVERS:
            ActionDoCCVGItemDriverSrcCodeView();
            break;
          default:
            ;
          }
        }
      }
    }
    break;
  default:
    ; // Do nothing
  }
}

/// @brief Used by the application event framework to retrieve and log 
///        information about the state of the application. Returning this 
///        execution mode's name for the tree handler.
/// @return wchar_t* Identification text.
const wchar_t* CCVGGUIAppTreeHandler::eventObserverName() const
{
  return L"CCVGEventObserverAppTreeHandler";
}

/// @brief A signal slot call on application framework has created the tree
///        explorer view and can now take plugin registrations to extend the
///        tree handling for the purposed of that exeution mode (plugin).
void CCVGGUIAppTreeHandler::OnApplicationTreeCreated()
{
  afApplicationCommands *pApplicationCommands = afApplicationCommands::instance();
  GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
  {
    m_pApplicationTree = pApplicationCommands->applicationTree();
    GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
    {
      // Should add a signal to main window that the tree was created, and 
      // register all tree handler when handling this signal
      m_pApplicationTree->registerApplicationTreeHandler(this);
    }

    m_pProgramsRootNode = m_pApplicationTree->headerItem();
  }
}

/// @brief Overridden. Notification of when project data has just been changed.
///        Checks are not made for CCVG exection mode funcionality enabled or
///        CCVG execution mode is the current mode as we populate the tree
///        just not make it visible to the user. 
/// @return status Success = Task of updating for load ok, failure = error
///         condition.
status CCVGGUIAppTreeHandler::OnNotifyProjectDataChanged()
{
  status ok = success;
  fw::CCVGFWTheCurrentState &rCurrState(fw::CCVGFWTheCurrentState::Instance());
  const fw::CCVGFWTheCurrentStateProjectData &rProjData(rCurrState.GetProjectData());
  const fw::CCVGFWTheCurrentStateProjectData::MapItems_t &rMapItems(rProjData.ItemsGet());
  if (rMapItems.empty())
  {
    ok = TreeClear();
  }
  else
  {
    ok = UpdateTreeFromProjectData(rProjData);
    if (ok == success)
    {
      SetItemsVisibility();
    }
  }

  return ok;
}

/// @brief Overridden. Notification of when project just closed.
/// @return status Success = Task of updating for close state ok, failure = 
///         error condition.
status CCVGGUIAppTreeHandler::OnNotifyClose()
{
  return TreeClear();
}

/// @brief Create or change the the tree control to reflect the current 
///        project's data.
/// @param[in] vrProjData The CCVG object managing project data.
/// @return status Success = Task ok, failure = error condition.
status CCVGGUIAppTreeHandler::UpdateTreeFromProjectData(const fw::CCVGFWTheCurrentStateProjectData &vrProjData)
{
  status ok = success;
  GT_IF_WITH_ASSERT(m_pProgramsRootNode != nullptr)
  {
    // Qt performance turn off sorting
    TreeEnableSorting(false);

    const fw::CCVGFWTheCurrentStateProjectData::MapItems_t &rMapItems(vrProjData.ItemsGet());
    if (rMapItems.empty())
    {
      return success;
    }
    
    if (NodeAddRoots() == failure)
    {
      return failure;
    }

    // Iterate CCVGItems to build or change the application tree control
    const fw::CCVGFWTheCurrentStateProjectData::MapItemsState_t &rMapItemsState(vrProjData.ItemsChangeStateFromPrevLoad());
    for (const auto it : rMapItemsState)
    {
      const gtString &rItemName(it.first);
      const fw::CCVGFWTheCurrentStateProjectData::EnumCCVGItemChangeStateFromPrevLoad &rItemState(it.second);
      switch (rItemState)
      {
      case fw::CCVGFWTheCurrentStateProjectData::EnumCCVGItemChangeStateFromPrevLoad::kChangeState_New:
        {
          const auto &itCCVGItem = rMapItems.find(rItemName);
          const fw::CCVGFWTheCurrentStateProjectData::CCVGItem &rItem((*itCCVGItem).second);
          ok = ItemAddNew(rItem);
        }
        break;
      case fw::CCVGFWTheCurrentStateProjectData::EnumCCVGItemChangeStateFromPrevLoad::kChangeState_Changed:
        {
          const auto &itCCVGItem = rMapItems.find(rItemName);
          const fw::CCVGFWTheCurrentStateProjectData::CCVGItem &rItem((*itCCVGItem).second);
          ok = ItemChanged(rItem);
        }
        break;
      case fw::CCVGFWTheCurrentStateProjectData::EnumCCVGItemChangeStateFromPrevLoad::kChangeState_Removed:
        ok = ItemRemoved(rItemName);
        break;
      case fw::CCVGFWTheCurrentStateProjectData::EnumCCVGItemChangeStateFromPrevLoad::kChangeState_None:
        {
          const auto &itCCVGItem = rMapItems.find(rItemName);
          const fw::CCVGFWTheCurrentStateProjectData::CCVGItem &rItem((*itCCVGItem).second);
          ok = ItemAdd(rItem);
        }
        break;
      default:
        // Do nothing
        break;
      }
      if (ok == failure)
      {
        break;
      }
    }

    // Qt performance turn back on sorting previously turned off on entry to 
    // this function 
    TreeEnableSorting(m_bCCVGItemsSortedInTree);
  }
  
  return ok;
}

/// @brief Add a representation of the specified CCVGItem data object to the 
///        CodeXL application tree handler view (tree control). This does not
///        in the exist already. CCVGItems are added as a child of either the
///        CCVGItem Root Node Good or Bad.
/// @param[in] vrItem A CCVGItem data object.
/// @return status Success = Set of tree items created, failure = error in 
///         creating a tree item representation all others removed.
status CCVGGUIAppTreeHandler::ItemAddNew(const fw::CCVGFWTheCurrentStateProjectData::CCVGItem &vrItem)
{
  if (NodeFindValidateOnlyOneByThatName(vrItem.GetFileName()))
  {
    return failure;
  }

  status ok = success;
  const bool bItemOk = vrItem.GetIsGood();
  QTreeWidgetItem *pCCVGItemsRootNode = bItemOk ? m_pCCVGItemsRootNodeGood : m_pCCVGItemsRootNodeBad;
  QTreeWidgetItem *pNodeCCVGItem = nullptr;
  QTreeWidgetItem *pNodeCCVGItemResults = nullptr;
  QTreeWidgetItem *pNodeCCVGItemSource = nullptr;
  QTreeWidgetItem *pNodeCCVGItemDriver = nullptr;
  QTreeWidgetItem *pNodeCCVGItemExecuteable = nullptr;
  ok = NodeFactory(vrItem, AF_TREE_ITEM_CCVG_CCVGITEM, *pCCVGItemsRootNode, pNodeCCVGItem);
  ok = ok && NodeFactory(vrItem, AF_TREE_ITEM_CCVG_CCVGITEM_RESULTS, *pNodeCCVGItem, pNodeCCVGItemResults);
  ok = ok && NodeFactory(vrItem, AF_TREE_ITEM_CCVG_CCVGITEM_SOURCE, *pNodeCCVGItem, pNodeCCVGItemSource);
  ok = ok && NodeFactory(vrItem, AF_TREE_ITEM_CCVG_CCVGITEM_DRIVERS, *pNodeCCVGItem, pNodeCCVGItemDriver);
  ok = ok && NodeFactory(vrItem, AF_TREE_ITEM_CCVG_CCVGITEM_EXECUTEABLE, *pNodeCCVGItem, pNodeCCVGItemExecuteable);
  if (ok == failure)
  {
    if(pNodeCCVGItem != nullptr)
    {
      NodeRemoveRecursiveChildren(*pNodeCCVGItem);
    }
  }

  return ok;
}

/// @brief Remove the specified tree item from the tree control. Any child 
///        items will not be removed. The associated data object is deleted.
///        Qt documentation state item is removed but not deleted. No CodeXL
///        deletes a tree item.
/// @param[in] vrItem Tree item object.
/// @return status Success = removed, failure = not removed.
status CCVGGUIAppTreeHandler::NodeRemove(QTreeWidgetItem &vrNode)
{
  ItemCCVGExtnDataDelete(vrNode);
  
  // Block selection changed signal to fix crash - taken from kaApplicationTreeHandler::OnRemoveProgramFromProject()
  bool bOk = disconnect(m_pApplicationTree->treeControl(), SIGNAL(itemSelectionChanged()), m_pApplicationTree, SLOT(onItemSelectionChanged()));
  GT_ASSERT(bOk);

  m_pApplicationTree->removeTreeItem(&vrNode, false);
  
  bOk = connect(m_pApplicationTree->treeControl(), SIGNAL(itemSelectionChanged()), m_pApplicationTree, SLOT(onItemSelectionChanged()));
  GT_ASSERT(bOk);

  return success;
}

/// @brief Add a representation of the specified CCVGItem data object to the 
///        CodeXL application tree handler view (tree control). A check is 
///        made to see if a tree item by that name exists already and if it 
///        does then the does not take place. CCVGItems are added as a child 
///        of the specified parent.
/// @param[in] vrItem A CCVGItem data object.
/// @return status Success = Set of tree items created, failure = error in 
///         creating a tree item representation al others removed.
status CCVGGUIAppTreeHandler::ItemAdd(const fw::CCVGFWTheCurrentStateProjectData::CCVGItem &vrItem)
{
  const bool bItemOk = vrItem.GetIsGood();
  QTreeWidgetItem *pCCVGItemsRootNode = bItemOk ? m_pCCVGItemsRootNodeGood : m_pCCVGItemsRootNodeBad;
  QTreeWidgetItem *pCCVGItem = NodeFind(*pCCVGItemsRootNode, vrItem.GetFileName());
  if (pCCVGItem != nullptr)
  {
    return success;
  }

  return ItemAddNew(vrItem);
}

/// @brief A lower level function to create a different tree control items and 
///        add them to the tree control. A CCVGItem is made up of many tree 
///        items or icons. This function adds those items.
/// @param[in] veType Global enumeration tree item type, specify a CCVG item 
///            type.
/// @param[inout] vrParent Parent tree item to add this new child item too.
/// @param[inout] vpNewNode Pointer to a new tree item or NULL on failure.
/// @return status Success = creation success, failure = error occurred.
status CCVGGUIAppTreeHandler::NodeAdd(afTreeItemType veType, QTreeWidgetItem &vrParent, QTreeWidgetItem *&vpNewNode)
{
  return NodeFactory(veType, vrParent, vpNewNode);
}

/// @brief Tree item widget factory function for CCVGItem Root nodes Good and
///        Bad. Creates the specified tree item object filling the details; 
///        pixmap, label, tooltip text. It then adds it to the tree control as 
///        a child of the specified parent item. CCVGItem data object 
///        associated with each tree item is also created and attached. 
/// @param[in] veType Global enumeration tree item type, specify a CCVG item 
///            type.
/// @param[inout] vrParent Parent tree item to add this new child item too.
/// @param[inout] vpNewNode Pointer to a new tree item or NULL on failure.
/// @return status Success = creation success, failure = error occurred.
status CCVGGUIAppTreeHandler::NodeFactory(afTreeItemType veType, QTreeWidgetItem &vrParent, QTreeWidgetItem *&vpNewNode)
{
  std::unique_ptr<afApplicationTreeItemData> pAppTreeItemData(new (std::nothrow) afApplicationTreeItemData);
  if (pAppTreeItemData == nullptr)
  {
    return failure;
  }
  pAppTreeItemData->m_itemType = veType;

  const wchar_t *pNodeText = nullptr;
  const wchar_t *pNodeToolTip = nullptr;
  QPixmap *pIconPixmap = nullptr;
  switch (veType)
  {
  case AF_TREE_ITEM_CCVG_ROOTNODE_SETFILESGOOD:
    pNodeText = CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesGood_label);
    pNodeToolTip = CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesGood_tooltip);
    pIconPixmap = CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeRootNode_SetFilesGood);
    break;
  case AF_TREE_ITEM_CCVG_ROOTNODE_SETFILESBAD:
    pNodeText = CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesBad_label);
    pNodeToolTip = CCVGRSRC(kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesBad_tooltip);
    pIconPixmap = CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeRootNode_SetFilesBad);
    break;
  default:
    return failure;
  }

  QTreeWidgetItem *pNewNode = m_pApplicationTree->addTreeItem(pNodeText, pAppTreeItemData.release(), &vrParent);
  if (pNewNode == nullptr)
  {
    return failure;
  }

  pNewNode->setToolTip(0, acGTStringToQString(pNodeToolTip));
  pNewNode->setIcon(0, QIcon(*pIconPixmap));

  vpNewNode = pNewNode;
  return success;
}

/// @brief Tree item widget factory function for CCVGItem data objects. 
///        Creates the specified set of tree item object filling the details; 
///        pixmap, label, tooltip text. It then adds it to the tree control as 
///        a child of the specified parent item. One CCVGItem data object is
///        associated with each CCVGItem (node) tree item is also created and 
///        attached. Tree items (nodes) refer to the CCVGItem node (parent) 
///        when needing to refer to CCVGItem data as they do no hold this
///        data themselves. This reduces dupliction of data held in the tree.
/// @param[in] vrItem CCVGItem data object.
/// @param[in] veType Global enumeration tree item type, specify a CCVG item 
///            type.
/// @param[inout] vrParent Parent tree item to add this new child item too.
/// @param[inout] vpNewNode Pointer to a new tree item or NULL on failure.
/// @return status Success = creation success, failure = error occurred.
status CCVGGUIAppTreeHandler::NodeFactory(const fw::CCVGFWTheCurrentStateProjectData::CCVGItem &vrItem, afTreeItemType veType, QTreeWidgetItem &vrParent, QTreeWidgetItem *&vpNewNode)
{
  // Create tree item associate data object put a CCVGItem object into it
  std::unique_ptr<afApplicationTreeItemData> pAppTreeItemData(new (std::nothrow) afApplicationTreeItemData);
  if (pAppTreeItemData == nullptr)
  {
    return failure;
  }
  
  // Create the QWidgetTreeItem object
  const bool bItemIsGood = vrItem.GetIsGood();
  const wchar_t *pFileName = vrItem.GetFileName().asCharArray();
  const wchar_t *pNodeText = nullptr;
  QPixmap *pIconPixmap = nullptr;
  gtString nodeText;
  gtString nodeToolTip;
  switch (veType)
  {
  case AF_TREE_ITEM_CCVG_CCVGITEM:
    {
      pAppTreeItemData->m_itemType = bItemIsGood ? AF_TREE_ITEM_CCVG_CCVGITEM : AF_TREE_ITEM_CCVG_CCVGITEM_BAD;
      pIconPixmap = bItemIsGood ? CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeCCVGItem) : CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeCCVGItem_Bad);
      pNodeText = pFileName;
      std::unique_ptr<fw::CCVGFWTreeDataExtn> pCCVGItem(new (std::nothrow) fw::CCVGFWTreeDataExtn);
      if (pCCVGItem == nullptr)
      {
        return failure;
      }
      if (pCCVGItem->Create(vrItem.GetFileName(), vrItem) == failure)
      {
        return failure;
      }
      pAppTreeItemData->setExtendedData(pCCVGItem.release());
    }
    break;
  case AF_TREE_ITEM_CCVG_CCVGITEM_RESULTS:
    {
      fw::CCVGFWTheCurrentState &rCurrState(fw::CCVGFWTheCurrentState::Instance());
      const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSetting(rCurrState.GetProjectSetting().GetSettingsWorking());
      pAppTreeItemData->m_itemType = bItemIsGood ? AF_TREE_ITEM_CCVG_CCVGITEM_RESULTS : AF_TREE_ITEM_CCVG_CCVGITEM_RESULTS_BAD;
      pIconPixmap = vrItem.IsPresentCcvgr() ? CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeCCVGItem_Results) : CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeCCVGItem_Results_Bad);
      nodeText.appendFormattedString(L"%ls%ls", pFileName, rSetting.GetCcvgFilesExtnResult().asCharArray());
      pNodeText = nodeText.asCharArray();
      nodeToolTip = rSetting.GetCcvgFolderDirResult();
    }
    break;
  case AF_TREE_ITEM_CCVG_CCVGITEM_SOURCE:
    {
      fw::CCVGFWTheCurrentState &rCurrState(fw::CCVGFWTheCurrentState::Instance());
      const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSetting(rCurrState.GetProjectSetting().GetSettingsWorking());
      pAppTreeItemData->m_itemType = bItemIsGood ? AF_TREE_ITEM_CCVG_CCVGITEM_SOURCE : AF_TREE_ITEM_CCVG_CCVGITEM_SOURCE_BAD;
      pIconPixmap = vrItem.IsPresentKc() ? CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeCCVGItem_Source) : CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeCCVGItem_Source_Bad);
      nodeText.appendFormattedString(L"%ls%ls", pFileName, rSetting.GetCcvgFilesExtnSource().asCharArray());
      pNodeText = nodeText.asCharArray();
      nodeToolTip = rSetting.GetCcvgFolderDirSource();
    }
    break;
  case AF_TREE_ITEM_CCVG_CCVGITEM_DRIVERS:
    {
      fw::CCVGFWTheCurrentState &rCurrState(fw::CCVGFWTheCurrentState::Instance());
      const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSetting(rCurrState.GetProjectSetting().GetSettingsWorking());
      pAppTreeItemData->m_itemType = bItemIsGood ? AF_TREE_ITEM_CCVG_CCVGITEM_DRIVERS : AF_TREE_ITEM_CCVG_CCVGITEM_DRIVERS_BAD;
      pIconPixmap = vrItem.IsPresentCcvgd() ? CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeCCVGItem_Drivers) : CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeCCVGItem_Drivers_Bad);
      nodeText.appendFormattedString(L"%ls%ls", pFileName, rSetting.GetCcvgFilesExtnDriver().asCharArray());
      pNodeText = nodeText.asCharArray();
      nodeToolTip = rSetting.GetCcvgFolderDirDriver();
    }
    break;
  case AF_TREE_ITEM_CCVG_CCVGITEM_EXECUTEABLE:
  {
    fw::CCVGFWTheCurrentState &rCurrState(fw::CCVGFWTheCurrentState::Instance());
    const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSetting(rCurrState.GetProjectSetting().GetSettingsWorking());
    pAppTreeItemData->m_itemType = bItemIsGood ? AF_TREE_ITEM_CCVG_CCVGITEM_EXECUTEABLE : AF_TREE_ITEM_CCVG_CCVGITEM_EXECUTEABLE_BAD;
    pIconPixmap = vrItem.IsPresentCcvge() ? CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeCCVGItem_Executeable) : CCVGRSRCICON(kIDS_CCVG_ICON_AppTreeCCVGItem_Executeable_Bad);
    nodeText.appendFormattedString(L"%ls%ls", pFileName, rSetting.GetCcvgFilesExtnExecuteable().asCharArray());
    pNodeText = nodeText.asCharArray();
    nodeToolTip = rSetting.GetCcvgFolderDirExecuteable();
  }
  break;
  default:
    return failure;
  }

  QTreeWidgetItem *pNewNode = m_pApplicationTree->addTreeItem(pNodeText, pAppTreeItemData.release(), &vrParent);
  if (pNewNode == nullptr)
  {
    return failure;
  }

  pNewNode->setToolTip(0, acGTStringToQString(nodeToolTip));
  pNewNode->setIcon(0, QIcon(*pIconPixmap));

  vpNewNode = pNewNode;
  return success;
}

/// @brief Search the whole CodeXL App Tree Control for a QWidgetTreeItem with
///        the matching specified label text. If more than one tree item is
///        found (which there should not be for a CCVG tree of items) it will
///        return the  first item found.
/// @param[in] The tree item label name to find.
/// @return QTreeWidgetItem* Pointer to an object, NULL = not found.
QTreeWidgetItem* CCVGGUIAppTreeHandler::NodeFind(const gtString &vrItemName)
{
  QTreeWidgetItem *pItemFound = nullptr;

  if (m_pApplicationTree == nullptr)
  {
    return nullptr;
  }
  afTreeCtrl *pCtrl = m_pApplicationTree->treeControl();
  if (pCtrl == nullptr)
  {
    return nullptr;
  }
  
  const QString itemName = acGTStringToQString(vrItemName);
  QList<QTreeWidgetItem *> listWgt = m_pApplicationTree->treeControl()->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive);
  if(listWgt.count() > 0)
  {
    pItemFound = listWgt[0];
  }  

  return pItemFound;
}

/// @brief Checks there is no more than one tree item in the whole tree control
///        with the specified label text.
/// @param[in] The tree item label name to find.
/// @return bool True = found and only one, false = not found or more than one.
bool CCVGGUIAppTreeHandler::NodeFindValidateOnlyOneByThatName(const gtString &vrItemName)
{
  if (m_pApplicationTree == nullptr)
  {
    return false;
  }
  afTreeCtrl *pCtrl = m_pApplicationTree->treeControl();
  if (pCtrl == nullptr)
  {
    return false;
  }

  const QString itemName = acGTStringToQString(vrItemName);
  QList<QTreeWidgetItem *> listWgt = m_pApplicationTree->treeControl()->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive);
  return (listWgt.count() == 1);
}

/// @brief Search the CodeXL App Tree Control from the specified parent 
///        QWidgetTreeItem down through any child items for an item of a 
///        specified type.
/// @param[in] vCurrentTreeItem Start search from this item.
/// @param[in] veType Global enumeration tree item type, specify a CCVG item 
///            type.
/// @return QTreeWidgetItem* Pointer to an object, NULL = not found.
QTreeWidgetItem* CCVGGUIAppTreeHandler::NodeFind(QTreeWidgetItem &vCurrentTreeItem, afTreeItemType veType)
{
  QTreeWidgetItem *pRetVal = nullptr;
  
    // Look at the current object
    const afApplicationTreeItemData *pCurrentData = m_pApplicationTree->getTreeItemData(&vCurrentTreeItem);
    if (nullptr != pCurrentData)
    {
      if (veType == pCurrentData->m_itemType)
      {
        pRetVal = &vCurrentTreeItem;
      }
    }

    // If it is not it, look at its children
    if (nullptr == pRetVal)
    {
      const int nChildren = vCurrentTreeItem.childCount();
      for (int nChild = 0; (nChild < nChildren) && (nullptr == pRetVal); nChild++)
      {
        pRetVal = NodeFind(*vCurrentTreeItem.child(nChild), veType);
      }
    }

  return pRetVal;
}

/// @brief Search the CodeXL App Tree Control from the specified parent 
///        QWidgetTreeItem down through any child items for an item with the
///        same label text.
/// @param[in] vCurrentTreeItem Start search from this item.
/// @param[in] vSearchText Tree item label text.
/// @return QTreeWidgetItem* Pointer to an object, NULL = not found.
QTreeWidgetItem* CCVGGUIAppTreeHandler::NodeFind(QTreeWidgetItem &vrParent, const gtString &vSearchText)
{
  QString searchText = acGTStringToQString(vSearchText);
  QTreeWidgetItem *pItem = m_pApplicationTree->treeControl()->FindChild(&vrParent, searchText);
  return pItem;
}

/// @brief Locate the specified CCVGItem data object in the CodeXL application 
///        tree handler view (tree control) and change its attributes values to 
///        those that have changed since the last load or refresh (commands) by
///        the user. It may be that a located item will also be moved from one
///        CCVGItem Root Node to the other. If the CCVGitem representation 
///        cannot be found it is assumed this is a new project just opened in
///        which case the CCVGItem is added to the tree control.
/// @param[in]    vrItem A CCVGItem data object.
/// @return status Success = Set of tree items updated, failure = error 
///         occurred.
status CCVGGUIAppTreeHandler::ItemChanged(const fw::CCVGFWTheCurrentStateProjectData::CCVGItem &vrItem)
{
  status ok = success;
  const gtString &rItemName(vrItem.GetFileName());
  QTreeWidgetItem *pCCVGItemGood = NodeFind(*m_pCCVGItemsRootNodeGood, rItemName);
  QTreeWidgetItem *pCCVGItemBad = NodeFind(*m_pCCVGItemsRootNodeBad, rItemName);
  if ((pCCVGItemGood == nullptr) && (pCCVGItemBad == nullptr))
  {
    return ItemAddNew(vrItem);
  }

  if (pCCVGItemGood != nullptr)
  {
    const fw::CCVGFWTheCurrentStateProjectData::CCVGItem *pPrevCCVGItemData = NodeGetDataExtn(*pCCVGItemGood);
    if (*pPrevCCVGItemData != vrItem)
    {
      ok = NodeRemove(*pCCVGItemGood);
      return ok && ItemAdd(vrItem);
    }
  }
  else if (pCCVGItemBad != nullptr)
  {
    const fw::CCVGFWTheCurrentStateProjectData::CCVGItem *pPrevCCVGItemData = NodeGetDataExtn(*pCCVGItemBad);
    if (*pPrevCCVGItemData != vrItem)
    {
      ok = NodeRemove(*pCCVGItemBad);
      return ok && ItemAdd(vrItem);
    }
  }

  return success;
}

/// @brief Retrieve the pointer to the CCVGItem object stored in a specified
///        tree node.
/// @return CCVGItem* Object pointer or NULL if data not present.
const fw::CCVGFWTheCurrentStateProjectData::CCVGItem* CCVGGUIAppTreeHandler::NodeGetDataExtn(QTreeWidgetItem &vNode)
{
  const fw::CCVGFWTheCurrentStateProjectData::CCVGItem *pData = nullptr;
  afApplicationTreeItemData *pAppTreeItemData = m_pApplicationTree->getTreeItemData(&vNode);
  const afTreeDataExtension *pDataExtn = pAppTreeItemData->extendedItemData();
  const fw::CCVGFWTreeDataExtn *pCCVGDataExtn = static_cast<const fw::CCVGFWTreeDataExtn *>(pDataExtn);
  pData = pCCVGDataExtn->GetDataItem();
  return pData;
}

/// @brief Locate the specified CCVGItem data object in the CodeXL application 
///        tree handler view (tree control) and remove it. Likely action from 
///        project last load or refresh (commands) of files in specifed 
///        directories.
/// @return status success = CCVGItem removed, false = error occurred.
status CCVGGUIAppTreeHandler::ItemRemoved(const gtString &vrItemName)
{
  QTreeWidgetItem *pNode = NodeFind(vrItemName);
  if (pNode != nullptr)
  {
    return NodeRemove(*pNode);
  }

  return success;
}

/// @brief Clear the tree control of all the CCVG widget items up to but not 
///        inlcuding the tree head node.
/// @return status Success = Task ok, failure = error condition.
status CCVGGUIAppTreeHandler::TreeClear()
{
  CCVGuint errCount = 0;
  
  GT_IF_WITH_ASSERT((m_pProgramsRootNode != nullptr) && (m_pApplicationTree != nullptr))
  {
    const int count = m_pProgramsRootNode->childCount();
    for (int i = count - 1; i >= 0; i--)
    {
      // Get the next child
      QTreeWidgetItem *pChild = m_pProgramsRootNode->child(i);
      afApplicationTreeItemData *pTreeItemData = m_pApplicationTree->getTreeItemData(pChild);
      if ((pTreeItemData != nullptr) && (pChild != nullptr))
      {
        if (IsItemCCVGType(pTreeItemData->m_itemType))
        {
          // It is assumed all nodes below Root Node Good/Bad are all CCVG item nodes
          errCount = (NodeRemoveRecursiveChildren(*pChild) == failure) ? ++errCount : errCount;
        }
      }
    }
  }

  // Reset
  m_pCCVGItemsRootNodeGood = nullptr;
  m_pCCVGItemsRootNodeBad = nullptr;
  m_bCCVGItemsSortedInTree = false;

  return ((errCount == 0) ? success : failure);
}

/// @brief Remove the specified tree item from the tree control. Any child 
///        items will be removed. The associated data object is deleted. 
///        Qt documentation state item is removed but not deleted. No CodeXL
///        code deletes a tree item.
/// @param[in] vrItem Tree item object.
/// @return status Success = removed, failure = not removed.
status CCVGGUIAppTreeHandler::NodeRemoveRecursiveChildren(QTreeWidgetItem &vrNode)
{
  CCVGuint errCount = 0;
  const int count = vrNode.childCount();
  for (int i = count - 1; i >= 0; i--)
  {
    QTreeWidgetItem *pChild = vrNode.child(i);
    if (pChild != nullptr)
    {
      errCount = (NodeRemoveRecursiveChildren(*pChild) == failure) ? ++errCount : errCount;
    }
  }

  ItemCCVGExtnDataDelete(vrNode);
  
  // Block selection changed signal to fix crash - taken from kaApplicationTreeHandler::OnRemoveProgramFromProject()
  bool bOk = disconnect(m_pApplicationTree->treeControl(), SIGNAL(itemSelectionChanged()), m_pApplicationTree, SLOT(onItemSelectionChanged()));
  GT_ASSERT(bOk);

  m_pApplicationTree->removeTreeItem(&vrNode, false);

  bOk = connect(m_pApplicationTree->treeControl(), SIGNAL(itemSelectionChanged()), m_pApplicationTree, SLOT(onItemSelectionChanged()));
  GT_ASSERT(bOk);

  return ((errCount == 0) ? success : failure);
}

/// @brief Delete the CCVGItem extension object bound to the tree item, the 
///        specified node, and delete the copy of the CCVGItem object. These
///        objects are deleted when a tree item is deleted.
///        Note we are manually deleting the afTreeDataExtension derived object
///        here as not asked afApplicationTreeItemData object to do it.
/// @param[in] vrItem Tree item object.
void CCVGGUIAppTreeHandler::ItemCCVGExtnDataDelete(QTreeWidgetItem &vrNode)
{
  afApplicationTreeItemData *pAppTreeItemData = m_pApplicationTree->getTreeItemData(&vrNode);
  if (pAppTreeItemData == nullptr)
  {
    return;
  }
  const afTreeDataExtension *pDataExtn = pAppTreeItemData->extendedItemData();
  const fw::CCVGFWTreeDataExtn *pCCVGDataExtn = static_cast<const fw::CCVGFWTreeDataExtn *>(pDataExtn);
  if (pCCVGDataExtn == nullptr)
  {
    return;
  }
  const fw::CCVGFWTheCurrentStateProjectData::CCVGItem *pPrevCCVGItemData = pCCVGDataExtn->GetDataItem();
  delete pDataExtn;
  if (pPrevCCVGItemData == nullptr)
  {
    return;
  }
  delete pPrevCCVGItemData;
}

/// @brief Determine if the application tree widget item is an item which
///        belongs to the CCVG set of item types.
/// @return bool True = Yes a CCVG item, false = another type.
bool CCVGGUIAppTreeHandler::IsItemCCVGType(afTreeItemType veType) const
{
  return ((veType >= AF_TREE_FIRST_CCVG_ITEM_TYPE) && (veType <= AF_TREE_LAST_CCVG_ITEM_TYPE));
}

/// @brief By default in all other CodeXL Execution modes the application tree 
///        view displays items in the order they were added to the tree. The
///        CCVG execution mode requires for easier user experience that tree
///        items by name are display in ascending order. Enabling disabling
///        sorting does not affect the any ordering for other modes. The tree
///        item label is used for the sorting.
/// @param[in] bool True = sort in ascending order, false = inserted order.
/// @return status Success = all ok, failure = error occurred.
status CCVGGUIAppTreeHandler::TreeEnableSorting(bool vbYes)
{
  if (m_pApplicationTree != nullptr)
  {
    afTreeCtrl *pTreeCtrl = m_pApplicationTree->treeControl();
    if (pTreeCtrl != nullptr)
    {
      m_bCCVGItemsSortedInTree = vbYes;
      pTreeCtrl->setSortingEnabled(vbYes);
      pTreeCtrl->sortByColumn(0, Qt::AscendingOrder);
    }
  }

  return success;
}

/// @brief Insert into the CodeXL application tree control under the root node
///        the CCVGItem Root nodes CCVGItems Good and root node Bad. If the 
///        nodes are already present then do nothing.
/// @return status Success = all ok, failure = error occurred.
status CCVGGUIAppTreeHandler::NodeAddRoots()
{
  status ok = success;

  if (m_pCCVGItemsRootNodeGood == nullptr)
  {
    m_pCCVGItemsRootNodeGood = NodeFind(*m_pProgramsRootNode, AF_TREE_ITEM_CCVG_ROOTNODE_SETFILESGOOD);
    if (m_pCCVGItemsRootNodeGood == nullptr)
    {
      ok = NodeAdd(AF_TREE_ITEM_CCVG_ROOTNODE_SETFILESGOOD, *m_pProgramsRootNode, m_pCCVGItemsRootNodeGood);
    }
  }
  if (m_pCCVGItemsRootNodeBad == nullptr)
  {
    m_pCCVGItemsRootNodeBad = NodeFind(*m_pProgramsRootNode, AF_TREE_ITEM_CCVG_ROOTNODE_SETFILESBAD);
    if (m_pCCVGItemsRootNodeBad == nullptr)
    {
      ok = NodeAdd(AF_TREE_ITEM_CCVG_ROOTNODE_SETFILESBAD, *m_pProgramsRootNode, m_pCCVGItemsRootNodeBad);
    }
  }

  return ok;
}

/// @brief Overridden. Notification that the CodeXL global CCVG setting flag
///        which indicates that CCVG execution mode functionality has been
///        enabled or disable for the current project and future projects.
/// @return status Success = Notifcation action is ok, failure = an error 
///        occurred.
status CCVGGUIAppTreeHandler::OnNotifySettingChangedCcvgAllProjectsEnabledFlag()
{
  if (!GetCCVGExecModeCurrent())
  {
    return success;
  }

  m_pApplicationTree->updateTreeRootText();

  const bool bCCVGExecModeEnabled = GetCCVGExecModeEnabled();
  SetItemsVisibility(bCCVGExecModeEnabled);

  return success;
}

/// @brief Determine if CCVG Execution mode is enabled or disabled at this 
///        time.
/// @return bool True = enabled, false = not enabled.
bool CCVGGUIAppTreeHandler::GetCCVGExecModeEnabled() const
{
  // Also see function CCVGExecutionMode::modeNameDisplayInGui() for 
  // similar functionality
  fw::CCVGFWTheCurrentState &rCurrState(fw::CCVGFWTheCurrentState::Instance());
  const fw::CCVGFWTheCurrentState::Settings &currSetting(rCurrState.GetSettingsWorking());
  const fw::CCVGFWTheCurrentStateProjectSettings &rProjSettings = rCurrState.GetProjectSetting();
  const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSetting(rProjSettings.GetSettingsWorking());
  const bool bAppWideCcvgEnabled = currSetting.GetCcvgAllProjectsEnabledFlag();
  const bool bCurrentProjCcvgEnabled = rSetting.GetCcvgProjectEnabledFlag();
  return bAppWideCcvgEnabled && bCurrentProjCcvgEnabled;
}

/// @brief Determine if CodeXL application is currently in CCVG Execution mode.
/// @return bool True = CCVG mode, false = Another execution mode.
bool CCVGGUIAppTreeHandler::GetCCVGExecModeCurrent() const
{
  fw::CCVGFWTheCurrentState &rCurrState(fw::CCVGFWTheCurrentState::Instance());
  return rCurrState.IsInCCVGExecutionMode();
}

/// @brief Determine if CodeXL project is currently open.
/// @return bool True = open, false = closed.
bool CCVGGUIAppTreeHandler::GetCCVGIsProjectOpen() const
{
  fw::CCVGFWTheCurrentState &rCurrState(fw::CCVGFWTheCurrentState::Instance());
  return rCurrState.IsProjectOpen();
}

/// @brief Determine if CodeXL project has 1 or more CCVGItems present from
///        previous working with the project.
/// @return bool True = CCVGItems present, false = None.
bool CCVGGUIAppTreeHandler::GetCCVGIsProjectEmpty() const
{
  fw::CCVGFWTheCurrentState &rCurrState(fw::CCVGFWTheCurrentState::Instance());
  const fw::CCVGFWTheCurrentStateProjectData &rProjData = rCurrState.GetProjectData();
  return rProjData.ItemsEmpty();
}

/// @brief Setup and execute command to scan the code coverage source 
///        directories and read in files.
void CCVGGUIAppTreeHandler::ActionDoRefreshCCVGItemsReadFiles()
{
  cmd::ExecuteCmdFilesReadAll();
}

/// @brief Open a MDI CCVG view of the source code overlayed with code coverage
///        results.
void CCVGGUIAppTreeHandler::ActionDoCCVGItemViewCoverageResults()
{
  const fw::CCVGFWTheCurrentStateProjectData::CCVGItem *pCCVGItem = GetSelectedItemDataCCVGItem();
  if (pCCVGItem != nullptr)
  {
    const bool bCCVGSourceFileValid = pCCVGItem->IsPresentKc();
    const gtString &rFileName = pCCVGItem->GetFileName();
    cmd::ExecuteCmdWndSrcOpenSourceFile(rFileName);
  } 
}

/// @brief Open a MDI CCVG view of the driver source code used to create the
///        code coverage results for the CCVGItem.
void CCVGGUIAppTreeHandler::ActionDoCCVGItemDriverSrcCodeView()
{
  const fw::CCVGFWTheCurrentStateProjectData::CCVGItem *pCCVGItem = GetSelectedItemDataCCVGItem();
  if (pCCVGItem != nullptr)
  {
    const bool bCCVGDriverFileValid = pCCVGItem->IsPresentCcvgd();
    const gtString &rFileName = pCCVGItem->GetFileName();
    cmd::ExecuteCmdWndSrcOpenDriverFile(rFileName);
  } 
}

/// @brief Open a MDI CCVG view of the code coverage results file itself,
///         which should be a plain text file generated by llvm.
void CCVGGUIAppTreeHandler::ActionDoCCVGItemResultFileView()
{
  const fw::CCVGFWTheCurrentStateProjectData::CCVGItem *pCCVGItem = GetSelectedItemDataCCVGItem();
  if (pCCVGItem != nullptr)
  {
    const bool bCCVGDriverFileValid = pCCVGItem->IsPresentCcvgr();
    const gtString &rFileName = pCCVGItem->GetFileName();
    cmd::ExecuteCmdWndSrcOpenResultFile(rFileName);
  } 
}

/// @brief Open a MDI CCVG view of the hex dump of the relevant executable
///         segment (i.e. the segment containing coverage mapping info).
void CCVGGUIAppTreeHandler::ActionDoCCVGItemExecFileView()
{
  const fw::CCVGFWTheCurrentStateProjectData::CCVGItem *pCCVGItem = GetSelectedItemDataCCVGItem();
  if (pCCVGItem != nullptr)
  {
    const bool bCCVGDriverFileValid = pCCVGItem->IsPresentCcvge();
    const gtString &rFileName = pCCVGItem->GetFileName();
    cmd::ExecuteCmdWndSrcOpenExecFile(rFileName);
  } 
}

} // namespace gui
} // namespace ccvg
