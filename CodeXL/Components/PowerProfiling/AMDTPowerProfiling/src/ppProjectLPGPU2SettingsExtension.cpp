// TinyXml:
#include <tinyxml.h>

// Import STL file streams, strings and streambuffers
#include <fstream>
#include <string>
#include <streambuf>
//++AT:LPGPU2
// Import:
// std::find_if()
// std::chrono
#include <algorithm>
#include <chrono>
//--AT:LPGPU2

// QT:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTRemoteClient/Include/RemoteClientDataTypes.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// AMDTPowerProfilingMidTier:
#include <AMDTPowerProfilingMidTier/include/LPGPU2ppPowerModels.h>

// Local:
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/src/ppProjectLPGPU2SettingsExtension.h>
#include <AMDTPowerProfiling/src/ppAidFunctions.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFrameTerminatorSelectionWidget.h>
#include <AMDTPowerProfiling/src/LPGPU2ppADBCommands.h>
#include <AMDTPowerProfiling/src/LPGPU2ppAppsListWidget.h>
#include <AMDTPowerProfiling/src/LPGPU2ppInstallAPKDialog.h>

using lpgpu2::PPFnStatus;

// The port number on which to communicate with the remote device
const QString kDefaultDevicePortStr = "21845";
const gtSizeType kMaxTimerMSDuration = 12000000;
const gtSizeType kCountersListHeight = 270;
const gtSizeType kPwrModelsListHeight = 80;
// The API IDs are hard-coded for now, but they will be generalised
const gtSizeType kOGLAPIID = 256;
const gtSizeType kEGLAPIID = 1024;
const gtSizeType kCLAPIID = 4096;

/// @brief      Utility method. Convert from a null-terminated
///             string of chars to a signed integer.
/// @param      charArray The string to convert
/// @return     The converted string.
gtInt32 charToI32(const char *charArray)
{
    if(!charArray)
    {
      return 0;
    }

    gtString strVer;
    strVer.fromASCIIString(charArray);
    GT_ASSERT(strVer.length() > 0);
    gtInt32 numVal;
    GT_ASSERT(strVer.toIntNumber(numVal));
    return numVal;
}

/// @brief      Utility method. Convert from a null-terminated
///             string of chars to an unsigned integer.
/// @param      charArray The string to convert
/// @return     The converted string.
gtUInt32 charToUI32(const char *charArray)
{
    gtString strVer;
    strVer.fromASCIIString(charArray);
    GT_ASSERT(strVer.length() > 0);
    gtUInt32 numVal;
    GT_ASSERT(strVer.toUnsignedIntNumber(numVal));
    return numVal;
}

/// @brief Ctor. No work is done here apart from setting flag
ppProjectLPGPU2SettingsExtension::ppProjectLPGPU2SettingsExtension()
    : afProjectSettingsExtension{}
{
    ppAppController::instance().ClearAfterLoadFlag();
}

/// @brief Dtor. No work is done here apart from setting flag
ppProjectLPGPU2SettingsExtension::~ppProjectLPGPU2SettingsExtension()
{
}

/// @brief Initialize the widget
/// @note We return void to conform with the interface expected by CXL
void ppProjectLPGPU2SettingsExtension::Initialize()
{
    auto *pMainLayout = new QVBoxLayout;
    auto *pMainScrollLayout = new QVBoxLayout;
    auto *pScrollArea = new QScrollArea{ this };
    auto *pDummyWidget = new QWidget;
    pDummyWidget->setStyleSheet(AF_STR_WhiteBG);
    pScrollArea->setWidgetResizable(true);
    pScrollArea->setWidget(pDummyWidget);
    pDummyWidget->setLayout(pMainLayout);
    pMainScrollLayout->addWidget(pScrollArea);
    setLayout(pMainScrollLayout);

    auto *pVLayoutMode = new QVBoxLayout;
    auto *pCaptionMode = new QLabel{ PP_STR_LPGPU2HostMode };
    pCaptionMode->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
    auto *pModePredefinedText =
        new QLabel{ PP_STR_LPGPU2HostCurrentMode };
    m_hostModeLabel = new QLabel{};
    auto *pHLayoutMode = new QHBoxLayout;
    pHLayoutMode->addWidget(pModePredefinedText);
    pHLayoutMode->addWidget(m_hostModeLabel);
    pHLayoutMode->addStretch();
    pVLayoutMode->addWidget(pCaptionMode);
    pVLayoutMode->addLayout(pHLayoutMode);
    pMainLayout->addLayout(pVLayoutMode);

    // Sampling interval and other configuration entries for the CollectionDefs
    auto *pVLayoutConfig = new QVBoxLayout;
    auto *pCaptionConfig =
        new QLabel{ PP_STR_LPGPU2projectSettingsSamplingInterval };
    pCaptionConfig->setStyleSheet(AF_STR_captionLabelStyleSheetSecondaryBold);
    m_pSamplingIntervalSpinBox = new QSpinBox;
    m_pSamplingIntervalSpinBox->setMaximum(PP_MAX_SAMPLING_INTERVAL);
    m_pSamplingIntervalSpinBox->setMinimum(PP_MIN_SAMPLING_INTERVAL);
    m_pSamplingIntervalSpinBox->setSingleStep(PP_DEFAULT_SAMPLING_INTERVAL);
    auto *pIntervalSpinBoxLabel = new QLabel{ PP_STR_projectSettingsSampleEvery };
    pVLayoutConfig->addWidget(pCaptionConfig);
    auto *pHLayoutConfig = new QHBoxLayout;
    pHLayoutConfig->addWidget(pIntervalSpinBoxLabel);
    pHLayoutConfig->addWidget(m_pSamplingIntervalSpinBox);
    pHLayoutConfig->addStretch();
    pVLayoutConfig->addLayout(pHLayoutConfig);
    pMainLayout->addLayout(pVLayoutConfig);

    QLabel* pCaption2 = new QLabel(PP_STR_LPGPU2MainLabel);
    pCaption2->setStyleSheet(AF_STR_captionLabelStyleSheetSecondaryBold);
    pMainLayout->addWidget(pCaption2);

    auto pAdbControlsLayout = new QVBoxLayout;

    auto &adbCommand = LPGPU2ppADBCommands::Instance();

    auto *pAdbVersionTitleLabel = new QLabel{ PP_STR_ADBVersionLabel, this };
    m_pADBVersionLabel = new QLabel{ this };

    auto *pAdbRevisionTitleLabel = new QLabel{ PP_STR_ADBRevision, this };
    m_pADBRevisionLabel = new QLabel{ this };

    auto *pAdbStatusTitleLabel = new QLabel{ PP_STR_ADBStatusLabel, this };
    m_pADBStatusLabel = new QLabel{ this };

    auto *pAdbDeviceTitleLabel = new QLabel{ PP_STR_ADBDeviceLabel, this };
    m_pADBDeviceComboBox = new QComboBox{ this };
    m_pADBDeviceComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    auto *pAdbRemoteAgentStatusLabelTitle = new QLabel{ PP_STR_ADBRAgentStatusLabel, this };
    m_pRAgentStatusLabel = new QLabel{ this };

    m_pAlwaysRestartRAgentCheckBox = new QCheckBox{ PP_STR_ADBAlwaysRestartRAgentLabel, this };
    m_pAlwaysRestartRAgentCheckBox->setChecked(true);

    ResetADBStatusLabels();

    connect(&adbCommand, &LPGPU2ppADBCommands::ADBStatusChanged, this, &ppProjectLPGPU2SettingsExtension::OnADBStatusChanged);
    connect(&adbCommand, &LPGPU2ppADBCommands::RAgentInstalledStatusChanged, this, &ppProjectLPGPU2SettingsExtension::OnRAgentInstalledStatusChanged);
    connect(&adbCommand, &LPGPU2ppADBCommands::RAgentStatusChanged, this, &ppProjectLPGPU2SettingsExtension::OnRAgentStatusChanged);
    connect(&adbCommand, &LPGPU2ppADBCommands::OnPreviousActionsCompleted, this, &ppProjectLPGPU2SettingsExtension::OnADBPreviousActionsCompleted);
    connect(&adbCommand, &LPGPU2ppADBCommands::ADBPathChanged, this, &ppProjectLPGPU2SettingsExtension::ResetADBStatusLabels);
    connect(&adbCommand, &LPGPU2ppADBCommands::ADBDevicesChanged, this, &ppProjectLPGPU2SettingsExtension::ResetADBStatusLabels);

    m_pStartADBButton = new QPushButton{ PP_STR_StartADBButton, this };
    m_pStartADBButton->setEnabled(!adbCommand.IsADBRunning());

    m_pInstallRAgentButton = new QPushButton{ PP_STR_InstallRAgentButton, this };
    m_pInstallRAgentButton->setEnabled(adbCommand.IsADBRunning() && !adbCommand.IsRAgentInstalled());

    m_pRestartRAgentButton = new QPushButton{ PP_STR_RestartRAgentButton, this };
    m_pRestartRAgentButton->setEnabled(adbCommand.IsADBRunning() && adbCommand.IsRAgentInstalled());

    connect(m_pStartADBButton, &QPushButton::clicked, this, &ppProjectLPGPU2SettingsExtension::OnStartADB);
    connect(m_pInstallRAgentButton, &QPushButton::clicked, this, &ppProjectLPGPU2SettingsExtension::OnInstallRAgentAPK);
    connect(m_pRestartRAgentButton, &QPushButton::clicked, this, &ppProjectLPGPU2SettingsExtension::OnRestartRAgent);

    auto *pAdbVersionLayout = new QHBoxLayout;	
    pAdbVersionLayout->addWidget(pAdbVersionTitleLabel);
    pAdbVersionLayout->addWidget(m_pADBVersionLabel);
    pAdbVersionLayout->addStretch();

    auto *pAdbRevisionLayout = new QHBoxLayout;
    pAdbRevisionLayout->addWidget(pAdbRevisionTitleLabel);
    pAdbRevisionLayout->addWidget(m_pADBRevisionLabel);
    pAdbRevisionLayout->addStretch();

    auto *pAdbDeviceLayout = new QHBoxLayout;
    pAdbDeviceLayout->addWidget(pAdbDeviceTitleLabel);
    pAdbDeviceLayout->addWidget(m_pADBDeviceComboBox);
    pAdbDeviceLayout->addStretch();

    auto *pAdbStatusLayout = new QHBoxLayout;
    pAdbStatusLayout->addWidget(pAdbStatusTitleLabel);
    pAdbStatusLayout->addWidget(m_pADBStatusLabel);
    pAdbStatusLayout->addStretch();
    pAdbStatusLayout->addWidget(m_pStartADBButton);

    auto *pAdbRAgentStautsLayout = new QHBoxLayout;
    pAdbRAgentStautsLayout->addWidget(pAdbRemoteAgentStatusLabelTitle);
    pAdbRAgentStautsLayout->addWidget(m_pRAgentStatusLabel);
    pAdbRAgentStautsLayout->addStretch();
    pAdbRAgentStautsLayout->addWidget(m_pInstallRAgentButton);
    pAdbRAgentStautsLayout->addWidget(m_pRestartRAgentButton);

    m_pADBPortForwardCheckBox = new QCheckBox{ PP_STR_ADBPortForwardLabel, this };
    connect(m_pADBPortForwardCheckBox, &QCheckBox::stateChanged, this, &ppProjectLPGPU2SettingsExtension::OnPortForwardCheckboxStateChanged);	
        
    pAdbControlsLayout->addLayout(pAdbVersionLayout);
    pAdbControlsLayout->addLayout(pAdbRevisionLayout);
    pAdbControlsLayout->addLayout(pAdbStatusLayout);
    pAdbControlsLayout->addLayout(pAdbDeviceLayout);
    pAdbControlsLayout->addLayout(pAdbRAgentStautsLayout);
    pAdbControlsLayout->addWidget(m_pAlwaysRestartRAgentCheckBox);
    pAdbControlsLayout->addWidget(m_pADBPortForwardCheckBox);

    pMainLayout->addLayout(pAdbControlsLayout);

    // ADB path selection, IP address specification and device name output
    // Labels column
    QLabel* pSetupLabel = new QLabel(PP_STR_GetAppsFromLPGPU2Device);
    pMainLayout->addWidget(pSetupLabel);

    auto* pSetupLayout = new QHBoxLayout;
    auto* pSetupLabelsLayout = new QVBoxLayout;
    auto* pAdbLabel = new QLabel(PP_STR_DebuggerPathLabel);    
    auto* pIpLabel = new QLabel(PP_STR_DeviceIpAddressLabel);
    auto* pDeviceNameLabel = new QLabel(PP_STR_DeviceNameLabel);
    pSetupLabelsLayout->addWidget(pAdbLabel);
    pSetupLabelsLayout->addWidget(pIpLabel);
    pSetupLabelsLayout->addWidget(pDeviceNameLabel);
    pSetupLayout->addLayout(pSetupLabelsLayout);
    // Input column
    QVBoxLayout* pSetupInputLayout = new QVBoxLayout;
    // ADB path input row
    QHBoxLayout* pAdbInputLayout = new QHBoxLayout;
    m_pAdbTextBox = new QLineEdit;
    // TODO Re-enable once RAgent supports this
    m_pAdbTextBox->setEnabled(false);
    QToolButton* pAdbBrowseButton = new QToolButton;
    pAdbBrowseButton->setContentsMargins(0, 0, 0, 0);
    m_pAdbBrowseAction = new afBrowseAction(PP_STR_LPGPU2BrowseForDbgrEXE);
    m_pAdbBrowseAction->setEnabled(false);
    pAdbBrowseButton->setDefaultAction(m_pAdbBrowseAction);
    pAdbBrowseButton->setToolTip(PP_STR_LPGPU2BrowseForDbgrPathTooltip);
    m_pAdbBrowseAction->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    m_pAdbBrowseAction->setToolTip(PP_STR_LPGPU2BrowseForDbgrPathTooltip);
    pAdbInputLayout->addWidget(m_pAdbTextBox);
    pAdbInputLayout->addWidget(pAdbBrowseButton);
    pSetupInputLayout->addLayout(pAdbInputLayout);
    // IP address & port input row
    QHBoxLayout* pAddressInputLayout = new QHBoxLayout;
    m_pIpTextBox = new QLineEdit;
    QLabel* pPortLabel = new QLabel(PP_STR_DevicePortNumberLabel);
    m_pPortTextBox = new QLineEdit;
    m_pPortTextBox->setEnabled(false);
    m_pPortTextBox->setFixedWidth(50);
    m_pPortTextBox->setText(kDefaultDevicePortStr);
    m_pGetAppsButton = new QPushButton(PP_STR_GetDeviceInfoButton);
    pAddressInputLayout->addWidget(m_pIpTextBox);
    pAddressInputLayout->addWidget(pPortLabel);
    pAddressInputLayout->addWidget(m_pPortTextBox);
    pAddressInputLayout->addWidget(m_pGetAppsButton);
    pSetupInputLayout->addLayout(pAddressInputLayout);
    // Device name output row
    m_pDeviceNameTextBox = new QLineEdit;
    m_pDeviceNameTextBox->setReadOnly(true);
    pSetupInputLayout->addWidget(m_pDeviceNameTextBox);
    // Finalize
    pSetupLayout->addLayout(pSetupInputLayout);
    pMainLayout->addLayout(pSetupLayout);    

    // Profilable applications and counter list
    {
        QLabel* pCaption2 = new QLabel(PP_STR_ProfilableLPGPU2AppsCaption);
        pCaption2->setStyleSheet(AF_STR_captionLabelStyleSheetSecondaryBold);
        pMainLayout->addWidget(pCaption2);
    }

    QHBoxLayout* pAppsAndCountersLayout = new QHBoxLayout;
    // Supported apps list
    QVBoxLayout* pAppsLayout = new QVBoxLayout;
    QLabel* pAppsLabel = new QLabel(PP_STR_ProfilableAppsLabel);
    m_pAppList = new lpgpu2::AppsList;
    m_pAppList->Initialise();
    m_pAppList->setFixedHeight(kCountersListHeight);
    pAppsLayout->addWidget(pAppsLabel);
    pAppsLayout->addWidget(m_pAppList);
    pAppsAndCountersLayout->addLayout(pAppsLayout);
    // Counters list
    QVBoxLayout* pCountersLayout = new QVBoxLayout;
    QLabel* pCountersLabel = new QLabel(PP_STR_AvailableCountersLabel);
    m_pCounterList = new lpgpu2::ppCounterList;
    m_pCounterList->setFixedHeight(kCountersListHeight);
    pCountersLayout->addWidget(pCountersLabel);
    pCountersLayout->addWidget(m_pCounterList);
    pAppsAndCountersLayout->addLayout(pCountersLayout);
    pMainLayout->addLayout(pAppsAndCountersLayout);


    auto *pLPGPU2CaptionConfig = new QLabel{ PP_STR_LPGPU2SamplingIntervalLabel };
    pLPGPU2CaptionConfig->setStyleSheet(AF_STR_captionLabelStyleSheetSecondaryBold);

    QLabel* pCaption3 = new QLabel(PP_STR_ApiOptionsCaption);
    pCaption3->setStyleSheet(AF_STR_captionLabelStyleSheetSecondaryBold);
    pMainLayout->addWidget(pCaption3);

    // Enable timeline checkbox
    QLabel* pApiSelectionDescriptionLabel =
        new QLabel(PP_STR_EnableTimelineTracing);
    pMainLayout->addWidget(pApiSelectionDescriptionLabel);
    m_pTimelineCheckBox = new QCheckBox(PP_STR_EnableTimelineCheckBox);
    m_pTimelineCheckBox->setEnabled(false);
    pMainLayout->addWidget(m_pTimelineCheckBox);

    auto *pApiOptionsLayout = new QHBoxLayout;
    auto *pApiOptionsVLayout = new QVBoxLayout;

    // API selection checkboxes
    pApiOptionsVLayout->addStretch();
    QHBoxLayout* pApiSelectionLayout = new QHBoxLayout;
    m_pApiSelectionLabel = new QLabel(PP_STR_CollectionApiSelectionLabel);
    m_pApiSelectionLabel->setEnabled(false);
    m_pGLCheckBox = new QCheckBox(PP_STR_OpenGLCheckBox);
    m_pGLCheckBox->setEnabled(false);
    m_pEGLCheckBox = new QCheckBox(PP_STR_EGLCheckBox);
    m_pEGLCheckBox->setEnabled(false);
    m_pCLCheckBox = new QCheckBox(PP_STR_OpenCLCheckBox);
    m_pCLCheckBox->setEnabled(false);
    m_pVKCheckBox = new QCheckBox(PP_STR_VulkanCheckBox);
    m_pVKCheckBox->setEnabled(false);
    // TODO restore creation of NemaGFX checkbox once it is clear that the
    // checkbox is needed
    pApiSelectionLayout->addWidget(m_pGLCheckBox);
    pApiSelectionLayout->addWidget(m_pEGLCheckBox);
    pApiSelectionLayout->addWidget(m_pCLCheckBox);
    pApiSelectionLayout->addWidget(m_pVKCheckBox);
    pApiOptionsVLayout->addWidget(m_pApiSelectionLabel);
    pApiOptionsVLayout->addLayout(pApiSelectionLayout);
    // Parameter capture checkbox
    m_pParamCaptureCheckBox = new QCheckBox(PP_STR_EnableParamCapture);
    pApiOptionsVLayout->addWidget(m_pParamCaptureCheckBox);
    m_pCaptureScreenshotsCheckBox =
        new QCheckBox(PP_STR_CaptureScreenshotsCheckBox);
    m_pCaptureScreenshotsCheckBox->setEnabled(false);
    pApiOptionsVLayout->addWidget(m_pCaptureScreenshotsCheckBox);
    pApiOptionsLayout->addLayout(pApiOptionsVLayout);

    // Additional API options checkboxes
    auto *pApiOptionsVLayout2 = new QVBoxLayout;
    pApiOptionsVLayout2->addStretch();
    m_pGpuTimingCheckBox = new QCheckBox(PP_STR_GpuTimingCheckBox);
    QHBoxLayout* pGpuTimingModeLayout = new QHBoxLayout;
    m_pCallStacksCheckBox = new QCheckBox(PP_STR_CallStacksCheckBox);
    m_pKHRDebugCheckBox = new QCheckBox(PP_STR_KHRCheckBox);
    m_pShaderSrcCheckBox = new QCheckBox(PP_STR_ShaderSourceCheckBox);
    pApiOptionsVLayout2->addWidget(m_pGpuTimingCheckBox);
    pGpuTimingModeLayout->addStretch();
    pApiOptionsVLayout2->addLayout(pGpuTimingModeLayout);
    pApiOptionsVLayout2->addWidget(m_pCallStacksCheckBox);
    pApiOptionsVLayout2->addWidget(m_pKHRDebugCheckBox);
    pApiOptionsVLayout2->addWidget(m_pShaderSrcCheckBox);
    pApiOptionsLayout->addLayout(pApiOptionsVLayout2);

    // Final row of API options
    auto *pApiOptionsVLayout3 = new QVBoxLayout;
    m_termWidget = new lpgpu2::FrameTerminatorSelWidget{};
    m_termWidget->Initialise();
    pApiOptionsVLayout3->addStretch();
    pApiOptionsVLayout3->addWidget(m_termWidget);
    pApiOptionsLayout->addLayout(pApiOptionsVLayout3);
    pApiOptionsLayout->addStretch();
    pMainLayout->addLayout(pApiOptionsLayout);

    QLabel* pCaption5 = new QLabel{ PP_STR_CollectionModeCaption };
    pCaption5->setStyleSheet(AF_STR_captionLabelStyleSheetSecondaryBold);
    pMainLayout->addWidget(pCaption5);

    // Collection range description
    QLabel* pCollectionModeLabel = new QLabel{ PP_STR_CollectionModeDescription };
    pMainLayout->addWidget(pCollectionModeLabel);

    // Main vertical layout for the rest of the widgets in the Collection
    // mode section
    QVBoxLayout* pCollectionModeLayout = new QVBoxLayout;

    // Manual collection mode
    m_pManualRadioBtn = new QRadioButton{ PP_STR_CollectionModeManualRadioBtn };
    m_pManualRadioBtn->setChecked(true);
    QString manualRadioBtnTooltip = PP_STR_CollectionModeManualRadioBtnCaption;
    m_pManualRadioBtn->setToolTip(manualRadioBtnTooltip);
    pCollectionModeLayout->addWidget(m_pManualRadioBtn);

    // Timer collection mode
    m_pTimerRadioBtn = new QRadioButton{ PP_STR_CollectionModeTimerRadioBtn };
    m_pTimerRadioBtn->setChecked(false);
    pCollectionModeLayout->addWidget(m_pTimerRadioBtn);
    QHBoxLayout* pCollectionTimerSpinnersLayout = new QHBoxLayout;
    pCollectionTimerSpinnersLayout->setContentsMargins(20, 0, 0, 0);
    m_pTimerDurationLabel = new QLabel(PP_STR_CollectionModeTimerDuration);
    m_pTimerDurationLabel->setEnabled(false);
    m_pTimerDurationSpinBox = new QSpinBox;
    m_pTimerDurationSpinBox->setEnabled(false);
    m_pTimerDurationSpinBox->setRange(0, kMaxTimerMSDuration);
    pCollectionTimerSpinnersLayout->addWidget(m_pTimerDurationLabel);
    pCollectionTimerSpinnersLayout->addWidget(m_pTimerDurationSpinBox);
    pCollectionTimerSpinnersLayout->addStretch();
    pCollectionModeLayout->addLayout(pCollectionTimerSpinnersLayout);

    // Duration collection mode
    m_pDurationRadioBtn =
        new QRadioButton{ PP_STR_CollectionRangeCheckBox };
    m_pDurationRadioBtn->setEnabled(false);
    m_pDurationRadioBtn->setChecked(false);
    pCollectionModeLayout->addWidget(m_pDurationRadioBtn);
    QHBoxLayout* pCollectionSpinnersLayout = new QHBoxLayout;
    pCollectionSpinnersLayout->setContentsMargins(20, 0, 0, 0);
    m_pStartDrawLabel = new QLabel(PP_STR_StartDrawCheckBox);
    m_pStartDrawLabel->setEnabled(false);
    m_pStartDrawSpinBox = new QSpinBox;
    m_pStartDrawSpinBox->setEnabled(false);
    m_pStartFrameLabel = new QLabel(PP_STR_StartFrameCheckBox);
    m_pStartFrameLabel->setEnabled(false);
    m_pStartFrameSpinBox = new QSpinBox;
    m_pStartFrameSpinBox->setEnabled(false);
    m_pStopDrawLabel = new QLabel(PP_STR_StopDrawCheckBox);
    m_pStopDrawLabel->setEnabled(false);
    m_pStopDrawSpinBox = new QSpinBox;
    m_pStopDrawSpinBox->setEnabled(false);
    m_pStopFrameLabel = new QLabel(PP_STR_StopFrameCheckBox);
    m_pStopFrameLabel->setEnabled(false);
    m_pStopFrameSpinBox = new QSpinBox;
    m_pStopFrameSpinBox->setEnabled(false);
    pCollectionSpinnersLayout->addWidget(m_pStartDrawLabel);
    pCollectionSpinnersLayout->addWidget(m_pStartDrawSpinBox);
    pCollectionSpinnersLayout->addWidget(m_pStartFrameLabel);
    pCollectionSpinnersLayout->addWidget(m_pStartFrameSpinBox);
    pCollectionSpinnersLayout->addWidget(m_pStopDrawLabel);
    pCollectionSpinnersLayout->addWidget(m_pStopDrawSpinBox);
    pCollectionSpinnersLayout->addWidget(m_pStopFrameLabel);
    pCollectionSpinnersLayout->addWidget(m_pStopFrameSpinBox);
    pCollectionSpinnersLayout->addStretch();
    pCollectionModeLayout->addLayout(pCollectionSpinnersLayout);

    // Explicit application control duration
    m_pExplicitControlRadioBtn =
      new QRadioButton{ PP_STR_CollectionModeExplicitControlRadioBtn};
    m_pExplicitControlRadioBtn->setChecked(false);
    m_pExplicitControlRadioBtn->setEnabled(false);
    m_pExplicitControlRadioBtn->setToolTip(
        QString{PP_STR_CollectionModeExplicitControlRadioBtnCaption});
    pCollectionModeLayout->addWidget(m_pExplicitControlRadioBtn);

    pMainLayout->addLayout(pCollectionModeLayout);

    auto *pCaption1 = new QLabel(PP_STR_projectSettingsSamplingInterval);
    pCaption1->setStyleSheet(AF_STR_captionLabelStyleSheetSecondaryBold);

    // Power models lists
    QLabel *pPwrModelsMainLabel = new QLabel{ PP_STR_PwrModelsMainLabelLabel };
    pPwrModelsMainLabel->setStyleSheet(AF_STR_captionLabelStyleSheetSecondaryBold);
    pMainLayout->addWidget(pPwrModelsMainLabel);
    {
        auto *pPwrModelsCheckBox = new QCheckBox{ PP_STR_PwrModelsCheckboxLabel };
        pPwrModelsCheckBox->setEnabled(false);
        pMainLayout->addWidget(pPwrModelsCheckBox);
    }
    auto pSupportedPwrModelsLabel =
        new QLabel{ PP_STR_PwrModelsSupportedModelsLabel };
    // TODO Rm this temporary and the disabilitation once Nadjib has provided
    // more details
    pSupportedPwrModelsLabel->setEnabled(false);
    pMainLayout->addWidget(pSupportedPwrModelsLabel);
    QHBoxLayout* pPwrModelsHLayout = new QHBoxLayout;

    {
        QVBoxLayout* pPwrModelsNameVLayout = new QVBoxLayout;
        QLabel *pMainLabel = new QLabel{ PP_STR_PwrModelsNamesLabel };
        pMainLabel->setStyleSheet(AF_STR_captionLabelStyleSheetSecondary);
        pPwrModelsNameVLayout->addWidget(pMainLabel);
        m_pPwrModelsNamesList = new QListWidget;
        // TODO Rm this temporary and the disabilitation once Nadjib has provided
        // more details
        m_pPwrModelsNamesList->setEnabled(false);
        m_pPwrModelsNamesList->setFixedHeight(kPwrModelsListHeight);
        pPwrModelsNameVLayout->addWidget(m_pPwrModelsNamesList);
        pPwrModelsHLayout->addLayout(pPwrModelsNameVLayout);
    }
    {
        QVBoxLayout* pPwrModelsNameVLayout = new QVBoxLayout;
        QLabel *pMainLabel = new QLabel{ PP_STR_PwrModelsDescriptionLabel };
        pMainLabel->setStyleSheet(AF_STR_captionLabelStyleSheetSecondary);
        pPwrModelsNameVLayout->addWidget(pMainLabel);
        m_pPwrModelsDescList = new QListWidget;
        // TODO Rm this temporary and the disabilitation once Nadjib has provided
        // more details
        m_pPwrModelsDescList->setEnabled(false);
        m_pPwrModelsDescList->setFixedHeight(kPwrModelsListHeight);
        pPwrModelsNameVLayout->addWidget(m_pPwrModelsDescList);
        pPwrModelsHLayout->addLayout(pPwrModelsNameVLayout);
    }
    pMainLayout->addLayout(pPwrModelsHLayout);

    // Connections    
    // TODO Re-enable once RAgent supports this
    //connect(m_pDurationRadioBtn, &QRadioButton::toggled, this, &ppProjectLPGPU2SettingsExtension::SetCollectionModeToggled);
    connect(m_pTimelineCheckBox, &QCheckBox::stateChanged, this, &ppProjectLPGPU2SettingsExtension::SetApiSelectionEnabled);
    connect(m_pGetAppsButton, &QPushButton::clicked, this, &ppProjectLPGPU2SettingsExtension::OnGetAppsClicked);
    connect(m_pAppList, &QListWidget::itemClicked, this, &ppProjectLPGPU2SettingsExtension::OnAppsListItemActivated);
    connect(m_pAdbBrowseAction, &afBrowseAction::triggered, this, &ppProjectLPGPU2SettingsExtension::OnADBPathBrowse);
    connect(m_pAdbTextBox, &QLineEdit::textChanged, this, &ppProjectLPGPU2SettingsExtension::OnADBPathTextChanged);
    connect(m_pADBDeviceComboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, &ppProjectLPGPU2SettingsExtension::OnADBDeviceSelectionChanged);
    
    // Disable all checkboxes/options which can be disabled
    SetApiSelectionEnabled(Qt::Unchecked);
    m_pManualRadioBtn->toggled(true);
    pMainLayout->addStretch();

    m_pADBPortForwardCheckBox->setChecked(true);
    //--KB:LPGPU2
}

/// @brief Return the extension string to be used in the project settings for
///        this extension
/// @return The extension string
/// @author Gilad Yarnitzky, AMD
gtString ppProjectLPGPU2SettingsExtension::ExtensionXMLString()
{
    gtString retVal = PP_STR_projectSettingExtensionName;
    return retVal;
}

/// @brief Return the display name for the extension
/// @return The display name for this extension
/// @author Gilad Yarnitzky, AMD
gtString ppProjectLPGPU2SettingsExtension::ExtensionTreePathAsString()
{
    gtString retVal = PP_STR_projectSettingExtensionDisplayName;
    return retVal;
}

/// @brief Dump an XML file into the ProjectSettings XML
/// @param fileName The name of the XML file to dump
/// @param fileExt The extension of the XML file to dump
/// @param xmlTag The XML tag to put as guard to the file in the Project XML
/// @param projectAsXMLString The object representing the Project XML settings
/// @return Always PPFnStatus::success
PPFnStatus ppProjectLPGPU2SettingsExtension::DumpXMLIntoProjectSettings(
    const gtString &fileName, const gtString &fileExt,
    const gtString &xmlTag, gtString& projectAsXMLString)
{
    // Target Characteristics
    osFilePath filePath;
    afGetUserDataFolderPath(filePath);
    filePath.setFileName(fileName);
    filePath.setFileExtension(fileExt);

    std::wifstream wifs{ filePath.asString().asASCIICharArray() };
    std::wstringstream wbuffer;
    wbuffer << wifs.rdbuf();
    gtString fileAsString;
    fileAsString.assign(wbuffer.str().c_str());

    // Insert start marker
    gtString startMarker = xmlTag;
    startMarker.prepend(L"<");
    startMarker.append(L">");
    projectAsXMLString.append(startMarker);

    // Insert the XML document
    projectAsXMLString.append(fileAsString);

    // Insert the end marker
    gtString endMarker = xmlTag;
    endMarker.prepend(L"</");
    endMarker.append(L">");
    projectAsXMLString.append(endMarker);

    return PPFnStatus::success;
}

/// @brief Return the current project settings as an XML so that they can be
///        saved in the project's XML file
/// @param projectAsXMLString The project settings as an XML string
/// @return True if success, false if failure
bool ppProjectLPGPU2SettingsExtension::GetXMLSettingsString(
    gtString& projectAsXMLString)
{
    bool retVal = false;
    auto &ppAppCtrl = ppAppController::instance();

    projectAsXMLString.appendFormattedString(
        L"<%ls>", ExtensionXMLString().asCharArray());

    gtVector<int> enabledCounters;
    ppAppCtrl.GetCurrentProjectEnabledCounters(enabledCounters);
    gtString enabledCountersStr;

    for (int counter : enabledCounters)
    {
        enabledCountersStr.appendUnsignedIntNumber(counter);
        enabledCountersStr.append(L",");
    }

    afUtils::addFieldToXML(projectAsXMLString,
                           PP_STR_projectSettingEnabledCounters, enabledCountersStr);

    unsigned int samplingInterval = ppAppCtrl.GetCurrentProjectSamplingInterval();
    gtString samplingIntervalStr;
    samplingIntervalStr.appendUnsignedIntNumber(samplingInterval);
    afUtils::addFieldToXML(projectAsXMLString,
                           PP_STR_projectSettingSamplingInterval, samplingIntervalStr);

    //++AT:LPGPU2
    afUtils::addFieldToXML(projectAsXMLString,
                           PP_STR_LPGPU2RemoteHostNameXMLProjTagL,
                           ppAppCtrl.GetRemoteTargetHostname());
    gtString portNumber;
    portNumber.appendUnsignedIntNumber(ppAppCtrl.GetRemoteTargetDaemonPort());
    afUtils::addFieldToXML(projectAsXMLString,
                           PP_STR_LPGPU2RemoteHostPortXMLProjTagL,
                           portNumber);

    // Target Characteristics
    auto rc = DumpXMLIntoProjectSettings(PP_STR_TargetCharFileName,
                                         PP_STR_TargetCharFileExt, PP_STR_TargetCharacteristicsXMLProjTagL,
                                         projectAsXMLString);
    if (rc != PPFnStatus::success)
    {
        return false;
    }

    // CollectionDefinition
    rc = DumpXMLIntoProjectSettings(PP_STR_CollectionDefFileName,
                                    PP_STR_CollectionDefFileExt, PP_STR_CollectionsDefinitionXMLProjTagL,
                                    projectAsXMLString);
    if (rc != PPFnStatus::success)
    {
        return false;
    }

    // Target Definition
    rc = DumpXMLIntoProjectSettings(PP_STR_TargetDefFileName,
                                    PP_STR_TargetDefFileExt, PP_STR_TargetDefXMLProjTagL,
                                    projectAsXMLString);
    if (rc != PPFnStatus::success)
    {
        return false;
    }

    {
        // Set the timing mode
        gtString collectionTimeMode;
        if (m_pTimerRadioBtn->isChecked())
        {
            collectionTimeMode.appendUnsignedIntNumber(
                static_cast<unsigned int>(lpgpu2::DurationMode::Timer));
            gtString durationValue;
            durationValue.appendUnsignedIntNumber(m_pTimerDurationSpinBox->value());
            afUtils::addFieldToXML(projectAsXMLString,
                                   PP_STR_TimerDurationModeLengthXMLProjTagL,
                                   durationValue);
        }
        else if (m_pDurationRadioBtn->isChecked())
        {
            collectionTimeMode.appendUnsignedIntNumber(
                static_cast<unsigned int>(lpgpu2::DurationMode::DeviceDuration));
        }
        else if (m_pManualRadioBtn->isChecked())
        {
            collectionTimeMode.appendUnsignedIntNumber(
                static_cast<unsigned int>(lpgpu2::DurationMode::Manual));
        }
        else if (m_pExplicitControlRadioBtn->isChecked())
        {
            collectionTimeMode.appendUnsignedIntNumber(
                static_cast<unsigned int>(lpgpu2::DurationMode::ExplicitControl));
        }
        afUtils::addFieldToXML(projectAsXMLString,
                               PP_STR_CollectionModelTypeXMLProjTagL, collectionTimeMode);
    }

    // Set the profilable applications
    auto appsListCount = m_pAppList->count();
    gtString appsList;
    for (decltype(appsListCount) i = 0; i < appsListCount; ++i)
    {
        auto *item = m_pAppList->item(i);
        auto appName = acQStringToGTString(item->text());
        appsList.append(appName);
        appsList.append(L":");
    }
    afUtils::addFieldToXML(projectAsXMLString,
                           PP_STR_ProfilableAppsListXMLProjTagL, appsList);

    // Set the selected app, if any, by storing an integer
    auto selectedItemsList = m_pAppList->selectedItems();
    gtString selectedRowAsString;
    if (selectedItemsList.count() > 0)
    {
        auto rowId = m_pAppList->row(selectedItemsList[0]);
        selectedRowAsString.appendUnsignedIntNumber(
            static_cast<gtUInt32>(rowId));
    }
    afUtils::addFieldToXML(projectAsXMLString,
                           PP_STR_SelectedAppXMLProjTagL, selectedRowAsString);
    //--AT:LPGPU2
      
    ppAppController::instance().SetAlwaysRestartRAgent(m_pAlwaysRestartRAgentCheckBox->isChecked());

    afUtils::addFieldToXML(projectAsXMLString, PP_STR_AlwaysRestartRAgentXMLTag, m_pAlwaysRestartRAgentCheckBox->isChecked());
    afUtils::addFieldToXML(projectAsXMLString, PP_STR_ADBPortForwardXMLTag, m_pADBPortForwardCheckBox->isChecked());
    afUtils::addFieldToXML(projectAsXMLString, PP_STR_ADBPathXMLTag, acQStringToGTString(m_pAdbTextBox->text()));

    projectAsXMLString.appendFormattedString(L"</%ls>", ExtensionXMLString().asCharArray());

    retVal = true;

    return retVal;
}

/// @brief Set the settings for this extension by reading them from the project
///        settings
/// @param projectAsXMLString The project settings as an XML string
/// @return True if success, false if failure
bool ppProjectLPGPU2SettingsExtension::SetSettingsFromXMLString(
    const gtString& projectAsXMLString)
{
    bool retVal = false;
    auto &ppAppCtrl = ppAppController::instance();

    gtString enabledCountersStr, samplingIntervalStr;

    auto pPPNode = std::unique_ptr<TiXmlElement>(
        new TiXmlElement{ ExtensionXMLString().asASCIICharArray() });

    QString projectAsQtXML = acGTStringToQString(projectAsXMLString);
    QByteArray projectAsQtXMLAsUTF8 = projectAsQtXML.toUtf8();

    pPPNode->Parse(projectAsQtXMLAsUTF8.data(), 0, TIXML_DEFAULT_ENCODING);
    gtString ppNodeTitle;
    ppNodeTitle.fromASCIIString(pPPNode->Value());

    if (ExtensionXMLString() == ppNodeTitle.asCharArray())
    {
        afUtils::getFieldFromXML(*pPPNode, PP_STR_projectSettingEnabledCounters,
                                 enabledCountersStr);
        afUtils::getFieldFromXML(*pPPNode, PP_STR_projectSettingSamplingInterval,
                                 samplingIntervalStr);
    }

    gtVector<int> enabledCounters;

    int startPos = 0;
    int endPos = enabledCountersStr.findFirstOf(L",", startPos);

    // assume that enabledCountersStr is of the following: number followed by
    // delimiter and so on
    while (endPos > startPos)
    {
        gtString subStr;
        enabledCountersStr.getSubString(startPos, endPos - 1, subStr);

        if (subStr.isIntegerNumber())
        {
            int counter;
            subStr.toIntNumber(counter);
            enabledCounters.push_back(counter);
        }

        startPos = endPos + 1;
        endPos = enabledCountersStr.findFirstOf(L",", startPos);
    }

    if (enabledCounters.size())
    {
        // update enabled counters only if we have setting from previous session
        ppAppCtrl.SetCurrentProjectEnabledCounters(enabledCounters);
    }

    retVal = true;
    // TODO Investigate more this flag; from a quick glance it seems unused
    ppAppCtrl.SetAfterLoadFlag();

    if (!samplingIntervalStr.isEmpty())
    {
        unsigned int samplingInteral = PP_DEFAULT_SAMPLING_INTERVAL;
        retVal = samplingIntervalStr.toUnsignedIntNumber(samplingInteral);
        GT_IF_WITH_ASSERT(retVal)
        {
            ppAppCtrl.SetCurrentProjectSamplingInterval(samplingInteral);
        }
    }

    // Get hostname and port
    gtString hostName;
    afUtils::getFieldFromXML(*pPPNode, PP_STR_LPGPU2RemoteHostNameXMLProjTagL,
                             hostName);
    ppAppCtrl.SetRemoteTargetHostname(hostName);
    int portNumber = 0;
    afUtils::getFieldFromXML(*pPPNode, PP_STR_LPGPU2RemoteHostPortXMLProjTagL,
                             portNumber);
    ppAppCtrl.SetRemoteTargetDaemonPort(static_cast<gtUInt16>(portNumber));

    // Parse the TargetCharacteristics from the project's XML
    auto *pTargetCharsNode = pPPNode->FirstChildElement(
        PP_STR_TargetCharacteristicsXMLProjTag);
    if (pTargetCharsNode != nullptr)
    {
        auto *pTargetNode = pTargetCharsNode->FirstChildElement("Target");
        if (pTargetNode)
        {
            m_targetCharacteristics = TargetCharacteristics{ *pTargetNode };

            // Save the target defs XML file in the user data folder
            osFilePath filePath;
            afGetUserDataFolderPath(filePath);
            filePath.setFileName(PP_STR_TargetCharFileName);
            filePath.setFileExtension(PP_STR_TargetCharFileExt);
            gtString filePathStr = filePath.asString();

            m_targetCharacteristics.writeToFile(filePathStr);

            // Pass the target characteristics to the project manager
            ppAppCtrl.SetCurrentProjectAndroidDeviceTargetCharacteristics(
                m_targetCharacteristics);
        }
    }

    // Parse the CollectionDefinitions from the project's XML
    auto *pCollectionDefinitionsNode =
        pPPNode->FirstChildElement(PP_STR_CollectionsDefinitionXMLProjTag);
    if (pCollectionDefinitionsNode != nullptr)
    {
        auto *pCollectionNode =
            pCollectionDefinitionsNode->FirstChildElement("Collection");
        if (pCollectionNode)
        {
            auto rc = GenerateCollectionDefinitionsFromXMLElem(pCollectionNode);
            if (rc != PPFnStatus::success)
            {
                return retVal;
            }

            ppAppCtrl.SetCurrentProjectAndroidCollectionOptions(
                m_collectionDefs);

        }
    }

    // Parse the collection definition mode and set the project details locally
    int collModeTimerInt;
    afUtils::getFieldFromXML(*pPPNode, PP_STR_CollectionModelTypeXMLProjTagL,
                             collModeTimerInt);
    auto collTimerMode = static_cast<lpgpu2::DurationMode>(collModeTimerInt);
    if (collTimerMode == lpgpu2::DurationMode::Timer)
    {
        int duration;
        afUtils::getFieldFromXML(*pPPNode,
                                 PP_STR_TimerDurationModeLengthXMLProjTagL,
                                 duration);
        ppAppCtrl.SetDurationModeTimerDuration(duration);
    }
    ppAppCtrl.SetDurationMode(collTimerMode);

    // Parse the profilable applications
    afUtils::getFieldFromXML(*pPPNode, PP_STR_ProfilableAppsListXMLProjTagL,
                             m_appListStr);
    // Parse the selected app, if any
    afUtils::getFieldFromXML(*pPPNode, PP_STR_SelectedAppXMLProjTagL,
                             m_selectedAppRowAsStr);

    // Parse the TargetDefinition from the project's XML
    auto *pTargetDefsNode =
        pPPNode->FirstChildElement(PP_STR_TargetDefXMLProjTag);
    if (pTargetCharsNode)
    {
        auto *pTargetNode = pTargetDefsNode->FirstChildElement("Target");
        if (pTargetNode)
        {
            m_targetDefinition = lpgpu2::TargetDefinition{};
            auto rc = m_targetDefinition.init(*pTargetNode);

            GT_ASSERT(rc == lpgpu2::PPFnStatus::success);
            if (rc != lpgpu2::PPFnStatus::success)
            {
                return false;
            }

            // Save the target defs XML file in the user data folder
            osFilePath filePath;
            afGetUserDataFolderPath(filePath);
            filePath.setFileName(PP_STR_TargetDefFileName);
            filePath.setFileExtension(PP_STR_TargetDefFileExt);
            gtString filePathStr = filePath.asString();

            m_targetDefinition.writeToFile(filePathStr);

            // Pass the target characteristics to the project manager
            ppAppCtrl.SetCurrentProjectAndroidTargetDefinition(
                m_targetDefinition);

            // Parse the LPGPU2 sampling interval from the project's XML
            ppAppCtrl.SetLPGPU2SamplingInterval(
                static_cast<double>(m_collectionDefs.GetIntervalHz()));
        }
    }

    auto bAlwaysRestartRemoteAgent = true;
    afUtils::getFieldFromXML(*pPPNode, PP_STR_AlwaysRestartRAgentXMLTag, bAlwaysRestartRemoteAgent);
    m_pAlwaysRestartRAgentCheckBox->setChecked(bAlwaysRestartRemoteAgent);
    ppAppController::instance().SetAlwaysRestartRAgent(bAlwaysRestartRemoteAgent);

    auto bADBPortForward = true;
    afUtils::getFieldFromXML(*pPPNode, PP_STR_ADBPortForwardXMLTag, bADBPortForward);
    m_pADBPortForwardCheckBox->setChecked(bADBPortForward);

    afUtils::getFieldFromXML(*pPPNode, PP_STR_ADBPathXMLTag, m_adbPath);
    m_pAdbTextBox->setText(acGTStringToQString(m_adbPath));

    // Load settings to the controls:
    retVal = RestoreCurrentSettings() && retVal;

    return retVal;
}

/// @brief Save the current settings for this extension for the current project
///        into the appropriate classes
/// @return True if success, false if failure
bool ppProjectLPGPU2SettingsExtension::SaveCurrentSettings()
{
    bool retVal = true;
    auto &ppAppCtrl = ppAppController::instance();

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSamplingIntervalSpinBox != nullptr)
    {
        // Set the user sampling interval:
        unsigned int userSamplingInterval =
            m_pSamplingIntervalSpinBox->text().toUInt(&retVal);
        GT_IF_WITH_ASSERT(retVal)
        {
            ppAppCtrl.SetCurrentProjectSamplingInterval(userSamplingInterval);
        }
    }

    if (afProjectManager::instance().currentProjectSettings().isAndroidDeviceTarget())
    {
        // Generate and send the collection definitions file
        osFilePath defFilePath;
        GenerateCollectionDefinitions(defFilePath);
        /*GT_ASSERT_EX(isOk, L"Failed to generate collection definitions file.");
        if (!isOk)
        {
            auto p_msgBox = acMessageBox::instance();
            p_msgBox.critical("Project Setup", "Failed to generate the collection \
definitions file.");

            return false;
        }*/

        // Set android-related settings
        ppAppCtrl.SetRemoteTargetHostname(GetDeviceAddress());
        ppAppCtrl.SetRemoteTargetDaemonPort(GetDevicePortNumber());
        ppAppCtrl.SetCurrentProjectAndroidDeviceTargetCharacteristics(
            m_targetCharacteristics);
        ppAppCtrl.SetCurrentProjectAndroidTargetDefinition(
            m_targetDefinition);
        ppAppCtrl.SetCurrentProjectAndroidCollectionOptions(
            m_collectionDefs);

        ppAppCtrl.SetADBPortForward(m_pADBPortForwardCheckBox->isChecked());

        if (m_pTimerRadioBtn->isChecked())
        {
            ppAppCtrl.SetDurationMode(lpgpu2::DurationMode::Timer);
            ppAppCtrl.SetDurationModeTimerDuration(m_pTimerDurationSpinBox->value());
        }
        else if (m_pManualRadioBtn->isChecked())
        {
            ppAppCtrl.SetDurationMode(lpgpu2::DurationMode::Manual);
        }
        else if (m_pDurationRadioBtn->isChecked())
        {
            ppAppCtrl.SetDurationMode(lpgpu2::DurationMode::DeviceDuration);
        }
        else if (m_pExplicitControlRadioBtn->isChecked())
        {
            ppAppCtrl.SetDurationMode(lpgpu2::DurationMode::ExplicitControl);
        }

        auto appsListCount = m_pAppList->count();
        m_appListStr.assign(L"");
        for (decltype(appsListCount) i = 0; i < appsListCount; ++i)
        {
            auto *item = m_pAppList->item(i);
            auto appName = acQStringToGTString(item->text());
            m_appListStr.append(appName);
            m_appListStr.append(L":");
        }
        
        // Set the selected app, if any, by storing an integer
        auto selectedItemsList = m_pAppList->selectedItems();
        m_selectedAppRowAsStr.assign(L"");
        if (selectedItemsList.count() > 0)
        {
            auto rowId = m_pAppList->row(selectedItemsList[0]);
            m_selectedAppRowAsStr.appendUnsignedIntNumber(
                static_cast<gtUInt32>(rowId));
        }
        else
        {
            m_selectedAppRowAsStr.assign(L"-1");
        }

        afApplicationCommands::instance()->ShowMessageBox(QMessageBox::Information, PP_STR_InfoAfterOkTitle, PP_STR_InfoAfterOk);
    }

    return retVal;
}

/// @brief Restore the default project settings
/// @autor Gilad Yarnitzky, AMD
void ppProjectLPGPU2SettingsExtension::RestoreDefaultProjectSettings()
{
    GT_IF_WITH_ASSERT(m_pSamplingIntervalSpinBox != nullptr)
    {
        // Get the current project sampling interval:
        unsigned int samplingInterval = PP_DEFAULT_SAMPLING_INTERVAL;

        // Set the value in the spin box:
        m_pSamplingIntervalSpinBox->setValue(samplingInterval);
    }

    // Reset the XML containers
    m_targetCharacteristics = TargetCharacteristics{};
    m_collectionDefs = CollectionDefs{};
    m_targetDefinition = lpgpu2::TargetDefinition{};

    // Setup the GUI elements
    PopulateCounterList();

    m_pADBPortForwardCheckBox->setChecked(true);
    m_pDeviceNameTextBox->setText("");
    m_pAppList->clear();
    m_appListStr.assign(L"");
    m_selectedAppRowAsStr.assign(L"-1");
    m_pEGLCheckBox->setChecked(false);
    m_pGLCheckBox->setChecked(false);
    m_pCLCheckBox->setChecked(false);
    m_pVKCheckBox->setChecked(false);
    m_pADBPortForwardCheckBox->setChecked(true);
    m_termWidget->ResetCheckboxes();

    // Set the activated counters
    RestoreCounterList();

    auto isTimelineChecked = m_collectionDefs.IsTimelineEnabled();
    m_pTimelineCheckBox->setChecked(isTimelineChecked);
    auto areThereSelectedItems =
      static_cast<bool>(m_pAppList->selectedItems().size());
    m_pTimelineCheckBox->setEnabled(areThereSelectedItems);
    RestoreTimelineCheckboxes();
}

/// @brief Check if the current project settings are valid
/// @param invalidMessageStr Message to return if the settings are invalid
/// @return Always true
/// @author Gilad Yarnitzky, AMD
bool ppProjectLPGPU2SettingsExtension::AreSettingsValid(gtString& invalidMessageStr)
{
    GT_UNREFERENCED_PARAMETER(invalidMessageStr);
    // No project setting page at this stage for this extension
    bool retVal = true;

    return retVal;
}

/// @brief       Are current settings valid with other settings in other
///              setting pages?
/// @param[out]  vInvalidMessageStr Error description of invalid setting.
/// @return      bool true on success, else false equal settings error.
bool ppProjectLPGPU2SettingsExtension::AreSettingsValidSecondPass(
    gtString &vInvalidMessageStr)
{
    GT_UNREFERENCED_PARAMETER(vInvalidMessageStr);

    // Do not have to validate *this setting with other extension settings

    return true;
}

/// @brief Restore the settings for the widgets after the settings have been
///        loaded
/// @return True if success, false if failure
bool ppProjectLPGPU2SettingsExtension::RestoreCurrentSettings()
{
    bool retVal = true;
    auto &ppAppCtrl = ppAppController::instance();

    // Retrieve the previous settings from the ppAppCtrl
    m_targetCharacteristics = TargetCharacteristics{
            ppAppCtrl.GetCurrentProjectAndroidDeviceTargetCharacteristics() };
    m_collectionDefs = CollectionDefs{
          ppAppCtrl.GetCurrentProjectAndroidCollectionOptions() };
    m_targetDefinition = lpgpu2::TargetDefinition{
          ppAppCtrl.GetCurrentProjectAndroidTargetDefinition() };

    GT_IF_WITH_ASSERT(m_pSamplingIntervalSpinBox != nullptr)
    {
        // Get the current project sampling interval:
        unsigned int samplingInterval =
            ppAppCtrl.GetCurrentProjectSamplingInterval();

        // Set the value in the spin box:
        m_pSamplingIntervalSpinBox->setValue(samplingInterval);
    }

    // Setup the GUI elements
    auto rc = PopulateCounterList();
    if (rc != PPFnStatus::success)
    {
        retVal = false;
    }

    m_pIpTextBox->setText(
        acGTStringToQString(ppAppCtrl.GetRemoteTargetHostname()));
    // Retrieve the name of the device and store it in the gui
    m_pDeviceNameTextBox->setText(acGTStringToQString(
        m_targetDefinition.GetTargetElement().GetHardware()));

    RestoreCounterList();

    auto isTimelineChecked = m_collectionDefs.IsTimelineEnabled();
    m_pTimelineCheckBox->setChecked(isTimelineChecked);
    auto areThereSelectedItems =
      static_cast<bool>(m_pAppList->selectedItems().size());
    m_pTimelineCheckBox->setEnabled(areThereSelectedItems);
    RestoreTimelineCheckboxes();

    RestoreAppList();


    switch (ppAppCtrl.GetDurationMode())
    {
        case lpgpu2::DurationMode::Manual:
        {
            m_pManualRadioBtn->setChecked(true);
            break;
        }
        case lpgpu2::DurationMode::Timer:
        {
            m_pTimerRadioBtn->setChecked(true);
            m_pTimerDurationSpinBox->setValue(ppAppCtrl.GetDurationModeTimerDuration());
            break;
        }
        case lpgpu2::DurationMode::DeviceDuration:
        {
            m_pDurationRadioBtn->setChecked(true);
            m_pStartDrawSpinBox->setValue(m_collectionDefs.GetStartDraw());
            m_pStartFrameSpinBox->setValue(m_collectionDefs.GetStartFrame());
            m_pStopDrawSpinBox->setValue(m_collectionDefs.GetStopDraw());
            m_pStopFrameSpinBox->setValue(m_collectionDefs.GetStopFrame());
            break;
        }
        case lpgpu2::DurationMode::ExplicitControl:
        {
            m_pExplicitControlRadioBtn->setChecked(true);
            break;
        }
        default:
        {
          break;
        }

    }
    ppAppCtrl.SetDurationModeTimerDuration(m_pTimerDurationSpinBox->value());

    return retVal;
}

/// @brief Restore the content of the timeline checkboxes and related
///        widgets
/// @return Always PPFnStatus::success
PPFnStatus ppProjectLPGPU2SettingsExtension::RestoreTimelineCheckboxes()
{
  // Read the collection definitions and determine what is activated or not
  // What needs enabling:
  //
  // Enable timeline
  auto isTimelineEnabled =
    m_pTimelineCheckBox->isChecked() & m_pTimelineCheckBox->isEnabled();

  // API selection
  SetupAPICheckBoxesFromTargetCharacteristics();

  m_pVKCheckBox->setEnabled(isTimelineEnabled);
  m_pVKCheckBox->setChecked(m_collectionDefs.IsAPIBeingTraced(APIType::VK));
  m_pGLCheckBox->setEnabled(isTimelineEnabled);
  m_pGLCheckBox->setChecked(m_collectionDefs.IsAPIBeingTraced(APIType::GLES));
  m_pEGLCheckBox->setEnabled(isTimelineEnabled);
  m_pEGLCheckBox->setChecked(m_collectionDefs.IsAPIBeingTraced(APIType::GL));
  m_pCLCheckBox->setEnabled(isTimelineEnabled);
  m_pCLCheckBox->setChecked(m_collectionDefs.IsAPIBeingTraced(APIType::CL));

  // Enable parameter capture
  m_pParamCaptureCheckBox->setChecked(
      m_collectionDefs.IsParamCaptureEnabled());
  m_pParamCaptureCheckBox->setEnabled(isTimelineEnabled);
  // Capture screenshot
  // TODO Uncomment these once this feature is available
  //m_pCaptureScreenshotsCheckBox->setChecked(
      //m_collectionDefs.IsScreenshotCaptureEnabled());
  //m_pCaptureScreenshotsCheckBox->setEnabled(isTimelineEnabled);
  // Enable GPU timing
  auto isGPUTimingEnabled = m_collectionDefs.IsGPUTimingEnabled();
  m_pGpuTimingCheckBox->setChecked(isGPUTimingEnabled);
  m_pGpuTimingCheckBox->setEnabled(isTimelineEnabled);


  // Enable call stack dumps
  m_pCallStacksCheckBox->setChecked(
      m_collectionDefs.IsCallStackDumpEnabled());
  m_pCallStacksCheckBox->setEnabled(isTimelineEnabled);
  // Enable KHR Debug
  m_pKHRDebugCheckBox->setChecked(
      m_collectionDefs.IsKHRDebugEnabled());
  m_pKHRDebugCheckBox->setEnabled(isTimelineEnabled);
  // Capture shader sources
  m_pShaderSrcCheckBox->setChecked(
      m_collectionDefs.IsShaderSourceCaptureEnabled());
  m_pShaderSrcCheckBox->setEnabled(isTimelineEnabled);

  m_termWidget->SetCheckboxesFromBitfield(
      m_collectionDefs.GetFrameTerminator());
  m_termWidget->SetEnabled(isTimelineEnabled);

  return PPFnStatus::success;
}

/// @brief Set the checkboxes of the counter lists according to what is set in
///        the CollectionDefinition file
/// @return PPFnStatus::success if it succeede, PPFnStatus::failure if it failed
PPFnStatus ppProjectLPGPU2SettingsExtension::RestoreCounterList()
{
    const auto &counterSets = m_collectionDefs.GetCounterSets();
    gtSizeType setIdx = 0;
    gtSizeType counterIdx = 0;
    for (const auto &counterSet : counterSets)
    {
        m_pCounterList->SetCounterSetCheckedState(setIdx,
                                                  counterSet.enabled);
        m_pCounterList->SetCounterSetSamplingInterval(setIdx++,
                                                      counterSet.samplingInterval);

        const auto &counters = counterSet.counters;
        for (const auto &counter : counters)
        {
            m_pCounterList->SetCounterCheckedState(counterIdx++,
                                                   counter.enabled);
        }
    }

    return PPFnStatus::success;
}

/// @brief Retrieves the list of profilable applications from the remote device
///        and displays them in the profilable apps list. Called when the
///        'Get apps' button is pressed.
/// @return PPFnStatus::success if it succeede, PPFnStatus::failure if it failed
PPFnStatus ppProjectLPGPU2SettingsExtension::GetProfilableApplications()
{
    gtVector<gtString> supportedApps;

    CXLDaemonClient* pDmnClient = nullptr;
    if (GetCXLDaemonClient(&pDmnClient) == PPFnStatus::failure)
    {
        return PPFnStatus::failure;
    }
    std::function<bool()> f = std::bind(
        &CXLDaemonClient::LPGPU2_GetAndroidSupportedApps,
        pDmnClient, std::ref(supportedApps));
    bool isOk = DaemonCommunicate(pDmnClient, f);
    GT_ASSERT_EX(isOk,
                 L"Failed to get profilable applications list from remote device.");
    if (!isOk)
    {
        return PPFnStatus::failure;
    }

    m_pAppList->clear();
    for (const auto& appName : supportedApps)
    {
        m_pAppList->addItem(QString(appName.asASCIICharArray()));
    }

    return PPFnStatus::success;
}

/// @brief Retrieves and displays the list of available counters by receiving
///        and reading the target characteristics file sent by the remote device.
///        Called when the 'Get apps' button is pressed.
/// @return PPFnStatus::success if it succeede, PPFnStatus::failure if it failed
PPFnStatus ppProjectLPGPU2SettingsExtension::GetTargetCharacteristics()
{
    // Save the target characteristics XML file in the user data folder
    osFilePath filePath;
    afGetUserDataFolderPath(filePath);
    filePath.setFileName(PP_STR_TargetCharFileName);
    filePath.setFileExtension(PP_STR_TargetCharFileExt);
    gtString filePathStr = filePath.asString();

    CXLDaemonClient* pDmnClient = nullptr;
    if (GetCXLDaemonClient(&pDmnClient) == PPFnStatus::failure)
    {
        return PPFnStatus::failure;
    }
    std::function<bool()> f = std::bind(
        &CXLDaemonClient::LPGPU2_GetAndroidDeviceInfo, pDmnClient, filePathStr);
    bool isOk = DaemonCommunicate(pDmnClient, f);
    GT_ASSERT_EX(isOk, L"Failed to get available counters from remote device.");
    if (!isOk)
    {
        return PPFnStatus::failure;
    }

    // If the file was successfully received and exists
    GT_IF_WITH_ASSERT(filePath.exists())
    {
        m_targetCharacteristics = TargetCharacteristics{ filePathStr };
    }

    return PPFnStatus::success;
}

/// @brief Retrieve the TargetDefinition.xml Called when the
///        'Get apps' button is pressed.
/// @return PPFnStatus::success if it succeede, PPFnStatus::failure if it failed
PPFnStatus ppProjectLPGPU2SettingsExtension::GetTargetDefinition()
{
    // Save the target characteristics XML file in the user data folder
    osFilePath filePath;
    afGetUserDataFolderPath(filePath);
    filePath.setFileName(PP_STR_TargetDefFileName);
    filePath.setFileExtension(PP_STR_TargetDefFileExt);
    gtString filePathStr = filePath.asString();

    CXLDaemonClient* pDmnClient = nullptr;
    if (GetCXLDaemonClient(&pDmnClient) == PPFnStatus::failure)
    {
        return PPFnStatus::failure;
    }

    std::function<bool()> f = std::bind(
        &CXLDaemonClient::LPGPU2_GetAndroidRemoteAgentInfo,
        pDmnClient, std::ref(filePathStr));
    bool isOk = DaemonCommunicate(pDmnClient, f);
    GT_ASSERT_EX(isOk, L"Failed to get target defintion.");
    if (!isOk)
    {
        return PPFnStatus::failure;
    }

    // If the file was successfully received and exists
    GT_IF_WITH_ASSERT(filePath.exists())
    {
        auto rcInit = m_targetDefinition.init(filePath.asString());
        if (rcInit != lpgpu2::PPFnStatus::success)
        {
            return PPFnStatus::failure;
        }

        // Retrieve the name of the device and store it in the gui
        m_pDeviceNameTextBox->setText(acGTStringToQString(
            m_targetDefinition.GetTargetElement().GetHardware()));

        // Populate the counters list
        return PopulateCounterList();
    }

    return PPFnStatus::failure;
}

/// @brief Populate the GUI counters list using the retrieved TargetDefinition
/// @return PPFnStatus::success if it succeeded, PPFnStatus::failure if it failed
PPFnStatus ppProjectLPGPU2SettingsExtension::PopulateCounterList()
{
    // Populate the counters list
    m_pCounterList->clear();
    m_pCounterList->ClearCountersCache();

    const auto &counterSets =
        m_targetDefinition.GetTargetElement().GetCounterElemsVec();

    for (const auto &counterSet : counterSets)
    {
        // Add the counter category to the counters list
        QString categoryQStr = acGTStringToQString(counterSet.GetName());
        gtString idStr;
        idStr << counterSet.GetId();
        QString categoryIdQStr = acGTStringToQString(idStr);
        QString descriptionQStr = acGTStringToQString(counterSet.GetDescription());
        QTreeWidgetItem* categoryItem = m_pCounterList->AddCounterCategory(
            categoryQStr, categoryIdQStr, descriptionQStr,
            static_cast<double>(counterSet.GetMinSamplingInterval()));

        // Add all the counters in this category to the category tree item
        const auto &counters = counterSet.GetCounterElemsVec();
        for (const auto& counter : counters)
        {
            QString counterQStr = acGTStringToQString(counter.GetName());
            gtString idStr;
            idStr << counter.GetId();
            QString counterIdQStr = acGTStringToQString(idStr);
            QString descriptionQStr = acGTStringToQString(counter.GetDescription());
            m_pCounterList->AddCounterToCategory(
                categoryItem, counterQStr, counterIdQStr, descriptionQStr);
        }
    }

    return PPFnStatus::success;
}

/// @brief Enables/disables the API selection checkboxes based on the specified
///        check state. If the checkboxes are disabled, their state will be
///        reset to unchecked. Called when the 'Enable timeline' checkbox
///        state changes.
/// @param[in]  state   The Qt::CheckState of the controlling checkbox.
void ppProjectLPGPU2SettingsExtension::SetApiSelectionEnabled(int state)
{
  bool enabled = (state == Qt::Checked);
  m_pApiSelectionLabel->setEnabled(enabled);
  m_pGLCheckBox->setEnabled(enabled);
  m_pEGLCheckBox->setEnabled(enabled);
  m_pCLCheckBox->setEnabled(enabled);
  m_pVKCheckBox->setEnabled(enabled);
  m_pParamCaptureCheckBox->setEnabled(enabled);
  m_pGpuTimingCheckBox->setEnabled(enabled);
  m_pCallStacksCheckBox->setEnabled(enabled);
  m_pKHRDebugCheckBox->setEnabled(enabled);
  m_pShaderSrcCheckBox->setEnabled(enabled);
  m_termWidget->SetEnabled(enabled);
  if (enabled)
  {
    SetupAPICheckBoxesFromTargetCharacteristics();
  }
  else
  {
    m_pGLCheckBox->setEnabled(false);
    m_pEGLCheckBox->setEnabled(false);
    m_pCLCheckBox->setEnabled(false);
    m_pVKCheckBox->setEnabled(false);
  }
}

/// @brief Opens a file browser dialog which allows users to select EXE files
///        specifically. Called when the ADB path 'Browse' button is pressed.
void ppProjectLPGPU2SettingsExtension::OnADBPathBrowse()
{
    // Get the file selection:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();

    // Get the executable file types:
    QString exeFileTypes = AF_STR_executableFilesDetails;

    // Get the current selected executable:
    QString currentExecutableFile = m_pAdbTextBox->text();

    // Select the executable file:
    // TODO: Some of these parameters will need changed
    QString selectedExe = pApplicationCommands->ShowFileSelectionDialog(AF_STR_newProjecExecutableFileSelectionTitle, currentExecutableFile, exeFileTypes, m_pAdbBrowseAction, false);

    if (!selectedExe.isEmpty())
    {
        // Set the file name:
        m_pAdbTextBox->setText(selectedExe);
    }
}

void ppProjectLPGPU2SettingsExtension::OnADBPathTextChanged(const QString &vNewPath)
{
    auto &adbCommands = LPGPU2ppADBCommands::Instance();
    
    osFilePath newAdbFilePath{ acQStringToGTString(vNewPath) };
    if (newAdbFilePath.exists() && newAdbFilePath.isExecutable())
    {
        adbCommands.SetADBPath(newAdbFilePath);	
    }    
}

void ppProjectLPGPU2SettingsExtension::OnADBStatusChanged(bool bIsADBRunning)
{
    if (bIsADBRunning)
    {
        m_pADBStatusLabel->setText(PP_STR_ADBStatusRunning);
        m_pInstallRAgentButton->setEnabled(!LPGPU2ppADBCommands::Instance().IsRAgentInstalled());        
    }
    else
    {
        m_pADBStatusLabel->setText(PP_STR_ADBStatusStopped);
    }    

    m_pStartADBButton->setEnabled(!bIsADBRunning);
    m_pRestartRAgentButton->setEnabled(bIsADBRunning &&
        LPGPU2ppADBCommands::Instance().IsRAgentInstalled());
}

/// @brief Slot indicating that the user has selected a different ADB device.
/// @param[in] vSelectedDevice  The name of the device the user has selected.
void ppProjectLPGPU2SettingsExtension::OnADBDeviceSelectionChanged(const QString &vSelectedDevice)
{
    auto &adbCommands = LPGPU2ppADBCommands::Instance();
    const auto waitStatus = adbCommands.ChangeADBDevice(vSelectedDevice);
    if (waitStatus == EWaitStatus::kNeedToWait)
    {
        m_startADBGroupId = adbCommands.WaitForPreviousActions();
    }

    ResetDeviceInfo();
}

void ppProjectLPGPU2SettingsExtension::OnRAgentInstalledStatusChanged(bool vbIsRAgentInstalled)
{
    if (vbIsRAgentInstalled)
    {
        m_pRAgentStatusLabel->setText(PP_STR_ADBRAgentStatusInstalled);
    }
    else
    {
        m_pRAgentStatusLabel->setText(PP_STR_ADBRAgentStatusNotInstalled);
    }	

    m_pRestartRAgentButton->setEnabled(vbIsRAgentInstalled);
    m_pGetAppsButton->setEnabled(vbIsRAgentInstalled && m_getDeviceInfoGroupId == LPGPU2ppADBCommands::InvalidCommandGroupId);
}

void ppProjectLPGPU2SettingsExtension::OnRAgentStatusChanged(bool vbIsRAgentRunning)
{
    if (LPGPU2ppADBCommands::Instance().IsRAgentInstalled())
    {
        if (vbIsRAgentRunning)
        {
            m_pRAgentStatusLabel->setText(PP_STR_ADBRAgentStatusRunning);
        }
        else
        {
            m_pRAgentStatusLabel->setText(PP_STR_ADBRAgentStatusStopped);
        }

    }
    else
    {
        m_pRAgentStatusLabel->setText(PP_STR_ADBRAgentStatusNotInstalled);
    }
        
    m_pRestartRAgentButton->setEnabled(
        LPGPU2ppADBCommands::Instance().IsRAgentInstalled());
}

void ppProjectLPGPU2SettingsExtension::OnStartADB()
{
    auto &adbCommands = LPGPU2ppADBCommands::Instance();
    const auto waitStatus = adbCommands.StartADB();
    if (waitStatus == EWaitStatus::kNeedToWait)
    {
        m_startADBGroupId = adbCommands.WaitForPreviousActions();
    }
    else
    {
        ResetADBStatusLabels();
    }
}

void ppProjectLPGPU2SettingsExtension::OnInstallRAgentAPK()
{
    /*auto &adbCommands = LPGPU2ppADBCommands::Instance();
    const auto bRCInstall = adbCommands.InstallRAgentAPKs();
    if (bRCInstall == PPFnStatus::success)
    {
        m_pGetAppsButton->setEnabled(false);
        m_pInstallRAgentButton->setText(PP_STR_InstallingRAgentButton);
        m_pInstallRAgentButton->setEnabled(false);

        m_installRAgentAPKInfoGroupId = adbCommands.WaitForPreviousActions();
    }*/

    lpgpu2::LPGPU2ppInstallAPKDialog::OpenLPGPU2ppInstallAPKDialog();
}

void ppProjectLPGPU2SettingsExtension::OnRestartRAgent()
{
    auto &adbCommands = LPGPU2ppADBCommands::Instance();
    const auto waitStatus = adbCommands.RestartRAgent();

    if (waitStatus == EWaitStatus::kNeedToWait)
    {        
        m_pRestartRAgentButton->setEnabled(false);
        m_pRestartRAgentButton->setText(PP_STR_RestartingRAgentButton);

        m_restartRAgentGroupId = adbCommands.WaitForPreviousActions();
    }    
}

void ppProjectLPGPU2SettingsExtension::OnADBPreviousActionsCompleted(int vActionsGroupId)
{
    auto &adbCommands = LPGPU2ppADBCommands::Instance();

    if (vActionsGroupId == m_getDeviceInfoGroupId)
    {
        GetDeviceInfo();
        EndGetDeviceInfo();

        m_getDeviceInfoGroupId = adbCommands.InvalidCommandGroupId;
    }
    else if (vActionsGroupId == m_installRAgentAPKInfoGroupId)
    {        
        m_pGetAppsButton->setEnabled(adbCommands.IsADBRunning() && adbCommands.IsRAgentInstalled() && adbCommands.IsRAgentRunning());
        m_pInstallRAgentButton->setText(PP_STR_InstallRAgentButton);

        m_installRAgentAPKInfoGroupId = adbCommands.InvalidCommandGroupId;
    }
    else if (vActionsGroupId == m_startADBGroupId)
    {
        ResetADBStatusLabels();

        m_startADBGroupId = adbCommands.InvalidCommandGroupId;
    }
    else if (vActionsGroupId == m_restartRAgentGroupId)
    {
        m_pRestartRAgentButton->setText(PP_STR_RestartRAgentButton);
        m_pRestartRAgentButton->setEnabled(adbCommands.IsADBRunning() && adbCommands.IsRAgentInstalled());        

        m_restartRAgentGroupId = adbCommands.InvalidCommandGroupId;
    }
}

void ppProjectLPGPU2SettingsExtension::OnPortForwardCheckboxStateChanged(int vCheckState)
{
    if (vCheckState == Qt::Checked)
    {
        m_pIpTextBox->setText(PP_STR_ADBPortForwardDefaultHost);
        m_pIpTextBox->setEnabled(false);
    }
    else
    {        
        m_pIpTextBox->setEnabled(true);
    }
}

/// @brief Reads the device IP address that the user has entered into the
///        IP address textbox.
/// @return     gtString    The IP address as a string.
gtString ppProjectLPGPU2SettingsExtension::GetDeviceAddress()
{
    // Set the default hostname if one isn't specified.
    if (m_pIpTextBox->text().isEmpty())
    {
        m_pIpTextBox->setText(PP_STR_ADBPortForwardDefaultHost);
    }

    gtString dmnIp;
    std::string sDmnIp = m_pIpTextBox->text().toStdString();
    return dmnIp.fromUtf8String(sDmnIp);
}

/// @brief Reads the device port number from the port number textbox.
/// @return     gtUInt16    16-bit unsigned port number.
gtUInt16 ppProjectLPGPU2SettingsExtension::GetDevicePortNumber()
{
    bool isOk = false;
    gtUInt32 portNumber = m_pPortTextBox->text().toInt(&isOk);
    GT_ASSERT(isOk);

    return portNumber;
}

/// @brief Creates a TCP/IP connection between this client and the remote device.
///        This will initialize CodeXL's daemon-client communicator if not already
///        initialized.
/// @param[in]  pDmnClient  The daemon-client communicator to use.
/// @return     bool        True if the connection was successful, false if not.
bool ppProjectLPGPU2SettingsExtension::CreateDaemonConnection(CXLDaemonClient* pDmnClient)
{
    bool retVal = true;
    GT_IF_WITH_ASSERT(pDmnClient != NULL)
    {
        // Connect to the daemon.
        osPortAddress addrBuffer;
        retVal = pDmnClient->ConnectToDaemon(addrBuffer);
        GT_ASSERT(retVal);
        if (!retVal)
        {
            return false;
        }
        // Use the IsAlive command here because CXL will succeed at connecting with the
        // device even if the service is not running in foreground on the device. Hence,
        // this function would have returned true even if the service is not running
        // currently on the device.
        retVal = pDmnClient->LPGPU2_IsAndroidClientAlive();
        GT_ASSERT(retVal);
        if (!retVal)
        {
            // If IsAlive fails, the situation explained above happened. Hence reset
            // the state of the client obj so that it will correctly connect again
            // when asked
            pDmnClient->Close();
        }
    }
    else
    {
        GT_ASSERT_EX(false, L"Failed to get daemon client instance.");
        retVal = false;
    }

    return retVal;
}

/// @brief Generates the collection definitions file based on the state of the
///        widgets that belong to this widget.
/// @param[out] defFilePath Path to where the collection definitions file was created.
/// @return     bool        True if the file was successfully created, false if not.
bool ppProjectLPGPU2SettingsExtension::GenerateCollectionDefinitions(osFilePath& defFilePath)
{
    CollectionDefs::Builder builder;

    if (!m_targetCharacteristics.GetCommands().empty())
    {
        auto &apisLUT = m_targetCharacteristics.GetCommands()[0].apisLUT;
        // Set the state for timeline capture
        bool isTimelineEnabled = (m_pTimelineCheckBox->checkState() == Qt::Checked);
        builder.SetTimelineEnabled(isTimelineEnabled);

        // Set the enabled counters
        QList<lpgpu2::ppCounterCategory> counterCategories = m_pCounterList->GetAllCategories();
        gtList<CollectionDefCounterSet> collectionCounterSets;
        gtVector<int> enabledCounters;
        for (const auto& category : counterCategories)
        {
            CollectionDefCounterSet collSet;
            collSet.enabled = category.enabled;
            collSet.id = category.id;
            collSet.samplingInterval = category.samplingInterval;
            for (const lpgpu2::ppCounter& counter : category.counters)
            {
                CollectionDefCounter collCounter;
                collCounter.enabled = counter.enabled;
                collCounter.id = counter.id;
                collSet.counters.push_back(collCounter);
                if (collCounter.enabled)
                {
                    gtUInt32 idAsUI = 0U;
                    collCounter.id.toUnsignedIntNumber(idAsUI);
                    enabledCounters.push_back(idAsUI);
                }
            }
            collectionCounterSets.push_back(collSet);
        }
        builder.SetCounterSets(collectionCounterSets);
        if (enabledCounters.size() > 0)
        {
            ppAppController::instance().SetCurrentProjectEnabledCounters(enabledCounters);
        }

        // Set explicit frame terminator
        builder.SetExplicitFrameTerminator(
            m_termWidget->IsExplicitFrameTermEnabled());

        // Set explicit control
        builder.SetExplicitControl(
            m_pExplicitControlRadioBtn->isChecked());

        // Set the frameterminators
        {
            gtUInt32 frameTerminatorsBitfield = 0;
            m_termWidget->GetBitfieldFromCheckboxes(frameTerminatorsBitfield);
            builder.SetFrameTerminator(frameTerminatorsBitfield);
        }

        auto isParamCaptureEnabled =
            (m_pParamCaptureCheckBox->checkState() == Qt::Checked);
        if (isParamCaptureEnabled)
        {
           builder.SetCommandNamesOverride(CollectionDefs::recordParameters);
        }
        else
        {
           builder.SetCommandNamesOverride(CollectionDefs::none);
        }

        // Set the state for frame capture
        bool isFrameCaptureEnabled = (m_pCaptureScreenshotsCheckBox->checkState() == Qt::Checked);
        builder.SetFrameCaptureEnabled(isFrameCaptureEnabled);

        // Set the state for GPU timing
        bool isGpuTimingEnabled = (m_pGpuTimingCheckBox->checkState() == Qt::Checked);
        builder.SetGpuTimingMode(isGpuTimingEnabled, CollectionDefs::DisjointFrame);

        // Set the state for callstack dumps
        bool isCallStackDumpEnabled = (m_pCallStacksCheckBox->checkState() == Qt::Checked);
        builder.SetCallStackDumpEnabled(isCallStackDumpEnabled);

        // Set the state for KHR Debug
        bool isKHRDebugEnabled = (m_pKHRDebugCheckBox->checkState() == Qt::Checked);
        builder.SetKHRDebugEnabled(isKHRDebugEnabled, apisLUT);

        // Set the state for shader sources capture
        bool isCaptureShadersEnabled = (m_pShaderSrcCheckBox->checkState() == Qt::Checked);
        builder.SetCaptureShaderSourcesEnabled(isCaptureShadersEnabled, apisLUT);

        // Set the state for debuggroup capture (always enabled)
        builder.SetCaptureDebugGroupEnabled(true, apisLUT);

        // Get and set the application name
        if (m_pAppList->selectedItems().size() > 0)
        {
            gtString processName;
            QString qProcessName = m_pAppList->selectedItems()[0]->text();
            processName.fromUtf8String(qProcessName.toStdString());
            builder.SetProcessName(processName);
        }

        // Set the collection range values
        bool isCollectionRangeEnabled = (m_pDurationRadioBtn->isChecked());
        if (isCollectionRangeEnabled)
        {
            // Get the collection range values and set them in the builder
            gtInt32 startDraw = m_pStartDrawSpinBox->value();
            gtInt32 startFrame = m_pStartFrameSpinBox->value();
            gtInt32 stopDraw = m_pStopDrawSpinBox->value();
            gtInt32 stopFrame = m_pStopFrameSpinBox->value();
            builder.SetCollectionRange(startDraw, startFrame, stopDraw, stopFrame);
        }
        else
        {
            // Disable collection range if checkbox not checked
            builder.SetCollectionRange(-1, -1, -1, -1);
        }

        // What APIs to trace
        bool isVkTraceEnabled = (m_pVKCheckBox->checkState() == Qt::Checked);
        builder.SetAPITrace(APIType::VK, isVkTraceEnabled);
        bool isGLTraceEnabled = (m_pEGLCheckBox->checkState() == Qt::Checked);
        builder.SetAPITrace(APIType::GL, isGLTraceEnabled);
        bool isGLESTraceEnabled = (m_pGLCheckBox->checkState() == Qt::Checked);
        builder.SetAPITrace(APIType::GLES, isGLESTraceEnabled);
        bool isCLTraceEnabled = (m_pCLCheckBox->checkState() == Qt::Checked);
        builder.SetAPITrace(APIType::CL, isCLTraceEnabled);

        // Create the file in the user data folder
        afGetUserDataFolderPath(defFilePath);
        defFilePath.setFileName(PP_STR_CollectionDefFileName);
        defFilePath.setFileExtension(PP_STR_CollectionDefFileExt);

        builder.SetFileName(defFilePath.asString());

        m_collectionDefs = CollectionDefs{ builder.Build() };

        // Create the collection definitions XML string to write to a file
        TiXmlPrinter printer;
        printer.SetIndent("\t");
        m_collectionDefs.GetXML().Accept(&printer);
        gtString collectionDefsXmlStr;
        collectionDefsXmlStr.fromASCIIString(printer.CStr());

        // Create and/or write to the file
        osFile file(defFilePath);
        bool isOk = file.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        GT_ASSERT_EX(isOk, L"Failed to open collection definitions XML file.");
        isOk = file.write(collectionDefsXmlStr.asASCIICharArray(), collectionDefsXmlStr.length());
        GT_ASSERT_EX(isOk, L"Failed to write collection definitions XML file.");
        file.close();

        return isOk;
    }
    else
    {
        return false;
    }
}

/// @brief Gets the daemon-client communicator instance, which will be
///        initialized if it hasn't been already. The address and port
///        number that the communicator will use is pulled from the
///        relevant widgets in this GUI.
/// @return     CXLDaemonClient*Pointer to the daemon-client communicator.
PPFnStatus ppProjectLPGPU2SettingsExtension::GetCXLDaemonClient(
    CXLDaemonClient **p_dmnCli)
{
    gtString daemonAddress = GetDeviceAddress();
    gtUInt16 daemonPort = GetDevicePortNumber();

    if (daemonAddress.length() == 0)
    {
        auto p_msgBox = acMessageBox::instance();
        p_msgBox.critical("Invalid IP address", "Empty IP address. \
Please enter an IP address.");

        return PPFnStatus::failure;
    }
    osPortAddress daemonAddr(daemonAddress, daemonPort);

    bool retVal = CXLDaemonClient::IsInitialized(daemonAddr) ||
        CXLDaemonClient::Init(daemonAddr, kTCPTimeoutDefault);
    GT_ASSERT_EX(retVal, L"Failed to initialize CodeXL Remote Agent from LPGPU2 \
settings extension widget.");
    if (!retVal)
    {
        auto p_msgBox = acMessageBox::instance();
        p_msgBox.critical("Invalid IP address", "Please enter a valid IP address.");

        return PPFnStatus::failure;
    }

    *p_dmnCli = CXLDaemonClient::GetInstance();

    return PPFnStatus::success;
}

/// @brief Attempts to create a connection between this client and the remote
///        daemon/device. If a connection is made successfully, the specified
///        function will be called.
/// @param[in]  pDmnClient  The daemon-client communicator to use.
/// @param[in]  func        The bound function to execute.
/// @return     bool        True if communication is successful, false if not.
bool ppProjectLPGPU2SettingsExtension::DaemonCommunicate(CXLDaemonClient* pDmnClient, std::function<bool()>& func)
{
    bool ret = CreateDaemonConnection(pDmnClient);
    GT_IF_WITH_ASSERT_EX(ret, L"Failed initial connection to daemon.")
    {
        ret = func();
        if (!ret)
        {
            // TODO: Fix the need for a second connection to the daemon
            ret = CreateDaemonConnection(pDmnClient);
            GT_ASSERT_EX(ret, L"Failed second connection to daemon.");

            ret = func();
            GT_ASSERT_EX(ret, L"Failed second attempt to communicate with daemon.");
        }
    }

    // TODO Uncomment this section once the Android service does not disconnect and
    // need restarting every time you setup a new session
      //if (pDmnClient) pDmnClient->LPGPU2_DisconnectFromAndroidDevice();

    if (!ret)
    {
        auto p_msgBox = acMessageBox::instance();
        p_msgBox.warning("Device info retrieval",
                         "Failed to communicate with the device.\n\
Please ensure that your device is connected, accessible via ADB and \
that the LPGPU2 RAgent is running on the device. If the problem persists \
try to the \"Restart RAgent\" button to restart the remote agent.");
    }


    return ret;
}


/// @brief Triggered upon the activation of an item in the apps list. It
///        enables the ability to set the timeline parameters.
/// @param[in]  QListWidget The item that was activated
void ppProjectLPGPU2SettingsExtension::OnAppsListItemActivated(
    QListWidgetItem* item)
{
    (void) item;
    auto areThereSelectedItems =
      static_cast<bool>(m_pAppList->selectedItems().size());
    m_pTimelineCheckBox->setEnabled(areThereSelectedItems);
    RestoreTimelineCheckboxes();
}

/// @brief Triggered upon the activation of an item in the power models names list. It
///        enables the ability to enable/disbale required counters
/// @param[in]  QListWidget The item that was activated
void ppProjectLPGPU2SettingsExtension::OnmPwrModelsNamesListItemActivated(QListWidgetItem* item) 
{
    GT_UNREFERENCED_PARAMETER(item);
    
    // Get the selected indices
    auto indices = m_pPwrModelsNamesList->selectionModel()->selectedIndexes();
    ppAppController::instance().ClearPowerModelDescriptors();
    for (auto index : indices) {
        auto pwDesriptor = m_powerModelDescriptors[index.row()];

        ppAppController::instance().AddPowerModelDescriptor(pwDesriptor);
    }

    // TODO: disable required counters
}

/// @brief Slot method signaled when the user clicks on Get Info (formerly Get
///        Apps)
void ppProjectLPGPU2SettingsExtension::OnGetAppsClicked()
{
     auto &adbCommands = LPGPU2ppADBCommands::Instance();

    StartGetDeviceInfo();
    
    auto waitStatus = adbCommands.StartADB();

    if (m_pAlwaysRestartRAgentCheckBox->isChecked())
    {
        waitStatus = adbCommands.RestartRAgent();
    }    

    if (m_pADBPortForwardCheckBox->isChecked())
    {
        waitStatus = adbCommands.PortFoward(GetDevicePortNumber(), 21845);
    }

    if (waitStatus == EWaitStatus::kNeedToWait)
    {
        // Wait for all previous actions to be completed
        m_getDeviceInfoGroupId = adbCommands.WaitForPreviousActions();
    }
    else
    {
        GetDeviceInfo();
        EndGetDeviceInfo();
    }
}

/// @brief Populate the available power models list after a device has been
///        connected to depending on the type of device connected
/// @return PPFnStatus::success upon success, PPFnStatus::failure upon failure
/// @note Currently it always returns PPFnStatus::success
PPFnStatus ppProjectLPGPU2SettingsExtension::PopulatePowerModelsLists()
{
    GT_ASSERT(m_pPwrModelsNamesList);

  // Get current device
  const auto &deviceName = m_targetDefinition.GetTargetElement().GetHardware();

  m_powerModelDescriptors =  lpgpuPowerModelsConfigManager::instance().getSupportedPowerModels(deviceName);

  if (m_powerModelDescriptors.size() == 0)
  {
    OS_OUTPUT_DEBUG_LOG(L"Connected device is not supported by the power models!",
        OS_DEBUG_LOG_INFO);

    // Populate GUI with message that device is not supported
    m_pPwrModelsNamesList->addItem("Power Models do not support this device.");
    m_pPwrModelsDescList->addItem("Power Models do not support this device.");

    return PPFnStatus::success;
  }

  m_pPwrModelsNamesList->clear();
  m_pPwrModelsDescList->clear();
  for (const auto &pwDesciptor : m_powerModelDescriptors)
  {
    m_pPwrModelsNamesList->addItem(pwDesciptor.name.asASCIICharArray());
  }

  int idx = 0;
  for (const auto &pwDesciptor : m_powerModelDescriptors)
  {
    m_pPwrModelsDescList->addItem(pwDesciptor.description.asASCIICharArray());
    // Necessary to do this because addItem does not return the latest
    // itemWidget added
    auto currItem = m_pPwrModelsDescList->item(idx++);
    currItem->setToolTip(pwDesciptor.longDescription.asASCIICharArray());
  }

  return PPFnStatus::success;
}

/// @brief Load the CollectionDefinitions from an existing XML file which has already been
///        opened and creates the CollectionDefinitions object
/// @param[out] baseElem Root element of the XML to parse
/// @return     RC success if the CollectionDefs was loaded successfully, failure if not.
PPFnStatus ppProjectLPGPU2SettingsExtension::GenerateCollectionDefinitionsFromXMLElem(TiXmlElement *baseElement)
{
    if (!baseElement)
    {
      return PPFnStatus::failure;
    }

    auto &apisLUT = m_targetCharacteristics.GetCommands()[0].apisLUT;
    CollectionDefs::Builder builder;

    // Load the elements from the XML and insert them in the builder
    // Set the state for the timeline capture
    {
      auto *element = baseElement->FirstChildElement("Timeline");
      if (element)
      {
        auto isTimelineEnabled = charToI32(element->Attribute("Enable"));
        builder.SetTimelineEnabled(isTimelineEnabled);
      }
    }

    {
        // Set the enabled counters
      gtList<CollectionDefCounterSet> collectionCounterSets;
      auto* pCounterSetNode = baseElement->FirstChildElement("CounterSet");
      while (pCounterSetNode)
      {
        // CounterSet
        CollectionDefCounterSet collSet;
        collSet.enabled = charToI32(pCounterSetNode->Attribute("Enable"));
        collSet.id.fromASCIIString(pCounterSetNode->Attribute("Id"));
        collSet.samplingInterval = charToUI32(pCounterSetNode->Attribute("SamplingInterval"));

        auto *pCounterNode = pCounterSetNode->FirstChildElement("Counter");
        while (pCounterNode)
        {
            // Counter
          CollectionDefCounter collCounter;
          collCounter.enabled = charToI32(pCounterNode->Attribute("Enable"));
          collCounter.id.fromASCIIString(pCounterNode->Attribute("Id"));
          collSet.counters.push_back(collCounter);

          pCounterNode = pCounterNode->NextSiblingElement("Counter");
        }

        collectionCounterSets.push_back(collSet);

        pCounterSetNode = pCounterSetNode->NextSiblingElement("CounterSet");
      }
      builder.SetCounterSets(collectionCounterSets);
    }

    // Set the state for compression
    {
      auto *element = baseElement->FirstChildElement("Compressed");
      if (element)
      {
        auto isEnabled = charToI32(element->Attribute("Enable"));
        builder.SetCallStackDumpEnabled(isEnabled);
      }
    }

    // Set the state for frame capture
    {
      auto *element = baseElement->FirstChildElement("Compressed");
      if (element)
      {
        auto isFrameCaptureEnabled = charToI32(element->Attribute("Enable"));
        builder.SetFrameCaptureEnabled(isFrameCaptureEnabled);
      }
    }

    // Set the state for callstack dumps
    {
      auto *element = baseElement->FirstChildElement("Compressed");
      if (element)
      {
        auto isEnabled = charToI32(element->Attribute("Enable"));
        builder.SetCallStackDumpEnabled(isEnabled);
      }
    }

    {
        auto *frameCaptureElem = baseElement->FirstChildElement("FrameCapture");
        if (frameCaptureElem)
        {
            auto isEnabled = charToI32(frameCaptureElem->Attribute("Enable"));
            builder.SetFrameCaptureEnabled(isEnabled);
        }
    }

    {
        auto *element = baseElement->FirstChildElement("DumpCallstack");
        if (element)
        {
            auto isEnabled = charToI32(element->Attribute("Enable"));
            builder.SetCallStackDumpEnabled(isEnabled);
        }
    }
    
    // Set the state for GPU timing
    {
        auto* gpuTimingElement = baseElement->FirstChildElement("GPUTiming");
        if (gpuTimingElement)
        {
            auto isGPUTimingEnabled = charToI32(gpuTimingElement->Attribute("Enable"));
            if (isGPUTimingEnabled)
            {
                auto* gpuTimingModeElement = baseElement->FirstChildElement("GpuTimingMode");
                if (gpuTimingModeElement)
                {
                    gtString gpuTimingMode;
                    gpuTimingMode.fromUtf8String(gpuTimingModeElement->Attribute("Mode"));

                    builder.SetGpuTimingMode(true, gpuTimingMode);
                }
            }
            else
            {
                builder.SetGpuTimingMode(false, L"");
            }
        }
    }

    // Set the state for shader sources capture
    {
      auto *element = baseElement->FirstChildElement("CaptureShaders");
      if (element)
      {
        auto isEnabled = charToI32(element->Attribute("Enable"));
        builder.SetCaptureShaderSourcesEnabled(isEnabled, apisLUT);
      }
    }

    {
      auto *element = baseElement->FirstChildElement("ExplicitControl");
      if (element)
      {
        auto isEnabled = charToI32(element->Attribute("Enable"));
        builder.SetExplicitControl(isEnabled);
      }
    }

    {
      auto *element = baseElement->FirstChildElement("ExplicitFrameTerminator");
      if (element)
      {
        auto isEnabled = charToI32(element->Attribute("Enable"));
        builder.SetExplicitFrameTerminator(isEnabled);
      }
    }

    {
        auto *element = baseElement->FirstChildElement("KHRDebug");
        if (element)
        {
            auto isEnabled = charToI32(element->Attribute("Enable"));
            builder.SetKHRDebugEnabled(isEnabled, {});
        }
    }    

    // Set the state of the frame terminators
    {
      auto *terminatorsElem = baseElement->FirstChildElement("FrameTerminators");
      if (terminatorsElem)
      {
        auto terminatorBitflag = charToUI32(terminatorsElem->Attribute("Flags"));
        builder.SetFrameTerminator(terminatorBitflag);
      }
    }    

    // Set what apis to trace
    {
      auto *apisNameElem = baseElement->FirstChildElement("API");
      if (apisNameElem)
      {
        gtString attributesList;
        attributesList.fromASCIIString(apisNameElem->Attribute("Name"));

        // Disable all APIs first to handle the case where the "Name" attribute is empty
        builder.SetAPITrace(APIType::GL, false);
        builder.SetAPITrace(APIType::GLES, false);
        builder.SetAPITrace(APIType::CL, false);
        builder.SetAPITrace(APIType::VK, false);

        gtStringTokenizer tokeniser{ attributesList, gtString{ L" " } };
        gtString token;
        while (tokeniser.getNextToken(token))
        {
          if (token != L" " && !token.isEmpty())
          {
            if (token == L"egl")
            {
              builder.SetAPITrace(APIType::GL, true);
            }
            if (token == L"gles2")
            {
              builder.SetAPITrace(APIType::GLES, true);
            }
            if (token == L"cl")
            {
              builder.SetAPITrace(APIType::CL, true);
            }
            if (token == L"vulkan")
            {
              builder.SetAPITrace(APIType::VK, true);
            }
          }
        }
      }
    }

    // Set the state for the command names
    {
      auto *pCommandNamesNode = baseElement->FirstChildElement("CommandNames");
      if (pCommandNamesNode)
      {
        const auto *attrib = pCommandNamesNode->Attribute("DefaultOverride");
        if (attrib)
        {
          gtString attribute;
          attribute.fromUtf8String(attrib);
          builder.SetCommandNamesOverride(attribute);
        }
        else
        {
          builder.SetCommandNamesOverride(CollectionDefs::CommandNamesOverride::none);
        }

        // Set the commands
        auto *pCommandNode
            = pCommandNamesNode->FirstChildElement("Command");
        while (pCommandNode)
        {
          int cmdId = 0;
          GT_ASSERT(pCommandNode->Attribute("Id", &cmdId));
          gtString origCommandName;
          origCommandName.fromUtf8String(pCommandNode->Attribute("Name"));
          gtString replaceCommandName;
          replaceCommandName.fromUtf8String(pCommandNode->Attribute("Override"));

          builder.SetCommandName(static_cast<gtUInt32>(cmdId), origCommandName,
                                 replaceCommandName);

          pCommandNode = pCommandNode->NextSiblingElement("Command");
        }
      }
    }

    // For the "config" fields, go through each one of them
    auto *pConfigNode = baseElement->FirstChildElement("Config");
    while (pConfigNode)
    {
        // Get the name attribute of the config element
      gtString name;
      name.fromUtf8String(pConfigNode->Attribute("Name"));

      // The value vars are not declared outside of the if statements because
      // different branches need different types
      if (name == L"NumberedBufferSize")
      {
        // Get its value
        auto value = charToI32(pConfigNode->Attribute("Value"));

        builder.SetNumberedBufferSize(value);
      }
      else if (name == L"DcapiIntervalHz")
      {
        // Get its value
        auto asQStr = QString{ pConfigNode->Attribute("Value") };

        builder.SetDcApiIntervalHz(asQStr.toUInt());
      }
      else if (name == L"LogLevel")
      {
        // Get its value
        gtString value;
        value.fromUtf8String(pConfigNode->Attribute("Value"));

        builder.SetLogLevel(value);
      }
      else if (name == L"DataDirectory")
      {
        // Get its value
        gtString value;
        value.fromUtf8String(pConfigNode->Attribute("Value"));

        builder.SetLogLevel(value);
      }
      else if (name == L"StatsWriteThreshold")
      {
        // Get its value
        auto value = charToI32(pConfigNode->Attribute("Value"));

        builder.SetStatsWriteThreshold(value);
      }


      pConfigNode = pConfigNode->NextSiblingElement("Config");
    }

    // Set the app to launch
    {
      auto *element = baseElement->FirstChildElement("Process");
      if (element)
      {
        gtString value;
        value.fromUtf8String(element->Attribute("Name"));

        builder.SetProcessName(value);
      }
    }

    // Set the collection range values
    {
        auto *collRangeNode = baseElement->FirstChildElement("CollectionRange");
        if (collRangeNode)
        {
          auto startDraw = charToI32(collRangeNode->Attribute("StartDraw"));
          auto startFrame = charToI32(collRangeNode->Attribute("StartFrame"));
          auto stopDraw = charToI32(collRangeNode->Attribute("StopDraw"));
          auto stopFrame = charToI32(collRangeNode->Attribute("StopFrame"));

          builder.SetCollectionRange(startDraw, startFrame, stopDraw, stopFrame);
        }
    }

    // Create the file in the user data folder
    osFilePath defFilePath;
    afGetUserDataFolderPath(defFilePath);
    defFilePath.setFileName(PP_STR_CollectionDefFileName);
    defFilePath.setFileExtension(PP_STR_CollectionDefFileExt);

    builder.SetFileName(defFilePath.asString());

    m_collectionDefs = CollectionDefs{ builder.Build() };

    // Create the collection definitions XML string to write to a file
    TiXmlPrinter printer;
    printer.SetIndent("\t");
    m_collectionDefs.GetXML().Accept(&printer);
    gtString collectionDefsXmlStr;
    collectionDefsXmlStr.fromASCIIString(printer.CStr());

    // Create and/or write to the file
    osFile file(defFilePath);
    bool isOk = file.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
    GT_ASSERT_EX(isOk, L"Failed to open collection definitions XML file.");
    isOk = file.write(collectionDefsXmlStr.asASCIICharArray(), collectionDefsXmlStr.length());
    GT_ASSERT_EX(isOk, L"Failed to write collection definitions XML file.");
    file.close();

    return isOk ? PPFnStatus::success : PPFnStatus::failure;
}

/// @brief Restore the apps list and which one was selected, if any
/// @return Always PPFnStatus::success
PPFnStatus ppProjectLPGPU2SettingsExtension::RestoreAppList()
{
    gtStringTokenizer tokeniser{ m_appListStr, gtString{ L":" } };
    gtString token;
    m_pAppList->clear();
    while (tokeniser.getNextToken(token))
    {
        m_pAppList->addItem(acGTStringToQString(token));
    }

    // Set the selected app, if any
    if (m_selectedAppRowAsStr.length() > 0)
    {
        gtInt32 row = 0;
        GT_ASSERT(m_selectedAppRowAsStr.toIntNumber(row));
        if (row != -1)
        {
          m_pAppList->setCurrentRow(row);
          auto itemAtRow = m_pAppList->item(row);
          m_pAppList->setCurrentItem(itemAtRow);
        }
    }

    auto areThereSelectedItems =
      static_cast<bool>(m_pAppList->selectedItems().size());
    m_pTimelineCheckBox->setEnabled(areThereSelectedItems);

    return PPFnStatus::success;
}

/// @brief Change the GUI depending on what type of host was selected
/// @param hostType The type of host for the new host
/// @return True if success, false if failure
/// @note Always returns true
bool ppProjectLPGPU2SettingsExtension::NotifyHostChange(lpgpu2::HostType hostType)
{
    const auto isLPGPU2 = (hostType == lpgpu2::HostType::LPGPU2);    
    
    auto &adbCommands = LPGPU2ppADBCommands::Instance();
    if (isLPGPU2)
    {
        m_hostModeLabel->setText(PP_STR_LPGPU2ModeAMDControlsDisabled);
        
        adbCommands.StartMonitoring();		

        m_pADBStatusLabel->setText(PP_STR_WaitingADBStatus);
        m_pRAgentStatusLabel->setText(PP_STR_WaitingRAgentStatus);		

        const auto waitStatus = adbCommands.StartADB();        
        if (waitStatus == EWaitStatus::kNeedToWait)
        {
            m_startADBGroupId = adbCommands.WaitForPreviousActions();
        }
        else
        {
            ResetADBStatusLabels();
        }
    }
    else
    {
        m_hostModeLabel->setText(PP_STR_NonLPGPU2ModeLPGPU2ControlsDisabled);
        adbCommands.StopMonitoring();

        m_pADBStatusLabel->setText(PP_STR_ADBStatusNotLPGPU2Mode);
        m_pRAgentStatusLabel->setText(PP_STR_ADBStatusRAgentNotLPGPU2Mode);
    }

    m_pStartADBButton->setEnabled(isLPGPU2 && !adbCommands.IsADBRunning());
    m_pInstallRAgentButton->setEnabled(isLPGPU2 && adbCommands.IsADBRunning());

    m_pSamplingIntervalSpinBox->setEnabled(!isLPGPU2);
    m_pIpTextBox->setEnabled(isLPGPU2 && !m_pADBPortForwardCheckBox->isChecked());
    m_pPortTextBox->setEnabled(isLPGPU2);
    m_pDeviceNameTextBox->setEnabled(isLPGPU2);
    m_pAppList->setEnabled(isLPGPU2);
    m_pCounterList->setEnabled(isLPGPU2);
    m_pTimelineCheckBox->setEnabled(isLPGPU2);
    m_pParamCaptureCheckBox->setEnabled(isLPGPU2);
    m_pGpuTimingCheckBox->setEnabled(isLPGPU2);
    m_pCallStacksCheckBox->setEnabled(isLPGPU2);
    m_pKHRDebugCheckBox->setEnabled(isLPGPU2);
    m_pShaderSrcCheckBox->setEnabled(isLPGPU2);    
    m_pStartDrawSpinBox->setEnabled(isLPGPU2);
    m_pStartFrameSpinBox->setEnabled(isLPGPU2);
    m_pStopDrawSpinBox->setEnabled(isLPGPU2);
    m_pStopFrameSpinBox->setEnabled(isLPGPU2);
    m_pDurationRadioBtn->setEnabled(isLPGPU2);
    m_pStartDrawLabel->setEnabled(isLPGPU2);
    m_pStartFrameLabel->setEnabled(isLPGPU2);
    m_pStopDrawLabel->setEnabled(isLPGPU2);
    m_pStopFrameLabel->setEnabled(isLPGPU2);
    m_pAdbTextBox->setEnabled(isLPGPU2);
    m_pAdbBrowseAction->setEnabled(isLPGPU2);
    m_pManualRadioBtn->setEnabled(isLPGPU2);
    m_pTimerRadioBtn->setEnabled(isLPGPU2);
    m_pTimerDurationSpinBox->setEnabled(isLPGPU2);
    m_pExplicitControlRadioBtn->setEnabled(isLPGPU2);
    m_pGetAppsButton->setEnabled(isLPGPU2);
    m_pTimerDurationLabel->setEnabled(isLPGPU2);
    m_pADBPortForwardCheckBox->setEnabled(isLPGPU2);

    m_pRAgentStatusLabel->setEnabled(isLPGPU2);
    m_pADBStatusLabel->setEnabled(isLPGPU2);
    m_pAlwaysRestartRAgentCheckBox->setEnabled(isLPGPU2);

    if (isLPGPU2)
    {
      auto isTimelineChecked = m_collectionDefs.IsTimelineEnabled();
      m_pTimelineCheckBox->setChecked(isTimelineChecked);
      auto areThereSelectedItems =
        static_cast<bool>(m_pAppList->selectedItems().size());
      m_pTimelineCheckBox->setEnabled(areThereSelectedItems);
        RestoreTimelineCheckboxes();
    }

    return true;
}

/// @brief Enables timeline API checkboxes depending on whether they are
///        supported by the device connected
/// @return PPFnStatus::success if success,PPFnStatus::failure if failure
PPFnStatus ppProjectLPGPU2SettingsExtension::SetupAPICheckBoxesFromTargetCharacteristics()
{
    if (m_targetCharacteristics.GetCommands().size() <= 0)
    {
        m_pEGLCheckBox->setEnabled(false);
        m_pGLCheckBox->setEnabled(false);
        m_pCLCheckBox->setEnabled(false);
        m_pVKCheckBox->setEnabled(false);
        return PPFnStatus::failure;
    }

    auto &apisLUT = m_targetCharacteristics.GetCommands()[0].apisLUT;
    // OpenGL
    auto apiItor = apisLUT.find(kOGLAPIID);
    if (apiItor != apisLUT.end())
    {
        m_pGLCheckBox->setEnabled(true);
    }
    // OpenGLES
    apiItor = apisLUT.find(kEGLAPIID);
    if (apiItor != apisLUT.end())
    {
        m_pEGLCheckBox->setEnabled(true);
    }
    // OpenCL
    apiItor = apisLUT.find(kCLAPIID);
    if (apiItor != apisLUT.end())
    {
        m_pCLCheckBox->setEnabled(true);
    }

    return PPFnStatus::success;
}

void ppProjectLPGPU2SettingsExtension::StartGetDeviceInfo()
{
    m_pGetAppsButton->setText(PP_STR_GettingDeviceInfoButton);
    m_pGetAppsButton->setEnabled(false);
}

void ppProjectLPGPU2SettingsExtension::GetDeviceInfo()
{
    ResetDeviceInfo();

    auto rc = GetProfilableApplications();
    if (rc != PPFnStatus::success)
    {
        return;
    }
    rc = GetTargetDefinition();
    if (rc != PPFnStatus::success)
    {
        return;
    }
    rc = GetTargetCharacteristics();
    if (rc != PPFnStatus::success)
    {
        return;
    }
    else
    {
        // Ignore return value for now as it always returns PPFnStatus::success
        //SetSamplingIntervalSpinBoxMin();
    }

    rc = PopulatePowerModelsLists();
    if (rc != PPFnStatus::success)
    {
        return;
    }
}

void ppProjectLPGPU2SettingsExtension::EndGetDeviceInfo()
{
    m_pGetAppsButton->setText(PP_STR_GetDeviceInfoButton);
    m_pGetAppsButton->setEnabled(true);
}

void ppProjectLPGPU2SettingsExtension::ResetADBInfoLabels()
{
    auto &adbCommand = LPGPU2ppADBCommands::Instance();
    if (adbCommand.IsADBPathValid())
    {
        gtString adbVersion;
        gtString adbRevision;
        gtString adbDevice;
        gtVector<gtString> adbDevices;

        adbCommand.GetADBVersion(adbVersion);
        adbCommand.GetADBRevision(adbRevision);
        adbCommand.GetADBDevices(adbDevices);
        adbCommand.GetADBActiveDevice(adbDevice);

        m_pADBVersionLabel->setText(acGTStringToQString(adbVersion));
        m_pADBRevisionLabel->setText(acGTStringToQString(adbRevision));
        m_pADBDeviceComboBox->clear();
        for (const auto& device : adbDevices)
        {
            m_pADBDeviceComboBox->addItem(acGTStringToQString(device));
        }

        m_pADBDeviceComboBox->setCurrentText(acGTStringToQString(adbDevice));
    }
    else
    {
        m_pADBVersionLabel->setText(PP_STR_ADBNotFoundError);
        m_pADBRevisionLabel->setText("");
    }
}

void ppProjectLPGPU2SettingsExtension::ResetADBStatusLabels()
{
    auto &adbCommand = LPGPU2ppADBCommands::Instance();
    m_pADBStatusLabel->setText(adbCommand.IsADBRunning() ? PP_STR_ADBStatusRunning : PP_STR_ADBStatusStopped);

    ResetADBInfoLabels();

    if (!adbCommand.IsRAgentInstalled())
    {
        m_pRAgentStatusLabel->setText(PP_STR_ADBRAgentStatusNotInstalled);
    }
    else if (adbCommand.IsRAgentRunning())
    {
        m_pRAgentStatusLabel->setText(PP_STR_ADBRAgentStatusRunning);
    }
    else
    {
        m_pRAgentStatusLabel->setText(PP_STR_ADBRAgentStatusStopped);
    }
}

/// @brief Clears all displayed information that is device-specififc.
void ppProjectLPGPU2SettingsExtension::ResetDeviceInfo()
{
    m_pDeviceNameTextBox->clear();
    m_pAppList->clear();
    m_pCounterList->clear();

    // Drop existing CXLDaemonClient connection
    CXLDaemonClient* p_dmnCli = nullptr;
    const auto rc = GetCXLDaemonClient(&p_dmnCli);
    GT_ASSERT(p_dmnCli != nullptr && rc != PPFnStatus::failure);
    if(rc != PPFnStatus::failure && p_dmnCli != nullptr)
    {
        p_dmnCli->Close();
    }    
}

void ppProjectLPGPU2SettingsExtension::showEvent(QShowEvent *event)
{
  afProjectSettingsExtension::showEvent(event);

  auto &adbCommand = LPGPU2ppADBCommands::Instance();

  m_pInstallRAgentButton->setEnabled(adbCommand.IsADBRunning());

  m_pRestartRAgentButton->setEnabled(adbCommand.IsADBRunning() &&
      adbCommand.IsRAgentInstalled());
}
