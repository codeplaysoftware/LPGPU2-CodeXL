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
///
/// CCVGResourcesString  implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

// Instantiations:
CCVGStringResource CCVGResourcesString::ms_thisErrorGetFailedTxt = {
  kIDS_resource_err_internal_fail, L"Error: String resource failure use ErrorGetDescription() for fault description"
};
/// @Brief String resoures used only for the CCVG code base.
CCVGStringResource CCVGResourcesString::ms_arrayStringResources[] = {
  { kIDS_CCVG_STR_ProjectName, L"LPGPU2 CCVG" },
  { kIDS_CCVG_STR_VersionDescriptionDebug, L"Version 0.2.0.0 (Debug)" }, // Match CCVG Readme.txt
  { kIDS_CCVG_STR_VersionDescription, L"Version 0.2.0.0" },              // Match CCVG Readme.txt
  { kIDS_CCVG_STR_AppNameLong, L"LPGPU2 CodeXL Kernel Code Coverage" },
  { kIDS_CCVG_STR_AppNameShort, L"CCVG" },
  { kIDS_resource_err_id_mismatch, L"Error: String resource ID mismatch array position" },
  { kIDS_resource_err_id_not_recognised, L"Error: String resource ID not recognised" },
  { kIDS_resource_err_internal_fail, L"Error: String resource failure use ErrorGetDescription() for fault description" },
  { kIDS_resource_icon_err_id_mismatch, L"Error: Icon resource ID mismatch array position" },
  { kIDS_resource_icon_err_id_not_recognised, L"Error: Icon resource ID not recognised" },
  { kIDS_resource_icon_err_create_failed, L"Error: Icon resource ID not recognised" },
  { kIDS_resource_icon_err_internal_fail, L"Error: Icon resource failure use ErrorGetDescription() for fault description" },
  { kIDS_CCVG_STR_err_plugin_failed_init, L"Error: DLL plugin '%ls' failed to initialise" },
  { kIDS_CCVG_STR_err_plugin_failed_shutdown, L"Error: DLL plugin '%ls' failed to shutdown" },
  { kIDS_CCVG_STR_execmode_name, L"Code Coverage Mode" },
  { kIDS_CCVG_STR_execmode_name_gui, L"Code Coverage" },
  { kIDS_CCVG_STR_execmode_name_gui_ccvgEnabled, L"Code Coverage - %d items" },
  { kIDS_CCVG_STR_execmode_name_gui_ccvgDisabled, L"Code Coverage - Disabled" },
  { kIDS_CCVG_STR_execmode_menu, L"&Code Coverage Mode" },
  { kIDS_CCVG_STR_execmode_fw_switchToAnalyzeMode, L"Switch to &Code Coverage Mode" },
  { kIDS_CCVG_STR_execmode_fw_statusBarString, L"Code Coverage Mode - Switch to Code Coverage Mode"},
  { kIDS_CCVG_STR_execmode_action, L"View code coverage" },
  { kIDS_CCVG_STR_execmode_verb, L"code coverage" },
  { kIDS_CCVG_STR_execmode_session_type, L"Code Coverage" },
  { kIDS_CCVG_STR_execmode_description, L"Import and view LLVM code coverage analysis results of OpenCL1.2 kernels" }, // For next 2 lines below, change the text matching text in CodeXL\AMDTApplicationFramework\Include\res\welcomeLPGPU2XXX.html files
  { kIDS_CCVG_STR_execmode_user_msg_stand_alone, L"Create a new project to import code LLVM code coverage analysis result files. In the project settings dialog set the source of the files." },
  { kIDS_CCVG_STR_execmode_user_msg_vstudio, L"Create a new project to import LLVM code coverage analysis result files. In the project settings dialog set the source of the files." },
  { kIDS_CCVG_STR_execmode_start_button, L"View Code Coverage" },
  { kIDS_CCVG_STR_pkg_TheCurrentState_err_fail_init, L"Error: Package Resource failed to initialise. %ls" },
  { kIDS_CCVG_STR_pkg_TheCurrentState_err_fail_shtdwn, L"Error: Package Resource failed to shutdown. %ls" },
  { kIDS_CCVG_STR_pkg_resource_string_err_fail_init, L"Error: Package Resource failed to initialise. %ls" },
  { kIDS_CCVG_STR_pkg_resource_string_err_fail_shtdwn, L"Error: Package Resource failed to shutdown. %ls" },
  { kIDS_CCVG_STR_pkg_resource_icon_err_fail_init, L"Error: Package Icon Resource failed to initialise. %ls" },
  { kIDS_CCVG_STR_pkg_resource_icon_err_fail_shtdwn, L"Error: Package Icon Resource failed to shutdown. %ls" },
  { kIDS_CCVG_STR_pkg_cmd_invoker_string_err_fail_init, L"Error: Package Command Invoker failed to initialise. %ls" },
  { kIDS_CCVG_STR_pkg_cmd_invoker_string_err_fail_shtdwn, L"Error: Package Command Invoker failed to shutdown. %ls" },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err, L"Error: Package Command Invoker. %ls" },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_init_fail, L"Error: Package Command Invoker create command '%ls' initialisation failed. %ls" },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_not_init_for_use, L"Error: Package Command Invoker. No command has been prepared for use." },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_execute_failed, L"Error: Package Command Invoker. Command '%ls' execution failed. %ls" },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_execute_exceptionCaught, L"Error: Package Command Invoker. Command '%ls' execution failed. Exception caught." },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_failed, L"Error: Package Command Invoker. Command '%ls' undo failed. %ls" },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_exceptionCaught, L"Error: Package Command Invoker. Command '%ls' undo failed. Exception caught." },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_subCmdFromCmdStack, L"Error: Package Command Invoker. Command '%ls' is a sub command on command stack. Should not be." },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_subCmdNotSubCmd, L"Error: Package Command Invoker. Command '%ls' is a command called as sub command. Should not be." },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_err_msg, L"Error: Comand '%ls'. %ls" },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err_undo_cmd_stack_empty, L"Error: Package Command Invoker call to Undo() when the command stack is empty" },
  { kIDS_CCVG_STR_pkg_cmd_invoker_err_undo_already_in_operation, L"Error: Package Command Invoker undo functioanlity already in use. Cannot disable undo once in use." },
  { kIDS_CCVG_STR_pkg_cmd_invoker_msg_noCurrentCmdATM, L"Package Command Invoker: No current command ATM" },
  { kIDS_CCVG_STR_pkg_cmd_factory_string_err_fail_init_register_cmds, L"Error: Package Command Factory failed to initialise registering commands" },
  { kIDS_CCVG_STR_pkg_cmd_factory_string_err_fail_shtdwn, L"Error: Package Command Factory failed to shutdown. %ls" },
  { kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_invalid, L"Error: Invalid command ID registering command '%ls'" },
  { kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_invalid_noCmdName, L"Error: Invalid command ID" },
  { kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_invalid_useID, L"Error: Invalid command ID %d" },
  { kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_alreadyRegistered, L"Error: Command ID already registered '%ls' command" },
  { kIDS_CCVG_STR_pkg_cmd_factory_cmd_err_cmd_name_invalid, L"Error: Invalid command name given registering command" },
  { kIDS_CCVG_STR_pkg_cmd_factory_cmd_err_cmd_selfFnPtr_invalid, L"Error: Invalid command creation function for '%ls' command" },
  { kIDS_CCVG_STR_pkg_cmd_factory_cmd_err_create_failed, L"Error: Failed to create command, ID = %d" },
  { kIDS_CCVG_STR_pkg_cmd_err_called_undo_when_not_undoable, L"Error: The command '%ls' is not undoable yet Undo() called." },
  { kIDS_CCVG_STR_pkg_cmd_err_called_undo_when_exec_not_finished, L"Error: The command '%ls' about undo but acknowledge is has completed it execute task" },
  { kIDS_CCVG_STR_pkg_cmd_err_fw_command_instance_not_available, L"Error: The framework command instance not available" },
  { kIDS_CCVG_STR_pkg_fw_MDIViewMgr_string_err_fail_init, L"Error: Package MDI View Manager failed to initialise. %ls" },
  { kIDS_CCVG_STR_pkg_fw_MDIViewMgr_string_err_fail_shtdwn, L"Error: Package MDI View Manager failed to shutdown. %ls" },
  { kIDS_CCVG_STR_pkg_fw_MDIViewMgr_err, L"Error: Package MDI View Manager. %ls" },
  { kIDS_CCVG_STR_pkg_fw_MDIViewFactory_string_err_fail_init_register_views, L"Error: Package MDI View Factory failed to initialise registering views" },
  { kIDS_CCVG_STR_pkg_fw_MDIViewFactory_string_err_fail_shtdwn, L"Error: Package MDI View Factory failed to shutdown. %ls" },
  { kIDS_CCVG_STR_pkg_fw_MDIViewFactory_err_view_name_invalid, L"Error: Invalid view name given registering view" },
  { kIDS_CCVG_STR_pkg_fw_MDIViewFactory_viewType_err_invalid, L"Error: Invalid view type specified '%d'" },
  { kIDS_CCVG_STR_pkg_fw_MDIViewFactory_viewType_err_alreadyRegistered, L"Error: View type already registered for '%ls' view" },
  { kIDS_CCVG_STR_pkg_fw_MDIViewFactory_viewType_err_notRegisteredType, L"Error: View type %d is not registered type with the view factory" },
  { kIDS_CCVG_STR_pkg_fw_MDIViewFactory_err_view_selfFnPtr_invalid, L"Error: Invalid view creation function for '%ls' view" },
  { kIDS_CCVG_STR_pkg_fw_MDIViewFactory_err_create_failed, L"Error: Failed to create view, type = %d" },
  { kIDS_CCVG_STR_gui_toolbar_mainWnd_err_fail_init, L"Error: Toolbar CCVG main window failed to initialise. %ls" },
  { kIDS_CCVG_STR_gui_toolbar_mainWnd_err_fail_shtdwn, L"Error: Toolbar CCVG main window failed to shutdown. %ls" },
  { kIDS_CCVG_STR_gui_toolbar_mainWnd_label_info, L"Coverage Coverage options" },
  { kIDS_CCVG_STR_gui_TabPageSettings_err_fail_init, L"Error: CCVG Tab Page Settings failed to initialise. %ls" },
  { kIDS_CCVG_STR_gui_TabPageSettings_err_fail_create_gui_elements, L"Error: CCVG Tab Page Settings QT objects failed creation" },
  { kIDS_CCVG_STR_gui_TabPageSettings_err_fail_shtdwn, L"Error: CCVG Tab Page Settings failed to shutdown. %ls" },
  { kIDS_CCVG_STR_gui_TabPageSettings_page_title, L"Code Co&verage" },
  { kIDS_CCVG_STR_gui_TabPageSettings_txtEnableForAllProjs, L"Enable for all CodeXL projects" },
  { kIDS_CCVG_STR_gui_ProjSettings_fail_create_gui_elements, L"Error: CCVG Project Settings QT objects failed creation" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_title, L"General settings" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_chkbx_txtEnabledDisablePluginForCurrentProj, L"Enable Code Coverage for this current project (overridden by tool options)" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_label_txtExplain3SetsOfFiles, L"One coverage item is made up of 4 files; results, source, executable and driver. Each file name must be unique to an item, each differentiated by the file extension. Result, source and executable files are mandatory." },
  { kIDS_CCVG_STR_gui_ProjSettings_page_label_requiredFields, L"Required fields *" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_label_folder, L"<font style=\"italic\"><sup>*</sup></font>Folder path: " },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_label, L"Code coverage result files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_dirBtnTooltip, L"Click to choose the folder location containing the code coverage result files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_editTxtTip, L"Folder directory location containing the code coverage result files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_labelExtn, L"<font style=\"italic\"><sup>*</sup></font>Coverage result files' file extension: " },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_folderDlgCaption, L"Code coverage results files folder" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_label, L"Kernel source files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_dirBtnTooltip, L"Click to choose the folder location containing the kernel source files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_editTxtTip, L"Folder directory location containing the kernel source files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_labelExtn, L"<font style=\"italic\"><sup>*</sup></font>Kernel source files' file extension: " },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_folderDlgCaption, L"Code coverage kernel source files folder" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_chkbx_mustHave, L"Mandatory" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_chkbx_mustHaveTooltip, L"Check to include matching driver files to items" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_label, L"Driver source files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_dirBtnTooltip, L"Click to choose the folder location containing the driver source files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_editTxtTip, L"Folder directory location containing the driver source files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_labelExtn, L"<font style=\"italic\"><sup>*</sup></font>Driver source files' file extension: " },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_folderDlgCaption, L"Code coverage driver files folder" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_label, L"Executable files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_dirBtnTooltip, L"Click to choose the folder location containing the executable files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_editTxtTip, L"Folder directory location containing the executable files" },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_labelExtn, L"<font style=\"italic\"><sup>*</sup></font>executable files' file extension: " },
  { kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_folderDlgCaption, L"Code coverage executable files folder" },
  { KIDS_CCVG_STR_gui_MDIViewCreator_err_fail_init, L"Error: CCVG MDI View Creator failed to initialise" },
  { KIDS_CCVG_STR_gui_MDIViewCreator_err_fail_shtdwn, L"Error: CCVG MDI View Creator failed to shutdown" },
  { kIDS_CCVG_STR_gui_ProjectSettings_err_fail_init, L"Error: CCVG Project Settings GUI extension failed initialise" },
  { kIDS_CCVG_STR_gui_ProjectSettings_err_fail_shtdwn, L"Error: CCVG Project Settings GUI extension failed shutdown" },
  { kIDS_CCVG_STR_gui_ProjectSettings_err_ccvg_not_readyToWork, L"On CodeXL start up the Code Coverage plugin (DLL) was unable to allocate or finds resources necessary to carry out its work and so is not available." },
  { kIDS_CCVG_STR_gui_ProjectSettings_warning_ccvg_loose_dataAndSettings, L"Note: Code coverage data and settings will be lost if this project is loaded into CodeXL which does not have the Code Coverage plugin." },
  { kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesGood_label, L"Code coverage results" },
  { kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesGood_tooltip, L"Here all items' Results Source Driver and Executable files are present and ok" },
  { kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesBad_label, L"Corrupt coverage results" },
  { kIDS_CCVG_STR_gui_AppTreeHandler_ccvgRootNodeSetFilesBad_tooltip, L"Here one or more items' files Results Source Driver or Executable is either missing or corrupt" },
  { kIDS_CCVG_STR_gui_AppTreeHandler_err_creatingActionObjs, L"Error: App tree handler: Failed to create one or more QAction objects" },
  { kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewSrcCode, L"View coverage results" },
  { kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewDriverCode, L"View source code" },
  { kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewResultFile, L"View result file" },
  { kIDS_CCVG_STR_gui_AppTreeHandler_qactionLabel_viewExecFile, L"View executable file" },
  { kIDS_CCVG_STR_fw_action_err_unknown_ID, L"Error: Unknown action command event id" },
  { kIDS_CCVG_STR_fw_action_err_command_failed, L"Error: On do CCVG menu action command '%ls' failed to do its task" },
  { KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFolderPath, L"%ls files directory path invalid. It must contain a valid path." },
  { KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFileExtn, L"%ls file extension invalid. A valid file extension must be at least 2 alpha numeric characters. First charactor is '.'" },
  { KIDS_CCVG_STR_fw_projectData_err_invalidCCVGItemId, L"Error: Invalid CCVG item ID" },
  { KIDS_CCVG_STR_fw_projectData_err_dupicateCCVGItemId, L"Error: Duplicated CCVG item ID '%ls'" },
  { KIDS_CCVG_STR_fw_projectData_err_failCreateCCVGItemData, L"Error: Failed to create CCVG item data object for item ID '%ls" },
  { KIDS_CCVG_STR_fw_MDIViewCreator_err_failCreateViewCodeLogic, L"Error: CCVGMDIViewCreator: Code logic failure" },
  { KIDS_CCVG_STR_fw_MDIView_err_invalidViewId, L"Error: CCVGMDIView: Invalid view ID '%ls'" },
  { kIDS_CCVG_STR_project_settings_menu, L"Code Coverage &Settings..." },
  { kIDS_CCVG_STR_project_settings_fw_statusBarString, L"Open Code Coverage settings dialog" },
  { kIDS_CCVG_STR_project_settings_extn_display_name, L"Code Coverage" },
  { kIDS_CCVG_STR_project_refreshResultsFiles, L"Refresh files" },
  { kIDS_CCVG_STR_project_loadResultsFiles, L"Load files" },
  { kIDS_CCVG_STR_project_fw_statusBarString_refreshResultsFiles, L"Rescan code coverage source directories updating content or add new files match file extensions" },
  { kIDS_CCVG_STR_project_fw_statusBarString_loadResultsFiles, L"Scan code coverage source directories load in files matching file extensions" },
  { kIDS_CCVG_STR_cmd_filesReadAll_err_subCmdFail, L"Error: Command '%ls' sub-command '%ls' failed. Error: '%ls'" },
  { kIDS_CCVG_STR_cmd_filesReadAll_err_aTaskFail, L"Error: Command '%ls' task failed. '%ls'" },
  { kIDS_CCVG_STR_cmd_filesRead_err_dirNotExist, L"Error: Command '%ls' parameter '%ls' data '%ls' folder not exist" },
  { kIDS_CCVG_STR_cmd_filesReadCcvgr_err_fileNameInValid, L"Error: Command '%ls'. Read invalid filename '%ls'." },
  { kIDS_CCVG_STR_cmd_filesReadKc_err_fileNameInValid, L"Error: Command '%ls'. Read invalid filename '%ls'." },
  { kIDS_CCVG_STR_cmd_filesReadCcvgd_err_fileNameInValid, L"Error: Command '%ls'. Read invalid filename '%ls'." },
  { kIDS_CCVG_STR_cmd_filesReadCcvgs_err_fileNameInValid, L"Error: Command '%ls'. Read invalid filename '%ls'." },
  { kIDS_CCVG_STR_cmd_filesReadCcvge_err_fileNameInValid, L"Error: Command '%ls'. Read invalid filename '%ls'." },
  { kIDS_CCVG_STR_cmd_fileRead_err_reading, L"Error: Command '%ls'. File error '%ls'."},
  { kIDS_CCVG_STR_cmd_fileRead_err_maxFileSizeExceeded, L"Error: Command '%ls'. File '%ls' exceeds maximum size allowed of %d bytes." },
  { kIDS_CCVG_STR_cmd_fileRead_err_failedToStoreData, L"Error: Command '%ls'. Failed to store data internally for file '%ls'. %ls." },
  { kIDS_CCVG_STR_cmd_fileRead_err_bufferAllocFail, L"Error: Command '%ls'. Buffer allocation %d bytes failure." },
  { kIDS_CCVG_STR_util_file_err_fileNamePathInvalid, L"Error: File name '%ls' and or path '%ls' is not valid" },
  { kIDS_CCVG_STR_util_file_err_fileNameInvalid, L"Error: File name '%ls' is not valid or empty" },
  { kIDS_CCVG_STR_util_file_err_fileDirPathInvalid, L"Error: File directory path '%ls' is not valid" },
  { kIDS_CCVG_STR_util_file_err_fileNameToLong,  L"Error: File name '%ls' is to long. Maximum length is %d." },
  { kIDS_CCVG_STR_util_file_err_fileDirPathToLong, L"Error: File directory path '%ls' is to long. Maximum length is %d." },
  { kIDS_CCVG_STR_util_file_err_invalidBufferSize, L"Error: Buffer size specified of %d bytes is invalid" },
  { kIDS_CCVG_STR_util_file_err_readFailed, L"Error: Failed to read file '%ls'." },
  { kIDS_CCVG_STR_util_file_err_openFailed, L"Error: Failed to open file '%ls'." },
  { kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferSizeZero, L"Error: CCVGItem '%ls' file data buffer size 0 bytes." },
  { kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferMaxSizeExceeded, L"Error: CCGVItem '%ls' file data buffer size exceeded %d bytes." },
  { kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferAllocFail, L"Error: CCVGItem '%ls' file data buffer size %d bytes allocation memory failed." },
  { kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFail, L"Error: CCVGItem '%ls' re-reading the file to see if it has changed failed. %ls." },
  { kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailResults, L"CCVG results file" },
  { kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailSource, L"CCVG source file" },
  { kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailDrivers, L"CCVG driver file" },
  { kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailExecuteable, L"CCVG executable file"},
  { kIDS_CCVG_STR_cmn_CCVGItemDataExecuteable_err_allocateExecutable, L"Error: Allocation failure for AMDTExecutableFormat (out of memory?)."},
  { kIDS_CCVG_STR_cmn_CCVGItemDataExecuteable_err_createAMDTExecutable, L"Error: Could not create AMDTExecutableFormat from executable data."},
  { kIDS_CCVG_STR_cmn_CCVGItemDataExecuteable_err_readCovMapSegment, L"Error: Executable did not contain expected segment: '%ls' in '%ls'."},
  { kIDS_CCVG_STR_cmn_CCVGLLVMResultTextReader_err_readCCVGRFile, L"Error: Malformed CCVG results file."},
  { kIDS_CCVG_STR_cmn_CCVGLLVMCovMapReader_err_readCovMapData, L"Error: Malformed coverage mapping data in executable."},
  { kIDS_CCVG_STR_cmn_LLVMCovMapReader_FunctionRecord, L"Function Record: "},
  { kIDS_CCVG_STR_cmn_LLVMCovMapReader_FunctionHash, L"Function Hash: "},
  { kIDS_CCVG_STR_cmn_LLVMCovMapReader_FilenameIndex, L"Filename Index "},
  { kIDS_CCVG_STR_cmn_LLVMCovMapReader_FilenamePointsTo, L"\tPoints to: "},
  { kIDS_CCVG_STR_cmn_LLVMCovMapReader_NumCounterExpressions, L"Number of counter expressions: "},
  { kIDS_CCVG_STR_cmn_LLVMCovMapReader_CounterExpression, L"Counter expression "},
  { kIDS_CCVG_STR_cmn_LLVMCovMapReader_ExpansionTag, L"\tExpansion region tag: "},
  { kIDS_CCVG_STR_cmn_LLVMCovMapReader_ExpansionFile, L"\tExpansion region file index: "},
  { kIDS_CCVG_STR_cmn_LLVMFunctionCovMap_err_FailedPushBack, L"\tError: Could not insert data into container (allocation failure/out of memory?)."},
  { kIDS_CCVG_STR_cmn_LLVMCoverageHighlighter_err_MissingResults, L"Error: Missing entry in result data for function with hash %lu (mismatched files?)."},
  { kIDS_CCVG_STR_cmn_MDISourceView_HighlightKeyHeader, L"Code Coverage Highlighting Key: "},
  { kIDS_CCVG_STR_cmn_MDISourceView_HighlightKeyExecuted, L" Executed "},
  { kIDS_CCVG_STR_cmn_MDISourceView_HighlightKeyNotExecuted, L" Not Executed "},
  { kIDS_CCVG_STR_cmn_MDISourceView_SummaryText, L"Summary for '%1': %2/%3 (%4%) regions covered."},
  { kIDS_CCVG_STR_code_err_defaultReached, L"Error: Code logic: reach default in switch in function '%ls'" },
  { kIDS_CCVG_STR_word_none_brked, L"<none>" },
  { kIDS_CCVG_STR_word_unknown_brked, L"<unknown>" },
  { KIDS_CCVG_STR_word_new_sentence, L". %ls" }
};

/// @brief  Package initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGResourcesString::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised) 
  {
    return success;
  }

  // Do nothing here

  m_bBeenInitialised = true;
  return success;
}

/// @brief  Package shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGResourcesString::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0)) 
  {
    return success;
  }

  // Do nothing here

  m_bBeenShutdown = true;
  return success;
}

/// @brief  Class constructor. No work is done here.
CCVGResourcesString::CCVGResourcesString()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGResourcesString::~CCVGResourcesString()
{
  Shutdown();
}

/// @brief Retrieve the resource structure represented by the resource ID.
/// @param[in]   vId Unique resource ID.
/// @param[out]  vRsrc Pointer the resource structure holding the resource.
/// @return bool True = function succeeded in its task,
///              false = function failed in its task.
bool CCVGResourcesString::GetStringFromResource(CCVGuint vId, const CCVGStringResource *&vRsrc)
{
  if (vId >= kStringResourceItemCount) 
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_err_id_not_recognised));
  }
     
  const CCVGStringResource &resource = ms_arrayStringResources[vId];
  if (resource.m_resourceId != vId) 
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_err_id_mismatch));
  }

  vRsrc = &resource;

  return true;
}

/// @brief Retrieve the resource structure represented by the resource ID.
///        Debug assert is performed should the ID not be recognised. If in
///        release build the default error resource structure in returned
///        containing a error message in the case of a string resource or
///        a dummy resource object.
/// @param[in] vId Unique resource ID.
/// @return    CCVGStringResource& Reference to object.
const CCVGStringResource& CCVGResourcesString::GetResource(CCVGuint vId)
{
  const CCVGStringResource *pRsrc = nullptr;
  const bool bFound = GetStringFromResource(vId, pRsrc);
  GT_ASSERT_EX(bFound, CCVGRSRC(kIDS_resource_err_id_not_recognised));
  if (pRsrc != nullptr)
  {
    return *pRsrc;
  }

  return ms_thisErrorGetFailedTxt;
}

/// @brief Determine if the resource structure represented by the resource ID
///        is present and recognised.
/// @param[in] vId Unique resource ID.
/// @return    bool True = ID is valid, false = not valid.
bool CCVGResourcesString::HasStringResource(CCVGuint vId)
{
  const CCVGStringResource *pRsrc = nullptr;
  return GetStringFromResource(vId, pRsrc);
}

