// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdCmdProjectNew implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadAll.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>

namespace ccvg {
namespace cmd {

/// @brief  Retrieve a new instance of *this class.
/// @return CCVGCmdBase* Pointer to new object, NULL on allocation failure.
CCVGCmdBase* CCVGCmdFilesReadAll::CreateSelf()
{
  return new (std::nothrow) CCVGCmdFilesReadAll();
}

/// @brief Class constructor. Initialise *this command's mandatory attributes.
CCVGCmdFilesReadAll::CCVGCmdFilesReadAll()
: m_paramFileDirResults(kParamID_FileDirResults, true, L"string", L"Results", L"The folder directory path")
, m_paramFileExtnResults(kParamID_FileExtnResults, true, L"string", L"Results file extension", L"Filter files by extension")
, m_paramFileDirSource(kParamID_FileDirSource, true, L"string", L"Source", L"The folder directory path")
, m_paramFileExtnSource(kParamID_FileExtnSource, true, L"string", L"Source file extension", L"Filter files by extension")
, m_paramFileDirDrivers(kParamID_FileDirDrivers, false, L"string", L"Driver", L"The folder directory path")
, m_paramFileExtnDriver(kParamID_FileExtnDriver, false, L"string", L"Driver file extension", L"Filter files by extension")
, m_paramFlagReadDriverFiles(kParamID_FlagReadDriverFiles, true, L"bool", L"Drivers flag", L"Read folder yes no")
, m_paramFileDirExecuteable(kParamID_FileDirExecuteable, false, L"string", L"Executeable", L"The folder directory path")
, m_paramFileExtnExecuteable(kParamID_FileExtnExecuteable, false, L"string", L"Executeable file extension", L"Filter files by extension")
, m_pCmdSubCmdReadExecuteable(nullptr)
, m_pCmdSubCmdReadDrivers(nullptr)
, m_pCmdSubCmdReadResults(nullptr)
, m_pCmdSubCmdReadSource(nullptr)
{
  m_cmdID = CmdID::kFilesReadAll;
  m_cmdName = L"CmdFilesReadAll";
  m_pSelfCreatorFn = &CCVGCmdFilesReadAll::CreateSelf;

  m_listCmdParameters.push_back(&m_paramFileDirResults);
  m_listCmdParameters.push_back(&m_paramFileExtnResults);
  m_listCmdParameters.push_back(&m_paramFileDirSource);
  m_listCmdParameters.push_back(&m_paramFileExtnSource);
  m_listCmdParameters.push_back(&m_paramFileDirDrivers);
  m_listCmdParameters.push_back(&m_paramFileExtnDriver);
  m_listCmdParameters.push_back(&m_paramFlagReadDriverFiles);
  m_listCmdParameters.push_back(&m_paramFileDirExecuteable);
  m_listCmdParameters.push_back(&m_paramFileExtnExecuteable);
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdFilesReadAll::~CCVGCmdFilesReadAll()
{
  Shutdown();
}

/// @brief  Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdFilesReadAll::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  DeleteSubCmds();

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Class shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdFilesReadAll::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  DeleteSubCmds();

  m_bBeenShutdown = true;

  return success;
}

/// @brief *this command has sub-commands which need to be deleted.
///        Sub-commands are not deleted by the command Invoker.
void CCVGCmdFilesReadAll::DeleteSubCmds()
{
  if (m_pCmdSubCmdReadExecuteable != nullptr)
  {
    delete m_pCmdSubCmdReadExecuteable;
    m_pCmdSubCmdReadExecuteable = nullptr;
  }
  if (m_pCmdSubCmdReadDrivers != nullptr)
  {
    delete m_pCmdSubCmdReadDrivers;
    m_pCmdSubCmdReadDrivers = nullptr;
  }
  if (m_pCmdSubCmdReadResults != nullptr)
  {
    delete m_pCmdSubCmdReadResults;
    m_pCmdSubCmdReadResults = nullptr;
  }
  if (m_pCmdSubCmdReadSource != nullptr)
  {
    delete m_pCmdSubCmdReadSource;
    m_pCmdSubCmdReadSource = nullptr;
  }
}

/// @brief  Retrieve *this command's creation function.
/// @return CCVGCmdFactory::CmdCreatorFnPtr Function pointer.
CCVGCmdFactory::CmdCreatorFnPtr CCVGCmdFilesReadAll::GetCmdCreatorFn() const
{
  return m_pSelfCreatorFn;
}

/// @brief  The Execute() is the command work function to carry out work. Part 
///         of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdFilesReadAll::Execute()
{
  m_bFinishedTask = false;
  DeleteSubCmds();

  CCVGCmdInvoker &rInvoke(CCVGCmdInvoker::Instance());
  CCVGCmdParams *pCmdParams = nullptr;
  fnstatus::status status = rInvoke.CmdGetParams(CmdID::kFilesReadResults, pCmdParams);
  if (status == success)
  {
    bool bOk = rInvoke.CmdGetSubCmd<CCVGCmdFilesReadResults>(m_pCmdSubCmdReadResults);
    CCVGCmdParameter *pParamFileDirResults = pCmdParams->GetParam(CCVGCmdFilesReadResults::kParamID_FileDirResults);
    CCVGCmdParameter *pParamFileExtnResults = pCmdParams->GetParam(CCVGCmdFilesReadResults::kParamID_FileExtnResults);
    bOk = bOk && (pParamFileDirResults != nullptr);
    bOk = bOk && (pParamFileExtnResults != nullptr);
    bOk = bOk && pParamFileDirResults->param.Set<gtString>(*m_paramFileDirResults.param.Get<gtString>());
    bOk = bOk && pParamFileExtnResults->param.Set<gtString>(*m_paramFileExtnResults.param.Get<gtString>());
    bOk = bOk && (rInvoke.CmdExecute() == success);
    if (!bOk)
    {
      return HandleSubCmdError(m_pCmdSubCmdReadResults);
    }
  }

  status = rInvoke.CmdGetParams(CmdID::kFilesReadSource, pCmdParams);
  if (status == success)
  {
    bool bOk = rInvoke.CmdGetSubCmd<CCVGCmdFilesReadSource>(m_pCmdSubCmdReadSource);
    CCVGCmdParameter *pParamFileDirSource = pCmdParams->GetParam(CCVGCmdFilesReadSource::kParamID_FileDirSource);
    CCVGCmdParameter *pParamFileExtnSource = pCmdParams->GetParam(CCVGCmdFilesReadSource::kParamID_FileExtnSource);
    bOk = bOk && (pParamFileDirSource != nullptr);
    bOk = bOk && (pParamFileExtnSource != nullptr);
    bOk = bOk && pParamFileDirSource->param.Set<gtString>(*m_paramFileDirSource.param.Get<gtString>());
    bOk = bOk && pParamFileExtnSource->param.Set<gtString>(*m_paramFileExtnSource.param.Get<gtString>());
    bOk = bOk && (rInvoke.CmdExecute() == success);
    if (!bOk)
    {
      return HandleSubCmdError(m_pCmdSubCmdReadSource);
    }
  }

  const bool bDoDrivers = *m_paramFlagReadDriverFiles.param.Get<bool>();
  status = rInvoke.CmdGetParams(CmdID::kFilesReadDrivers, pCmdParams);
  if (status == success)
  {
    bool bOk = rInvoke.CmdGetSubCmd<CCVGCmdFilesReadDrivers>(m_pCmdSubCmdReadDrivers);
    CCVGCmdParameter *pParamFileDirDrivers = pCmdParams->GetParam(CCVGCmdFilesReadDrivers::kParamID_FileDirDrivers);
    CCVGCmdParameter *pParamFileExtnDrivers = pCmdParams->GetParam(CCVGCmdFilesReadDrivers::kParamID_FileExtnDriver);
    CCVGCmdParameter *pParamFlagReadDriverFiles = pCmdParams->GetParam(CCVGCmdFilesReadDrivers::kParamID_FlagReadDriverFiles);
    bOk = bOk && (pParamFileDirDrivers != nullptr);
    bOk = bOk && (pParamFileExtnDrivers != nullptr);
    bOk = bOk && (pParamFlagReadDriverFiles != nullptr);
    bOk = bOk && pParamFileDirDrivers->param.Set<gtString>(*m_paramFileDirDrivers.param.Get<gtString>());
    bOk = bOk && pParamFileExtnDrivers->param.Set<gtString>(*m_paramFileExtnDriver.param.Get<gtString>());
    bOk = bOk && pParamFlagReadDriverFiles->param.Set<bool>(bDoDrivers);
    bOk = bOk && (rInvoke.CmdExecute() == success);
    if (!bOk)
    {
      return HandleSubCmdError(m_pCmdSubCmdReadDrivers);
    }
  }

  status = rInvoke.CmdGetParams(CmdID::kFilesReadExecuteable, pCmdParams);
  if (status == success)
  {
    //TODO defer file loading here??
    bool bOk = rInvoke.CmdGetSubCmd<CCVGCmdFilesReadExecuteable>(m_pCmdSubCmdReadExecuteable);
    CCVGCmdParameter *pParamFileDirExecuteable = pCmdParams->GetParam(CCVGCmdFilesReadExecuteable::kParamID_FileDirExecuteable);
    CCVGCmdParameter *pParamFileExtnExecuteable = pCmdParams->GetParam(CCVGCmdFilesReadExecuteable::kParamID_FileExtnExecuteable);
    bOk = bOk && (pParamFileDirExecuteable != nullptr);
    bOk = bOk && (pParamFileExtnExecuteable != nullptr);
    bOk = bOk && pParamFileDirExecuteable->param.Set<gtString>(*m_paramFileDirExecuteable.param.Get<gtString>());
    bOk = bOk && pParamFileExtnExecuteable->param.Set<gtString>(*m_paramFileExtnExecuteable.param.Get<gtString>());
    bOk = bOk && (rInvoke.CmdExecute() == success);
    if (!bOk)
    {
      return HandleSubCmdError(m_pCmdSubCmdReadExecuteable);
    }
  }

  fw::CCVGFWTheCurrentStateProjectData &rProjData = fw::CCVGFWTheCurrentState::Instance().GetProjectData();
  fw::CCVGFWTheCurrentStateProjectData::VecItems_t &items = rProjData.ItemsGetThenCommit();
  const CCVGCmdFilesReadResults::MapFileNames_t &rMapResultFileNames(m_pCmdSubCmdReadResults->GetFilesNames());
  const CCVGCmdFilesReadSource::MapFileNames_t &rMapSourceFileNames(m_pCmdSubCmdReadSource->GetFilesNames());
  const CCVGCmdFilesReadDrivers::MapFileNames_t &rMapDriverFileNames(m_pCmdSubCmdReadDrivers->GetFilesNames());
  const CCVGCmdFilesReadExecuteable::MapFileNames_t &rMapExecuteableFileNames(m_pCmdSubCmdReadExecuteable->GetFilesNames());
  const size_t nResults = rMapResultFileNames.size();
  const size_t nSource = rMapSourceFileNames.size();
  const size_t nDrivers = bDoDrivers ? rMapDriverFileNames.size() : 0;
  const size_t nExecuteable = rMapExecuteableFileNames.size();
  size_t nFile = (nResults > nSource) ? nResults : nSource;
  nFile = (nFile > nDrivers) ? nFile : nDrivers;
  nFile = (nFile > nExecuteable) ? nFile : nExecuteable;
  using MapFileNames_t = std::map<gtString, size_t>;
  const MapFileNames_t *pMapBiggestList((nResults > nSource) ? &rMapResultFileNames : &rMapSourceFileNames);
  pMapBiggestList = ((nFile > nDrivers) ? pMapBiggestList : &rMapDriverFileNames);
  pMapBiggestList = ((nFile > nExecuteable) ? pMapBiggestList : &rMapExecuteableFileNames);
  for (const auto &it : *pMapBiggestList)
  {
    const gtString &rFileName(it.first);
    const auto itFileResults = rMapResultFileNames.find(rFileName);
    const auto itFileSource = rMapSourceFileNames.find(rFileName);
    const auto itFileDrivers = rMapDriverFileNames.find(rFileName);
    const auto itFileExecuteable = rMapExecuteableFileNames.find(rFileName);
    const bool bHaveResult = (itFileResults != rMapResultFileNames.end());
    const bool bHaveSource = (itFileSource != rMapSourceFileNames.end());
    const bool bDriver = (itFileDrivers != rMapDriverFileNames.end());
    const bool bHaveDriver = bDoDrivers ? bDriver : true;
    const bool bHaveExecuteable = (itFileExecuteable != rMapExecuteableFileNames.end());
    fw::CCVGFWTheCurrentStateProjectData::CCVGItem item;
    item.SetIsGood(bHaveResult && bHaveSource && bHaveDriver && bHaveExecuteable);
    item.SetFileName(rFileName);
    item.SetPresentCcvgr(bHaveResult);
    item.SetPresentKc(bHaveSource);
    item.SetPresentCcvgd(bDriver);
    item.SetPresentCcvge(bHaveExecuteable);
    items.push_back(item);
  }

  status = rProjData.ItemsCommit();
  if (status == failure)
  {
    const wchar_t *pTaskErrMsg = rProjData.ErrorGetDescription().asCharArray();
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmd_filesReadAll_err_aTaskFail);
    gtString errMsg;
    errMsg.appendFormattedString(pErrMsg, pTaskErrMsg);
    ErrorSet(errMsg);
    OnErrorReleaseResourcesAllocByThisCmd();
  }

  m_bFinishedTask = true;

  return success;
}

/// @brief On *this command failing to complete its task any and all resources,
///        bindings etc need to released or reset to a state as if the command
///        had never executed.
void CCVGCmdFilesReadAll::OnErrorReleaseResourcesAllocByThisCmd()
{
  fw::CCVGFWTheCurrentStateProjectData &rProjData = fw::CCVGFWTheCurrentState::Instance().GetProjectData();
  rProjData.ItemsClear();
}

/// @brief Helper function to build a meaningful error message for *this 
///        command appending messages from aggregate object(s). 
/// @param[in] vpSubCmd The sub-command that has an error condition.
/// @return status Always failure.
status CCVGCmdFilesReadAll::HandleSubCmdError(CCVGCmdBase *vpSubCmd)
{
  const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmd_filesReadAll_err_subCmdFail);
  const wchar_t *pCmdName = GetCmdName().asCharArray();
  const wchar_t *pCmdSubCmdName = (vpSubCmd != nullptr) ? vpSubCmd->GetCmdName().asCharArray() : L"<sub-command>";
  const wchar_t *pSubCmdMsg = (vpSubCmd != nullptr) ? vpSubCmd->ErrorGetDescription().asCharArray() : L"<sub-command msg not available>";
  gtString errMsg;
  errMsg.appendFormattedString(pErrMsg, pCmdName, pCmdSubCmdName, pSubCmdMsg);
  return ErrorSet(errMsg);
}

/// @brief  The opposite function to Execute(). Part of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdFilesReadAll::Undo()
{
  // Overidden from CCVGCmdBase

  m_bFinishedTask = false;

  // Todo:
  // Clean out the data from CCVG_fw_TheCurrentStateProjectData
  
  if (!GetIsUndoableFlag())
  {
    //m_bFinishedTask = true;
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_err_called_undo_when_not_undoable));
  }

  DeleteSubCmds();
  m_bFinishedTask = true;
  return success;
}

/// @brief  Ask the command if it should be undoable, it is able to reverse
///         the task carried out in its Execute() function. It may be the has
///         indicate is cannot undo a previous action because it for example
///         performing a GUI operation i.e. open a dialog.
/// @return bool True = can undo, false = cannot undo.
bool CCVGCmdFilesReadAll::GetIsUndoableFlag() const
{
  return true;
}

} // namespace ccvg 
} // namespace cmd 