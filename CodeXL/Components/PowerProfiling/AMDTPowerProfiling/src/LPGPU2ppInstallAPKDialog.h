// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the feedback engine configuration dialog.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_INSTALL_RAGENT_DIALOG_H_INCLUDE
#define LPGPU2_PP_INSTALL_RAGENT_DIALOG_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTApplicationComponents/Include/acDialog.h>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppADBCommands.h>

namespace lpgpu2 {

enum class PPFnStatus;

/// @brief   Displays a dialogue that allows the user to select an APK to install from a list.
///          The list of APKs will be the ones available in the folder <CodeXL>/bin/LPGPU2_RAgent.
/// @warning To display the dialog, use LPGPU2ppInstallAPKDialog::OpenLPGPU2ppInstallAPKDialog().
/// @author  Thales Sabino.
/// @date    05/03/2018
// clang-format off
class LPGPU2ppInstallAPKDialog final : public acDialog
{
    Q_OBJECT

// Static Methods
public:
    static PPFnStatus OpenLPGPU2ppInstallAPKDialog();

// Methods
public:
    explicit LPGPU2ppInstallAPKDialog(QWidget *vpParent = nullptr);
    ~LPGPU2ppInstallAPKDialog() override;

    PPFnStatus Initialise();
    PPFnStatus Shutdown();

// Methods
private:
    PPFnStatus InitViewLayout();    
    PPFnStatus GetAvailableAPKs(gtVector<gtString> &vAvailableAPks);    

// Slots
private slots:
    void OnAPKListSelectionChanged();
    void OnInstallAPKsClicked();
    void OnInstallAPKRequested(const QString &vApkPath);
    void OnPreviousActionsCompleted(LPGPU2ppADBCommands::CommandGroupId vCommandGroupId);

// Attributes
private:
    QListWidget *m_pApksListWidget = nullptr;
    QPushButton *m_pInstallButton = nullptr;
    QLabel *m_pStatusLabel = nullptr;
    LPGPU2ppADBCommands::CommandGroupId m_installApksGroupId = LPGPU2ppADBCommands::InvalidCommandGroupId;
};
// clang-format on

} // namespace lpgpu2


#endif // LPGPU2_PP_INSTALL_RAGENT_DIALOG_H_INCLUDE