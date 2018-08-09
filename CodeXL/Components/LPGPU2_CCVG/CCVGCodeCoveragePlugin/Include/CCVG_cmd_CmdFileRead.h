// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdFileRead interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDFILEREAD_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDFILEREAD_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBaseWithParams.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Parameter.h>

// Declarations:
class CCVGUtilFile;

namespace ccvg {
namespace cmd {

/// @brief    CCVG command open a file and read its contents and store in the
///           the approprite ProjectData class'es container.
/// @warning  None.
/// @see      cmd::Factory, cmd::Invoker, cmd::Params, cmd::CmdID.
/// @date     14/02/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdFileRead final
: public CCVGCmdBaseWithParams
{
// Enums:
public:
  enum ParamID
  {
    kParamID_CCVGItemId, 
    kParamID_FileName,
    kParamID_FileDirPath,
    kParamID_FileMaxSizeBytes,
    kParamID_FileDataType,
    kParamID_FileDataTempBufferUseThisFlag,
  };

  enum FileDataType
  {
    kFileDataType_None,    // The file data is not put in project data
    kFileDataType_Results,
    kFileDataType_Source,
    kFileDataType_Driver,
    kFileDataType_Executeable
  };

// Statics:
public:
  // Required by the CCVGCmdFactory when registering *this command
  static CCVGCmdBase* CreateSelf();

// Methods:
public:
  CCVGCmdFileRead();
  const char * const  GetData() const;

// Overridden:
public:
  virtual ~CCVGCmdFileRead() override;
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
  status HandleError(const CCVGUtilFile &vrFile);
  status ReadFileData();
  status StoreFileData();
  void   DeleteFileDataBuffer();

// Attributes:
private:
  CCVGCmdParameter m_paramCCVGItemId;
  CCVGCmdParameter m_paramFileName;
  CCVGCmdParameter m_paramFileDirPath;
  CCVGCmdParameter m_paramFileMaxSizeBytes;
  CCVGCmdParameter m_paramFileDataType;
  CCVGCmdParameter m_paramFileDataTempBufferUseThisFlag;
  char            *m_pFileReadBufferArray;
  size_t           m_nFileReadBufferSize;
};
// clang-format on

} // namespace cmd
} // namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDFILEREAD_H_INCLUDE