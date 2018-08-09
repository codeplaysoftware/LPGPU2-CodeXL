// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// Command global functions set to be called already:
///   CCVGCmdFilesReadAll
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_CommandIDs.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Params.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadAll.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdWndSrcOpen.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFileRead.h>

namespace ccvg {
namespace cmd {

constexpr size_t constExprFileContentBufferSizeBytesMax = 10 * 1024 * 1024;

/// @brief Setup and execute command to scan the code coverage source 
///        directories and read in files.
/// @return Status Success = task succeeded, failure = error occurred.
status ExecuteCmdFilesReadAll()
{
  CCVGCmdInvoker &rCmdInvoker(CCVGCmdInvoker::Instance());
  CCVGCmdParams *pCmdParams = nullptr;
  fnstatus::status status = rCmdInvoker.CmdGetParams(CmdID::kFilesReadAll, pCmdParams);
  const gtString cmdName(rCmdInvoker.CmdGetNameAboutToExecute());
  bool bCaughtException = false;
  if (status == success)
  {
    try
    {
      CCVGCmdParameter *pParamFileDirResults = pCmdParams->GetParam(CCVGCmdFilesReadAll::kParamID_FileDirResults);
      CCVGCmdParameter *pParamFileExtnResults = pCmdParams->GetParam(CCVGCmdFilesReadAll::kParamID_FileExtnResults);
      CCVGCmdParameter *pParamFileDirSource = pCmdParams->GetParam(CCVGCmdFilesReadAll::kParamID_FileDirSource);
      CCVGCmdParameter *pParamFileExtnSource = pCmdParams->GetParam(CCVGCmdFilesReadAll::kParamID_FileExtnSource);
      CCVGCmdParameter *pParamFileDirDrivers = pCmdParams->GetParam(CCVGCmdFilesReadAll::kParamID_FileDirDrivers);
      CCVGCmdParameter *pParamFileExtnDriver = pCmdParams->GetParam(CCVGCmdFilesReadAll::kParamID_FileExtnDriver);
      CCVGCmdParameter *pParamFileReadDriverFiles = pCmdParams->GetParam(CCVGCmdFilesReadAll::kParamID_FlagReadDriverFiles);
      CCVGCmdParameter *pParamFileDirExecuteable = pCmdParams->GetParam(CCVGCmdFilesReadAll::kParamID_FileDirExecuteable);
      CCVGCmdParameter *pParamFileExtnExecuteable = pCmdParams->GetParam(CCVGCmdFilesReadAll::kParamID_FileExtnExecuteable);
      const fw::CCVGFWTheCurrentStateProjectSettings &rProjData = fw::CCVGFWTheCurrentState::Instance().GetProjectSetting();
      const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSettings(rProjData.GetSettingsWorking());
      bool bOk = pParamFileDirResults->param.Set<gtString>(rSettings.GetCcvgFolderDirResult());
      bOk = bOk && pParamFileExtnResults->param.Set<gtString>(rSettings.GetCcvgFilesExtnResult());
      bOk = bOk && pParamFileDirSource->param.Set<gtString>(rSettings.GetCcvgFolderDirSource());
      bOk = bOk && pParamFileExtnSource->param.Set<gtString>(rSettings.GetCcvgFilesExtnSource());
      bOk = bOk && pParamFileDirDrivers->param.Set<gtString>(rSettings.GetCcvgFolderDirDriver());
      bOk = bOk && pParamFileExtnDriver->param.Set<gtString>(rSettings.GetCcvgFilesExtnDriver());
      bOk = bOk && pParamFileReadDriverFiles->param.Set<bool>(rSettings.GetCcvgDriverFilesMandatoryFlag());
      bOk = bOk && pParamFileDirExecuteable->param.Set<gtString>(rSettings.GetCcvgFolderDirExecuteable());
      bOk = bOk && pParamFileExtnExecuteable->param.Set<gtString>(rSettings.GetCcvgFilesExtnExecuteable());
      bOk = bOk && (rCmdInvoker.CmdExecute() == success);
      status = bOk;
    }
    catch (...)
    {
      bCaughtException = true;
      status = failure;
    }
  }

  return rCmdInvoker.CmdHandleError(status, bCaughtException, cmdName);
}

/// @brief Setup and execute command to open CodeXL CCVG MDI view to display
///        the contents of any type of file. Called internally by other
///        CmdWndSrcOpen* functions with appropriate file extension.
/// @param[in] vrFileName Name of the file excluding the extension
/// @param[in] vrTypeExtn  Name of the extension used by the CCVG item's type.
/// @param[in] vrFileDir  Name of the directory for the CCVG item's type
/// @param[in] vrTypeExtn  Name of the file extension used by the CCVG item's 
////           type.
/// @return Status Success = task succeeded, failure = error occurred.
status ExecuteCmdWndSrcOpenCcvgFile(const gtString &vrFileName, const gtString &vrTypeExtn, const gtString &vrFileDir, const gtString &vrFileExtn)
{
  if (vrFileName.isEmpty())
  {
    return failure;
  }

  CCVGCmdInvoker &rCmdInvoker(CCVGCmdInvoker::Instance());
  CCVGCmdParams *pCmdParams = nullptr;
  fnstatus::status status = rCmdInvoker.CmdGetParams(CmdID::kWndSrcOpen, pCmdParams);
  const gtString cmdName(rCmdInvoker.CmdGetNameAboutToExecute());
  bool bCaughtException = false;
  if (status == success)
  {
    try
    {
      gtString fileName;
      fileName.appendFormattedString(L"%ls%ls", vrFileName.asCharArray(), vrFileExtn.asCharArray());

      CCVGCmdParameter *pParamCCVGItemId = pCmdParams->GetParam(CCVGCmdWndSrcOpen::kParamID_CCVGItemId);
      CCVGCmdParameter *pParamCCVGItemExt = pCmdParams->GetParam(CCVGCmdWndSrcOpen::kParamID_CCVGItemExt);
      CCVGCmdParameter *pParamFileName = pCmdParams->GetParam(CCVGCmdWndSrcOpen::kParamID_FileName);
      CCVGCmdParameter *pParamFileDirPath = pCmdParams->GetParam(CCVGCmdWndSrcOpen::kParamID_FileDirPath);
      bool bOk = (pParamCCVGItemId != nullptr);
      bOk = bOk && (pParamFileName != nullptr);
      bOk = bOk && (pParamFileDirPath != nullptr);
      bOk = bOk && pParamCCVGItemId->param.Set<gtString>(vrFileName);
      bOk = bOk && pParamCCVGItemExt->param.Set<gtString>(vrTypeExtn);
      bOk = bOk && pParamFileName->param.Set<gtString>(fileName);
      bOk = bOk && pParamFileDirPath->param.Set<gtString>(vrFileDir);
      bOk = bOk && (rCmdInvoker.CmdExecute() == success);
      status = bOk;
    }
    catch (...)
    {
      bCaughtException = true;
      status = failure;
    }
  }

  return rCmdInvoker.CmdHandleError(status, bCaughtException, cmdName);
}

/// @brief Setup and execute command to open CodeXL CCVG MDI view to display
///        the contents of a Driver source file.
/// @param[in] vrDriverFileName Name of the Driver file not including the 
///        file extension.
/// @return Status Success = task succeeded, failure = error occurred.
status ExecuteCmdWndSrcOpenDriverFile(const gtString &vrDriverFileName)
{
  const fw::CCVGFWTheCurrentStateProjectSettings &rProjData = fw::CCVGFWTheCurrentState::Instance().GetProjectSetting();
  const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSettings(rProjData.GetSettingsWorking());

  return ExecuteCmdWndSrcOpenCcvgFile(vrDriverFileName, L"Driver", rSettings.GetCcvgFolderDirDriver(), rSettings.GetCcvgFilesExtnDriver());
}

/// @brief Setup and execute command to open CodeXL CCVG MDI view to display
///        the contents of a kernel source file.
/// @param[in] vrSourceFileName Name of the source file not including the 
///        file extension.
/// @return Status Success = task succeeded, failure = error occurred.
status ExecuteCmdWndSrcOpenSourceFile(const gtString &vrSourceFileName)
{
  const fw::CCVGFWTheCurrentStateProjectSettings &rProjData = fw::CCVGFWTheCurrentState::Instance().GetProjectSetting();
  const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSettings(rProjData.GetSettingsWorking());

  return ExecuteCmdWndSrcOpenCcvgFile(vrSourceFileName, L"Source", rSettings.GetCcvgFolderDirSource(), rSettings.GetCcvgFilesExtnSource());
}

/// @brief Setup and execute command to open CodeXL CCVG MDI view to display
///        the contents of a kernel source file.
/// @param[in] vrSourceFileName Name of the source file not including the 
///        file extension.
/// @return Status Success = task succeeded, failure = error occurred.
status ExecuteCmdWndSrcOpenResultFile(const gtString &vrResultFileName)
{
  const fw::CCVGFWTheCurrentStateProjectSettings &rProjData = fw::CCVGFWTheCurrentState::Instance().GetProjectSetting();
  const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSettings(rProjData.GetSettingsWorking());

  return ExecuteCmdWndSrcOpenCcvgFile(vrResultFileName, L"Results", rSettings.GetCcvgFolderDirResult(), rSettings.GetCcvgFilesExtnResult());
}

/// @brief Setup and execute command to open CodeXL CCVG MDI view to display
///        the relevant section of the executable file (the segment containing
///        the code coverage mapping).
/// @param[in] vrSourceFileName Name of the source file not including the 
///        file extension.
/// @return Status Success = task succeeded, failure = error occurred.
status ExecuteCmdWndSrcOpenExecFile(const gtString &vrResultFileName)
{
  const fw::CCVGFWTheCurrentStateProjectSettings &rProjData = fw::CCVGFWTheCurrentState::Instance().GetProjectSetting();
  const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSettings(rProjData.GetSettingsWorking());

  return ExecuteCmdWndSrcOpenCcvgFile(vrResultFileName, L"Executeable", rSettings.GetCcvgFolderDirExecuteable(), rSettings.GetCcvgFilesExtnExecuteable());
}

/// @brief Setup and execute command to read in the contents of a file.
/// @param[in] vrFileName Name of the file not including the file extension.
///            An empty filename value will produce an error.
/// @param[in] veFileType Valid values are:
///               kFileDataType_Results
///               kFileDataType_Source
///               kFileDataType_Driver
///            Any other type will produce a function failure.
/// param[in] vbStoreDataInTempBuffer True = Copy file data to CCVGItemData's 
///           internal temporary comparison buffer, false = copy data to 
///           CCVGItemData's file data buffer.
/// @return Status Success = task succeeded, failure = error occurred.
status ExecuteCmdReadFile(const gtString &vrFileName, cmd::CCVGCmdFileRead::FileDataType veFileType, bool vbStoreDataInTempBuffer)
{
  if (vrFileName.isEmpty())
  {
    return failure;
  }

  // Create command object and get its parameters
  CCVGCmdInvoker &rCmdInvoker(CCVGCmdInvoker::Instance());
  CCVGCmdParams *pCmdParams = nullptr;
  fnstatus::status status = rCmdInvoker.CmdGetParams(CmdID::kFileRead, pCmdParams);
  const gtString cmdName(rCmdInvoker.CmdGetNameAboutToExecute());
  bool bCaughtException = false;
  if (status == success)
  {
    try
    {
      // Access internal data and state
      const fw::CCVGFWTheCurrentStateProjectSettings &rProjData = fw::CCVGFWTheCurrentState::Instance().GetProjectSetting();
      const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rSettings(rProjData.GetSettingsWorking());

      // Form appropriate file name for file to read
      const gtString *pFileExtn = nullptr;
      const gtString *pFileDir = nullptr;
      switch (veFileType)
      {
      case cmd::CCVGCmdFileRead::FileDataType::kFileDataType_None:
        // This is not allowed as a value to pass in to this function.
        return failure;
      case cmd::CCVGCmdFileRead::FileDataType::kFileDataType_Results:
        pFileExtn = &rSettings.GetCcvgFilesExtnResult();
        pFileDir = &rSettings.GetCcvgFolderDirResult();
        break;
      case cmd::CCVGCmdFileRead::FileDataType::kFileDataType_Source:
        pFileExtn = &rSettings.GetCcvgFilesExtnSource();
        pFileDir = &rSettings.GetCcvgFolderDirSource();
        break;
      case cmd::CCVGCmdFileRead::FileDataType::kFileDataType_Driver:
        pFileExtn = &rSettings.GetCcvgFilesExtnDriver();
        pFileDir = &rSettings.GetCcvgFolderDirDriver();
        break;
      case cmd::CCVGCmdFileRead::FileDataType::kFileDataType_Executeable:
        pFileExtn = &rSettings.GetCcvgFilesExtnExecuteable();
        pFileDir = &rSettings.GetCcvgFolderDirExecuteable();
        break;
      default:
        // Should not get here
        return failure;
      }
      gtString fileName;
      fileName.appendFormattedString(L"%ls%ls", vrFileName.asCharArray(), (*pFileExtn).asCharArray());

      // Prep and execute the command
      CCVGCmdParameter *pParamCCVGItemId = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_CCVGItemId);
      CCVGCmdParameter *pParamFileName = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_FileName);
      CCVGCmdParameter *pParamFileDirPath = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_FileDirPath);
      CCVGCmdParameter *pParamFileMaxSize = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_FileMaxSizeBytes);
      CCVGCmdParameter *pParamFileDataType = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_FileDataType);
      CCVGCmdParameter *pParamFileDataTempBufferUseThisFlag = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_FileDataTempBufferUseThisFlag);
      bool bOk = (pParamCCVGItemId != nullptr);
      bOk = bOk && (pParamFileName != nullptr);
      bOk = bOk && (pParamFileDirPath != nullptr);
      bOk = bOk && (pParamFileMaxSize != nullptr);
      bOk = bOk && (pParamFileDataType != nullptr);
      bOk = bOk && (pParamFileDataTempBufferUseThisFlag != nullptr);
      bOk = bOk && pParamCCVGItemId->param.Set<gtString>(vrFileName);
      bOk = bOk && pParamFileName->param.Set<gtString>(fileName);
      bOk = bOk && pParamFileDirPath->param.Set<gtString>(*pFileDir);
      bOk = bOk && pParamFileMaxSize->param.Set<size_t>(constExprFileContentBufferSizeBytesMax);
      bOk = bOk && pParamFileDataType->param.Set<cmd::CCVGCmdFileRead::FileDataType>(veFileType);
      bOk = bOk && pParamFileDataTempBufferUseThisFlag->param.Set<bool>(vbStoreDataInTempBuffer);
      bOk = bOk && (rCmdInvoker.CmdExecute() == success);
      status = bOk;
    }
    catch (...)
    {
      bCaughtException = true;
      status = failure;
    }
  }

  return rCmdInvoker.CmdHandleError(status, bCaughtException, cmdName);
}

} // namespace ccvg 
} // namespace cmd 
