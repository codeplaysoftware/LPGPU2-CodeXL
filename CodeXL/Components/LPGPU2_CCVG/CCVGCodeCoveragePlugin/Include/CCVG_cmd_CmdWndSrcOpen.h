// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdWndSrcOpeninterface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDWNDSRCOPEN_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDWNDSRCOPEN_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBaseWithParams.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Parameter.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFileRead.h>

namespace ccvg {
namespace cmd {

/// @brief    CCVG command to open a Qt MDI window on CodeXL and display the
///           source file text content of a specified Code Coverage Driver
///           file.
/// @warning  None.
/// @see      cmd::Factory, cmd::Invoker, cmd::Params, cmd::CmdID.
/// @date     14/02/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdWndSrcOpen final
: public CCVGCmdBaseWithParams
{
// Enums:
public:
  enum ParamID
  {
    kParamID_CCVGItemId,
    kParamID_CCVGItemExt,
    kParamID_FileName,
    kParamID_FileDirPath,
    kParamID_ViewAttributeShowLineNumbersFlag,
    kParamID_ViewAttributeIsUserEditable,
  };

// Statics:
public:
  // Required by the CCVGCmdFactory when registering *this command
  static CCVGCmdBase* CreateSelf();

// Methods:
public:
  CCVGCmdWndSrcOpen();

// Overridden:
public:
  virtual ~CCVGCmdWndSrcOpen() override;
  // From CCVGCmdFactory
  virtual CCVGCmdFactory::CmdCreatorFnPtr GetCmdCreatorFn() const override;
  // From CCVGCmdInvoker
  virtual status Initialise() override;
  virtual status Shutdown() override;
  virtual status Execute() override;
  virtual status Undo() override;
  virtual bool   GetIsUndoableFlag() const override;

// Methods:
private:
  void   DeleteSubCmds();
  status HandleSubCmdError(CCVGCmdBase *vpSubCmd);
  status FileContentsRead();
  status FileContentsShowMDIView();
  void   OnErrorReleaseResourcesAllocByThisCmd();

// Attributes:
private:
  CCVGCmdParameter m_paramCCVGItemId;
  CCVGCmdParameter m_paramCCVGItemExt;
  CCVGCmdParameter m_paramFileName;
  CCVGCmdParameter m_paramFileDirPath;
  CCVGCmdParameter m_paramViewAttributeShowLineNumbersFlag;  
  CCVGCmdParameter m_paramViewAttributeIsUserEditable;  
  const size_t     m_constReadFileBufferMaxSizeBytes;
  //
  // Sub commands
  CCVGCmdFileRead *m_pCmdSubCmdFileRead;
};
// clang-format on

} // namespace cmd
} // namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDWNDSRCOPEN_H_INCLUDE