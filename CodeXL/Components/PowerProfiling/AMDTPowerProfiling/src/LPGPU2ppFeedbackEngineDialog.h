// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the feedback engine configuration dialog.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_FEEDBACK_ENGINE_DIALOG_H_INCLUDE
#define LPGPU2_PP_FEEDBACK_ENGINE_DIALOG_H_INCLUDE

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acDialog.h>
#include <AMDTSharedProfiling/inc/SessionExplorerDefs.h>

// LPGPU2Database
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>

namespace lpgpu2 { enum class PPFnStatus; }
class QPushButton;
class QCheckBox;
class QVBoxLayout;
class afApplicationTreeItemData;
class LPGPU2ppFeedbackEngineOptionWidget;

/// @brief    This is the entry point to the feedback engine as seen by the user.
///           It handles the selection of which feedback script to run and display
///           all the options available for a certain feedback script.
/// @warning  To display the dialog, use LPGPU2ppFeedbackEngineDialog::OpenLPGPU2ppFeedbackEngineDialog().
/// @see      ExplorerSessionId.
/// @author   Thales Sabino.
/// @date     15/11/2017
// clang-format off
class LPGPU2ppFeedbackEngineDialog final : public acDialog
{
    Q_OBJECT
// Static Methods
public:
    static lpgpu2::PPFnStatus OpenLPGPU2ppFeedbackEngineDialog(const ExplorerSessionId vSessionId);
    static lpgpu2::PPFnStatus OpenLPGPU2ppFeedbackEngineDialog(const osFilePath& vDbFilePath);

// Methods
public:
    explicit LPGPU2ppFeedbackEngineDialog(const ExplorerSessionId vSessionId, QWidget *vpParent = nullptr);
    explicit LPGPU2ppFeedbackEngineDialog(const osFilePath& vDbFilePath, QWidget* vpParent = nullptr);
    ~LPGPU2ppFeedbackEngineDialog() override;

    lpgpu2::PPFnStatus Initialise();
    lpgpu2::PPFnStatus Shutdown();    

// Declarations
private:
    enum class EnumEngineStatusStyle;

// Methods
private:
    lpgpu2::PPFnStatus InitViewLayout();
    lpgpu2::PPFnStatus ClearFeedbackWidgets();
    lpgpu2::PPFnStatus SetEngineStatus(const QString &vEngineStatusText, EnumEngineStatusStyle vEngineStatus) const;
    gtString GetCurrentScriptFullPath() const;

// Methods
private slots:
    lpgpu2::PPFnStatus OnLoadScript();
    lpgpu2::PPFnStatus OnRunFeedbackEngine();
    lpgpu2::PPFnStatus OnOptionEnabledChanged(const bool vbOptionEnabled);

// Attributes
private:    
    ExplorerSessionId m_sessionId = SESSION_ID_ERROR;
    osFilePath m_dbFilePath;
    gtString m_loadedScriptName;

    lpgpu2::db::LPGPU2DatabaseAdapter m_dbAdapter;    

    QList<LPGPU2ppFeedbackEngineOptionWidget*> m_feedbackOptionsList;
    QList<QWidget*> m_feedbackWidgetsList;
    
    QLabel *m_pLoadedScriptNameLabel = nullptr;
    QComboBox *m_pScriptsComboBox = nullptr;
    QPushButton *m_pLoadScriptButton = nullptr;
    QGroupBox *m_pAvailableFeedbacksGroupBox = nullptr;
    QCheckBox *m_pSelectAllCheckBox = nullptr;
    QWidget *m_pFeedbackOptionsWidget = nullptr;
    QLabel *m_pFeedbackEngineStatusLabel = nullptr;
    QLabel *m_pFeedbackEngineVersionLabel = nullptr;
    QPushButton *m_pRunEngineButton = nullptr;
};
// clang-format on

#endif // LPGPU2_PP_FEEDBACK_ENGINE_DIALOG_H_INCLUDE