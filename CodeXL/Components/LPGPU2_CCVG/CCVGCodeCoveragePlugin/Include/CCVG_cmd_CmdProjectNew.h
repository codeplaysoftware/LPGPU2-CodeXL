// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdProjectNew interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDPROJECTNEW_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDPROJECTNEW_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBaseWithParams.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Parameter.h>

namespace ccvg {
namespace cmd {

/// @brief    CCVG command Project New. Create a CCVG project.
/// @warning  None.
/// @see      cmd::Factory, cmd::Invoker, cmd::Params, cmd::CmdID.
/// @date     19/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdProjectNew final
: public CCVGCmdBaseWithParams
{
// Enums:
public:
  enum ParamID
  {
    kParamID_ProjName
  };

// Statics:
public:
  // Required by the CCVGCmdFactory when registering *this command
  static CCVGCmdBase* CreateSelf();

// Methods:
public:
  CCVGCmdProjectNew();

// Overridden:
public:
  virtual ~CCVGCmdProjectNew() noexcept override;
  // From CCVGCmdFactory
  virtual CCVGCmdFactory::CmdCreatorFnPtr GetCmdCreatorFn() const override;
  // From CCVGCmdInvoker
  virtual status Initialise() override;
  virtual status Shutdown() override;
  virtual status Execute() override;
  virtual status Undo() override;
  virtual bool   GetIsUndoableFlag() const override;

// Attributes:
private:
  CCVGCmdParameter m_paramProjectName;     
};
// clang-format on

} // namespace cmd
} //  namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDPROJECTNEW_H_INCLUDE