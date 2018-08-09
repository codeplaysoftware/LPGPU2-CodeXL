// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGGUIAppTreeHandler interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_GUI_APPTREEHANDLER_H_INCLUDE
#define LPGPU2_CCVG_GUI_APPTREEHANDLER_H_INCLUDE

// Qt:
#include <QtWidgets>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_SingletonBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_ProjectSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>

namespace ccvg {
namespace gui {

/// @brief    CCVGGUIAppTreeHandler derived from AMDT's 
///           afApplicationTreeHandler and apIEventsObserver. It manages the
///           interaction with the project objects tree view. Usually a tree
///           handler inherits from the event observer to handle tree
///           related events such as 
///           apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT which handles the
///           selection of a tree item. The handler must register itself with
///           the application tree.
/// @warning  None.
/// @see      None.
/// @date     12/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGGUIAppTreeHandler final
: public afApplicationTreeHandler
, public apIEventsObserver
, public CCVGClassCmnBase
, public fw::CCVGFWTheCurrentStateProjectData::NotifyOnProjectDataChanged
, public fw::CCVGFWTheCurrentState::NotifyOnSettingChangedCcvgAllProjectsEnabledFlag
, public CCVGGUIProjectSettings::NotifyOnProjectClose
, public ISingleton<CCVGGUIAppTreeHandler>
{
  friend ISingleton<CCVGGUIAppTreeHandler>;
 
  Q_OBJECT

// Overridden:
public:
  virtual ~CCVGGUIAppTreeHandler() override;
  // From ISingleton
  virtual status Initialise() override;
  virtual status Shutdown() override;
  // From afApplicationTreeHandler
  virtual bool                       BuildContextMenuForItems(const gtList<const afApplicationTreeItemData *> vContextMenuItemsList, QMenu &vMenu) override;
  virtual afApplicationTreeItemData* FindMatchingTreeItem(const afApplicationTreeItemData &vDisplayedItemId) override;
  virtual bool                       BuildItemHTMLPropeties(const afApplicationTreeItemData &vDisplayedItemId, afHTMLContent &vHtmlContent) override;
  virtual void                       SetItemsVisibility() override;
  // From apIEventObserver
  virtual void           onEvent(const apEvent &vEvent, bool &vbVetoEvent) override;
  virtual const wchar_t* eventObserverName() const override;
  // From CCVGFWTheCurrentStateProjectData::NotifyOnProjectDataChanged
  virtual status OnNotifyProjectDataChanged() override;
  // From CCVGGUIProjectSettings::NotifyOnProjectClose
  virtual status OnNotifyClose() override;
  // From CCVGFWTheCurrentState::NotifyOnSettingChangedCcvgAllProjectsEnabledFlag
  virtual status OnNotifySettingChangedCcvgAllProjectsEnabledFlag() override;

// Methods:
private:
  // Singleton enforcement
  CCVGGUIAppTreeHandler();
  CCVGGUIAppTreeHandler(const CCVGGUIAppTreeHandler&) = delete;
  CCVGGUIAppTreeHandler(CCVGGUIAppTreeHandler&&) = delete;
  CCVGGUIAppTreeHandler& operator= (const CCVGGUIAppTreeHandler&) = delete;
  CCVGGUIAppTreeHandler& operator= (CCVGGUIAppTreeHandler&&) = delete;
  //
  // Manipulate the tree control
  void              SetItemsVisibility(bool vbYesVisible);
  bool              IsItemCCVGType(afTreeItemType veType) const;
  status            ItemAddNew(const fw::CCVGFWTheCurrentStateProjectData::CCVGItem &vrItem);
  status            ItemAdd(const fw::CCVGFWTheCurrentStateProjectData::CCVGItem &vrItem);
  status            ItemChanged(const fw::CCVGFWTheCurrentStateProjectData::CCVGItem &vrItem);
  void              ItemCCVGExtnDataDelete(QTreeWidgetItem &vrNode);
  status            ItemRemoved(const gtString &vrItemName);
  status            NodeAdd(afTreeItemType veType, QTreeWidgetItem &vrParent, QTreeWidgetItem *&vpNewNode);
  status            NodeAddRoots();
  QTreeWidgetItem*  NodeFind(const gtString &vrItemName);
  bool              NodeFindValidateOnlyOneByThatName(const gtString &vrItemName);
  QTreeWidgetItem*  NodeFind(QTreeWidgetItem &vCurrentTreeItem, afTreeItemType veType);
  QTreeWidgetItem*  NodeFind(QTreeWidgetItem &vrParent, const gtString &vSearchText);
  status            NodeFactory(afTreeItemType veType, QTreeWidgetItem &vrParent, QTreeWidgetItem *&vpNewNode);
  status            NodeFactory(const fw::CCVGFWTheCurrentStateProjectData::CCVGItem &vrItem, afTreeItemType veType, QTreeWidgetItem &vrParent, QTreeWidgetItem *&vpNewNode);
  const fw::CCVGFWTheCurrentStateProjectData::CCVGItem* 
                    NodeGetDataExtn(QTreeWidgetItem &vNode);
  status            NodeRemove(QTreeWidgetItem &vrNode);
  status            NodeRemoveRecursiveChildren(QTreeWidgetItem &vrNode);
  status            TreeClear();
  status            TreeEnableSorting(bool vbYes);
  status            UpdateTreeFromProjectData(const fw::CCVGFWTheCurrentStateProjectData &vrProjData);
  //
  // CCVG state
  bool GetCCVGExecModeEnabled() const;
  bool GetCCVGExecModeCurrent() const;
  bool GetCCVGIsProjectOpen() const;
  bool GetCCVGIsProjectEmpty() const;
  //
  // Tree item context handling
  bool    ContextMenuBuildForSingleItem(const afApplicationTreeItemData *vpItemData, QMenu &vrwMenu);
  bool    ContextBuildMenuRootNode(QMenu &vrwMenu);
  bool    ContextBuildMenuCCVGRootNodeSetFilesGood(QMenu &vrwMenu); 
  bool    ContextBuildMenuCCVGItem(QMenu &vrwMenu);
  bool    ContextBuildMenuCCVGItemDrivers(QMenu &vrwMenu);
  bool    ContextBuildMenuCCVGItemResult(QMenu &vrwMenu);
  bool    ContextBuildMenuCCVGItemExec(QMenu &vrwMenu);
  status  ContextMenuCreateActions();
  //
  // Tree item selection
  QTreeWidgetItem*                                      GetSelectedItemFirst();
  const fw::CCVGFWTheCurrentStateProjectData::CCVGItem* GetSelectedItemDataCCVGItem();
  
// Qt signal slot methods:
private slots:
  void OnApplicationTreeCreated();
  void ActionDoRefreshCCVGItemsReadFiles();
  void ActionDoCCVGItemViewCoverageResults();
  void ActionDoCCVGItemDriverSrcCodeView();
  void ActionDoCCVGItemResultFileView();
  void ActionDoCCVGItemExecFileView();
  
// Attributes:
private:
  afApplicationTree *m_pApplicationTree;
  bool               m_bRegisterAppTreeHandlerViaSigSlot; // True = yes signal slot used for registration, false = no used
  QTreeWidgetItem   *m_pProgramsRootNode;
  QTreeWidgetItem   *m_pCCVGItemsRootNodeGood;
  QTreeWidgetItem   *m_pCCVGItemsRootNodeBad;
  bool               m_bCCVGItemsSortedInTree;            // True = yes sorted tree, false = other Exec Mode sorting
  //
  // Context menu actions
  QAction *m_pActionCCVGItemsRefreshAll;
  QAction *m_pActionCCVGItemViewCoverageResults;
  QAction *m_pActionCCVGItemDriverSrcCodeView;
  QAction *m_pActionCCVGItemResultFileView;
  QAction *m_pActionCCVGItemExecFileView;
};
// clang-format on

} // namespace gui
}  //namespace ccvg

#endif // #define LPGPU2_CCVG_GUI_APPTREEHANDLER_H_INCLUDE
