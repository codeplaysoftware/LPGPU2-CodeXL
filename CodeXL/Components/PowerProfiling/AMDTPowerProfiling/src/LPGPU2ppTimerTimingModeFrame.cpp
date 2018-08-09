// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppTimerTimingModeFrame.cpp
///
/// @brief Represents a frame displaying the remaining time when the profiling
///        session uses a timeout to stop profiling
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppTimerTimingModeFrame.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

// Infra:
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acColours.h>

// Qt:
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QElapsedTimer>

// STL
#include <chrono>

static const QString kExplanationLabel = QString{"Time left:"};
constexpr auto kUpdateDuration = std::chrono::milliseconds(50);

namespace lpgpu2 {

/// @brief              Class constructor. No work is performed here.
/// @param[in] vpParent The parent of this frame.
ppTimerTimingModeFrame::ppTimerTimingModeFrame(
    QWidget* vpParent /* = nullptr */)
: QFrame{ vpParent }
{
}

/// @brief Class destructor. Calls the Shutdown method to cleanup
///        resources used by this class.
ppTimerTimingModeFrame::~ppTimerTimingModeFrame()
{
  Shutdown();
}

/// @brief  Initialises the components and the layout of this widget.
/// @return PPFnStatus  success: If the components were initialised,
///                     failure: An error has occurred.
PPFnStatus ppTimerTimingModeFrame::Initialise(const QTimer *timerToReadFrom)
{
  GT_ASSERT(timerToReadFrom);
  if (!timerToReadFrom)
  {
    return PPFnStatus::failure;
  }

  m_pTimerToReadFrom = timerToReadFrom;

  QLabel *pPixmapLabel = nullptr;
  LPGPU2PPNewQtWidget(&pPixmapLabel, this);
  QPixmap infoIcon;
  acSetIconInPixmap(infoIcon, AC_ICON_WARNING_INFO);
  pPixmapLabel->setPixmap(infoIcon);

  QLabel *pTextExplanationLabel = nullptr;
  LPGPU2PPNewQtWidget(&pTextExplanationLabel,
      kExplanationLabel, this);

  LPGPU2PPNewQtWidget(&m_pTimerCountdownBox,
      this);
  m_pTimerCountdownBox->setReadOnly(true);
  m_pTimerCountdownBox->setText(
        QString::number(timerToReadFrom->interval(), 10));

  QHBoxLayout *pMainLayout = nullptr;
  LPGPU2PPNewQtWidget(&pMainLayout, this);
  pMainLayout->addWidget(pPixmapLabel);
  pMainLayout->addWidget(pTextExplanationLabel);
  pMainLayout->addWidget(m_pTimerCountdownBox);
  pMainLayout->addStretch();
  //pMainLayout->addWidget(m_pNoRegionLabel);
  //pMainLayout->addStretch();
  //pMainLayout->addWidget(m_pRunFeedbackEngineButton);

  // Set the background color
  //auto framePalette = palette();
  //framePalette.setColor(backgroundRole(), acYELLOW_INFO_COLOUR);
  //framePalette.setColor(QPalette::Base, acYELLOW_INFO_COLOUR);
  setAutoFillBackground(true);
  //setPalette(framePalette);
  setMinimumHeight(40);
  setFrameStyle(Panel);
  setFrameShape(Panel);

  LPGPU2PPNewQtWidget(&m_pInternalUpdateTimer, this);
  connect(m_pInternalUpdateTimer, SIGNAL(timeout()), this,
      SLOT(OnUpdateTimerTimeout()));
  //connect(m_pRunFeedbackEngineButton, &QPushButton::clicked, this, &LPGPU2ppTimerTimingModeFrame::RunFeedbackButtonClicked);

  LPGPU2PPNewQtWidgetNoParent(&m_pInternalElapsedTimer);

  return PPFnStatus::success;
}

/// @brief             Cleanup resources used by this class.
/// @return PPFnStatus Always return success.
PPFnStatus ppTimerTimingModeFrame::Shutdown()
{
  if(m_pTimerCountdownBox!= nullptr)
  {
    delete m_pTimerCountdownBox;
    m_pTimerCountdownBox= nullptr;
  }

  return PPFnStatus::success;
}

/// @brief Called when a session starts. Starts the internal update timers so
///        that the GUI can be updated
/// @return Always PPFnStatus::Success
PPFnStatus ppTimerTimingModeFrame::StartedSession()
{
  GT_ASSERT(m_pTimerToReadFrom);

  setVisible(true);
  m_isSessionRunning = true;

  m_pInternalElapsedTimer->start();
  OnUpdateTimerTimeout();

  return PPFnStatus::success;
}

/// @brief Called when a session stops. Stops the internal update timers so
///        that the GUI can be updated
/// @return Always PPFnStatus::Success
PPFnStatus ppTimerTimingModeFrame::StoppedSession()
{
  GT_ASSERT(m_pTimerToReadFrom);

  m_isSessionRunning = false;
  setVisible(false);

  m_pInternalUpdateTimer->stop();
  m_pTimerCountdownBox->setText(
        QString::number(0, 10));

  return PPFnStatus::success;
}

/// @brief Periodically called by the internal timer to update the GUI
void ppTimerTimingModeFrame::OnUpdateTimerTimeout()
{
  GT_ASSERT(m_pTimerToReadFrom);

  const auto remainingTimeMS = m_pTimerToReadFrom->interval() -
    m_pInternalElapsedTimer->elapsed();

  m_pTimerCountdownBox->setText(
        QString::number(remainingTimeMS, 10));

  if (!m_isSessionRunning)
  {
    return;
  }

  m_pInternalUpdateTimer->start(kUpdateDuration.count());
}

} // namespace lpgpu2
