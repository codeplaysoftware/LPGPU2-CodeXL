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

#ifndef LPGPU2_CCVG_FW_MDIGENERALACTIONSCREATOR_H_INCLUDE
#define LPGPU2_CCVG_FW_MDIGENERALACTIONSCREATOR_H_INCLUDE

// Infra:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afActionPositionData.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGMDIGeneralActionsCreator derived from 
///           afActionCreatorAbstract. Create local commands.
///
/// Commands are added to the main menu. There are two types of commmand both  
/// of which appear in the main menu in the same way. They are implemented 
/// through the same class interface. 
/// Local commands: Commands that are checked according to the current active 
/// view. An example would be the command 'Select'. If the current active view
/// does not enable the select command it will appear disabled. The execution
/// will be handled by the current active view. A 'Paste' command will be 
/// executed in the current active view.
/// A view creator makes the connection between the command and the relevant 
/// view.
/// This class is used for handling the 
/// creation and execution of all actions related to the framework.
/// These actions here are known as Local Commands. They are not
/// registered but instead connected to the view's creator object
/// i.e. pViewActionCreator->initializeCreator in the view's
/// constructor function.
/// 
/// @warning  None.
/// @see      None.
/// @date     16/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGMDIGeneralActionsCreator 
: public afActionCreatorAbstract
, public CCVGClassCmnBase
{
// Methods:
public:
  CCVGMDIGeneralActionsCreator();

// Overridden:
public:
  // From afActionCreatorAbstract
  virtual ~CCVGMDIGeneralActionsCreator() override;
  virtual gtString  menuPosition(int vActionIndex, afActionPositionData &vPositionData) override;
  virtual gtString  toolbarPosition(int vActionIndex) override;
  virtual void      groupAction(int vActionIndex) override;
  virtual bool      actionText(int vActionIndex, gtString &vActionText, gtString &vTooltip, gtString &vKeyboardShortcut) override;

// Overridden:
protected:
  // From afActionCreatorAbstract
  virtual void populateSupportedCommandIds() override;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif //LPGPU2_CCVG_FW_MDIGENERALACTIONSCREATOR_H_INCLUDE

