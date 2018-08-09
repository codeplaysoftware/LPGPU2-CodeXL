// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the feedback engine configuration dialog.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFeedbackEngineDialog.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFeedbackEngineOptionWidget.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

// Infra:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <LPGPU2Database/LPGPU2FeedbackEngine/LPGPU2_db_FeedbackEngine.h>

// Qt:
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>

using lpgpu2::PPFnStatus;

/// @brief  Defines the possible states of the feedback engine
/// @warning Internal use only
enum class LPGPU2ppFeedbackEngineDialog::EnumEngineStatusStyle
{
    kDefault,
    kGreen,
    kRed
};

// Static constants used to avoid typing true/false on the constructor
static constexpr bool g_bHasOkButton = false;
static constexpr bool g_bHasCancelButton = false;

/// @brief  Redirect the feedback engine output to CodeXL's Output window.
/// @param[in]  vFeedbackEngine The instance of the feedback engine to redirect the output.
/// @warning    Case afApplicationCommands::instance() this will redirect the output to stdout.
static void SetFeedbackEngineOutputCallback(lpgpu2::db::LPGPU2FeedbackEngine &vFeedbackEngine)
{
    auto *pAppCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pAppCommands != nullptr)
    {
        pAppCommands->ClearInformationView();
        vFeedbackEngine.SetOutputCallbackFunc([=](const std::string &str)
        {
            pAppCommands->AddStringToInformationView(QString::fromStdString(str));
        });
    }    
}

/// @brief  Display the Feedback Engine dialog for the session specified in vSessionId.
/// @param[in] vSessionId This is the session id as stored by CodeXL tree view.
/// @return PPFnStatus  success: The dialog was displayed and no error has occurred,
///                     failure: An error has occurred when initialising the dialog.
/// @see ProfileApplicationTreeHandler
PPFnStatus LPGPU2ppFeedbackEngineDialog::OpenLPGPU2ppFeedbackEngineDialog(const ExplorerSessionId vSessionId)
{
    auto *pAppCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pAppCommands != nullptr)
    {
        LPGPU2ppFeedbackEngineDialog dialog{ vSessionId };
        if (dialog.Initialise() == PPFnStatus::success)
        {
            pAppCommands->showModal(&dialog);
        }
        else
        {
            return PPFnStatus::failure;
        }
    }
    return PPFnStatus::success;
}

PPFnStatus LPGPU2ppFeedbackEngineDialog::OpenLPGPU2ppFeedbackEngineDialog(const osFilePath& vDbFilePath)
{
    auto *pAppCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pAppCommands != nullptr)
    {
        LPGPU2ppFeedbackEngineDialog dialog{ vDbFilePath };
        if (dialog.Initialise() == PPFnStatus::success)
        {
            pAppCommands->showModal(&dialog);
        }
        else
        {
            return PPFnStatus::failure;
        }
    }
    return PPFnStatus::success;
}

/// @brief  Class constructor. No work is one here.
/// @param[in] vSessionId This is the session id as stored by CodeXL tree view.
/// @param[in] vpParent To conform with the QWidget convention, this class accepts
///                     a pointer to another widget, but this pointer is not used.
/// @warning The dialog will the a child of the main window.
/// @see afMainAppWindow.
LPGPU2ppFeedbackEngineDialog::LPGPU2ppFeedbackEngineDialog(const ExplorerSessionId vSessionId, QWidget *vpParent /* = nullptr */)
    : acDialog{ afMainAppWindow::instance(), g_bHasOkButton, g_bHasCancelButton }
    , m_sessionId(vSessionId)
{
    GT_UNREFERENCED_PARAMETER(vpParent);    
}

LPGPU2ppFeedbackEngineDialog::LPGPU2ppFeedbackEngineDialog(const osFilePath& vDbFilePath, QWidget *vpParent /* = nullptr */)
    : acDialog{ afMainAppWindow::instance(), g_bHasOkButton, g_bHasCancelButton }
    , m_dbFilePath(vDbFilePath)
{
    GT_UNREFERENCED_PARAMETER(vpParent);
}

/// @brief   Class destructor. Calls Shutdown to free the resources used by this class.
LPGPU2ppFeedbackEngineDialog::~LPGPU2ppFeedbackEngineDialog()
{
    Shutdown();
}

/// @brief  Initialises the class elements and layout, also opens the database
///         of the associated session
/// @return PPFnStatus  success: The dialog was initialised and it is ready to be
///                               displayed,
///                     failure: An error has occurred during the construction
///                              of the dialog layout.
PPFnStatus LPGPU2ppFeedbackEngineDialog::Initialise()
{    
    if(m_pCustomButtons == nullptr)
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

    auto *pProfAppTreeHandler = ProfileApplicationTreeHandler::instance();
    GT_IF_WITH_ASSERT(pProfAppTreeHandler != nullptr)
    {
        const auto *pTreeItemData = pProfAppTreeHandler->GetSessionNodeItemData(m_sessionId);
        if (pTreeItemData != nullptr)
        {
            // This means we are inside a project so read the dbFilePath and store in the class            
            m_dbFilePath = pTreeItemData->m_filePath.asString();
        }        

        // If there is no tree item data available the file path has already been setup
        // but check if is exists just in case
        if (m_dbFilePath.exists())
        {
            if (InitViewLayout() == PPFnStatus::success)
            {
                const auto bIsReadOnly = false;
                if (m_dbAdapter.OpenDb(m_dbFilePath.asString(), AMDT_PROFILE_MODE_TIMELINE, bIsReadOnly) == lpgpu2::db::success)
                {
                    gtVector<gtString> availableFeedbackScripts;
                    if (lpgpu2::db::LPGPU2FeedbackEngine::GetAvailableFeedbackScripts(availableFeedbackScripts) == lpgpu2::db::success)
                    {
                        if (m_pScriptsComboBox != nullptr)
                        {
                            // Populates the combo box for the user to select the feedback script.
                            for (const auto &scriptName : availableFeedbackScripts)
                            {
                                m_pScriptsComboBox->addItem(scriptName.asASCIICharArray());
                            }

                            m_pLoadScriptButton->setEnabled(!availableFeedbackScripts.empty());

                            return PPFnStatus::success;
                        }
                    }
                }
            }
        }        
    }   
            
    return PPFnStatus::failure;
}

/// @brief  Clear out the resources used by this class.
PPFnStatus LPGPU2ppFeedbackEngineDialog::Shutdown()
{
    ClearFeedbackWidgets();

    if(m_pFeedbackOptionsWidget != nullptr)
    {
        delete m_pFeedbackOptionsWidget;
        m_pFeedbackOptionsWidget = nullptr;
    }

    if(m_pSelectAllCheckBox != nullptr)
    {
        delete m_pSelectAllCheckBox;
        m_pSelectAllCheckBox = nullptr;
    }

    if(m_pLoadScriptButton != nullptr)
    {
        delete m_pLoadScriptButton;
        m_pLoadScriptButton = nullptr;
    }

    if(m_pScriptsComboBox != nullptr)
    {
        delete m_pScriptsComboBox;
        m_pScriptsComboBox = nullptr;
    }

    if(m_pLoadedScriptNameLabel != nullptr)
    {
        delete m_pLoadedScriptNameLabel;
        m_pLoadedScriptNameLabel = nullptr;
    }
    
    if (m_dbAdapter.CloseDb() == lpgpu2::db::failure)
    {
        return PPFnStatus::failure;
    }

    // Need to clear this since the parent class won't
    if(m_pCustomButtons != nullptr)
    {
        m_pCustomButtons->clear();

        delete m_pCustomButtons;
        m_pCustomButtons = nullptr;
    }   
    
    return PPFnStatus::success;
}

/// @brief  Initialises the layout of the dialog
/// @returns    PPFnStatus  success if all widgets were initialised,
///                         failure if something wrong happened.
PPFnStatus LPGPU2ppFeedbackEngineDialog::InitViewLayout()
{
    LPGPU2PPNewQtWidget(&m_pRunEngineButton, PP_STR_FeedbackEngineDialog_Btn_RunLabel, this);
    m_pRunEngineButton->setEnabled(false);
    if(m_pCustomButtons != nullptr) 
    {
        m_pCustomButtons->append(m_pRunEngineButton);         
    }
    else
    {
        return PPFnStatus::failure;
    }

    // Set window flags to disable the help button on the dialog
    auto flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    // Set the title:
    setWindowTitle(PP_STR_FeedbackEngineDialog_WindowTitle);

    // Add the Icon to the dialog:
    afLoadTitleBarIcon(this);
    
    QVBoxLayout *pMainLayout = nullptr;
    LPGPU2PPNewQtWidget(&pMainLayout, this);

    QLabel *pSessionNameLabel = nullptr;
    LPGPU2PPNewQtWidget(&pSessionNameLabel, this);

    // Find out the name to display under the session label
    QString sessionNameText;
    auto *sessionTreeData = ProfileApplicationTreeHandler::instance()->GetSessionTreeNodeData(m_sessionId);
    if (sessionTreeData != nullptr)
    {
        sessionNameText = QString{ PP_STR_FeedbackEngineDialog_CurrentSessionLabel }.arg(sessionTreeData->m_displayName);        
    }
    else
    {
        sessionNameText = QString{ PP_STR_FeedbackEngineDialog_CurrentSessionLabel }.arg(acGTStringToQString(m_dbFilePath.asString()));        
    }

    pSessionNameLabel->setText(sessionNameText);

    QHBoxLayout *pStatusLayout = nullptr;
    LPGPU2PPNewQtWidget(&pStatusLayout, nullptr);

    QLabel *pStatusPrefixLabel = nullptr;
    LPGPU2PPNewQtWidget(&pStatusPrefixLabel, PP_STR_FeedbackEngineDialog_EngineStatusPrefixLabel)

    LPGPU2PPNewQtWidget(&m_pFeedbackEngineStatusLabel, this);
    SetEngineStatus(PP_STR_FeedbackEngineDialog_StatusIdle, EnumEngineStatusStyle::kGreen);

    pStatusLayout->addWidget(pStatusPrefixLabel);
    pStatusLayout->addWidget(m_pFeedbackEngineStatusLabel);
    pStatusLayout->addStretch();
    
    LPGPU2PPNewQtWidget(&m_pScriptsComboBox, this);    
    LPGPU2PPNewQtWidget(&m_pLoadScriptButton, PP_STR_FeedbackEngineDialog_Btn_LoadScriptLabel, this);        

    m_pScriptsComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    QLabel *pScriptComboLabel = nullptr;
    LPGPU2PPNewQtWidget(&pScriptComboLabel, PP_STR_FeedbackEngineDialog_ScriptComboLabel, this);

    QSpacerItem *pHeaderSpacer = nullptr;
    LPGPU2PPNewQtWidget(&pHeaderSpacer, 20, 30, QSizePolicy::Expanding, QSizePolicy::Minimum);

    QHBoxLayout *pHeaderLayout = nullptr;
    LPGPU2PPNewQtWidget(&pHeaderLayout, nullptr);

    pHeaderLayout->addWidget(pScriptComboLabel);
    pHeaderLayout->addWidget(m_pScriptsComboBox);
    pHeaderLayout->addItem(pHeaderSpacer);
    pHeaderLayout->addWidget(m_pLoadScriptButton);    

    // "Current loaded script" section

    QHBoxLayout *pLoadedScriptNameLayout = nullptr;
    LPGPU2PPNewQtWidget(&pLoadedScriptNameLayout, nullptr);

    LPGPU2PPNewQtWidget(&m_pLoadedScriptNameLabel, this);
    QLabel *pLoadedScriptPrefixLabel = nullptr;
    LPGPU2PPNewQtWidget(&pLoadedScriptPrefixLabel, PP_STR_FeedbackEngineDialog_CurrentLoadedScriptPrefixLabel, this);
    
    pLoadedScriptNameLayout->addWidget(pLoadedScriptPrefixLabel);
    pLoadedScriptNameLayout->addWidget(m_pLoadedScriptNameLabel);
    pLoadedScriptNameLayout->addStretch();
    
    // "Script Version" section

    QHBoxLayout *pScriptVersionLayout = nullptr;
    LPGPU2PPNewQtWidget(&pScriptVersionLayout, nullptr);

    QLabel *pFeedbackEngineVersionPrefixLabel = nullptr;
    LPGPU2PPNewQtWidget(&pFeedbackEngineVersionPrefixLabel, PP_STR_FeedbackEngineDialog_ScriptVersionPrefixLabel);
    
    LPGPU2PPNewQtWidget(&m_pFeedbackEngineVersionLabel, PP_STR_FeedbackEngineDialog_ScriptVersionPlaceholder, this);

    pScriptVersionLayout->addWidget(pFeedbackEngineVersionPrefixLabel);
    pScriptVersionLayout->addWidget(m_pFeedbackEngineVersionLabel);
    pScriptVersionLayout->addStretch();

    // "Available Feedbacks" section

    LPGPU2PPNewQtWidget(&m_pAvailableFeedbacksGroupBox, PP_STR_FeedbackEngineDialog_AvailableFeedbackGroupBoxTitle, this);

    LPGPU2PPNewQtWidget(&m_pSelectAllCheckBox, PP_STR_FeedbackEngineDialog_SelectAllCheckBoxLabel, m_pAvailableFeedbacksGroupBox);
    const auto bIsTriState = false;
    m_pSelectAllCheckBox->setTristate(bIsTriState);

    LPGPU2PPNewQtWidget(&m_pFeedbackOptionsWidget, m_pAvailableFeedbacksGroupBox);
    QVBoxLayout *pFeedbackOptionsLayout = nullptr;
    LPGPU2PPNewQtWidget(&pFeedbackOptionsLayout, m_pFeedbackOptionsWidget);

    QVBoxLayout *pSelectFeedbackGroupBoxLayout = nullptr;
    LPGPU2PPNewQtWidget(&pSelectFeedbackGroupBoxLayout, m_pAvailableFeedbacksGroupBox);

    pSelectFeedbackGroupBoxLayout->addWidget(m_pSelectAllCheckBox);
    pSelectFeedbackGroupBoxLayout->addWidget(m_pFeedbackOptionsWidget);    
    
    pMainLayout->addWidget(pSessionNameLabel);
    pMainLayout->addLayout(pStatusLayout);
    pMainLayout->addLayout(pLoadedScriptNameLayout);
    pMainLayout->addLayout(pScriptVersionLayout);
    pMainLayout->addLayout(pHeaderLayout);    
    pMainLayout->addWidget(m_pAvailableFeedbacksGroupBox);
    pMainLayout->addLayout(getBottomButtonLayout());
    pMainLayout->addStretch();

    connect(m_pLoadScriptButton, &QPushButton::clicked, this, &LPGPU2ppFeedbackEngineDialog::OnLoadScript);
    connect(m_pRunEngineButton, &QPushButton::clicked, this, &LPGPU2ppFeedbackEngineDialog::OnRunFeedbackEngine);    

    m_pAvailableFeedbacksGroupBox->setVisible(false);

    return PPFnStatus::success;
}

/// @brief  Clear all the widgets created to display the options for the
///         feedback script that was loaded.
/// @return PPFnStatus success: The widgets were deleted,
///                    failure: an exception was thrown during the widget cleanup.
PPFnStatus LPGPU2ppFeedbackEngineDialog::ClearFeedbackWidgets()
{
    try
    {
        for (auto *feedbackWidget : m_feedbackWidgetsList)
        {
            delete feedbackWidget;
        }
        m_feedbackWidgetsList.clear();
        m_feedbackOptionsList.clear();
    }
    catch(...)
    {
        return PPFnStatus::failure;
    }   

    return PPFnStatus::success;
}

/// @brief      Set the engine status text and style based on an enum.
/// @param[in]  vEngineStatusText The status text to be displayed on the dialog.
/// @param[in]  vEngineStatusStyle An enum describing the text style.
/// @return     PPFnStatus  success: The label text and style was configured successfully,
///                         failure: An error has occurred while setting the label style.
PPFnStatus LPGPU2ppFeedbackEngineDialog::SetEngineStatus(const QString &vEngineStatusText, const EnumEngineStatusStyle vEngineStatusStyle) const
{
    auto bReturn = PPFnStatus::failure;

    if(m_pFeedbackEngineStatusLabel != nullptr)
    {
        QString formatedText{ PP_STR_FeedbackEngineDialog_StatusFormattedText };

        switch(vEngineStatusStyle)
        {
            case EnumEngineStatusStyle::kGreen:
                formatedText = formatedText.arg(PP_STR_FeedbackEngineDialog_StatusStyleGreen).arg(vEngineStatusText);
                break;

            case EnumEngineStatusStyle::kRed:
                formatedText = formatedText.arg(PP_STR_FeedbackEngineDialog_StatusStyleRed).arg(vEngineStatusText);
                break;            

            case EnumEngineStatusStyle::kDefault:
            default:
                formatedText = formatedText.arg(PP_STR_FeedbackEngineDialog_StatusStyleDefault).arg(vEngineStatusText);
                break;                
        }

        m_pFeedbackEngineStatusLabel->setText(formatedText);
        
        //FIXME: This is a hack to update the GUI immediately.
        qApp->processEvents();

        bReturn = PPFnStatus::success;
    }

    return bReturn;
}

/// @brief           Gets the absolute path of the current selected script.
/// @return gtString The absolute path of the current selected script.
gtString LPGPU2ppFeedbackEngineDialog::GetCurrentScriptFullPath() const
{
    osFilePath scriptFullPath{ osFilePath::OS_CODEXL_BINARIES_PATH };
    scriptFullPath.appendSubDirectory(PP_STR_FeedbackEngineScriptsFolder_W);
    scriptFullPath.setFileName(m_loadedScriptName);

    return scriptFullPath.asString();
}

/// @brief  This slot is called when a the loading of a script is required.
///         It populates the GUI with the options available from the script,
///         so the user can configure how the script will run.
/// @return PPFnStatus  success: The feedback script was loaded and all the GUI
///                              was populated with all the available options,
///                     failure: An error has occurred during the GUI initialisation.
PPFnStatus LPGPU2ppFeedbackEngineDialog::OnLoadScript()
{    
    if (m_pScriptsComboBox != nullptr && 
        m_pLoadedScriptNameLabel != nullptr && 
        m_pSelectAllCheckBox != nullptr && 
        m_pAvailableFeedbacksGroupBox != nullptr)
    {
        const auto currentScript = m_pScriptsComboBox->currentText();
        m_loadedScriptName = acQStringToGTString(currentScript);
        m_pLoadedScriptNameLabel->setText(currentScript);

        // Uncheck the select all checkbox because all the options are going to be created again
        m_pSelectAllCheckBox->setChecked(false);

        lpgpu2::db::LPGPU2FeedbackEngine feedbackEngine{ m_dbAdapter };

        // Redirect the script output to CodeXL output window
        SetFeedbackEngineOutputCallback(feedbackEngine);

        SetEngineStatus(PP_STR_FeedbackEngineDialog_StatusLoadingOptions, EnumEngineStatusStyle::kGreen);

        const auto scriptFullPath = GetCurrentScriptFullPath();

        // Read the script version
        gtString feedbackScriptVersion;
        feedbackEngine.GetFeedbackScriptVersion(scriptFullPath, feedbackScriptVersion);
        m_pFeedbackEngineVersionLabel->setText(acGTStringToQString(feedbackScriptVersion));

        gtVector<lpgpu2::db::AvailableFeedback> availableFeedbackOptions;
        const auto bGetAvailableFeedbackOptionsReturn = feedbackEngine.GetAvailableFeedbackOptions(scriptFullPath, availableFeedbackOptions);
        if(!bGetAvailableFeedbackOptionsReturn)
        {
            SetEngineStatus(PP_STR_FeedbackEngineDialog_StatusLoadingOptionsError, EnumEngineStatusStyle::kRed);
        }        
        else
        {
            // Remove all options that contain "shader_counter_match" in its name.
            // These options are not to be displayed to the user.
            const auto removeIt = std::remove_if(availableFeedbackOptions.begin(), availableFeedbackOptions.end(), [](const lpgpu2::db::AvailableFeedback &feedback)
            {
                return feedback.scriptFunction.find(PP_STR_LPGPU2_ShaderMatch_FeedbackScriptName) != -1;
            });
            availableFeedbackOptions.erase(removeIt, availableFeedbackOptions.end());

            if(ClearFeedbackWidgets() == PPFnStatus::success)
            {
                // This map is used to quickly find the QGroupBox based on an option category name.
                QMap<gtString, QGroupBox*> optionsCategoriesMap;

                for (const auto& feedbackOption : availableFeedbackOptions)
                {
                    QGroupBox *pCategoryGroupBox = nullptr;
                    QVBoxLayout *pCategoryGroupBoxLayout = nullptr;

                    // Try to find an existing QGroupBox for an option category
                    if (optionsCategoriesMap.contains(feedbackOption.scriptCatagory))
                    {
                        pCategoryGroupBox = optionsCategoriesMap.value(feedbackOption.scriptCatagory);

                        // This cast will never fail because all the layouts for the group boxes
                        // are QVBoxLayout's.
                        pCategoryGroupBoxLayout = static_cast<QVBoxLayout*>(pCategoryGroupBox->layout());
                    }
                    else
                    {
                        // Create a QGroupBox to display the options for the category.
                        LPGPU2PPNewQtWidget(&pCategoryGroupBox, acGTStringToQString(feedbackOption.scriptCatagory), m_pFeedbackOptionsWidget);
                        LPGPU2PPNewQtWidget(&pCategoryGroupBoxLayout, pCategoryGroupBox);

                        optionsCategoriesMap[feedbackOption.scriptCatagory] = pCategoryGroupBox;

                        m_pFeedbackOptionsWidget->layout()->addWidget(pCategoryGroupBox);

                        // Store the QGroupBx widget so we can delete it later
                        m_feedbackWidgetsList.append(pCategoryGroupBox);
                    }

                    // This will create the dedicated widget do display an option
                    LPGPU2ppFeedbackEngineOptionWidget *pOptionWidget = nullptr;
                    LPGPU2PPNewQtLPGPU2Widget(&pOptionWidget,
                                              feedbackOption.scriptFunction,
                                              feedbackOption.scriptName,
                                              feedbackOption.inputName,
                                              feedbackOption.inputDefault,
                                              feedbackOption.inputMax,
                                              feedbackOption.inputMin,
                                              feedbackOption.inputDescription,
                                              pCategoryGroupBox);

                    // Store the option widget so we can delete it later
                    m_feedbackOptionsList.append(pOptionWidget);

                    // Connect the select all checkbox event to enable the option widgets.
                    connect(m_pSelectAllCheckBox, &QCheckBox::stateChanged, pOptionWidget, &LPGPU2ppFeedbackEngineOptionWidget::SetOptionEnabled);
                    connect(pOptionWidget, &LPGPU2ppFeedbackEngineOptionWidget::OptionsEnabledChanged, this, &LPGPU2ppFeedbackEngineDialog::OnOptionEnabledChanged);

                    // Finally add the option widget to the group box layout
                    pCategoryGroupBoxLayout->addWidget(pOptionWidget);
                }

                // All options were loaded, set the engine status to idle
                SetEngineStatus(PP_STR_FeedbackEngineDialog_StatusIdle, EnumEngineStatusStyle::kGreen);
                m_pAvailableFeedbacksGroupBox->setVisible(true);
            }
        }        
    }

    return PPFnStatus::success;
}

/// @brief  Slot triggered to run the feedback engine with the selected options.
/// @return PPFnStatus  success: The feedback engine was executed successfully,
///                     failure: An error has occurred during the feedback engine
///                              run phase.
/// @warning m_sessionId Must be a valid session identifier.
PPFnStatus LPGPU2ppFeedbackEngineDialog::OnRunFeedbackEngine()
{
    auto bReturn = PPFnStatus::failure;

    // Builds the list of feedback options 
    gtVector<lpgpu2::db::FeedbackOption> feedbackOptions;
    for (const auto *feedbackOptionWidget : m_feedbackOptionsList)
    {
        if(feedbackOptionWidget != nullptr && feedbackOptionWidget->IsOptionEnabled())
        {
            lpgpu2::db::FeedbackOption feedbackOption;
            feedbackOption.functionName = feedbackOptionWidget->GetOptionName();
                
            if(feedbackOptionWidget->HasArg())
            {
                feedbackOption.functionArg.push_back(feedbackOptionWidget->GetArgValue());
            }                

            feedbackOptions.push_back(feedbackOption);
        }
    }

    SetEngineStatus(PP_STR_FeedbackEngineDialog_StatusRunning, EnumEngineStatusStyle::kGreen);
        
    lpgpu2::db::LPGPU2FeedbackEngine feedbackEngine{ m_dbAdapter };

    // Redirect the script output to CodeXL output window
    SetFeedbackEngineOutputCallback(feedbackEngine);

    const auto scriptFullPath = GetCurrentScriptFullPath();

    const auto bFeedbackEngineRunReturn = feedbackEngine.Run(scriptFullPath, feedbackOptions);
    if(!bFeedbackEngineRunReturn)
    {
        const auto feedbackEngineErrorMsg = feedbackEngine.GetErrorMessage();
        if(!feedbackEngineErrorMsg.isEmpty())
        {
            SetEngineStatus(acGTStringToQString(feedbackEngineErrorMsg), EnumEngineStatusStyle::kRed);
        }
        else
        {
            SetEngineStatus(PP_STR_FeedbackEngineDialog_StatusRunningError, EnumEngineStatusStyle::kRed);
        }            
    }
    else
    {
        SetEngineStatus(PP_STR_FeedbackEngineDialog_StatusRunSuccess, EnumEngineStatusStyle::kGreen);
        bReturn = PPFnStatus::success;
    }        
    

    return bReturn;
}

/// @brief Slot called when the user enable or disable a feedback option. This is used to enable
///        or disable the Run feedback engine button.
/// @param[in] vbOptionEnabled Indicates if the option that triggered this slot was enabled or disabled.
/// @return    PPFnStatus      success: The run button enabled state was configured successfully,
///                            failure: An error has occurred while processing the slot.
PPFnStatus LPGPU2ppFeedbackEngineDialog::OnOptionEnabledChanged(const bool vbOptionEnabled)
{
    auto bReturn = PPFnStatus::failure;

    if(m_pRunEngineButton != nullptr)
    {
        if (vbOptionEnabled == false)
        {
            // In case the option was disabled, we need to check if there is at least one option enabled to enable/disable the run button.
            const auto bAnyOptionIsEnabled = std::any_of(m_feedbackOptionsList.begin(), m_feedbackOptionsList.end(), [](LPGPU2ppFeedbackEngineOptionWidget *pOptionWidget)
            {
                return pOptionWidget != nullptr && pOptionWidget->IsOptionEnabled();
            });
            m_pRunEngineButton->setEnabled(bAnyOptionIsEnabled);
        }
        else
        {
            // In case it was enabled the run engine button can be enabled.
            m_pRunEngineButton->setEnabled(true);
        }

        bReturn = PPFnStatus::success;
    }    

    return bReturn;
}
