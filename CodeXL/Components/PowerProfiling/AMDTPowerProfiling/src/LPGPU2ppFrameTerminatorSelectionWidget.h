// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFrameTerminatorSelectionWidget.h
///
/// @brief Widget to display the available frame terminator choices
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_FRAMETERMINATORSELECTIONWIDGET_H
#define LPGPU2_PP_FRAMETERMINATORSELECTIONWIDGET_H

// Qt:
#include <QtWidgets>

// STL:
#include <vector>

// Framework:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

// Forward declarations
namespace lpgpu2 {
  enum class PPFnStatus;
} // namespace lpgpu2

namespace lpgpu2 {

/// @brief    Widget to display the available frame terminator choices for the
///           user.
/// @date     05/02/2018
/// @author   Alberto Taiuti.
class PP_API FrameTerminatorSelWidget final : public QWidget
{
  Q_OBJECT

// Methods
public:
  PPFnStatus Initialise();
  PPFnStatus Terminate();
  PPFnStatus ResetCheckboxes();
  PPFnStatus SetEnabled(bool isEnabled);
  PPFnStatus SetCheckboxesFromBitfield(gtUInt32 bitfield);
  PPFnStatus GetBitfieldFromCheckboxes(gtUInt32 &bitfield) const;
  bool IsExplicitFrameTermEnabled() const;
  PPFnStatus SetExplicitFrameTermChecked(bool isChecked);

// Type aliases
private:
  using TermsCheckboxesContainer =
    std::vector<QCheckBox *>;

// Attributes
private:
  QLabel *m_headerLabel = nullptr;
  QCheckBox *m_explicitFrameTermCheckbox = nullptr;
  gtUInt32 m_bitfield = 0U;
  TermsCheckboxesContainer m_termsCheckboxes;
  bool m_isWholeWidgetEnabled = true;

// Methods
private slots:
  void OnCheckboxStateChanged(int state);
  void OnExplicitCheckboxStateChanged(int state);

// Methods
private:
  PPFnStatus SetEnabledBitfieldCheckboxes(bool isEnabled);

};

} // namespace lpgpu2

#endif
