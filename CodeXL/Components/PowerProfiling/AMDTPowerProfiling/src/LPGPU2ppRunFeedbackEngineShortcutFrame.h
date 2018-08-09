// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the frame to display a shortcut to run the feedback engine
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_RUNFEEDBACKENGINE_SHORTCUT_FRAME_H_INCLUDE
#define LPGPU2_PP_RUNFEEDBACKENGINE_SHORTCUT_FRAME_H_INCLUDE

// Qt:
#include <QFrame>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

// Declarations:
class QLabel;
class QPushButton;

/// @brief   Represents the frame to display the run feedback engine
///          shortcut when opening a session that does not have the
///          feedback yet.
/// @warning None.
/// @author  Thales Sabino.
/// @date    30/11/2017
// clang-format off
class LPGPU2ppRunFeedbackEngineShortcutFrame final : public QFrame
{
    Q_OBJECT
// Methods
public:
    explicit LPGPU2ppRunFeedbackEngineShortcutFrame(QWidget *vpParent = nullptr);
    ~LPGPU2ppRunFeedbackEngineShortcutFrame() override;

    lpgpu2::PPFnStatus Initialise();
    lpgpu2::PPFnStatus Shutdown();

// Signals:
signals:
    void RunFeedbackButtonClicked();

// Attributes:
private:
    QLabel      *m_pNoRegionLabel = nullptr;
    QPushButton *m_pRunFeedbackEngineButton = nullptr;
    QPushButton *m_pCloseButton = nullptr;
};
// clang-format on

#endif // LPGPU2_PP_RUNFEEDBACKENGINE_SHORTCUT_FRAME_H_INCLUDE