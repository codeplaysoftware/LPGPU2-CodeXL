// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppTimerTimingModeFrame.h
///
/// @brief Represents a frame displaying the remaining time when the profiling
///        session uses a timeout to stop profiling
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_TIMERTIMINGMODEFRAME_H_INCLUDE
#define LPGPU2_PP_TIMERTIMINGMODEFRAME_H_INCLUDE

// Qt
#include <QFrame>

// Local
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

// Forward declarations
class QLineEdit;
class QTimer;
class QElapsedTimer;

namespace lpgpu2 {

/// @brief   Represents the frame to display the remaining time for a
///          profiling session when the timing mode is Timer-based.
/// @warning None.
/// @author  Alberto Taiuti.
/// @date    22/10/2018
// clang-format off
class ppTimerTimingModeFrame final : public QFrame
{
  Q_OBJECT
// Methods
public:
  explicit ppTimerTimingModeFrame(QWidget *vpParent = nullptr);
  ~ppTimerTimingModeFrame() override;

  PPFnStatus Initialise(const QTimer *timerToReadFrom);
  PPFnStatus Shutdown();

  PPFnStatus StartedSession();
  PPFnStatus StoppedSession();

// Signals
signals:
  void RunFeedbackButtonClicked();

// Slots
private slots:
  void OnUpdateTimerTimeout();

// Attributes
private:
  QLineEdit *m_pTimerCountdownBox = nullptr;
  const QTimer *m_pTimerToReadFrom = nullptr;
  QTimer *m_pInternalUpdateTimer = nullptr;
  QElapsedTimer *m_pInternalElapsedTimer = nullptr;
  bool m_isSessionRunning = false;

};
// clang-format on

} // namespace lpgpu2

#endif // LPGPU2_PP_TIMERTIMINGMODEFRAME_H_INCLUDE
