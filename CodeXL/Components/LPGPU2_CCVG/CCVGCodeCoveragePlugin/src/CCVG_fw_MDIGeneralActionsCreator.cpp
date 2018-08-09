// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGMDIGeneralActionsCreator implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIGeneralActionsCreator.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_CommandIDs.h>

namespace ccvg {
namespace fw {

/// @brief    Class constructor. No work is done here by this class.
CCVGMDIGeneralActionsCreator::CCVGMDIGeneralActionsCreator()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGMDIGeneralActionsCreator::~CCVGMDIGeneralActionsCreator()
{
}

/// @brief  Create a vector of command Ids that are supported by *this actions
///         actions creator object. Each derived class must populate the vector 
///         of supported command IDs. Populate vector m_supportedCommandIds. 
///         The index into the container is the action index. Action index is 
///         not the same as the command ID. Use actionIndexToCommandId() for 
///         converstion. 
///         Called by
///           afActionCreatorAbstract::initializeCreator().
void CCVGMDIGeneralActionsCreator::populateSupportedCommandIds()
{
  // The Framework's (FW) general actions creator
  // Edit menu:
  // ID_CUT);
  // ID_COPY);
  // ID_PASTE);
  // ID_FIND);
  // ID_FIND_NEXT);
  // ID_FIND_PREV);
  // ID_SELECT_ALL);
  // ID_GO_TO);

  // FW general actions creator
  // File menu:
  // AF_ID_SAVE_FILE);
  // AF_ID_SAVE_FILE_AS);
  
  // FW menu actions executor
  // File menu:
  // AF_ID_NEW_PROJECT);
  // AF_ID_OPEN_PROJECT);
  // AF_ID_CLOSE_PROJECT);
  // AF_ID_OPEN_FILE);
  // AF_ID_OPEN_STARTUP_PAGE);
  // AF_ID_SAVE_PROJECT);
  // AF_ID_SAVE_PROJECT_AS);
  // AF_ID_PROJECT_SETTINGS);
  // AF_ID_EXIT);

  // FW menu actions executor
  // View menu:
  // AF_ID_RESET_GUI_LAYOUTS);

  // FW menu actions executor
  // Tools menu:
  // AF_ID_TOOLS_SYSTEM_INFO);
  // AF_ID_TOOLS_OPTIONS);
  
  // FW menu actions executor
  // Help menu:
  // AF_ID_HELP_USER_GUIDE);
  // AF_ID_HELP_QUICK_START);
  // AF_ID_HELP_DEV_TOOLS_SUPPORT_FORUM);
  // AF_ID_HELP_ABOUT);
  
  // FW recent projects actions creator
  // File menu:
  // ID_RECENTLY_USED_PROJECT_0);
  // ID_RECENTLY_USED_PROJECT_1);
  // ID_RECENTLY_USED_PROJECT_2);
  // ID_RECENTLY_USED_PROJECT_3);
  // ID_RECENTLY_USED_PROJECT_4);
  // ID_RECENTLY_USED_PROJECT_5);
  // ID_RECENTLY_USED_PROJECT_6);
  // ID_RECENTLY_USED_PROJECT_7);
  // ID_RECENTLY_USED_PROJECT_8);
  // ID_RECENTLY_USED_PROJECT_9);
  // ID_RECENT_PROJECTS_SUB_MENU);

  // CCVG menu actions creator
  // Code Coverage menu:
  m_supportedCommandIds.push_back(ccvg::fw::kMainMenuItemCommandID_CCVGMode);
  m_supportedCommandIds.push_back(ccvg::fw::kMainMenuItemCommandID_ProjectSettings);
  m_supportedCommandIds.push_back(ccvg::fw::kMainMenuItemCommandID_RefreshCCVGResultsFiles);

  // CCVG source code actions creator
  // fill the vector of supported command ids:
  //m_supportedCommandIds.push_back(ID_CUT);
  //m_supportedCommandIds.push_back(ID_COPY);
  //m_supportedCommandIds.push_back(ID_PASTE);
  //m_supportedCommandIds.push_back(ID_SELECT_ALL);
  //m_supportedCommandIds.push_back(ID_FIND);
  //m_supportedCommandIds.push_back(ID_FIND_NEXT);
  //m_supportedCommandIds.push_back(ID_FIND_PREV);
  //m_supportedCommandIds.push_back(ID_SHOW_LINE_NUMBERS);
  //m_supportedCommandIds.push_back(AF_ID_SAVE_FILE);
  //m_supportedCommandIds.push_back(AF_ID_SAVE_FILE_AS);
  //m_supportedCommandIds.push_back(ID_GO_TO);
}

/// @brief:     Build the menu. Get the menu position from a formating string. 
///             appMainAppWindow::setNewActionProperties() calls this function 
///             to build the application's main menus.
///              
/// The string is composed of menu levels seperated by '/'. For example if 
/// commmand is "start" in "Debug" menu then return the value "Debug". However
/// is the command is in a submenu "Actions" under "Debug" then the value 
/// should be "Debug/Actions". Represents name/priority.
/// If separator is needed after the item then 's' after the priority is needed
/// in case of a sub menu if one item is marked with an 's' it is enough to 
/// mark a separator after it. 
/// Called by functions:
///    setNewActionProperties()
///    createSingleViewAction()
///    createSingleAction()
///
/// @param[in]   vActionIndex Index of the vector of supported command ids by 
///              *this excecutor.
/// @param[out]  vrPositionData Defines additional control of item position 
///              within its parent menu: afCommandPosition, start/middle/end 
///              block in the menu. afCommandSeperatorType if the command 
///              requires a seperator. Should the command come before another
///              action, use additional parameters; m_beforeActionMenuPosition
///              the menu position of the command, m_beforeActionText the 
///              command text.
/// @return      gtString The user facing menu item's text describing menu action.
gtString CCVGMDIGeneralActionsCreator::menuPosition(int vActionIndex, afActionPositionData &vrPositionData)
{
  GT_UNREFERENCED_PARAMETER(vActionIndex);
  GT_UNREFERENCED_PARAMETER(vrPositionData);
  return L"";
}

/// @brief Create toolbar item and its position. Get the name of the toolbar 
///        that the command belongs to. If no name is returned (empty) the the
///        command will not be added to any toolbar. 
///        Called by functions:
///           setNewActionProperties()
///           createSingleViewAction()
///           createSingleAction()
/// @param[in]   vActionIndex Index of the vector of supported command ids by 
///              *this excecutor.
/// @return gtString Toolbar ID.
gtString CCVGMDIGeneralActionsCreator::toolbarPosition(int vActionIndex)
{
  GT_UNREFERENCED_PARAMETER(vActionIndex);
  return L"";
}

/// @brief      If the requested action should be a part of a group then 
///             override this function. If the requested action should be a 
///             part of a group, create the action group and add the relevant 
///             actions to the group.Grouped actions are actions that only
///             one can be active at a time. Create the action group and add
///             the relevant actions to the group. Get the group command ids 
///             for the action. Called by:
/// @param[in]  vActionIndex Index of the vector of supported command ids by 
///             *this excecutor.
void CCVGMDIGeneralActionsCreator::groupAction(int vActionIndex)
{
  GT_UNREFERENCED_PARAMETER(vActionIndex);
}

/// @brief      Get the caption, tooltip and accelerator for a specified action 
///             item. The common edit actions 
///             that are shared by most action creators are covered by this 
///             function. A derived class should over-ride this function if it
///             implements actions that do not appear in this list. Called by
///             functions:
///               createSingleViewAction()
/// @param[in]  vActionIndex Index of the vector of supported command ids by 
///             *this excecutor.
/// param[out]  vrActionText Action's user facing text description of the 
///             option. The caption
/// param[out]  vrTooltip Action's tooltip assignment.
/// param[out]  vrKeyboardShortcut Action short cut.
/// return bool True = success, false = failure.
bool CCVGMDIGeneralActionsCreator::actionText(int vActionIndex, gtString &vrActionText, gtString &vrTooltip, gtString &vrKeyboardShortcut)
{
  GT_UNREFERENCED_PARAMETER(vActionIndex);
  GT_UNREFERENCED_PARAMETER(vrActionText);
  GT_UNREFERENCED_PARAMETER(vrTooltip);
  GT_UNREFERENCED_PARAMETER(vrKeyboardShortcut);
  return true;
}

/// @brief If a toolbar separator needs to be added before/after the toolbar
///        command then override this function. 
/// @param[in] vnActionID Action index.
/// @return int -1 = place seperator before, 1 = after
// int seperatorPosition(int vnActionID)

/// @brief Get the pixmap of the command and whether it should be used in the
///        actions menu (it will be used in the toolbar if a toolbar string 
///        was supplied)
/// @param[in]  vActionIndex Action Index.
/// @param[out] vbShouldBeUsedInMenu True = yes, false = no
// QPixmap* iconAsPixmap(int vActionIndex, bool &vbShouldBeUsedInMenu)

/// @brief If actions are using predefined icons override this function. If 
///        they do for each command that does use the function 
///        initSingleActionIcon() to connect between the two.
// void initActionIcon()


} // namespace fw
} // namespace ccvg
