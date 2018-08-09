//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppStringConstants.h
///
//==================================================================================

//------------------------------ ppStringConstants.h ------------------------------

#ifndef __PPSTRINGCONSTANTS_H
#define __PPSTRINGCONSTANTS_H


#include <Backend/AMDTPowerProfileAPI/src/ppCountersStringConstants.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Driver:
#define PP_STR_DriverPath L"\\drivers\\AMDTPWRPROF"

// Project setting
#define PP_STR_projectSettingExtensionName L"PowerProfile"
#define PP_STR_projectSettingExtensionNameASCII "PowerProfile"
#define PP_STR_projectSettingExtensionDisplayName L"Profile, Power Profile"

// Global settings xml file
#define PP_STR_projectSettingEnabledCounters L"EnabledCounters"
#define PP_STR_projectSettingSamplingInterval L"SamplingInterval"

// Project settings:
#define PP_STR_projectSettingsSamplingInterval "Sampling Interval"
//++AT:LPGPU2
#define PP_STR_LPGPU2projectSettingsSamplingInterval "Sampling Interval (AMD)"
//--AT:LPGPU2
#define PP_STR_projectSettingsSampleEvery "Counters sampling interval (ms)"

//++TLRS: LPGPU2:
#define PP_STR_LPGPU2RAgentFolderW L"LPGPU2_RAgent"
#define PP_STR_APKFileExtensionW L"*.apk"
//--TLRS: LPGPU2:

// Profiling info
#define PP_STR_OnlineProfileName L"Power Profiling"

// Power Graph yAxis units:
#define PP_STR_UnitsWatts "Watts"
#define PP_STR_UnitsJoules "Joules"
#define PP_STR_UnitsKiloJoules "Kilojoules"
#define PP_STR_UnitsMegaJoules "Megajoules"
#define PP_STR_UnitsGigaJoules "Gigajoules"
#define PP_STR_UnitsTeraJoules "Terajoules"
#define PP_STR_UnitsPostfixVolt "[V]"
#define PP_STR_UnitsPostfixWatt "[W]"
#define PP_STR_UnitsPostfixMHz "[MHz]"
#define PP_STR_UnitsPostfixPercentage "%"
#define PP_STR_UnitsPostfixCelsius "[C]"
#define PP_STR_UnitsPostfixmA "[mA]"

// file info
#define PP_STR_dbFileExt L"cxldb"
#define PP_STR_dbFileExtSearchString "*.cxldb"
#define PP_STR_PowerProfileFileExtensionCaption "Power Profile"


// Tree data
#define PP_STR_TreeNodeSummary L"Summary"
#define PP_STR_TreeNodeTimeline L"Timeline"
//++AT:LPGPU2
#define PP_STR_TreeNodeSources L"Sources"
//--AT:LPGPU2
//++CF:LPGPU2
#define PP_STR_TreeNodeFeedback L"Feedback"
//--CF:LPGPU2#
//++TLRS: LPGPU2
#define PP_STR_TreeNodeCompareToSession L"Compare to ..."
//--TLRS: LPGPU2
#define PP_STR_TreeNodeRegionsOfInterest L"Regions of Interest"
#define PP_STR_TreeNodeFrameDetails L"Frame Details"
//--CF:LPGPU2

#define PP_STR_sessionViewCaption L" (Power)"

// Summary view:
#define PP_STR_SummaryDurationLabel "Session Duration: <b>%02d:%02d:%02d.%03d</b>"
#define PP_STR_SummaryCumulativeEnergyCaption "Cumulative Energy Consumption [Joules]"
#define PP_STR_SummaryAveragePowerCaption "Average APU Power Consumption [Watts]"
#define PP_STR_SummaryCPUFrequencyCaption "CPU Avg Frequency [MHz]"
#define PP_STR_SummaryGPUFrequencyCaption "GPU Avg Frequency [MHz]"
#define PP_STR_SummaryCPUFrequencyYAxisLabel "Duration [Sec]"
#define PP_STR_SummaryViewPowerCaptionWithUnits "Power [%1]"
#define PP_STR_SummaryViewEnergyCaptionWithUnits "Energy [%1]"
#define PP_STR_SummaryCPUFrequencyCounterPrefix "CPU"
#define PP_STR_SummaryGPUFrequencyCounterPrefix "GPU"
#define PP_STR_SummaryiGPUFrequencyCounterPrefix "iGPU"
#define PP_STR_SummaryFrequencyCounterPostfix " " PP_STR_FrequencyCounterPostfix
#define PP_STR_SummaryCPUPowerCounterPostfix " Power"
#define PP_STR_SummaryViewPowerGraphTypeAverage "Total Energy Consumption of all supported devices: %1 [%2]"
#define PP_STR_SummaryViewEnergyGraphTypeCumulative "Average Power Consumption of all supported devices: %1 [%2]"

// Timeline view:
#define PP_STR_TimeLineDgpuCounterPart "dGPU"

// Session HTML summary strings:
#define PP_STR_SummaryExecutionHeader L"Execution Information"
#define PP_STR_SummaryTargetPath L"Target Path"
#define PP_STR_SummaryWorkingDirectory L"Working Directory"
#define PP_STR_SummarySessionDirectory L"Profile Session Directory"
#define PP_STR_SummaryCommandLineArgs L"Command Line Arguments"
#define PP_STR_SummaryEnvVars L"Environment Variables"
#define PP_STR_SummaryProfileScope L"Profile Scope"
#define PP_STR_SummaryProfileDuration L"Profile Duration"
#define PP_STR_SummaryProfileScopeSysWide L"System-wide"
#define PP_STR_SummaryProfileDetailsHeader L"Profile Details"
#define PP_STR_SummarySessionType L"Profile Session Type"
#define PP_STR_SummaryProfileStartTime L"Profile Start Time"
#define PP_STR_SummaryProfileEndTime L"Profile End Time"
#define PP_STR_SummaryProfilePower L"Power"
#define PP_STR_SummaryRunningMessage L"Session is currently running"
#define PP_STR_SummaryNewCreatedMessage L"Click play to start profiling"

// Navigation chart strings:

#define PP_STR_NavChartYAxisLabel PP_STR_Counter_Power_TotalAPU

//++TLRS: LPGPU2: Strings for the feedback engine dialog
// Session comparison view
#define PP_STR_CompareToSession_SelectSessionToCompareLabel "Select a session to compare"
#define PP_STR_CompareToSession_CompareButton "Compare"
#define PP_STR_CompareToSession_DiffPlot_XLabel "Time"
#define PP_STR_CompareToSession_DiffPlot_YLabel "CounterValue"
#define PP_STR_CompareToSession_ThisCounterLegend "This %0"
#define PP_STR_CompareToSession_IncomingCounterLegend "Incoming %0"
#define PP_STR_CompareToSession_DiffLegend "Difference"
#define PP_STR_CompareToSession_CounterIdColumn "Counter Id"
#define PP_STR_CompareToSession_CounterNameColumn "Name"
#define PP_STR_CompareToSession_Average1Column "This Average"
#define PP_STR_CompareToSession_Average2Column "Incoming Average"
#define PP_STR_CompareToSession_AvgRatioColumn "Avg. Ratio"
#define PP_STR_CompareToSession_StdDev1Column "This Std. Dev."
#define PP_STR_CompareToSession_StdDev2Column "Incoming Std. Dev."
#define PP_STR_CompareToSession_StdDevRatioColumn "Std. Dev. Ratio"
//--TLRS: LPGPU2: Strings for the feedback engine dialog

//++TLRS: LPGPU2: Define the highlight of items in a table view.
#define PP_STR_FeedbackViews_TableWidgetStyleSheet "QTableView::item:hover { background-color: rgb(188, 236, 254) }"
//--TLRS: LPGPU2: Define the highlight of items in a table view.

//++TLRS: LPGPU2: Strings for the Longest Running Functions Widget
#define PP_STR_LongestRunningFunctionsView_FunctionColumnName "Function"
#define PP_STR_LongestRunningFunctionsView_NumberOfCallsColumnName "# of calls"
#define PP_STR_LongestRunningFunctionsView_PercentageOfTimeColumnName "% of time"
#define PP_STR_LongestRunningFunctionsView_WindowTitle "Longest Running Calls"
//--TLRS: LPGPU2: Strings for the Longest Running Functions Widget

//++TLRS: LPGPU2: Strings for the Calls Per TypeView Widget
#define PP_STR_CallsPerTypeView_WindowTitle "API Calls Per Type"
#define PP_STR_CallsPerTypeView_DrawLabel L"Draw"
#define PP_STR_CallsPerTypeView_DrawToolTip L"Draw Calls"
#define PP_STR_CallsPerTypeView_BindLabel L"Bind"
#define PP_STR_CallsPerTypeView_BindTooltip L"Bind Calls"
#define PP_STR_CallsPerTypeView_SetupLabel L"Setup"
#define PP_STR_CallsPerTypeView_SetupTooltip L"Setup Calls"
#define PP_STR_CallsPerTypeView_WaitLabel L"Wait"
#define PP_STR_CallsPerTypeView_WaitTooltip L"Wait Calls"
#define PP_STR_CallsPerTypeView_ErrorLabel L"Error"
#define PP_STR_CallsPerTypeView_ErrorTooltip L"Error Calls"
#define PP_STR_CallsPerTypeView_DelimitersLabel L"Delimiters"
#define PP_STR_CallsPerTypeView_DelimitersTooltip L"Delimiters Calls"
#define PP_STR_CallsPerTypeView_OtherLabel L"Other"
#define PP_STR_CallsPerTypeView_OtherTooltip L"Other Calls"
//--TLRS: LPGPU2: Strings for the Calls Per TypeView Widget

//++TLRS: LPGPU2: Strings for the Call Suggestions View
#define PP_STR_CallSuggestions_FilterPlaceHolder "Filter"
#define PP_STR_CallSuggestionsTable_FrameNumTitle "Frame"
#define PP_STR_CallSuggestionsView_TableTitle "Suggestion"
//--TLRS: LPGPU2: Strings for the Call Suggestions View

//++TLRS: LPGPU2: Strings for the Feedback Analysis View
#define PP_STR_FeedbackAnalysisView_FeedbackTabTitle "Feedback Analysis"
#define PP_STR_FeedbackAnalysisView_APICallsSummaryGroupBoxTitle "API Calls Summary"
#define PP_STR_FeedbackAnalysisView_SuggestionGroupBoxTitle "Suggestions"
#define PP_STR_FeedbackAnalysisView_ShaderTabTitle "Shader (%0)"
//--TLRS: LPGPU2: Strings for the Feedback Analysis View

//++TLRS: LPGPU2: Strings for the Feedback Engine Dialog
// Feedback engine dialog
#define PP_STR_FeedbackEngineDialog_WindowTitle "Feedback Script Configuration"
#define PP_STR_FeedbackEngineDialog_CurrentSessionLabel "<b>Session</b>: %0"
#define PP_STR_FeedbackEngineDialog_EngineStatusPrefixLabel "<b>Status:</b>"

#define PP_STR_FeedbackEngineDialog_StatusIdle  "Idle"
#define PP_STR_FeedbackEngineDialog_StatusLoadingOptions "Loading feedback options ..."
#define PP_STR_FeedbackEngineDialog_StatusLoadingOptionsError "Cannot load options for this script."
#define PP_STR_FeedbackEngineDialog_StatusRunning "Running ..."
#define PP_STR_FeedbackEngineDialog_StatusRunningError "An error has occurred while running the feedback engine."
#define PP_STR_FeedbackEngineDialog_StatusRunSuccess "Feedback Engine executed successfully and is now idle."
#define PP_STR_FeedbackEngineDialog_StatusFormattedText "<span style='color: %0'><b>%1</b></span>"
#define PP_STR_FeedbackEngineDialog_StatusStyleGreen "green"
#define PP_STR_FeedbackEngineDialog_StatusStyleRed  "red"
#define PP_STR_FeedbackEngineDialog_StatusStyleDefault  "black"

#define PP_STR_FeedbackEngineDialog_Btn_RunLabel "Run"
#define PP_STR_FeedbackEngineDialog_Btn_LoadScriptLabel "Load Script"
#define PP_STR_FeedbackEngineDialog_ScriptComboLabel "Feedback script"
#define PP_STR_FeedbackEngineDialog_CurrentLoadedScriptPrefixLabel "<b>Current loaded script:</b>"
#define PP_STR_FeedbackEngineDialog_ScriptVersionPrefixLabel "<b>Script Version:</b>"
#define PP_STR_FeedbackEngineDialog_ScriptVersionPlaceholder "-"

#define PP_STR_FeedbackEngineDialog_AvailableFeedbackGroupBoxTitle "Available Feedbacks"
#define PP_STR_FeedbackEngineDialog_SelectAllCheckBoxLabel "Select All"
//--TLRS: LPGPU2: Strings for the Feedback Engine Dialog

//++TLRS: LPGPU2: Strings for the Run Feedback Engine Shortcut frame
#define PP_STR_RunFeedbackEngineShortcutFrame_NoFeedbackLabel "There is no feedback available yet."
#define PP_STR_RunFeedbackEngineShortcutFrame_RunButton "Run Feedback Engine ..."
//--TLRS: LPGPU2: Strings for the Run Feedback Engine Shortcut frame

//++TLRS: LPGPU2: Strings for the Shaders in Region View
#define PP_STR_ShadersInRegion_TableTitle "Shaders in Region"
//--TLRS: LPGPU2: Strings for the Shaders in Region View

// Timeline view strings:
#define PP_StrTimelineRibbonButtonIndexPropertyName "RibbonIndex"
#define PP_StrTimelineTimeLabelPrefix "Time:&nbsp;"
#define PP_StrTimelineCurrentGraphName "Current"
#define PP_StrTimelineAPUPowerGraphName "APU Power"
#define PP_StrTimelineScaledTemperatureGraphName "Scaled Temp."
#define PP_StrTimelineVoltageGraphName "Voltage"
#define PP_StrTimelineCPUCoreStateGraphName "CPU Core State (DVFS)"
#define PP_StrTimelineCStateGraphName "C - State Residency"
#define PP_StrTimelineProgressBarWrapperLabel L"Profile duration: "
#define PP_StrTimelineNavigationCounterSelectionLabel "Use <b>%1</b> for session navigation.<br><a href='change_counter'>Click to change</a>"
#define PP_StrTimelineRibbonButtonDownTooltip "Move \"%1\" graph down"
#define PP_StrTimelineRibbonButtonUpTooltip "Move \"%1\" graph up"
#define PP_StrTimelineRibbonButtonStyle "QPushButton { "\
    "border: none;" \
    "background-color: transparent;" \
    "}" \
    "QPushButton:pressed{ " \
    "border: solid gray 1;" \
    "background-color: transparent;" \
    "border: none;" \
    "}" \
    "QPushButton:hover{ " \
    "border: 1px  solid gray;" \
    "background-color: transparent;" \
    "}"

// mgb:ss - add string for counter type
#define PP_StrTimelineCounterGraphName "Counter"

// log strings
#define PP_STR_logMiddleTierNotInit L"Power Profiler middle tier not initialized. Can't start profiling"
#define PP_STR_logStartProfiling L"Power Profiling started"
#define PP_STR_logStopProfiling L"Power Profiling stopped"

// Main menu
#define PP_STR_CountersSelection                   L"S&elect Power Profiling Counters..."
#define PP_STR_CountersSelectionStatusbarString    L"Select which counters will be sampled"
//++TLRS: LPGPU2
#define PP_STR_RunLPGPU2FeedbackEngine_W L"Run LPGPU2 Feedback Engine..."
#define PP_STR_ExportSessionToCSV_W      L"Export Session to CSV..."
#define PP_STR_ExportToCSVDialogTitle    "Select output folder to export session \"%0\""
#define PP_STR_ExportToCSVSuccessTitle   "Export session to CSV Success"
#define PP_STR_ExportToCSVSuccessMsg     "The session was successfully exported to CSV"
#define PP_STR_ExportToCSVErrorTitle     "Export session to CSV Error"
#define PP_STR_ExportToCSVErrorMsg       "An error has occurred while exporting the session to CSV: %0"
//--TLRS: LPGPU2

// Counter selection dialog
#define PP_STR_DialogTitle                      "Counters Selection"
#define PP_STR_DialogDescription                "Add a performance counters to the 'Active Counters' list to view it's usage"
#define PP_STR_DialogGroupBoxTitle              "Performance Counters"
#define PP_STR_AvailableCountersTitle           "Available Counters"
#define PP_STR_ActiveCountersTitle              "Active Counters"
#define PP_STR_MultipleCountersSelectedHeading  "Multiple Counters Selected"
#define PP_STR_MultipleCountersSelectedMsg      "Select a single item to see its description"
#define PP_STR_CounterDescriptionCaption        "<font color=blue>%1</font><br>%2"
#define PP_STR_CounterSelectionSwitchToPowerProfileQuestion "Profile configuration settings were changed.\nDo you want to change the current profile type to 'Power Profile'?"

#define PP_STR_CounterSelectionRemoveErrorPrefix "Failed to remove counter"
#define PP_STR_CounterSelectionRemoveErrorPostfix " cannot be removed"
#define PP_STR_CounterSelectionProcessIdPrefix "Process Id"

// Messages:
#define PP_STR_PowerProfilerNotSupportedOnHW L"CodeXL Power Profiler : this station does not have devices that support power profiling. Local Power Profiling will be disabled.\n"
#define PP_STR_PowerProfilerNotInitializedErrorMessage L"Communication to CodeXL Power Profiling driver (%ls) returned error %d"
#define PP_STR_PowerProfilerNotSupportedErrorMessage L"Power profiler is not supported on current hardware"
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define PP_STR_PowerProfilerNotInitializedPrefix L"If you have recently upgraded or installed CodeXL, please reboot and try again.\n"
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define PP_STR_PowerProfilerNotInitializedPrefix  L"Please install the Power Profiler driver as detailed in the CodeXL User Guide and then re-run profiling.\n"
#endif

#define PP_STR_SessionStopConfirm "Closing this window will stop the running power profile session. Are you sure you want to continue?"
#define PP_STR_OTHER_GRAPH_DESCRIPTION "Power consumed by other APU sub-components, including leaks and other power losses."

#define PP_STR_TimelineContextMenuShowStackedGraph "Show Stacked Graph..."
#define PP_STR_TimelineContextMenuShowNonStackedGraph "Show Non Stacked Graph..."

// Error messages
#define PP_STR_DriverAlreadyInUseMessageLocal L"A power profiling session is already in progress. Please make sure that no other instance of CodeXL graphic client or command line tool is performing a power profiling session and click the \"Start\" button again."
#define PP_STR_DriverAlreadyInUseMessageRemote L"A power profiling session is already in progress on the remote machine. Please make sure that no other client is connected to the remote agent, and try again."

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define PP_STR_DriverVersionMismatchMessage L"The installed Power Profiling driver's version is incompatible with this instance of CodeXL. Please reinstall CodeXL, or run the AMDTPwrProfDriverInstall.run script."
#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define PP_STR_DriverVersionMismatchMessage L"The installed Power Profiling driver's version is incompatible with this instance of CodeXL. Please reinstall CodeXL."
#endif

#define PP_STR_RemoteConnectionErrorMessage L"Unable to connect to CodeXL Remote Agent. Please make sure that:\n1. CodeXL Remote Agent is running on the target machine.\n2. There is network access from the client machine to the remote machine.\n3. CodeXL is not blocked by a firewall on the client machine.\n4. CodeXL Remote Agent is not blocked by a firewall on the remote machine."
#define PP_STR_TargetApplicationNotFoundErrorMessage "The target application: "
#define PP_STR_TargetWorkingDirNotFoundErrorMessage "The target application's working directory: "
#define PP_STR_CouldNotBeLocatedOnRemoteMachineErrorMessage " could not be located on the remote machine."
#define PP_STR_RemoteFailedToLaunchTargetAppErrorMessage "Failed to launch the target application on the remote machine."
#define PP_STR_RemoteFatalCommunicationErrorMessage "The communication with the remote agent was lost.\nPlease make sure that the remote agent is running and that the remote machine is accessible to the client machine via the network."
#define PP_STR_RemoteHandshakeFailureErrorMessage "The remote agent's version does not match the client's version. Session aborted."
#define PP_STRHypervisorNotSupportedErrorMessage  "Failed to launch : Hypervisor machine not supported"
#define PP_STRCountersNotEnabledErrorMessage  "No counters are enabled for collecting profile data"
#define PP_STR_SmuDisabledMsg "SMU is Disabled. Some of the counters will be disabled."
#define PP_STR_iGPUDisabledMsg "iGPU is Disabled. Some of the counters will be disabled."
#define PP_STR_dbMigrateFailureMsg "Unable to migrate the older version of CodeXL profile database."
#define PP_STR_ProjectSettingsPathsInvalid AF_STR_newProjectExeDoesNotExistOrInvalid

// Strings for metrics calculations:
#define PP_STR_LongestCounterName PP_STR_Counter_AvgFreq_Core0

// HTML style definitions:
#define PP_STR_CSS_NavigationChartTimeLabelStyle "QLabel {color: %1; font-size: 10px; font-weight: normal}"
#define PP_STR_CSS_TimelineViewTooltipStyle "border: 1px solid %1; border-radius: 2px; padding: 2px; background-color: %2;"

// Data utils:
#define PP_STR_DebugMessageUnknownGraphType L"Unknown graph type"

//++KB: LPGPU2
//++AT:LPGPU2
#define PP_STR_LPGPU2MainLabel "LPGPU2 Mode"
#define PP_STR_ProfilableLPGPU2AppsCaption "Profilable LPGPU2 Applications and Counters (LPGPU2)"
#define PP_STR_GetAppsFromLPGPU2Device "Enter the path to the LPGPU2-enabled debugger and the IP address of the LPGPU2-enabled device you want to profile.\nIf the connection is successful, a list of profilable applications and available device hardware counters will be returned.\n\n NOTE: Currently, setting the debugger is DISABLED. It will be enabled in future versions."
//++AT:LPGPU2
#define PP_STR_GetAppsButton "Get apps"
//++AT:LPGPU2
#define PP_STR_GetDeviceInfoButton "Get info"
#define PP_STR_GettingDeviceInfoButton "Getting info ..."
#define PP_STR_DebuggerPathLabel "ADB path:"
//--AT:LPGPU2
//++TLRS: LPGPU2:
#define PP_STR_SelectAPKToInstallLabel "Select which APKs to install.\nMore than one can be selected."
#define PP_STR_InstallingAPKLabel "Installing: <b>%0</b>"
#define PP_STR_APKWasInstalled "APK was installed successfully."
#define PP_STR_StartADBButton "Start ADB"
#define PP_STR_InstallAPKButton "Install APK"
#define PP_STR_InstallSelectedButton "Install selected"
#define PP_STR_InstallRAgentButton "Install APKs"
#define PP_STR_InstallingRAgentButton "Installing APKs ..."
#define PP_STR_RestartRAgentButton "Restart RAgent"
#define PP_STR_RestartingRAgentButton "Restarting RAgent ..."
#define PP_STR_WaitingADBStatus "Waiting ADB status ..."
#define PP_STR_WaitingRAgentStatus "Waiting RAgent status ..."
#define PP_STR_LPGPU2ModeAMDControlsDisabled "LPGPU2. AMD-related controls are disabled."
#define PP_STR_NonLPGPU2ModeLPGPU2ControlsDisabled "Non-LPGPU2. LPGPU2-related controls are disabled."
//--TLRS: LPGPU2: Strings for ADB and Remote Agent Status
#define PP_STR_DeviceIpAddressLabel "IP address:"
#define PP_STR_DevicePortNumberLabel "Port:"
#define PP_STR_DeviceNameLabel "Device name:"
#define PP_STR_ProfilableAppsLabel "Select an application from the list"
#define PP_STR_ProfilableListHeader "Profilable apps"
#define PP_STR_AvailableCountersLabel "Enable counters from the list and set the counter sets sampling frequency"
#define PP_STR_ApiOptionsCaption "API Options (LPGPU2)"
#define PP_STR_EnableTimelineTracing "Enable timeline tracing to record API call timestamps.\n\nTo enable widgets, select an app from the list after connecting to a device.\nYou can select which APIs you wish to trace and whether you would like to capture the API call parameters.\nNote that capturing parameters will slow down execution significantly."
#define PP_STR_EnableTimelineCheckBox "Enable timeline"
#define PP_STR_OpenGLCheckBox "OpenGL ES"
#define PP_STR_EGLCheckBox "EGL"
#define PP_STR_OpenCLCheckBox "OpenCL"
#define PP_STR_VulkanCheckBox "Vulkan"
#define PP_STR_ADBPortForwardDefaultHost "localhost"
#define PP_STR_ADBStatusLabel "<b>ADB status:</b>"
#define PP_STR_ADBStatusRunning "<span style='color: green'>Started</span>"
#define PP_STR_ADBStatusStopped "<span style='color: red'>Stopped</span>"
#define PP_STR_ADBRAgentStatusLabel  "<b>RAgent status:</b>"
#define PP_STR_ADBAlwaysRestartRAgentLabel "Always Restart RAgent"
#define PP_STR_ADBRAgentStatusRunning "<span style='color: green'>Started</span>"
#define PP_STR_ADBRAgentStatusStopped "<span style='color: red'>Stopped</span>"
#define PP_STR_ADBRAgentStatusInstalled "<span style='color: green'>Installed</span>"
#define PP_STR_ADBRAgentStatusNotInstalled "<span style='color: red'>Not Installed</span>"
#define PP_STR_ADBPortForwardLabel "ADB Port forward"
#define PP_STR_ADBStatusNotLPGPU2Mode "Non-LPGPU2 mode. Switch to LPGPU2 Device mode to start monitoring the status of ADB."
#define PP_STR_ADBStatusRAgentNotLPGPU2Mode "Non-LPGPU2 mode. Switch to LPGPU2 Device mode to start monitoring the status of the RAgent."
#define PP_STR_ADBErrorStartProfiling "An error has occurred while starting the RAgent. Make sure your device is connected and active."
#define PP_STR_ADBNotFoundError "ADB was not found or provided executable is invalid."
#define PP_STR_ADBVersionLabel "<b>ADB Version:</b>"
#define PP_STR_ADBRevision "<b>ADB Revision:</b>"
#define PP_STR_ADBDeviceLabel "<b>ADB Device:</b>"
//--TLRS: LPGPU2: Strings for ADB and Remote Agent Status
//++AT:LPGPU2
#define PP_STR_NemaGFXCheckBox "NemaGFX"
//--AT:LPGPU2
#define PP_STR_EnableParamCapture "Enable parameter capture"
#define PP_STR_CaptureScreenshotsCheckBox "Capture screenshots"
#define PP_STR_GpuTimingCheckBox "Enable GPU timing"
#define PP_STR_GpuTimingModeLabel "Timing mode:"
#define PP_STR_GpuTimingModeDisjointFrame "DisjointFrame"
#define PP_STR_GpuTimingModePerDraw "PerDraw"
#define PP_STR_CallStacksCheckBox "Enable call stack dumps"
#define PP_STR_KHRCheckBox "Enable KHR Debug (OGL)"
#define PP_STR_ShaderSourceCheckBox "Capture shader sources (when available)"
#define PP_STR_PwrModelsMainLabelLabel "Power Models (LPGPU2)"
#define PP_STR_PwrModelsCheckboxLabel "Enable power estimation"
#define PP_STR_PwrModelsSupportedModelsLabel "Supported power models for the connected device"
#define PP_STR_PwrModelsNamesLabel "Name"
#define PP_STR_PwrModelsDescriptionLabel "Description"
#define PP_STR_PwrModelsNotes "Notes"
//++AT:LPGPU2
#define PP_STR_CollectionModeCaption "Collection Duration Mode (LPGPU2)"
#define PP_STR_CollectionApiSelectionLabel "API to trace selection:"
#define PP_STR_CollectionModeDescription "Select how to start and end the collection and when."
#define PP_STR_CollectionModeManualRadioBtn "Manual"
#define PP_STR_CollectionModeManualRadioBtnCaption "Manually press Start Profile and Stop Profile."
#define PP_STR_CollectionModeExplicitControlRadioBtn "Explicit control"
#define PP_STR_CollectionModeExplicitControlRadioBtnCaption "The application will automatically stop the profiling session when necessary."
#define PP_STR_CollectionModeTimerRadioBtn "Timer"
#define PP_STR_CollectionModeTimerDuration "Duration (in ms):"
#define PP_STR_CollectionRangeCheckBox "Collection range"
#define PP_STR_TargetCharacteristicsXMLProjTagL L"TargetCharacteristics"
#define PP_STR_CollectionsDefinitionXMLProjTagL L"CollectionsDefinition"
#define PP_STR_TargetDefXMLProjTagL L"TargetDefinition"
#define PP_STR_CollectionModelTypeXMLProjTagL L"CollectionTimeMode"
#define PP_STR_SelectedAppXMLProjTagL L"SelectedApp"
#define PP_STR_ProfilableAppsListXMLProjTagL L"ProfilableApplications"
#define PP_STR_TargetCharacteristicsXMLProjTag "TargetCharacteristics"
#define PP_STR_CollectionsDefinitionXMLProjTag "CollectionsDefinition"
#define PP_STR_TargetDefXMLProjTag "TargetDefinition"
#define PP_STR_CollectionModelTypeXMLProjTag "CollectionTimeMode"
#define PP_STR_ProfilableAppsListXMLProjTag "ProfilableApplications"
#define PP_STR_TargetDefFileName L"TargetDefinition"
#define PP_STR_SelectedAppXMLProjTag "SelectedApp"
#define PP_STR_TargetDefFileExt L"xml"
#define PP_STR_CollectionIntervalLabel "Collection Freq.:"
#define PP_STR_LPGPU2RemoteHostNameXMLProjTagL L"LPGPU2RemoteHostName"
#define PP_STR_LPGPU2RemoteHostNameXMLProjTag "LPGPU2RemoteHostName"
#define PP_STR_LPGPU2RemoteHostPortXMLProjTagL L"LPGPU2RemoteHostPort"
#define PP_STR_LPGPU2RemoteHostPortXMLProjTag "LPGPU2RemoteHostPort"
#define PP_STR_LPGPU2SamplingIntervalLabel "Sampling Interval (LPGPU2)"
#define PP_STR_LPGPU2SamplingIntervalDescription "Counters sampling interval:"
#define PP_STR_LPGPU2SamplingIntervalXMLProjTag "LPGPU2SamplingInverval"
#define PP_STR_ADBPortForwardXMLTag L"ADBPortForward"
#define PP_STR_AlwaysRestartRAgentXMLTag L"AlwaysRestartRAgent"
#define PP_STR_ADBPathXMLTag L"ADBPath"
//--AT:LPGPU2
#define PP_STR_StartDrawCheckBox "Start draw:"
#define PP_STR_StartFrameCheckBox "Start frame:"
#define PP_STR_StopDrawCheckBox "Stop draw:"
#define PP_STR_StopFrameCheckBox "Stop frame:"
//++AT:LPGPU2
#define PP_STR_LPGPU2BrowseForDbgrEXE "NewProjectBrowseForDbgrEXE"
#define PP_STR_LPGPU2BrowseForDbgrPathTooltip "Click to browse for the debugger's path"
#define PP_STR_LPGPU2HostMode "Host mode"
#define PP_STR_LPGPU2HostCurrentMode "Current mode:"
#define PP_STR_LPGPU2ModeEnabled "LPGPU2. AMD-related controls are disabled."
#define PP_STR_LPGPU2ModeDisabled "Non-LPGPU2. LPGPU2-related controls are disabled."
#define PP_STR_InfoAfterOk "To activate the LPGPU2 Power Profiling execution mode, navigate to:\n\n1) Profile Mode (the clock icon)\n2) The small down arrow besides the clock icon\n3) Power Profile"
#define PP_STR_InfoAfterOkTitle "How to launch LPGPU2 Power Profiling"
#define PP_STR_TimerDurationModeLengthXMLProjTag "TimerDurationModeLength"
#define PP_STR_TimerDurationModeLengthXMLProjTagL L"TimerDurationModeLength"
#define PP_STR_FormatGraphYaxis "%1\n%2 %3"
//--AT:LPGPU2
#define PP_STR_TargetCharFileName L"TargetCharacteristics"
#define PP_STR_TargetCharFileExt L"xml"
#define PP_STR_CollectionDefFileName L"CollectionDefinition"
#define PP_STR_CollectionDefFileExt L"xml"
#define PP_STR_TargetDefFileName L"TargetDefinition"
#define PP_STR_TargetDefFileExt L"xml"
//--KB: LPGPU2
//++TLRS: LPGPU2
#define PP_STR_SplitterStyleName "Fusion"
#define PP_STR_SplitterStyleSheet "QSplitter::handle:hover {"\
                              " background-color: lightgrey;"\
                              " border-radius: 5px;"\
                              "}"

#define PP_STR_LPGPU2_ShaderMatch_ShaderLabel "Shader:"
#define PP_STR_LPGPU2_ShaderMatch_CounterLabel "Counter:"
#define PP_STR_LPGPU2_ShaderMatch_FeedbackScriptLabel "Feedback Script:"
#define PP_STR_LPGPU2_ShaderMatch_CalculateStatsButton "Calculate stats"
#define PP_STR_LPGPU2_ShaderMatch_FeedbackScriptName L"shader_counter_match"
//--TLRS: LPGPU2

//++CF:LPGPU2
#define PP_STR_LPGPU2InfoNoFrameViewTitle "No frame detail views open."
#define PP_STR_LPGPU2InfoNoFrameViewMsg   "To open a frame, click on an item on the 'Frames' timeline, either from the main timeline view or from a region of interest."

#define PP_STR_LPGPU2InfoNoRegionViewTitle "No regions of interest open."
#define PP_STR_LPGPU2InfoNoRegionViewMsg   "To open a region of interest, first generate them by running the feedback engine (Profile > Run LPGPU2 Feedback Engine).\nThis will populate the 'Feedback' timeline in the timeline view. From there, click on an item to open the region view."

#define PP_STR_LPGPU2InfoNoSourceTitle "No source code views open."
#define PP_STR_LPGPU2InfoNoSourceMsg   "To open the source code of a shader, first run the feedback engine (Profile > Run LPGPU2 Feedback Engine). This\nwill generate Regions of Interest. From a Region of Interest, select a shader from the 'Shaders in Region' table.\nThen press the 'View Source' button."

#define PP_STR_LPGPU2InfoNoAPISuggestionsTitle "No API Call Suggestions."
#define PP_STR_LPGPU2InfoNoAPISuggestionsMsg   "The feedback engine did not generate any suggestions for API Calls in this region."

#define PP_STR_LPGPU2InfoNoShadersTitle "No Shaders in Region."
#define PP_STR_LPGPU2InfoNoShadersMsg   "No shaders were compiled in this Region of Interest."

#define PP_STR_LPGPU2ShaderAtFrameDraw   "Shader at Frame %0 Draw %1"
#define PP_STR_LPGPU2ViewSourceForShader "View source for shader %0:%1"

#define PP_STR_LPGPU2_CounterSummaryTableID "Counter ID"
#define PP_STR_LPGPU2_CounterSummaryTableName "Name"
#define PP_STR_LPGPU2_CounterSummaryTableValue "Value"
#define PP_STR_LPGPU2_CounterSummaryTableDelta "% of Average"
#define PP_STR_LPGPU2_CounterSummaryTableAvg   "Session Average"
#define PP_STR_LPGPU2_CounterSummaryTitle "Approximate Counter Values During This Frame"

#define PP_STR_LPGPU2_TooltipName "Name"
#define PP_STR_LPGPU2_TooltipStartTime "Start time"
#define PP_STR_LPGPU2_TooltipEndTime "End time"
#define PP_STR_LPGPU2_TooltipDuration "Duration"
#define PP_STR_LPGPU2_TooltipFrameId "Frame id"
#define PP_STR_LPGPU2_TooltipDrawId "Draw id"



#define PP_STR_LPGPU2_CSS_GroupBoxBold "QGroupBox { font-weight: bold; }"

//--CF:LPGPU2

#endif //__PPSTRINGCONSTANTS_H
