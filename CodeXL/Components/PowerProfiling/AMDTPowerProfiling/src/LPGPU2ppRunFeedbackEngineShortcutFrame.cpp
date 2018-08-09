// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the frame to display a shortcut to run the feedback engine
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppRunFeedbackEngineShortcutFrame.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

// Infra:
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acColours.h>

// Qt:
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

/// @brief              Class constructor. No work is performed here.
/// @param[in] vpParent The parent of this frame.
LPGPU2ppRunFeedbackEngineShortcutFrame::LPGPU2ppRunFeedbackEngineShortcutFrame(QWidget* vpParent /* = nullptr */)
    : QFrame{ vpParent }
{
}

/// @brief Class destructor. Calls the Shutdown method to cleanup
///        resources used by this class.
LPGPU2ppRunFeedbackEngineShortcutFrame::~LPGPU2ppRunFeedbackEngineShortcutFrame()
{
    Shutdown();
}

/// @brief  Initialises the components and the layout of this widget.
/// @return PPFnStatus  success: If the components were initialised,
///                     failure: An error has occurred.
lpgpu2::PPFnStatus LPGPU2ppRunFeedbackEngineShortcutFrame::Initialise()
{
    QLabel *pPixmapLabel = nullptr;
    LPGPU2PPNewQtWidget(&pPixmapLabel, this);
    QPixmap infoIcon;
    acSetIconInPixmap(infoIcon, AC_ICON_WARNING_INFO);
    pPixmapLabel->setPixmap(infoIcon);

    LPGPU2PPNewQtWidget(&m_pNoRegionLabel, PP_STR_RunFeedbackEngineShortcutFrame_NoFeedbackLabel, this);
    LPGPU2PPNewQtWidget(&m_pRunFeedbackEngineButton, PP_STR_RunFeedbackEngineShortcutFrame_RunButton, this);
    
    QHBoxLayout *pMainLayout = nullptr;
    LPGPU2PPNewQtWidget(&pMainLayout, this);
    pMainLayout->addWidget(pPixmapLabel);
    pMainLayout->addStretch();
    pMainLayout->addWidget(m_pNoRegionLabel);
    pMainLayout->addStretch();
    pMainLayout->addWidget(m_pRunFeedbackEngineButton);

    // Set the background color:    
    auto framePalette = palette();
    framePalette.setColor(backgroundRole(), acYELLOW_INFO_COLOUR);
    framePalette.setColor(QPalette::Base, acYELLOW_INFO_COLOUR);
    setAutoFillBackground(true);
    setPalette(framePalette);
    setMinimumHeight(40);
    setFrameStyle(Panel);
    setFrameShape(Panel);

    connect(m_pRunFeedbackEngineButton, &QPushButton::clicked, this, &LPGPU2ppRunFeedbackEngineShortcutFrame::RunFeedbackButtonClicked);

    return lpgpu2::PPFnStatus::success;
}

/// @brief             Cleanup resources used by this class.
/// @return PPFnStatus Always return success.
lpgpu2::PPFnStatus LPGPU2ppRunFeedbackEngineShortcutFrame::Shutdown()
{
    if(m_pNoRegionLabel != nullptr)
    {
        delete m_pNoRegionLabel;
        m_pNoRegionLabel = nullptr;
    }

    if(m_pRunFeedbackEngineButton != nullptr)
    {
        delete m_pRunFeedbackEngineButton;
        m_pNoRegionLabel = nullptr;
    }

    return lpgpu2::PPFnStatus::success;
}


