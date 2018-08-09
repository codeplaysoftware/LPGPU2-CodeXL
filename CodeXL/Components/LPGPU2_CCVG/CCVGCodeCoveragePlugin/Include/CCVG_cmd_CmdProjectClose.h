// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdProjectClose interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDPROJECTCLOSE_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDPROJECTCLOSE_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBase.h>

namespace ccvg {
namespace cmd {

/// @brief    CCVG command Project Close. Close the CCVG project.
/// @warning  None.
/// @see      cmd::Factory, cmd::Invoker, cmd::Params, cmd::CmdID.
/// @date     19/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdProjectClose final
: public CCVGCmdBase
{
// Statics:
public:
  // Required by the CCVGCmdFactory when registering *this command
  static CCVGCmdBase* CreateSelf();

// Methods:
public:
  CCVGCmdProjectClose();

// Overridden:
public:
  virtual ~CCVGCmdProjectClose() override;
  // From CCVGCmdBase
  virtual bool           GetHasParams() const override;
  virtual CCVGCmdParams& GetParams() override;
  // From CCVGCmdFactory
  virtual CCVGCmdFactory::CmdCreatorFnPtr GetCmdCreatorFn() const override;
  // From CCVGCmdInvoker
  virtual status Initialise() override;
  virtual status Shutdown() override;
  virtual status Execute() override;
  virtual status Undo() override;
  virtual bool   GetIsUndoableFlag() const override;
};
// clang-format on

} // namespace cmd
} //  namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDPROJECTCLOSE_H_INCLUDE