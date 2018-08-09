// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGMenuActionsExecutor interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_COMMAND_IDS_H_INCLUDE
#define LPGPU2_CCVG_FW_COMMAND_IDS_H_INCLUDE

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>

namespace ccvg {
namespace fw {

/// @brief Commands. 
///
/// Commands are added to main menu. There are two types of commmand both of 
/// which appear in the main menu in the same way. They are implemented through
/// the same class interface. 
/// Global commmands: Commands that are executed regardless of any views open, 
/// the current active view or views that are close. If a command is enabled and
/// executed it will carried out directly not going through the current actice
/// view or the current active view creator.
/// Local commands: Commands that are checked according to the current active 
/// view. An example would be the command 'Select'. If the current active view
/// does not enable the select command it will appear disabled. The execution
/// will be handled by the current active view. A 'Paste' command will be 
/// executed in the current active view.
///
/// Global commands are derived from afActionExecutorAbstract and registered.
/// Local commands are derived from afActionCreatorAbstract. A view creator
/// makes the connection between the command and the relevant view.
///
/// EnumMainMenuItemCommandID represents the menu item action IDs which are 
/// used to populate the m_supportedCommands container in both global and
/// and local command objects. The action IDs are indexes into the container
/// so use the ID to retrieve the action. An action is a menu item not a
/// command ID (though there is normally a one to one relationship). The 
/// action ID or index is used in all interfaces. To get the commmand 
/// represented by the action use the actionIndexToCommandId() function.
enum EnumMainMenuItemCommandID
{
  kMainMenuItemCommandID_CCVGMode = CCVG_FIRST_COMMAND_ID,
  kMainMenuItemCommandID_ProjectSettings,
  kMainMenuItemCommandID_RefreshCCVGResultsFiles,
  kMainMenuItemCommandID_count = kMainMenuItemCommandID_RefreshCCVGResultsFiles + 1
};

} // namespace fw
} // namespace ccvg

#endif  // LPGPU2_CCVG_FW_COMMAND_IDS_H_INCLUDE