// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  Add extra menu actions to the Profile menu in CodeXL interface.
///         The list of added actions is:
///             - "Run LPGPU2 Feedback Engine" 
///
/// LPGPU2ppMenuActionExecutor interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.
#ifndef LPGPU2_PP_MENU_ACTION_EXECUTOR_H_INCLUDE
#define LPGPU2_PP_MENU_ACTION_EXECUTOR_H_INCLUDE

// Local:
#include <AMDTPowerProfiling/src/ppMenuActionsExecutor.h>
#include <AMDTSharedProfiling/inc/SessionExplorerDefs.h>

class afApplicationCommands;

/// @brief   Extends the Power Profiling execution mode menu action executor
///          To allow addition of new commands, menu entries and tool buttons
///          into CodeXL interface.
/// @warning None.
/// @see     ppMenuActionsExecutor
/// @date    25/10/2017
/// @author  Thales Sabino
// clang-format off
class PP_API LPGPU2ppMenuActionExecutor : public ppMenuActionsExecutor
{
// Methods:
public:
    LPGPU2ppMenuActionExecutor();

// Overridden
public:    
    virtual ~LPGPU2ppMenuActionExecutor();

    // From ppMenuActionsExecutor
    bool actionText(int vActionIndex, gtString &vCaption, gtString &vTooltip, gtString &vKeyboardShortcut) override;
    gtString menuPosition(int actionIndex, afActionPositionData& positionData) override;
    gtString toolbarPosition(int actionIndex) override;
    void handleTrigger(int actionIndex) override;
    void handleUiUpdate(int actionIndex) override;

    // From ppMenuActionsExecutor
    void populateSupportedCommandIds() override;

// Attributes
private:
    bool m_bEnableFeedbackAction = false;
};
// clang-format on

#endif // LPGPU2_PP_MENU_ACTION_EXECUTOR_H_INCLUDE