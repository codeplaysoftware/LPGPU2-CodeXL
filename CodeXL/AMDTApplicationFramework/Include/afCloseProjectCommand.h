// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// afCloseProjectCommand interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_AFCLOSEPROJECTCOMMAND_H
#define __AFCLOSEPROJECTCOMMAND_H

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>

/// @brief    A command which is handling the closing og an application 
///           framework project.
/// @warning  None.
/// @see      None.
/// @date     13/01/2017.
/// @author   Illya Rudkin.
class AF_API afCloseProjectCommand 
: public afCommand
{
// Methods:
public:
  afCloseProjectCommand();

// Overridden:
public:
  // From afCommand
  virtual      ~afCloseProjectCommand() override;
  virtual bool canExecuteSpecificCommand() override;
  virtual bool executeSpecificCommand() override;
};

#endif // LPGPU2_CCVG_AFCLOSEPROJECTCOMMAND_H

