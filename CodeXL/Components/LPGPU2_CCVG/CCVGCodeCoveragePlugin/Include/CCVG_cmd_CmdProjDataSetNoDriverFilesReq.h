// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdProjDataSetNoDriverFilesReq interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDPROJECTSETNODRIVERSREQUIRED_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDPROJECTSETNODRIVERSREQUIRED_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBaseWithParams.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Parameter.h>

namespace ccvg {
namespace cmd {

/// @brief    CCVG command set driver are required state flag. The command will
///           iterate over all the project data, the CCVGItems, and change the 
///           state flag which indicates whether driver files are required or 
///           not required. This in turn changes the CCVGItem item's good state
///           flag. Which in turn will change how the data is presented to the
///           user.
/// @warning  None.
/// @see      cmd::Factory, cmd::Invoker, cmd::Params, cmd::CmdID.
/// @date     12/01/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdProjDataSetNoDriverFilesReq final
: public CCVGCmdBaseWithParams
{
// Enums:
public:
  enum ParamID
  {
    kParamID_FlagDriversRequired
  };

// Statics:
public:
  // Required by the CCVGCmdFactory when registering *this command
  static CCVGCmdBase* CreateSelf();

// Methods:
public:
  CCVGCmdProjDataSetNoDriverFilesReq();

// Overridden:
public:
  virtual ~CCVGCmdProjDataSetNoDriverFilesReq() override;
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
  CCVGCmdParameter m_paramFlagDriversRequired;
  bool             m_bPreviousSetDriversRequired; // True = Drivers required, false = optional
};
// clang-format on

} // namespace cmd
} //  namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDPROJECTSETNODRIVERSREQUIRED_H_INCLUDE