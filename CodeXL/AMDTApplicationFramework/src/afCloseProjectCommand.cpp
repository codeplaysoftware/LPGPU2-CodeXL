// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// afCloseProjectCommand implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTApplicationFramework/Include/afCloseProjectCommand.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

/// @brief Class constructor. No work done here.
afCloseProjectCommand::afCloseProjectCommand()
{
}

/// @brief Class destructor.
afCloseProjectCommand::~afCloseProjectCommand()
{
}

/// @brief Overidden. Answers the question - can we save the project. 
///        application Implementation Notes: Currently - the answer is always 
///        yes.
/// @return bool True = save project, false = no.
bool afCloseProjectCommand::canExecuteSpecificCommand()
{
    return true;
}

/// @brief Overridden. Close the project 
/// @return bool - True = success, false = failure.
bool afCloseProjectCommand::executeSpecificCommand()
{
  bool bOk  = true;

  afProjectManager &rProjMgr(afProjectManager::instance());
  int countErrors = 0;
  const int extensionsNumber = rProjMgr.amountOfProjectExtensions();
  for (int i = 0; i < extensionsNumber; i++)
  {
    countErrors += rProjMgr.closeCurrentProject(i) ? 0 : 1;
  }
 
  bOk = (countErrors == 0);
  return bOk;
}

