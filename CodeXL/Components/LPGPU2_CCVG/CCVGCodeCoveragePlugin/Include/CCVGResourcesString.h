// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVG resources handler for text messages. For more efficient use of 
/// resources and the sharing of them the resources object hands out resources
/// on being given a resource ID. If necessary when appropriate the resources 
/// for the case of text messages could handle language locality. 
/// CCVG resource handler is a Package. Packages in CCVG are classes or a set
/// classes (rare), probaby singletons, that carry out one task only. Packages 
/// initialise other Packages in their Initialise() function that need in 
/// order to operate themselves. They are also responsible to to shutdown
/// those packages they have initiated.
///
/// CCVGStringResource   interface
/// CCVGResourcesString  interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_RESOURCESTRING_H_INCLUDE
#define LPGPU2_CCVG_RESOURCESTRING_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_SingletonBase.h>

/// @Brief Common Resources String type resource IDs for the CCVG.
///        All the IDs must be unique.
enum {
  kIDS_CCVG_STR_ProjectName = 0,
  kIDS_CCVG_STR_VersionDescriptionDebug,
  kIDS_CCVG_STR_VersionDescription,
  kIDS_CCVG_STR_AppNameLong,
  kIDS_CCVG_STR_AppNameShort,
  kIDS_resource_err_id_mismatch,
  kIDS_resource_err_id_not_recognised,
  kIDS_resource_err_internal_fail,
  kIDS_resource_icon_err_id_mismatch,
  kIDS_resource_icon_err_id_not_recognised,
  kIDS_resource_icon_err_create_failed,
  kIDS_resource_icon_err_internal_fail,
  kIDS_CCVG_STR_err_plugin_failed_init,
  kIDS_CCVG_STR_err_plugin_failed_shutdown,
  kIDS_CCVG_STR_execmode_name,              // Used to determine if in CCVG execution mode and code logic
  kIDS_CCVG_STR_execmode_name_gui,          // Used in GUI only
  kIDS_CCVG_STR_execmode_name_gui_ccvgEnabled,
  kIDS_CCVG_STR_execmode_name_gui_ccvgDisabled,
  kIDS_CCVG_STR_execmode_menu,
  kIDS_CCVG_STR_execmode_fw_switchToAnalyzeMode,
  kIDS_CCVG_STR_execmode_fw_statusBarString, 
  kIDS_CCVG_STR_execmode_action,
  kIDS_CCVG_STR_execmode_verb,
  kIDS_CCVG_STR_execmode_session_type,
  kIDS_CCVG_STR_execmode_description,
  kIDS_CCVG_STR_execmode_user_msg_stand_alone,
  kIDS_CCVG_STR_execmode_user_msg_vstudio,
  kIDS_CCVG_STR_execmode_start_button,
  kIDS_CCVG_STR_pkg_TheCurrentState_err_fail_init,
  kIDS_CCVG_STR_pkg_TheCurrentState_err_fail_shtdwn,
  kIDS_CCVG_STR_pkg_resource_string_err_fail_init,
  kIDS_CCVG_STR_pkg_resource_string_err_fail_shtdwn,
  kIDS_CCVG_STR_pkg_resource_icon_err_fail_init,
  kIDS_CCVG_STR_pkg_resource_icon_err_fail_shtdwn,
  kIDS_CCVG_STR_pkg_cmd_invoker_string_err_fail_init,
  kIDS_CCVG_STR_pkg_cmd_invoker_string_err_fail_shtdwn,
  kIDS_CCVG_STR_pkg_cmd_invoker_err,
  kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_init_fail,
  kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_not_init_for_use,
  kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_execute_failed,
  kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_execute_exceptionCaught,
  kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_failed,
  kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_exceptionCaught,
  kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_subCmdFromCmdStack,
  kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_subCmdNotSubCmd,
  kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_err_msg,
  kIDS_CCVG_STR_pkg_cmd_invoker_err_undo_cmd_stack_empty,
  kIDS_CCVG_STR_pkg_cmd_invoker_err_undo_already_in_operation,
  kIDS_CCVG_STR_pkg_cmd_invoker_msg_noCurrentCmdATM,
  kIDS_CCVG_STR_pkg_cmd_factory_string_err_fail_init_register_cmds,
  kIDS_CCVG_STR_pkg_cmd_factory_string_err_fail_shtdwn,
  kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_invalid,
  kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_invalid_noCmdName,
  kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_invalid_useID,
  kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_alreadyRegistered,
  kIDS_CCVG_STR_pkg_cmd_factory_cmd_err_cmd_name_invalid,
  kIDS_CCVG_STR_pkg_cmd_factory_cmd_err_cmd_selfFnPtr_invalid,
  kIDS_CCVG_STR_pkg_cmd_factory_cmd_err_create_failed,
  kIDS_CCVG_STR_pkg_cmd_err_called_undo_when_not_undoable,
  kIDS_CCVG_STR_pkg_cmd_err_called_undo_when_exec_not_finished,
  kIDS_CCVG_STR_pkg_cmd_err_fw_command_instance_not_available,
  kIDS_CCVG_STR_pkg_fw_MDIViewMgr_string_err_fail_init, 
  kIDS_CCVG_STR_pkg_fw_MDIViewMgr_string_err_fail_shtdwn,
  kIDS_CCVG_STR_pkg_fw_MDIViewMgr_err,
  kIDS_CCVG_STR_pkg_fw_MDIViewFactory_string_err_fail_init_register_views, 
  kIDS_CCVG_STR_pkg_fw_MDIViewFactory_string_err_fail_shtdwn,
  kIDS_CCVG_STR_pkg_fw_MDIViewFactory_err_view_name_invalid,
  kIDS_CCVG_STR_pkg_fw_MDIViewFactory_viewType_err_invalid,
  kIDS_CCVG_STR_pkg_fw_MDIViewFactory_viewType_err_alreadyRegistered,
  kIDS_CCVG_STR_pkg_fw_MDIViewFactory_viewType_err_notRegisteredType,
  kIDS_CCVG_STR_pkg_fw_MDIViewFactory_err_view_selfFnPtr_invalid,
  kIDS_CCVG_STR_pkg_fw_MDIViewFactory_err_create_failed,
  kIDS_CCVG_STR_gui_toolbar_mainWnd_err_fail_init,
  kIDS_CCVG_STR_gui_toolbar_mainWnd_err_fail_shtdwn,
  kIDS_CCVG_STR_gui_toolbar_mainWnd_label_info,
  kIDS_CCVG_STR_gui_TabPageSettings_err_fail_init,
  kIDS_CCVG_STR_gui_TabPageSettings_err_fail_create_gui_elements,
  kIDS_CCVG_STR_gui_TabPageSettings_err_fail_shtdwn,
  kIDS_CCVG_STR_gui_TabPageSettings_page_title,
  kIDS_CCVG_STR_gui_TabPageSettings_txtEnableForAllProjs,
  kIDS_CCVG_STR_gui_ProjSettings_fail_create_gui_elements,
  kIDS_CCVG_STR_gui_ProjSettings_page_title,
  kIDS_CCVG_STR_gui_ProjSettings_page_chkbx_txtEnabledDisablePluginForCurrentProj,
  kIDS_CCVG_STR_gui_ProjSettings_page_label_txtExplain3SetsOfFiles,
  kIDS_CCVG_STR_gui_ProjSettings_page_label_requiredFields,
  kIDS_CCVG_STR_gui_ProjSettings_page_label_folder,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_label,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_dirBtnTooltip,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_editTxtTip,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_labelExtn,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_folderDlgCaption,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_label,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_dirBtnTooltip,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_editTxtTip,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_labelExtn,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_folderDlgCaption,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_chkbx_mustHave,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_chkbx_mustHaveTooltip,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_label,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_dirBtnTooltip,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_editTxtTip,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_labelExtn,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_folderDlgCaption,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_label,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_dirBtnTooltip,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_editTxtTip,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_labelExtn,
  kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_folderDlgCaption,
  KIDS_CCVG_STR_gui_MDIViewCreator_err_fail_init,
  KIDS_CCVG_STR_gui_MDIViewCreator_err_fail_shtdwn,
  kIDS_CCVG_STR_gui_ProjectSettings_err_fail_init,
  kIDS_CCVG_STR_gui_ProjectSettings_err_fail_shtdwn,
  kIDS_CCVG_STR_gui_ProjectSettings_err_ccvg_not_readyToWork,
  kIDS_CCVG_STR_gui_ProjectSettings_warning_ccvg_loose_dataAndSettings,
  kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesGood_label,
  kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesGood_tooltip,
  kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesBad_label,
  kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesBad_tooltip,
  kIDS_CCVG_STR_gui_AppTreeHandler_err_creatingActionObjs,
  kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewSrcCode,
  kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewDriverCode,
  kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewResultFile,
  kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewExecFile,
  kIDS_CCVG_STR_fw_action_err_unknown_ID,
  kIDS_CCVG_STR_fw_action_err_command_failed,
  KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFolderPath,
  KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFileExtn,
  KIDS_CCVG_STR_fw_projectData_err_invalidCCVGItemId,
  KIDS_CCVG_STR_fw_projectData_err_dupicateCCVGItemId,
  KIDS_CCVG_STR_fw_projectData_err_failCreateCCVGItemData,
  KIDS_CCVG_STR_fw_MDIViewCreator_err_failCreateViewCodeLogic,
  KIDS_CCVG_STR_fw_MDIView_err_invalidViewId,
  kIDS_CCVG_STR_project_settings_menu,
  kIDS_CCVG_STR_project_settings_fw_statusBarString,
  kIDS_CCVG_STR_project_settings_extn_display_name,
  kIDS_CCVG_STR_project_refreshResultsFiles,
  kIDS_CCVG_STR_project_loadResultsFiles,
  kIDS_CCVG_STR_project_fw_statusBarString_refreshResultsFiles,
  kIDS_CCVG_STR_project_fw_statusBarString_loadResultsFiles,
  kIDS_CCVG_STR_cmd_filesReadAll_err_subCmdFail, 
  kIDS_CCVG_STR_cmd_filesReadAll_err_aTaskFail,
  kIDS_CCVG_STR_cmd_filesRead_err_dirNotExist,
  kIDS_CCVG_STR_cmd_filesReadCcvgr_err_fileNameInValid,
  kIDS_CCVG_STR_cmd_filesReadKc_err_fileNameInValid,
  kIDS_CCVG_STR_cmd_filesReadCcvgd_err_fileNameInValid,
  kIDS_CCVG_STR_cmd_filesReadCcvgs_err_fileNameInValid,
  kIDS_CCVG_STR_cmd_filesReadCcvge_err_fileNameInValid,
  kIDS_CCVG_STR_cmd_fileRead_err_reading,
  kIDS_CCVG_STR_cmd_fileRead_err_maxFileSizeExceeded,
  kIDS_CCVG_STR_cmd_fileRead_err_failedToStoreData,
  kIDS_CCVG_STR_cmd_fileRead_err_bufferAllocFail,
  kIDS_CCVG_STR_util_file_err_fileNamePathInvalid,
  kIDS_CCVG_STR_util_file_err_fileNameInvalid,
  kIDS_CCVG_STR_util_file_err_fileDirPathInvalid,
  kIDS_CCVG_STR_util_file_err_fileNameToLong,
  kIDS_CCVG_STR_util_file_err_fileDirPathToLong,
  kIDS_CCVG_STR_util_file_err_invalidBufferSize,
  kIDS_CCVG_STR_util_file_err_readFailed,
  kIDS_CCVG_STR_util_file_err_openFailed,
  kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferSizeZero,
  kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferMaxSizeExceeded,
  kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferAllocFail,
  kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFail,
  kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailResults,
  kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailSource,
  kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailDrivers,
  kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailExecuteable,
  kIDS_CCVG_STR_cmn_CCVGItemDataExecuteable_err_allocateExecutable,
  kIDS_CCVG_STR_cmn_CCVGItemDataExecuteable_err_createAMDTExecutable,
  kIDS_CCVG_STR_cmn_CCVGItemDataExecuteable_err_readCovMapSegment,
  kIDS_CCVG_STR_cmn_CCVGLLVMResultTextReader_err_readCCVGRFile,
  kIDS_CCVG_STR_cmn_CCVGLLVMCovMapReader_err_readCovMapData,
  kIDS_CCVG_STR_cmn_LLVMCovMapReader_FunctionRecord,
  kIDS_CCVG_STR_cmn_LLVMCovMapReader_FunctionHash,
  kIDS_CCVG_STR_cmn_LLVMCovMapReader_FilenameIndex,
  kIDS_CCVG_STR_cmn_LLVMCovMapReader_FilenamePointsTo,
  kIDS_CCVG_STR_cmn_LLVMCovMapReader_NumCounterExpressions,
  kIDS_CCVG_STR_cmn_LLVMCovMapReader_CounterExpression,
  kIDS_CCVG_STR_cmn_LLVMCovMapReader_ExpansionTag,
  kIDS_CCVG_STR_cmn_LLVMCovMapReader_ExpansionFile,
  kIDS_CCVG_STR_cmn_LLVMFunctionCovMap_err_FailedPushBack,
  kIDS_CCVG_STR_cmn_LLVMCoverageHighlighter_err_MissingResults,
  kIDS_CCVG_STR_cmn_MDISourceView_HighlightKeyHeader,
  kIDS_CCVG_STR_cmn_MDISourceView_HighlightKeyExecuted,
  kIDS_CCVG_STR_cmn_MDISourceView_HighlightKeyNotExecuted,
  kIDS_CCVG_STR_cmn_MDISourceView_SummaryText,
  kIDS_CCVG_STR_code_err_defaultReached,
  kIDS_CCVG_STR_word_none_brked,
  kIDS_CCVG_STR_word_unknown_brked,
  KIDS_CCVG_STR_word_new_sentence,
  kIDS_CCVG_STR_resource_count
};

/// @Brief A resource structure. Common Resources String type for the CCVG.  
// clang_format off
struct CCVGStringResource {
  CCVGuint       m_resourceId;  // Resource ID
  const wchar_t *m_strText;     // Static global text string
};
// clang_format on

/// @Brief Common string resources handler class for the CCVG.  
///        Singleton pattern. CCVGResourcesString is Package.
///
/// The handler is the one of the very first objects to be setup. Its
/// primary use here is to hand out text or message resources.
/// All the resources are statically defined at compile time.
///
/// @warning  None.
/// @see      None.
/// @date     05/08/2016.
/// @author   Illya Rudkin.
// clang_format off
class CCVGResourcesString final 
: public CCVGClassCmnBase
, public ISingleton<CCVGResourcesString>
{
  friend ISingleton<CCVGResourcesString>;

// Enums:
private:
enum 
{
  kStringResourceItemCount = kIDS_CCVG_STR_resource_count
};

// Methods:
public:
  const CCVGStringResource& GetResource(CCVGuint vId);
  bool                      HasStringResource(CCVGuint vId);

// Overridden:
public:
  virtual ~CCVGResourcesString() override;
  // From util::ISingleton
  virtual status Initialise() override;
  virtual status Shutdown() override;

// Methods:
private:
   // Singleton enforcement
   CCVGResourcesString();
   CCVGResourcesString(const CCVGResourcesString&) = delete;
   CCVGResourcesString(CCVGResourcesString&&) = delete;
   CCVGResourcesString& operator= (const CCVGResourcesString&) = delete;
   CCVGResourcesString& operator= (CCVGResourcesString&&) = delete;
   //
   bool GetStringFromResource(CCVGuint vId, const CCVGStringResource *&vRsrc);

// Attributes:
private:
  static CCVGStringResource ms_arrayStringResources[kStringResourceItemCount + 1];
  static CCVGStringResource ms_thisErrorGetFailedTxt;
};
// clang-format on

/// @brief CCVG Resource retrieval shortcut macro for text resources
#define CCVGRSRC(x) CCVGResourcesString::Instance().GetResource(x).m_strText
#define CCVGRSRCA(x) gtString(CCVGRSRC(x)).asASCIICharArray()

#endif // LPGPU2_CCVG_RESOURCESTRING_H_INCLUDE