// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdWndSrcOpen implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdWndSrcOpen.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataEnumTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewCreator.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewId.h>


namespace ccvg {
namespace cmd {

/// @brief  Retrieve a new instance of *this class.
/// @return CCVGCmdBase* Pointer to new object, NULL on allocation failure.
CCVGCmdBase* CCVGCmdWndSrcOpen::CreateSelf()
{
  return new (std::nothrow) CCVGCmdWndSrcOpen();
}

/// @brief Class constructor. Initialise *this command's mandatory attributes.
CCVGCmdWndSrcOpen::CCVGCmdWndSrcOpen()
: m_paramCCVGItemId(kParamID_CCVGItemId, true, L"string", L"CCVGItemId", L"CCVG Item ID key to look up tables")
, m_paramCCVGItemExt(kParamID_CCVGItemExt, true, L"string", L"CCVGItemId", L"CCVG Item Type extension")
, m_paramFileName(kParamID_FileName, true, L"string", L"filename", L"File's name")
, m_paramFileDirPath(kParamID_FileDirPath, true, L"string", L"filepath", L"File's directory path")
, m_paramViewAttributeShowLineNumbersFlag(kParamID_ViewAttributeShowLineNumbersFlag, true, L"flag", L"linenumbers", L"Show hide text line numbers")
, m_paramViewAttributeIsUserEditable(kParamID_ViewAttributeIsUserEditable, true, L"flag", L"iseditable", L"User able to edit text in the view")
, m_pCmdSubCmdFileRead(nullptr)
, m_constReadFileBufferMaxSizeBytes(1024 * 1024 * 10)
{
  m_cmdID = CmdID::kWndSrcOpen;
  m_cmdName = L"CmdWndSrcOpen";
  m_pSelfCreatorFn = &CCVGCmdWndSrcOpen::CreateSelf;

  m_listCmdParameters.push_back(&m_paramCCVGItemId);
  m_listCmdParameters.push_back(&m_paramCCVGItemExt);
  m_listCmdParameters.push_back(&m_paramFileName);
  m_listCmdParameters.push_back(&m_paramFileDirPath);
  m_listCmdParameters.push_back(&m_paramViewAttributeShowLineNumbersFlag);
  m_listCmdParameters.push_back(&m_paramViewAttributeIsUserEditable);
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdWndSrcOpen::~CCVGCmdWndSrcOpen()
{
  Shutdown();
}

/// @brief  Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdWndSrcOpen::Initialise()
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
status CCVGCmdWndSrcOpen::Shutdown()
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
void CCVGCmdWndSrcOpen::DeleteSubCmds()
{
  if (m_pCmdSubCmdFileRead != nullptr)
  {
    delete m_pCmdSubCmdFileRead;
    m_pCmdSubCmdFileRead = nullptr;
  }
}

/// @brief  Retrieve *this command's creation function.
/// @return CCVGCmdFactory::CmdCreatorFnPtr Function pointer.
CCVGCmdFactory::CmdCreatorFnPtr CCVGCmdWndSrcOpen::GetCmdCreatorFn() const
{
  return m_pSelfCreatorFn;
}

/// @brief  The Execute() is the command work function to carry out work. Part 
///         of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdWndSrcOpen::Execute()
{
  m_bFinishedTask = false;
  
  DeleteSubCmds();

  status status = FileContentsRead();
  if (status == success)
  {
    status = FileContentsShowMDIView();
  }
  if (status == failure)
  {
    OnErrorReleaseResourcesAllocByThisCmd();
  }

  m_bFinishedTask = true;
  return status;
}

/// @brief Open the Code Coverage Driver file and read its contents putting 
///        the contents into CCVGItemData sub-item CCVGItemDataDriver object.
/// @return status Success = all ok, failure = error read error description.
status CCVGCmdWndSrcOpen::FileContentsRead()
{
  CCVGCmdInvoker &rInvoke(CCVGCmdInvoker::Instance());
  CCVGCmdParams *pCmdParams = nullptr;
  fnstatus::status status = rInvoke.CmdGetParams(CmdID::kFileRead, pCmdParams);
  if (status == success)
  {
    bool bOk = rInvoke.CmdGetSubCmd<CCVGCmdFileRead>(m_pCmdSubCmdFileRead);
    CCVGCmdParameter *pParamCCVGItemId = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_CCVGItemId);
    CCVGCmdParameter *pParamFileName = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_FileName);
    CCVGCmdParameter *pParamFileDirPath = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_FileDirPath);
    CCVGCmdParameter *pParamFileMaxSize = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_FileMaxSizeBytes);
    CCVGCmdParameter *pParamFileDataType = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_FileDataType);
    CCVGCmdParameter *pParamFileDataTempBufferUseThisFlag = pCmdParams->GetParam(CCVGCmdFileRead::kParamID_FileDataTempBufferUseThisFlag);
    fw::CCVGMDIViewId ccvgItemId(*m_paramCCVGItemId.param.Get<gtString>(), *m_paramCCVGItemExt.param.Get<gtString>());
    bOk = bOk && (pParamCCVGItemId != nullptr);
    bOk = bOk && (pParamFileName != nullptr);
    bOk = bOk && (pParamFileDirPath != nullptr);
    bOk = bOk && (pParamFileMaxSize != nullptr);
    bOk = bOk && (pParamFileDataType != nullptr);
    bOk = bOk && (pParamFileDataTempBufferUseThisFlag != nullptr);
    bOk = bOk && pParamCCVGItemId->param.Set<gtString>(*m_paramCCVGItemId.param.Get<gtString>());
    bOk = bOk && pParamFileName->param.Set<gtString>(*m_paramFileName.param.Get<gtString>());
    bOk = bOk && pParamFileDirPath->param.Set<gtString>(*m_paramFileDirPath.param.Get<gtString>());
    bOk = bOk && pParamFileMaxSize->param.Set<size_t>(m_constReadFileBufferMaxSizeBytes);
    // TODO reading the source file should also trigger a read of the results file
    //      so the source view can be correctly annotated.
    bOk = bOk && pParamFileDataType->param.Set<CCVGCmdFileRead::FileDataType>(ccvgItemId.GetDataTypeEnum());
    bOk = bOk && pParamFileDataTempBufferUseThisFlag->param.Set<bool>(false);
    bOk = bOk && (rInvoke.CmdExecute() == success);
    if (!bOk)
    {
      return HandleSubCmdError(m_pCmdSubCmdFileRead);
    }
  }

  return success;
}

/// @brief Open a CodeXL MDI view and display the CCVG Code Coverage Driver 
///        file's source in the view.
/// @return status Success = all ok, failure = error see description.
status CCVGCmdWndSrcOpen::FileContentsShowMDIView()
{
  // Trigger a MDI view creation event
  const gtString& rCCVGItemId(*m_paramCCVGItemId.param.Get<gtString>());
  const gtString& rCCVGItemExt(*m_paramCCVGItemExt.param.Get<gtString>());

  const fw::CCVGMDIViewId viewId(rCCVGItemId, rCCVGItemExt);
  const gtString& rCCVGMDIViewCreatorId(fw::CCVGMDIViewCreator::GetId());
  const int viewIndex = fw::kCCVGMDIViewTypeType_SourceCodeNonEdit;
  const int lineNumber = -1;
  const int programCounterIndex = -1;
  const gtString viewTitle(viewId.GetId());   // Param fudged to become the CCVGItemId + sub-item type
  const osFilePath filePath(viewId.GetId());  // FilePath object used as an ID by the framework to find MDI sub-widgets
  apMDIViewCreateEvent mdiCreateEvent(rCCVGMDIViewCreatorId, filePath, viewTitle, viewIndex, lineNumber, programCounterIndex);
  apEventsHandler::instance().registerPendingDebugEvent(mdiCreateEvent);

  return success;
}

/// @brief On *this command failing to complete its task any and all resources,
///        bindings etc need to released or reset to a state as if the command
///        had never executed.
void CCVGCmdWndSrcOpen::OnErrorReleaseResourcesAllocByThisCmd()
{
  const gtString &rCCVGItemId(*m_paramCCVGItemId.param.Get<gtString>());
  ccvg::fw::CCVGFWTheCurrentStateProjectData &rProjData(ccvg::fw::CCVGFWTheCurrentState::Instance().GetProjectData());
  bool bCCVGItemDataSubItemDeleted = false;
  bool bCCVGItemDataDeleted = false;
  rProjData.ItemsDataDeleteSubItem(rCCVGItemId, ccvg::fw::EnumCCVGItemDataSubObjType::kCCVGItemDataSubObjType_fileDataDrivers, true, bCCVGItemDataSubItemDeleted, bCCVGItemDataDeleted);
}

/// @brief Helper function to build a meaningful error message for *this 
///        command appending messages from aggregate object(s). 
/// @param[in] vpSubCmd The sub-command that has an error condition.
/// @return status Always failure.
status CCVGCmdWndSrcOpen::HandleSubCmdError(CCVGCmdBase *vpSubCmd)
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
status CCVGCmdWndSrcOpen::Undo()
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
bool CCVGCmdWndSrcOpen::GetIsUndoableFlag() const
{
  return true;
}

} // namespace ccvg 
} // namespace cmd 