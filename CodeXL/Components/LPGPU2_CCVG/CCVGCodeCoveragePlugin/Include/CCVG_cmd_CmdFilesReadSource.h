// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdFilesReadSource interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDFILESREADSOURCE_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDFILESREADSOURCE_H_INCLUDE

// std:
#include <vector>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBaseWithParams.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Parameter.h>

namespace ccvg {
namespace cmd {

/// @brief    CCVG command read in files from the code coverage source files
///           directory. Store the data in the 
///           CCVG_fw_TheCurrentStateProjectData object.
/// @warning  None.
/// @see      cmd::Factory, cmd::Invoker, cmd::Params, cmd::CmdID.
/// @date     30/12/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdFilesReadSource final
: public CCVGCmdBaseWithParams
{
// Enums:
public:
  enum ParamID
  {
    kParamID_FileDirSource,
    kParamID_FileExtnSource
  };

// Typedefs:
public:
  using MapFileNames_t = std::map<gtString, size_t>;
  using MapPairFileNames_t = std::pair<gtString, size_t>;

// Statics:
public:
  // Required by the CCVGCmdFactory when registering *this command
  static CCVGCmdBase* CreateSelf();

// Methods:
public:
  CCVGCmdFilesReadSource();
  const MapFileNames_t& GetFilesNames() const;

// Overridden:
public:
  virtual ~CCVGCmdFilesReadSource() override;
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
  void ClearData();
   
// Attributes:
private:
  CCVGCmdParameter m_paramFileDirSource;
  CCVGCmdParameter m_paramFileExtnSource;
  MapFileNames_t   m_mapFileNames;
};
// clang-format on

} // namespace cmd
} // namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDFILESREADSOURCE_H_INCLUDE