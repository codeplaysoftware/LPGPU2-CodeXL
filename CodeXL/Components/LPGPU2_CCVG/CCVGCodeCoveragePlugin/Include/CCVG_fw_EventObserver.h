// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWEventObserver interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_EVENTOBSERVER_H_INCLUDE
#define LPGPU2_CCVG_FW_EVENTOBSERVER_H_INCLUDE

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTApplicationFramework/Include/afIRunModeManager.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWTheCurrentState provides information on the current state
///           of the CodeXL framework and state of the CCVG execution mode.
///           This information can be used to by the GUI with such things like
///           menu items being checked unchecked enabled or greyed out. 
///           Commands can also use this information to carry out tasks.
/// @warning  None.
/// @see      None.
/// @date     09/09/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWEventObserver final
: public CCVGClassCmnBase
, public apIEventsObserver
, public afIRunModeManager
{
// Methods:
public:
  CCVGFWEventObserver();
 //
  status Initialise();
  status Shutdown();

// Overridden:
public:  
  virtual ~CCVGFWEventObserver() override;
  // From apIEventsObserver
  virtual void            onEvent(const apEvent &vEvent, bool &vbVetoEvent) override;
  virtual const wchar_t*  eventObserverName() const override;
  virtual void            onEventRegistration(apEvent &vEvent, bool &vbVetoEvent) override;
  // From afIRunModeManager
  virtual afRunModes getCurrentRunModeMask() override;
  virtual bool       canStopCurrentRun() override;
  virtual bool       stopCurrentRun() override;
  virtual bool       getExceptionEventDetails(const apExceptionEvent &vExceptionEvent, osCallStack &vExceptionCallStack, bool &vOpenCLEnglineLoaded, bool &vOpenGLEnglineLoaded, bool &vKernelDebuggingEnteredAtLeastOnce) override;

// Methods:
private: 
  void GetApplicationTree();

// Attributes:
private:
  bool m_bBeenInitialised;  // True = yes this has been initialised, false = not yet
  bool m_bBeenShutdown;     // True = yes this has been shutdown already, false = not yet
  //                         
  afApplicationTree *m_pApplicationTree;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif //LPGPU2_CCVG_FW_EVENTOBSERVER_H_INCLUDE