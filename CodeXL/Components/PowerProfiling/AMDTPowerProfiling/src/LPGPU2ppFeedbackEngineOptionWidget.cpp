// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the feedback engine configuration dialog.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFeedbackEngineOptionWidget.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Qt:
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

using lpgpu2::PPFnStatus;

/// @brief                         Class constructor, do work is done here. Just store the
///                                parameters as class members.
/// @param[in]  vOptionName        This is the option identifier. Used to inform the feedback
///                                engine which option is being set.
/// @param[in]  vOptionDisplayName The option display name what will appear to the user.
/// @param[in]  vArgName           In case this option has an input argument, this is the name
///                                that will be displayed to the user.
/// @param[in]  vArgValue          The initial value of the optional argument.
/// @param[in]  vpParent           Pointer to the parent widget.
LPGPU2ppFeedbackEngineOptionWidget::LPGPU2ppFeedbackEngineOptionWidget(const gtString &vOptionName,
                                                           const gtString &vOptionDisplayName,
                                                           const gtString& vArgName /* = L"" */,                                                        
                                                           const gtInt32 vArgValue /* = -1 */,
                                                           const gtInt32 vArgMaxValue /*= -1*/,
                                                           const gtInt32 vArgMinValue /*= -1*/,
                                                           const gtString &vArgDescription /* = L"" */,
                                                           QWidget *vpParent /* = nullptr */)
    : QWidget{ vpParent }
    , m_optionName{ vOptionName }
    , m_optionDisplayName{ vOptionDisplayName } 
    , m_argName{ vArgName }
    , m_argValue{ vArgValue }
    , m_argMinValue{ vArgMinValue }
    , m_argMaxValue{ vArgMaxValue }
    , m_argDescription{ vArgDescription }
{
}

/// @brief  Class destructor. Just calls Shutdown do clear resources.
LPGPU2ppFeedbackEngineOptionWidget::~LPGPU2ppFeedbackEngineOptionWidget()
{
    Shutdown();
}

/// @brief                  Sets the tooltip for the optional argument. The tooltip will
///                         be shown when the user hovers the mouse over the spinbox.
/// @param[in]  vArgTooltip The tooltip to set on the spin box.
/// @returns    PPFnStatus  success if the tool was set, failure otherwise.
PPFnStatus LPGPU2ppFeedbackEngineOptionWidget::SetArgTooltip(const QString &vArgTooltip) const
{
    auto bReturn = PPFnStatus::failure;

    if (m_pArgSpinBox != nullptr)
    {
        m_pArgSpinBox->setToolTip(vArgTooltip);

        bReturn = PPFnStatus::success;
    }

    return bReturn;
}

/// @brief  Initialises the widget elements and layout.
/// @returns PPFnStatus success if all widgets are ok, failure if something went wrong.
PPFnStatus LPGPU2ppFeedbackEngineOptionWidget::Initialise()
{
    QHBoxLayout *pMainLayout = nullptr;
    LPGPU2PPNewQtWidget(&pMainLayout, this);
    pMainLayout->setMargin(0);

    LPGPU2PPNewQtWidget(&m_pOptionCheckBox, acGTStringToQString(m_optionDisplayName));
    m_pOptionCheckBox->setTristate(false);

    pMainLayout->addWidget(m_pOptionCheckBox);
    pMainLayout->addStretch();

    if (!m_argName.isEmpty())
    {
        QLabel *pArgLabel = nullptr;
        LPGPU2PPNewQtWidget(&pArgLabel, acGTStringToQString(m_argName), this);
        pArgLabel->setToolTip(acGTStringToQString(m_argDescription));

        LPGPU2PPNewQtWidget(&m_pArgSpinBox, this);
        m_pArgSpinBox->setRange(m_argMinValue, m_argMaxValue);
        m_pArgSpinBox->setEnabled(m_pOptionCheckBox->isChecked());
        m_pArgSpinBox->setValue(m_argValue);        
        m_pArgSpinBox->setToolTip(acGTStringToQString(m_argDescription));

        pMainLayout->addWidget(pArgLabel);
        pMainLayout->addWidget(m_pArgSpinBox);

        // To avoid writing a separate slot just for updating the arg value, we connect here with a lambda.
        connect(m_pOptionCheckBox, &QCheckBox::stateChanged, m_pArgSpinBox, &QSpinBox::setEnabled);
        connect(m_pArgSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int newValue)
        {
            m_argValue = newValue;
        });
    }

    connect(m_pOptionCheckBox, &QCheckBox::stateChanged, this, &LPGPU2ppFeedbackEngineOptionWidget::OptionsEnabledChanged);

    return PPFnStatus::success;
}

/// @brief  Clear the resources used by this class.
/// @returns PPFnStatus always return success.
PPFnStatus LPGPU2ppFeedbackEngineOptionWidget::Shutdown()
{
    if (m_pOptionCheckBox != nullptr)
    {
        delete m_pOptionCheckBox;
        m_pOptionCheckBox = nullptr;
    }

    if (m_pArgSpinBox != nullptr)
    {
        delete m_pArgSpinBox;
        m_pArgSpinBox = nullptr;
    }

    return PPFnStatus::success;
}

/// @brief Returns whether or not this feedback option is enabled.
/// @returns bool  true: the option is enabled,
///                false: the option is disabled.
bool LPGPU2ppFeedbackEngineOptionWidget::IsOptionEnabled() const
{
    return m_pOptionCheckBox != nullptr && m_pOptionCheckBox->isChecked();
}

/// @brief  Returns the argument value for this option.
/// @returns gtUInt32   returns the argument value as configured by the user.
gtInt32 LPGPU2ppFeedbackEngineOptionWidget::GetArgValue() const
{
    return m_argValue;
}

/// @brief  Gets the option identifier name.
/// @returns gtString   The option identifier name.
const gtString& LPGPU2ppFeedbackEngineOptionWidget::GetOptionName() const
{
    return m_optionName;
}

/// @brief  Returns whether or not this option has an optional argument available.
/// @returns bool   true: This option has an argument available,
///                 false: This option does not have an argument available.
bool LPGPU2ppFeedbackEngineOptionWidget::HasArg() const
{
    return !m_argName.isEmpty();
}

/// @brief  Sets this option enabled or disabled.
/// @param[in]  vbEnabled State to set this option.
void LPGPU2ppFeedbackEngineOptionWidget::SetOptionEnabled(const bool vbEnabled) const
{
    if (m_pOptionCheckBox != nullptr && m_pOptionCheckBox->isChecked() != vbEnabled)
    {
        m_pOptionCheckBox->setChecked(vbEnabled);        
    }
}