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

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppMenuActionExecutor.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppAppController.h>

// QT:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

// AMDTSharedProfiling
#include <AMDTSharedProfiling/inc/StringConstants.h>
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>


enum EnumLPGPU2MainMenuCommands
{
    kCommandId_RunFeedbackEngine = COMMAND_ID_COUNTERS_SELECTION + 1,
    kCommandId_ExportSessionToCSV
};

/// @brief Class constructor. No work is done here in *this class.
LPGPU2ppMenuActionExecutor::LPGPU2ppMenuActionExecutor()
    : ppMenuActionsExecutor()
{
}

/// @brief Class destructor. No work is done here in this class.
LPGPU2ppMenuActionExecutor::~LPGPU2ppMenuActionExecutor() = default;

/// @brief  Get the caption and tooltip for a specified action item.
/// @param[in]  vActionIndex Index of the vector of supported command ids by 
///             *this executor.
/// param[out]  vrCaption Action's user facing text description of the 
///             option. The caption
/// param[out]  vrTooltip Action's tooltip assignment.
/// param[out]  vrKeyboardShortcut Action short cut.
/// return bool True = success, false = failure.
bool LPGPU2ppMenuActionExecutor::actionText(const int vActionIndex, gtString &vrCaption, gtString &vrTooltip, gtString &vrKeyboardShortcut)
{
    GT_UNREFERENCED_PARAMETER(vrKeyboardShortcut);
    GT_UNREFERENCED_PARAMETER(vrTooltip);
    
    auto bReturn = false;

    const auto commandId = actionIndexToCommandId(vActionIndex);
    switch (commandId)
    {
        case kCommandId_RunFeedbackEngine:
        {
            vrCaption = PP_STR_RunLPGPU2FeedbackEngine_W;
            bReturn = true;
            break;
        }
        case kCommandId_ExportSessionToCSV:
        {
            vrCaption = PP_STR_ExportSessionToCSV_W;
            bReturn = true;
            break;
        }
        default:
        {
            // In case this is an unknown command id, let the base class handle the action text.
            bReturn = ppMenuActionsExecutor::actionText(vActionIndex, vrCaption, vrTooltip, vrKeyboardShortcut);
            break;
        }
    }

    return bReturn;
}

/// @brief      Build the menu. Menu position. Where to insert a menu action
///             item to build up the various application menus.
///             appMainAppWindow::setNewActionProperties() calls this function 
///             to build the application's main menus.
/// @param[in]   vActionIndex Index of the vector of supported command ids by 
///              *this executor.
/// @param[out]  vrPositionData Defines additional control of item position 
///              within its parent menu: afCommandPosition, start/middle/end 
///              block in the menu. afCommandSeperatorType if the command 
///              requires a separator. Should the command come before another
///              action, use additional parameters; m_beforeActionMenuPosition
///              the menu position of the command, m_beforeActionText the 
///              command text.
/// @return      gtString The user facing menu item's text describing menu action.
gtString LPGPU2ppMenuActionExecutor::menuPosition(const int vActionIndex, afActionPositionData &vrPositionData)
{
    gtString retVal;
    
    vrPositionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;

    const auto commandId = actionIndexToCommandId(vActionIndex);

    switch (commandId)
    {
        case kCommandId_RunFeedbackEngine:
        {
            retVal = AF_STR_ProfileMenuString;
            vrPositionData.m_beforeActionMenuPosition = AF_STR_ProfileMenuString;
            vrPositionData.m_beforeActionText = PM_STR_MENU_SETTINGS;
            break;
        }
        case kCommandId_ExportSessionToCSV:
        {
            retVal = AF_STR_ProfileMenuString;
            vrPositionData.m_beforeActionMenuPosition = AF_STR_ProfileMenuString;
            vrPositionData.m_beforeActionText = PM_STR_MENU_SETTINGS;
            break;
        }
        default:
            // In case this is an unknown command id, let the base class handle the menu position
            retVal = ppMenuActionsExecutor::menuPosition(vActionIndex, vrPositionData);
            break;
    }

    return retVal;
}

/// @brief Get the name of the toolbar name that the action ID belongs to. If
///        no string is supplied (empty string) then the command will not be 
///        added to the toolbar. Create toolbar item and its position. 
///        Called by functions:
///           setNewActionProperties()
///           createSingleViewAction()
///           createSingleAction()
/// @param[in]   vActionIndex Index of the vector of supported command ids by 
///              *this executor.
/// return gtString Toolbar ID.
gtString LPGPU2ppMenuActionExecutor::toolbarPosition(const int vActionIndex)
{
    const auto commandId = actionIndexToCommandId(vActionIndex);

    switch (commandId)
    {
        case kCommandId_RunFeedbackEngine:
        case kCommandId_ExportSessionToCSV:
            return L"";
        default:
            break;
    }

    // In case this is an unknown command id, let he base class handle the toolbar position.
    return ppMenuActionsExecutor::toolbarPosition(vActionIndex);
}

/// @brief      Handle the action when it is triggered. Fire of a command 
///             perhaps. Called by:
///             appMainAppWindow::triggerGlobalAction()
///             appMainAppWindow::triggerViewAction().
/// @param[in]  vActionIndex Index of the vector of supported command ids by 
///             *this executor.
void LPGPU2ppMenuActionExecutor::handleTrigger(const int vActionIndex)
{
    GT_IF_WITH_ASSERT(m_pApplicationCommandsHandler != nullptr)
    {
        const auto commandId = actionIndexToCommandId(vActionIndex);
        const auto currentDisplaySessionId = ProfileApplicationTreeHandler::instance()->GetCurrentDisplaySessionId();

        auto &appController = ppAppController::instance();

        switch (commandId)
        {
            case kCommandId_RunFeedbackEngine:
            {
                // execute command                
                appController.RunLPGPU2FeedbackEngine(currentDisplaySessionId);
                return;
            }          
            case kCommandId_ExportSessionToCSV:
            {               
                appController.ExportSessionToCSV(currentDisplaySessionId);
                return;
            }
            default:
                ppMenuActionsExecutor::handleTrigger(vActionIndex);
                break;
        }        
    }
}

/// @brief     Handle UI updates of the command or menu action items. Change 
///            handling or look of the QAction. To get the associated QAction
///            use function QAction* action(vActionIndex).
///            Called by:
///               appMainAppWindow::updateViewAction()
///               appMainAppWindow::updateGlobalAction()
/// @param[in] vActionIndex Index of the vector of supported command ids by 
///            *this executor.
void LPGPU2ppMenuActionExecutor::handleUiUpdate(const int vActionIndex)
{      
    const auto commandId = actionIndexToCommandId(vActionIndex);

    auto &executionMode = afExecutionModeManager::instance();
    auto &thePluginConnectionManager = afPluginConnectionManager::instance();
    auto &theAppController = ppAppController::instance();
    auto *pAppTreeHandler = ProfileApplicationTreeHandler::instance();

    // Check if the session is stopped.            
    const auto currRunMode = thePluginConnectionManager.getCurrentRunModeMask();
    const auto bIsSessionRunning = static_cast<int>(currRunMode) != 0;
    const auto bIsPowerProfilingMode = executionMode.isActiveMode(PM_STR_PROFILE_MODE);
    const auto bIsSessionOn = theAppController.SessionIsOn();
    const auto currentDisplaySessionId = pAppTreeHandler->GetCurrentDisplaySessionId();

    switch (commandId)
    {
        case kCommandId_RunFeedbackEngine:
        {            
            const auto bCanRunFeedbackEngine = theAppController.CanRunLPGPU2FeedbackEngine(currentDisplaySessionId);

            auto bIsActionEnabled = !bIsSessionRunning && bIsPowerProfilingMode && !bIsSessionOn && bCanRunFeedbackEngine;

            if (!apEventsHandler::instance().areNoEventsPending())
            {
                bIsActionEnabled = false;
            }

            auto *pAction = action(vActionIndex);
            GT_IF_WITH_ASSERT(pAction != nullptr)
            {
                pAction->setEnabled(bIsActionEnabled);
                pAction->setCheckable(false);
                pAction->setChecked(false);
            }

            break;
        }
        case kCommandId_ExportSessionToCSV:
        {
            auto bIsActionEnabled = !bIsSessionRunning && bIsPowerProfilingMode && !bIsSessionOn && currentDisplaySessionId != SESSION_ID_ERROR;

            if (!apEventsHandler::instance().areNoEventsPending())
            {
                bIsActionEnabled = false;
            }

            auto *pAction = action(vActionIndex);
            GT_IF_WITH_ASSERT(pAction != nullptr)
            {
                pAction->setEnabled(bIsActionEnabled);
                pAction->setCheckable(false);
                pAction->setChecked(false);
            }

            break;
        }
        default:
            ppMenuActionsExecutor::handleUiUpdate(vActionIndex);
            break;
    }

}

/// @brief  Create a vector of command Ids that are supported by *this executor
///         actions. Each derived class must populate the vector of supported 
///         command IDs. Populate vector m_supportedCommandIds. The index into
///         the container is the action index. Action index is not the same as
///         the command ID. Use actionIndexToCommandId() for conversion. 
///         Called by
///           afActionCreatorAbstract::initializeCreator().
void LPGPU2ppMenuActionExecutor::populateSupportedCommandIds()
{
    // Let the base class add its commands.
    ppMenuActionsExecutor::populateSupportedCommandIds();

    // Add our custom menu action
    m_supportedCommandIds.push_back(kCommandId_RunFeedbackEngine);
    m_supportedCommandIds.push_back(kCommandId_ExportSessionToCSV);
}




