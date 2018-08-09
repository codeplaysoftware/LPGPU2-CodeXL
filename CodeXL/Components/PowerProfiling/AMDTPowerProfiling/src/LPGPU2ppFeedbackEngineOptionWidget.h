// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the feedback engine configuration dialog.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_FEEDBACK_OPTION_WIDGET_H_INCLUDE
#define LPGPU2_PP_FEEDBACK_OPTION_WIDGET_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Qt:
#include <QWidget>

// Forward declarations:
namespace lpgpu2 { enum class PPFnStatus; }
class QCheckBox;
class QSpinBox;

/// @brief  Each LPGPU2ppFeedbackEngineOptionWidget represents an entry
///         on the LPGPU2ppFeedbackEngineDialog view. Each option is
///         represented by check box, so it can be enabled or disabled
///         and an optional spin box if the option accepts an input
///         parameter.
/// @see    LPGPU2ppFeedbackEngineDialog
/// @author Thales Sabino
/// @date   15/11/2017
// clang-format off
class LPGPU2ppFeedbackEngineOptionWidget : public QWidget
{
    Q_OBJECT

// Methods
public:
    LPGPU2ppFeedbackEngineOptionWidget(const gtString &vOptionName,
                                       const gtString &vOptionDisplayName,
                                       const gtString &vArgName = L"",
                                       const gtInt32 vArgValue = -1,
                                       const gtInt32 vArgMaxValue = -1,
                                       const gtInt32 vArgMinValue = -1,
                                       const gtString &vArgDescription = L"",
                                       QWidget *vpParent = nullptr);
    ~LPGPU2ppFeedbackEngineOptionWidget() override;

    lpgpu2::PPFnStatus Initialise();
    lpgpu2::PPFnStatus Shutdown();

    lpgpu2::PPFnStatus SetArgTooltip(const QString &vArgTooltip) const;
    bool IsOptionEnabled() const;
    const gtString& GetOptionName() const;
    bool HasArg() const;
    gtInt32 GetArgValue() const;

// Methods
public slots:
    void SetOptionEnabled(const bool vbEnabled) const;

// Signals
signals:
    void OptionsEnabledChanged(const bool vbEnabled);

// Attributes
private:
    gtString   m_optionName;
    gtString   m_optionDisplayName;
    gtString   m_argName;
    gtInt32    m_argValue;
    gtInt32    m_argMinValue;
    gtInt32    m_argMaxValue;
    gtString   m_argDescription;
    QCheckBox *m_pOptionCheckBox = nullptr;
    QSpinBox  *m_pArgSpinBox = nullptr;
};
// clang-format on


#endif // LPGPU2_PP_FEEDBACK_OPTION_WIDGET_H_INCLUDE