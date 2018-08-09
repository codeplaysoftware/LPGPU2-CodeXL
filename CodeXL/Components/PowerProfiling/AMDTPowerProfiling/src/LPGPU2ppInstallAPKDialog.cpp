
// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppInstallAPKDialog.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/src/LPGPU2ppADBCommands.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// Qt:
#include <QListWidget>

namespace lpgpu2
{

// Static constants used to avoid typing true/false on the constructor
static constexpr bool g_bHasOkButton = false;
static constexpr bool g_bHasCancelButton = false;

/// @brief              Utility function to get the folder where the available APKs reside.
/// @return osFilePath  The absolute folder path of "LPGPU2_RAgent" folder.
static osFilePath GetAPksFolderPath()
{
    osFilePath apksFolderPath{ osFilePath::OS_CODEXL_BINARIES_PATH };
    apksFolderPath.appendSubDirectory(PP_STR_LPGPU2RAgentFolderW);
    return apksFolderPath;
}

/// @brief              Display the Install APKs dialogue so the user can directly install an APK to
///                     an attached device.      
/// @return PPFnStatus  success: The dialog was displayed and no error has occurred,
///                     failure: An error has occurred when initialising the dialog.
PPFnStatus LPGPU2ppInstallAPKDialog::OpenLPGPU2ppInstallAPKDialog()
{
    auto bReturn = PPFnStatus::failure;

    auto *pAppCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pAppCommands != nullptr)
    {
        LPGPU2ppInstallAPKDialog dialog;
        if (dialog.Initialise() == PPFnStatus::success)
        {
            pAppCommands->showModal(&dialog);

            bReturn = PPFnStatus::success;
        }
    }

    return bReturn;
}

/// @brief              Class constructor. Set the window flags and the window title.
/// @param[in] vpParent To conform with the QWidget convention, this class accepts
///                     a pointer to another widget, but this pointer is not used.
/// @warning            The dialog will the a child of the main window.
/// @see                afMainAppWindow.
LPGPU2ppInstallAPKDialog::LPGPU2ppInstallAPKDialog(QWidget *vpParent /* = nullptr */)
    : acDialog{ afMainAppWindow::instance(), g_bHasOkButton, g_bHasCancelButton  }
{
    GT_UNREFERENCED_PARAMETER(vpParent);

    // Set window flags to disable the help button on the dialog
    auto flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    setWindowTitle(PP_STR_InstallAPKButton);
}

/// @brief   Class destructor. Calls Shutdown to free the resources used by this class.
LPGPU2ppInstallAPKDialog::~LPGPU2ppInstallAPKDialog()
{
    Shutdown();
}

/// @brief              Initialises the class elements and layout.
/// @return PPFnStatus  success: The dialog was initialised and it is ready to be
///                               displayed,
///                     failure: An error has occurred during the construction
///                              of the dialog layout.
PPFnStatus LPGPU2ppInstallAPKDialog::Initialise()
{
    auto bReturn = PPFnStatus::failure;

    if (m_pCustomButtons == nullptr)
    {
        // This comes from the base class, this is a list of custom 
        // buttons to be displayed in the bottom part of the dialog.
        // Since this is a pointer to a list, it needs to be allocated
        // here.
        m_pCustomButtons = new (std::nothrow) QList<QPushButton*>{};
        if(m_pCustomButtons == nullptr)
        {
            return PPFnStatus::failure;
        }
    }

    QVBoxLayout *pMainLayout = nullptr;
    LPGPU2PPNewQtWidget(&pMainLayout, this);

    QLabel *pSelectLabel = nullptr;
    LPGPU2PPNewQtWidget(&pSelectLabel, PP_STR_SelectAPKToInstallLabel, this);
    
    LPGPU2PPNewQtWidget(&m_pApksListWidget, this);

    m_pApksListWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    // Initialise the list of APKs
    gtVector<gtString> availableApks;
    if (GetAvailableAPKs(availableApks) == PPFnStatus::success)
    {
        for (const auto &apkName : availableApks)
        {
            QListWidgetItem *apkListItem = nullptr;
            LPGPU2PPNewQtWidget(&apkListItem, acGTStringToQString(apkName), m_pApksListWidget);

            bReturn = PPFnStatus::success;
        }        
    }

    m_pApksListWidget->sortItems();

    LPGPU2PPNewQtWidget(&m_pInstallButton, PP_STR_InstallSelectedButton, this);
    m_pInstallButton->setEnabled(false);
    m_pCustomButtons->append(m_pInstallButton);

    LPGPU2PPNewQtWidget(&m_pStatusLabel, this);

    pMainLayout->addWidget(pSelectLabel);
    pMainLayout->addWidget(m_pApksListWidget);
    pMainLayout->addLayout(getBottomButtonLayout());
    pMainLayout->addWidget(m_pStatusLabel);
    pMainLayout->addStretch();

    connect(m_pApksListWidget, &QListWidget::itemSelectionChanged, this, &LPGPU2ppInstallAPKDialog::OnAPKListSelectionChanged);
    connect(m_pInstallButton, &QPushButton::clicked, this, &LPGPU2ppInstallAPKDialog::OnInstallAPKsClicked);

    connect(&LPGPU2ppADBCommands::Instance(), &LPGPU2ppADBCommands::InstallAPKRequested, this, &LPGPU2ppInstallAPKDialog::OnInstallAPKRequested);
    connect(&LPGPU2ppADBCommands::Instance(), &LPGPU2ppADBCommands::OnPreviousActionsCompleted, this, &LPGPU2ppInstallAPKDialog::OnPreviousActionsCompleted);

    return bReturn;
}

/// @brief  Clear out the resources used by this class.
/// @return PPFnStatus always return success.
PPFnStatus LPGPU2ppInstallAPKDialog::Shutdown()
{
    if (m_pApksListWidget != nullptr)
    {
        delete m_pApksListWidget;
        m_pApksListWidget = nullptr;
    }

    if (m_pInstallButton != nullptr)
    {
        delete m_pInstallButton;
        m_pInstallButton = nullptr;
    }

    if (m_pStatusLabel != nullptr)
    {
        delete m_pStatusLabel;
        m_pStatusLabel = nullptr;
    }

    // Need to clear this since the parent class won't
    if (m_pCustomButtons != nullptr)
    {
        m_pCustomButtons->clear();

        delete m_pCustomButtons;
        m_pCustomButtons = nullptr;
    }

    return PPFnStatus::success;
}

/// @brief                     Get the list of available APKs in the folder <CodeXL>/bin/LPGPU2_RAgent.
/// @param[out] vAvailableAPks The list of APKs available.
/// @return     PPFnStatus     success: The list was retrieved and it has at least one APK,
///                            failure: There is no APK available.
/// @warning                   Only files with the extension "*.apk" will be returned in the list.
PPFnStatus LPGPU2ppInstallAPKDialog::GetAvailableAPKs(gtVector<gtString> &vAvailableAPks)
{
    auto bReturn = PPFnStatus::failure;

    auto&& apksFolderPath = GetAPksFolderPath();
    if (apksFolderPath.exists())
    {
        osDirectory apksDir{ apksFolderPath };

        gtList<osFilePath> availableApks;
        const auto bCleanOutput = false;
        apksDir.getContainedFilePaths(PP_STR_APKFileExtensionW, osDirectory::SORT_BY_NAME_ASCENDING, availableApks, bCleanOutput);

        for (const auto &apkFilePath : availableApks)
        {
            gtString apkFileNameAndExtension;
            apkFilePath.getFileNameAndExtension(apkFileNameAndExtension);

            vAvailableAPks.push_back(apkFileNameAndExtension);
            bReturn = PPFnStatus::success;
        }
    }

    return bReturn;
}

/// @brief  Slot called when the current selection in the list of APKs changed.
void LPGPU2ppInstallAPKDialog::OnAPKListSelectionChanged()
{
    GT_IF_WITH_ASSERT(m_pApksListWidget != nullptr && m_pInstallButton != nullptr)
    {
        const auto bIsInstallingAPK = m_installApksGroupId != LPGPU2ppADBCommands::InvalidCommandGroupId;
        const auto&& selectedItems = m_pApksListWidget->selectedItems();
        m_pInstallButton->setEnabled(!selectedItems.isEmpty() && !bIsInstallingAPK);
    }    
}

/// @brief  Slot called when the user clicks on the button to install the selected APK.
///         This will trigger the installation on the device.
void LPGPU2ppInstallAPKDialog::OnInstallAPKsClicked()
{
    GT_IF_WITH_ASSERT(m_pApksListWidget != nullptr && m_pInstallButton != nullptr)
    {
        auto &adbCommands = LPGPU2ppADBCommands::Instance();

        // Trigger the installation        
        for (const auto &item : m_pApksListWidget->selectedItems())
        {
            auto &&apkFilePath = GetAPksFolderPath();
            const auto apkName = acQStringToGTString(item->text());
            apkFilePath.setFileName(apkName);
            
            GT_IF_WITH_ASSERT(apkFilePath.exists())
            {
                adbCommands.InstallAPK(apkFilePath);
            }
        }

        m_installApksGroupId = adbCommands.WaitForPreviousActions();
    }    
}

/// @brief  Slot called when the APK installation starts. This will display
///         The status label with the file path of the APK being installed.
void LPGPU2ppInstallAPKDialog::OnInstallAPKRequested(const QString &vApkPath)
{
    GT_IF_WITH_ASSERT(m_pInstallButton != nullptr && m_pStatusLabel != nullptr)
    {
        m_pInstallButton->setEnabled(false);
        m_pStatusLabel->show();
        m_pStatusLabel->setText(QString{ PP_STR_InstallingAPKLabel }.arg(vApkPath));
    }   
}

/// @brief                      Slot called when previous actions to the call of 
///                             LPGPU2ppADBCommands::WaitForPreviousActions() completed. 
///                             This is used to keep the UI responsive while some long 
///                             background task is running.
/// @param[in]  vCommandGroupId The group id as returned by a call to LPGPU2ppADBCommands::WaitForPreviousActions().
/// @see                        LPGPU2ppADBCommands::WaitForPreviousActions()
void LPGPU2ppInstallAPKDialog::OnPreviousActionsCompleted(LPGPU2ppADBCommands::CommandGroupId vCommandGroupId)
{
    if (m_installApksGroupId == vCommandGroupId)
    {
        m_pInstallButton->setEnabled(true);
        m_pStatusLabel->setText(PP_STR_APKWasInstalled);
        
        m_installApksGroupId = LPGPU2ppADBCommands::InvalidCommandGroupId;
    }
}

} // namespace lpgpu2