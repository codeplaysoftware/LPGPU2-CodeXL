// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdFileRead implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFileRead.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_File.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataResults.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataSource.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataDriver.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataExecuteable.h>

namespace ccvg {
namespace cmd {

/// @brief  Retrieve a new instance of *this class.
/// @return CCVGCmdBase* Pointer to new object, NULL on allocation failure.
CCVGCmdBase* CCVGCmdFileRead::CreateSelf()
{
  return new (std::nothrow) CCVGCmdFileRead();
}

/// @brief Class constructor. Initialise *this command's mandatory attributes.
CCVGCmdFileRead::CCVGCmdFileRead()
: m_paramCCVGItemId(kParamID_CCVGItemId, true, L"string", L"CCVGItemId", L"CCVG Item ID key to look up tables")
, m_paramFileName(kParamID_FileName, true, L"string", L"filename", L"File's name")
, m_paramFileDirPath(kParamID_FileDirPath, true, L"string", L"filedirpath", L"File's directory path")
, m_paramFileMaxSizeBytes(kParamID_FileMaxSizeBytes, true, L"size_t", L"Max size bytes", L"File's maximum allowed size in bytes")
, m_paramFileDataType(kParamID_FileDataType, true, L"FileDataType", L"File data storage type", L"Where to store the file data read in")
, m_paramFileDataTempBufferUseThisFlag(kParamID_FileDataTempBufferUseThisFlag, true, L"bool", L"Use temp data buffer flag", L"True = transfer file data to temp buffer of CCVGItemData, false = store data in CCVGItemData object")
, m_pFileReadBufferArray(nullptr)
, m_nFileReadBufferSize(0)
{
  m_cmdID = CmdID::kFileRead;
  m_cmdName = L"CmdFileRead";
  m_pSelfCreatorFn = &CCVGCmdFileRead::CreateSelf;

  m_listCmdParameters.push_back(&m_paramCCVGItemId);
  m_listCmdParameters.push_back(&m_paramFileName);
  m_listCmdParameters.push_back(&m_paramFileDirPath);
  m_listCmdParameters.push_back(&m_paramFileMaxSizeBytes);
  m_listCmdParameters.push_back(&m_paramFileDataType);
  m_listCmdParameters.push_back(&m_paramFileDataTempBufferUseThisFlag);
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdFileRead::~CCVGCmdFileRead()
{
  Shutdown();
}

/// @brief  Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdFileRead::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Class shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdFileRead::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  DeleteFileDataBuffer();

  m_bBeenShutdown = true;

  return success;
}

/// @brief  Retrieve *this command's creation function.
/// @return CCVGCmdFactory::CmdCreatorFnPtr Function pointer.
CCVGCmdFactory::CmdCreatorFnPtr CCVGCmdFileRead::GetCmdCreatorFn() const
{
  return m_pSelfCreatorFn;
}

/// @brief The Execute() is the command work function to carry out work. Part 
///        of the command pattern.
///        Retrieve a file's contents. The contents are given to the CCVG 
///        Project Data object for storage. The command adheres to read
///        size limit specified as the command's input parameters. An error
///        will occur if the limit is exceeded.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdFileRead::Execute()
{
  m_bFinishedTask = false;

  status status = ReadFileData();
  if (status == success)
  {
    status = StoreFileData();
  }

  // Tidy up
  DeleteFileDataBuffer();

  m_bFinishedTask = true;
  return status;
}

/// @brief Utility function. Destroy *this object's file data buffer.
void CCVGCmdFileRead::DeleteFileDataBuffer()
{
  if (m_pFileReadBufferArray != nullptr)
  {
    delete[] m_pFileReadBufferArray;
    m_pFileReadBufferArray = nullptr;
    m_nFileReadBufferSize = 0;
  }
}

/// @brief Open the specified file and read the contents. Place those contents
///        in buffer within *this commmand object.
/// @return status Success == all ok, fialure = see *this object error text.
status CCVGCmdFileRead::ReadFileData()
{
  DeleteFileDataBuffer();

  const gtString * const pFileName = m_paramFileName.param.Get<gtString>();
  const gtString * const pFileDirPath = m_paramFileDirPath.param.Get<gtString>();
  const size_t *pFileSizeMax = m_paramFileMaxSizeBytes.param.Get<size_t>();
  const FileDataType fileDataType = *m_paramFileDataType.param.Get<FileDataType>();

  bool bTextMode = true;
  switch (fileDataType)
  {
  case kFileDataType_None:
  case kFileDataType_Results:
  case kFileDataType_Source:
  case kFileDataType_Driver:
  break;
  case kFileDataType_Executeable:
    bTextMode = false;
  break;
  default:
    // Should not get here
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_code_err_defaultReached);
    gtString errMsg;
    errMsg.appendFormattedString(pErrMsg, "CCVGCmdFileRead::ReadFileData()");
    return ErrorSet(errMsg);
  }

  size_t fileSizeByte = 0;
  CCVGUtilFile file;
  if (file.SetFileName(*pFileName) == failure)
  {
    return HandleError(file);
  }
  if (file.SetFilePath(*pFileDirPath) == failure)
  {
    return HandleError(file);
  }
  if (!file.FileExists())
  {
    return success;
  }
  if (file.GetFileSizeBytes(fileSizeByte) == failure)
  {
    return HandleError(file);
  }
  fileSizeByte++; // For NUL termination
  if (fileSizeByte >= *pFileSizeMax)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmd_fileRead_err_maxFileSizeExceeded);
    const wchar_t *pCmdName = GetCmdName().asCharArray();
    const wchar_t *pFileNamePath = file.GetFileNamePath().asCharArray();
    gtString errMsg;
    errMsg.appendFormattedString(pErrMsg, pCmdName, pFileNamePath, *pFileSizeMax);
    return ErrorSet(errMsg);
  }

  m_pFileReadBufferArray = new (std::nothrow) char[fileSizeByte];
  if (m_pFileReadBufferArray == nullptr)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmd_fileRead_err_bufferAllocFail);
    const wchar_t *pCmdName = GetCmdName().asCharArray();
    gtString errMsg;
    errMsg.appendFormattedString(pErrMsg, pCmdName, fileSizeByte);
    return ErrorSet(errMsg);
  }

  size_t dataRead = 0;
  if (file.FileRead(m_pFileReadBufferArray, fileSizeByte, dataRead, bTextMode, false) == failure)
  {
    return HandleError(file);
  }

  m_nFileReadBufferSize = fileSizeByte;

  return success;
}

/// @brief Place those contents *this commmand object's data buffer into the 
///        specified CCVG Project data CCVGItem data sub object either 
///        results, source, or driver.Depending on the FileDataType enumeration 
///        the data may not be transfer to CCVG project data but jsut held in 
///        this command object for possible extraction.
/// @return status Success == all ok, fialure = see *this object error text.
status CCVGCmdFileRead::StoreFileData()
{
  const gtString &rCCVGitemId = *m_paramCCVGItemId.param.Get<gtString>();
  const FileDataType fileDataType = *m_paramFileDataType.param.Get<FileDataType>();
  const bool bUseTempBuffer = *m_paramFileDataTempBufferUseThisFlag.param.Get<bool>();
  ccvg::fw::CCVGFWTheCurrentStateProjectData &rProjData = ccvg::fw::CCVGFWTheCurrentState::Instance().GetProjectData();
  ccvg::fw::CCVGFWTheCurrentStateProjectDataCcvg *pData = nullptr;
  status status = rProjData.ItemsDataGet(rCCVGitemId, pData);
  if (status == failure)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmd_fileRead_err_failedToStoreData);
    const gtString * const pFileName = m_paramFileName.param.Get<gtString>();
    const wchar_t *pErrMsgPrjData = rProjData.ErrorGetDescription().asCharArray();
    const wchar_t *pCmdName = GetCmdName().asCharArray();
    gtString errMsg;
    errMsg.appendFormattedString(pErrMsg, pCmdName, *pFileName->asCharArray(), pErrMsgPrjData);
    return ErrorSet(errMsg);
  }

  const wchar_t *pErrMsgStoreData = nullptr;
  switch (fileDataType)
  {
  case kFileDataType_None:
    // Do nothing. Let the client access the file data direct from *this command
    break;
  case kFileDataType_Results:
    { // Put the file data into CCVGFWTheCurrentStateProjectDataCcvg::m_ccvgItemDataResults obj
      ccvg::fw::CCVGFWCCVGItemDataResults &rDataResultsFile = pData->GetDataResults();
      status = bUseTempBuffer ? rDataResultsFile.FileContentBufferTempForComparisonSet(m_pFileReadBufferArray, m_nFileReadBufferSize)
                              : rDataResultsFile.SetFileContent(m_pFileReadBufferArray, m_nFileReadBufferSize);
      rDataResultsFile.SetInUse(true);
      // If parsing fails, the file load is still successful, but the content is invalid:
      status = rDataResultsFile.ParseResults();
      if (status == failure)
      {
        pData->GetDataResults().SetFileContentValid(false);
      }
      status = success;
    }
    break;
  case kFileDataType_Source:
    { // Put the file data into CCVGFWTheCurrentStateProjectDataCcvg::m_ccvgItemDataSource obj
      ccvg::fw::CCVGFWCCVGItemDataSource &rDataResultsSource = pData->GetDataSource();
      status = bUseTempBuffer ? rDataResultsSource.FileContentBufferTempForComparisonSet(m_pFileReadBufferArray, m_nFileReadBufferSize)
                              : rDataResultsSource.SetFileContent(m_pFileReadBufferArray, m_nFileReadBufferSize);
      rDataResultsSource.SetInUse(true);
      // Generate highlight data
      rDataResultsSource.GenerateHighlights(pData->GetDataExecuteable(), pData->GetDataResults());
    }
  break;
  case kFileDataType_Driver:
    { // Put the file data into CCVGFWTheCurrentStateProjectDataCcvg::m_ccvgItemDataDrivers obj
      ccvg::fw::CCVGFWCCVGItemDataDriver &rDataResultsDriver = pData->GetDataDrivers();
      status = bUseTempBuffer ? rDataResultsDriver.FileContentBufferTempForComparisonSet(m_pFileReadBufferArray, m_nFileReadBufferSize)
                              : rDataResultsDriver.SetFileContent(m_pFileReadBufferArray, m_nFileReadBufferSize);
      rDataResultsDriver.SetInUse(true);
    }
    break;
  case kFileDataType_Executeable:
  { // Put the file data into CCVGFWTheCurrentStateProjectDataCcvg::m_ccvgItemDataExecuteable obj
    // Executable-specific initialisation (to load AMDT ExecutableFile object for segment data reading).
    // The ExecutableFile object handles its own I/O.
    ccvg::fw::CCVGFWCCVGItemDataExecuteable &rDataResultsExecuteable = pData->GetDataExecuteable();
    // If creation of the executable object fails, the file load is still successful, but the content is invalid:
    status = rDataResultsExecuteable.CreateExecutable(*m_paramFileName.param.Get<gtString>(), *m_paramFileDirPath.param.Get<gtString>());
    if (status == failure)
    {
      pData->GetDataExecuteable().SetFileContentValid(false);
    }
    status = success;
    rDataResultsExecuteable.SetInUse(true);
  }
  break;
  default:
    // Should not get here
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_code_err_defaultReached);
    gtString errMsg;
    errMsg.appendFormattedString(pErrMsg, "CCVGCmdFileRead::StoreFileData()");
    return ErrorSet(errMsg);
  }

  if (status == failure)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmd_fileRead_err_failedToStoreData);
    const gtString * const pFileName = m_paramFileName.param.Get<gtString>();
    const wchar_t *pCmdName = GetCmdName().asCharArray();
    gtString errMsg;
    errMsg.appendFormattedString(pErrMsg, pCmdName, *pFileName->asCharArray(), pErrMsgStoreData);
    return ErrorSet(errMsg);
  }

  return status;
}

/// @brief Retrieve the pointer to the file's data buffer.
/// @return char* Pointer = have file data, nullptr = no data.
const char * const CCVGCmdFileRead::GetData() const
{
  return m_pFileReadBufferArray;
}

/// @brief Helper function to build a meaningful error message for *this 
///        command appending messages from aggregate object(s). 
/// @param[in] vrFile File object that has an error condition.
/// @return status Always failure.
status CCVGCmdFileRead::HandleError(const CCVGUtilFile &vrFile)
{
  const wchar_t *pFileErr = vrFile.ErrorGetDescription().asCharArray();
  const wchar_t *pCmdName = GetCmdName().asCharArray();
  const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmd_fileRead_err_reading);
  gtString errMsg;
  errMsg.appendFormattedString(pErrMsg, pCmdName, pFileErr);
  return ErrorSet(errMsg);
}

/// @brief  The opposite function to Execute(). Part of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdFileRead::Undo()
{
  m_bFinishedTask = false;
  
  if (!GetIsUndoableFlag())
  {
    m_bFinishedTask = true;
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_err_called_undo_when_not_undoable));
  }

  m_bFinishedTask = true;
  return success;
}

/// @brief  Ask the command if it should be undoable, it is able to reverse
///         the task carried out in its Execute() function. It may be the has
///         indicate is cannot undo a previous action because it for example
///         performing a GUI operation i.e. open a dialog.
/// @return bool True = can undo, false = cannot undo.
bool CCVGCmdFileRead::GetIsUndoableFlag() const
{
  return false;
}

} // namespace ccvg 
} // namespace cmd 