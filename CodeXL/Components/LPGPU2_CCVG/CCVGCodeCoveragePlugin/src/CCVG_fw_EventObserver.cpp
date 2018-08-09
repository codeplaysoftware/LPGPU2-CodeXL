// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWEventObserver implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTAPIClasses/Include//Events/apExecutionModeChangedEvent.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_EventObserver.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>

namespace ccvg {
namespace fw {

/// @brief    Class constructor. No work is done here by this class.
CCVGFWEventObserver::CCVGFWEventObserver()
: m_bBeenInitialised(false)
, m_bBeenShutdown(false)
, m_pApplicationTree(nullptr)
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGFWEventObserver::~CCVGFWEventObserver()
{
  Shutdown();
}

/// @brief Object initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWEventObserver::Initialise()
{
  if (m_bBeenInitialised)
  {
    return success;
  }

  // Register as an events observer. The priority parameter sets how *this 
  // observer gets queued such that it handles the event before others or 
  // later.
  apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Package shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWEventObserver::Shutdown()
{
  if (m_bBeenShutdown)
  {
    return success;
  }

  // Tidy up
  // Register as an events observer:
  apEventsHandler::instance().unregisterEventsObserver(*this);
  
  m_pApplicationTree = nullptr;

  m_bBeenShutdown = true;

  return success;
}

/// @brief     Called when an application wide or execution mode event occurs.
///            The function gets visited for all events that can occur. Any 
///            events not handled must be allowed to pass on to other 
///            observers. An observer can veto or block an event being passed
///            on to later observers if desired. Is called when a debugged 
///            process event occurs (debugger execution mode ?).
/// @param[in] vEvent An event object.
/// @param[out] vbVetoEvent True = do not pass on, false = allow other 
///             handlers.
void CCVGFWEventObserver::onEvent(const apEvent &vEvent, bool & vbVetoEvent)
{
  GT_UNREFERENCED_PARAMETER(vbVetoEvent);
  GT_UNREFERENCED_PARAMETER(vEvent);

  // Handle the Global variable changed event
  const apEvent::EventType eventType = vEvent.eventType();
  switch (eventType)
  {
    case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
    {
      // Get id of the global variable that was changed
      const afGlobalVariableChangedEvent &rGlobalVarChangedEvent = dynamic_cast<const afGlobalVariableChangedEvent &>(vEvent);
      afGlobalVariableChangedEvent::GlobalVariableId variableId = rGlobalVarChangedEvent.changedVariableId();

      // If the project file path was changed
      if (variableId == afGlobalVariableChangedEvent::CURRENT_PROJECT)
      {
        // Closed the current project
        if (afProjectManager::instance().currentProjectSettings().projectName().isEmpty())
        {
          // Left here to show CodeXL authors way of handling closed project. Replaced by 
          // afCloseProjectCommand and notification sent out from that.
        }
      }
    }
    break;
    case apEvent::AP_MDI_ACTIVATED_EVENT:
    {
      // For future implementation reference
      //const apMDIViewActivatedEvent &rActivationEvent = (const apMDIViewActivatedEvent &) vEvent;
      //OnMdiActivateEvent(rActivationEvent);
    }
    break;
    case apEvent::AP_MDI_CREATED_EVENT:
    {
      // For future implementation reference
      //const apMDIViewCreateEvent &rCreationEvent = (const apMDIViewCreateEvent &) vEvent;
      //RegisterToSaveSignal(rCreationEvent);
    }
    break;
    case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
    {
      const apExecutionModeChangedEvent &rModeEvent = (const apExecutionModeChangedEvent &) vEvent;
      const wchar_t *pModeName = CCVGRSRC(kIDS_CCVG_STR_execmode_name);
      const gtString &rModeName(rModeEvent.modeType());
      if (rModeName == pModeName)
      {
        // For future implementation reference
      }
    }
    break;
    default:
      break;
  }
}

/// @brief retrieve *this event observer's unique name. 
const wchar_t* CCVGFWEventObserver::eventObserverName() const
{ 
  return L"CCVGEventObserver"; 
}

/// @brief     This method should be overridden to allow usage of event 
///            registration.
/// @param[in] vEvent An event object.
/// @param[out] vbVetoEvent True = do not pass on, false = allow other 
///             handlers.
void CCVGFWEventObserver::onEventRegistration(apEvent &vEvent, bool &vbVetoEvent)
{
  GT_UNREFERENCED_PARAMETER(vEvent); 
  GT_UNREFERENCED_PARAMETER(vbVetoEvent);
  // Not implemented
}

/// @brief Gets the run modes mask from all plugins. A bit mask of operation
///        modes describing whether there is a session running.
/// @return afRunModes 0 = no mask, >0 = Bitwise OR of afRunMode values.
afRunModes CCVGFWEventObserver::getCurrentRunModeMask()
{
  const afRunModes retVal = 0;

  return retVal;
}

/// @brief  Retrieve whether the current working process or state of this 
///         execution mode can be stopped.
/// @return bool true = yes able to stop, false = no not at the moment.
bool CCVGFWEventObserver::canStopCurrentRun()
{
  return true;
}

/// @brief Have attempts to stop any existing working process in any plugin
///        been successful or failed.
/// @return bool - True = success, false = failure.
bool CCVGFWEventObserver::stopCurrentRun()
{
  return true;
}

/// @brief Query the run mode managers which one generated the exception event 
///        and gets the details from that one.
/// @return bool - True = success, false = failure.
bool CCVGFWEventObserver::getExceptionEventDetails(const apExceptionEvent &vExceptionEvent, osCallStack &vExceptionCallStack, bool &vOpenCLEnglineLoaded, bool &vOpenGLEnglineLoaded, bool &vKernelDebuggingEnteredAtLeastOnce)
{
  GT_UNREFERENCED_PARAMETER(vExceptionEvent);
  GT_UNREFERENCED_PARAMETER(vExceptionCallStack);
  GT_UNREFERENCED_PARAMETER(vOpenCLEnglineLoaded);
  GT_UNREFERENCED_PARAMETER(vOpenGLEnglineLoaded);
  GT_UNREFERENCED_PARAMETER(vKernelDebuggingEnteredAtLeastOnce);
  // Not implemented
  return true;
}

/// @brief Get the pointer to the CodeXL application tree control.
void CCVGFWEventObserver::GetApplicationTree()
{
  afApplicationCommands *pApplicationCommands = afApplicationCommands::instance();
  GT_ASSERT(nullptr != pApplicationCommands);
  m_pApplicationTree = pApplicationCommands->applicationTree();
  GT_ASSERT(nullptr != m_pApplicationTree);
}

} // namespace fw
} // namespace ccvg
