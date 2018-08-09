// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdFilesReadAll interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDFILESREADALL_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDFILESREADALL_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBaseWithParams.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Parameter.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadSource.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadResults.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadDrivers.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadExecuteable.h>

namespace ccvg {
namespace cmd {

/// @brief    CCVG command read in files from the code coverage results
///           files directory, code coverage source files directory and
///           optionally the code coverage driver (exercisor source files)
///           files. Store the data in the CCVG_fw_TheCurrentStateProjectData
///           object.
/// @warning  None.
/// @see      cmd::Factory, cmd::Invoker, cmd::Params, cmd::CmdID.
/// @date     30/12/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdFilesReadAll final
: public CCVGCmdBaseWithParams
{
// Enums:
public:
  enum ParamID
  {
    kParamID_FileDirResults,
    kParamID_FileExtnResults,
    kParamID_FileDirSource,
    kParamID_FileExtnSource,
    kParamID_FileDirDrivers,
    kParamID_FileExtnDriver,
    kParamID_FlagReadDriverFiles,
    kParamID_FileDirExecuteable,
    kParamID_FileExtnExecuteable
  };

// Statics:
public:
  // Required by the CCVGCmdFactory when registering *this command
  static CCVGCmdBase* CreateSelf();

// Methods:
public:
  CCVGCmdFilesReadAll();

// Overridden:
public:
  virtual ~CCVGCmdFilesReadAll() override;
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
  void   OnErrorReleaseResourcesAllocByThisCmd();

// Attributes:
private:
  CCVGCmdParameter m_paramFileDirResults;
  CCVGCmdParameter m_paramFileExtnResults;
  CCVGCmdParameter m_paramFileDirSource;
  CCVGCmdParameter m_paramFileExtnSource;
  CCVGCmdParameter m_paramFileDirDrivers;
  CCVGCmdParameter m_paramFileExtnDriver;
  CCVGCmdParameter m_paramFlagReadDriverFiles;
  CCVGCmdParameter m_paramFileDirExecuteable;
  CCVGCmdParameter m_paramFileExtnExecuteable;
  //
  // Sub commands
  CCVGCmdFilesReadDrivers     *m_pCmdSubCmdReadDrivers;
  CCVGCmdFilesReadResults     *m_pCmdSubCmdReadResults;
  CCVGCmdFilesReadSource      *m_pCmdSubCmdReadSource;
  CCVGCmdFilesReadExecuteable *m_pCmdSubCmdReadExecuteable;
};
// clang-format on

} // namespace cmd
} // namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDFILESREADALL_H_INCLUDE