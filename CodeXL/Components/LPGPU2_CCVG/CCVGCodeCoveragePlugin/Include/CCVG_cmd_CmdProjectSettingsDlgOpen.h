// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdProjectSettingDlgOpen interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDPROJECTSETTINGDLGOPEN_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDPROJECTSETTINGDLGOPEN_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBaseWithParams.h>

namespace ccvg {
namespace cmd {

/// @brief    CCVG command Project Settings Open Dialog. 
///
/// This command does not need parameters to be set externally to it by the
/// client (though it could). The parameters can be taken from the dialog on 
/// 'ok' closure. The command stores the parameters for later undo if
/// if required. Command attribute m_paramFlagUseDlgToGetSettings decides if
/// to use the dialog or pass parameters to command using its parameters 
/// container.
///
/// @warning  None.
/// @see      cmd::Factory, cmd::Invoker, cmd::Params, cmd::CmdID.
/// @date     06/09/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdProjectSettingDlgOpen final
: public CCVGCmdBaseWithParams
{
// Enums:
public:
  enum ParamID
  {
    kParamID_UseDlgFlag
  };

// Statics:
public:
  // Required by the CCVGCmdFactory when registering *this command
  static CCVGCmdBase* CreateSelf();

// Methods:
public:
  CCVGCmdProjectSettingDlgOpen();

// Overridden:
public:
  virtual ~CCVGCmdProjectSettingDlgOpen() noexcept override;
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
  CCVGCmdParameter m_paramFlagUseDlgToGetSettings; 
};
// clang-format on

} // namespace cmd
} //  namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDPROJECTSETTINGDLGOPEN_H_INCLUDE