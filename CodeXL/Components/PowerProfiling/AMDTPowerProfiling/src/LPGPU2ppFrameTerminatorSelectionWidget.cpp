// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFrameTerminatorSelectionWidget.cpp
///
/// @brief Widget to display the available frame terminator choices
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/LPGPU2ppFrameTerminatorSelectionWidget.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <type_traits>
#include <array>

namespace lpgpu2 {

// Using old-style enum here naming here to match the one used by SAMS in their
// DCAPI code, however we do not list all the entries SAMS lists in their
// implementation of the DCAPI because we do not need enums for more elments
// combined
struct FrameTerminatorsIndices {
  enum Indices
  {
    FT_GL_CLEAR = 0,
    FT_GL_FLUSH,
    FT_GL_FINISH,
    FT_EGL_SWAPBUFFERS,
    FT_CL_FLUSH,
    FT_CL_FINISH,
    FT_CL_WAITFOREVENTS,
    FT_VK_QUEUEPRESENTKHR,

    NUMBER_OF_API_CALLS
  };
};

/// @brief Helper type to easily manage the association between an API call name
///        and its equivalent bitfield value
struct APICallData
{
  QString name;
  gtUInt32 bitfieldVal;
};

const std::array<APICallData, FrameTerminatorsIndices::NUMBER_OF_API_CALLS>
kCallsList {
  {
    {"glClear", FrameTerminatorApiCalls::FT_GL_CLEAR},
    {"glFlush", FrameTerminatorApiCalls::FT_GL_FLUSH},
    {"glFinish", FrameTerminatorApiCalls::FT_GL_FINISH},
    {"eglSwapBuffers", FrameTerminatorApiCalls::FT_EGL_SWAPBUFFERS},
    {"clFlush", FrameTerminatorApiCalls::FT_CL_FLUSH},
    {"clFinish", FrameTerminatorApiCalls::FT_CL_FINISH},
    {"clWaitForEvents", FrameTerminatorApiCalls::FT_CL_WAITFOREVENTS},
    {"vkQueuePresentKHR", FrameTerminatorApiCalls::FT_VK_QUEUEPRESENTKHR},
  }
};

const QString kCheckBoxBitfieldPropertyName {"bitfield_val"};
const QString kHeaderLabel {"Frame Terminators:"};
const QString kExplicitFrameTerminatorName{"Explicit Frame Terminator"};

/// @brief Create the necessary widgets and add them in a specific layout
/// @return Always PPFnStatus::success
PPFnStatus FrameTerminatorSelWidget::Initialise()
{
  QVBoxLayout *vLayout = nullptr;
  LPGPU2PPNewQtWidget(&vLayout, this);
  LPGPU2PPNewQtWidget(&m_headerLabel, kHeaderLabel);
  vLayout->addWidget(m_headerLabel);

  QHBoxLayout *hLayout = nullptr;
  LPGPU2PPNewQtWidget(&hLayout, nullptr);
  vLayout->addLayout(hLayout);

  QVBoxLayout *vLayoutInternal1 = nullptr;
  LPGPU2PPNewQtWidget(&vLayoutInternal1, nullptr);
  vLayoutInternal1->addStretch();
  QVBoxLayout *vLayoutInternal2 = nullptr;
  LPGPU2PPNewQtWidget(&vLayoutInternal2, nullptr);
  vLayoutInternal2->addStretch();
  QVBoxLayout *vLayoutInternal3 = nullptr;
  LPGPU2PPNewQtWidget(&vLayoutInternal3, nullptr);
  vLayoutInternal3->addStretch();
  hLayout->addLayout(vLayoutInternal1);
  hLayout->addLayout(vLayoutInternal2);
  hLayout->addLayout(vLayoutInternal3);
  hLayout->addStretch();

  size_t index = 0;
  for (const auto &el : kCallsList)
  {
    QCheckBox *checkBox = nullptr;
    LPGPU2PPNewQtWidget(&checkBox, el.name);
    // Add a property to each checkbox which contains the bitfield value
    // related to the API call it represents
    checkBox->setProperty(kCheckBoxBitfieldPropertyName.toStdString().c_str(),
        QVariant::fromValue(el.bitfieldVal));
    connect(checkBox, SIGNAL(stateChanged(int)), this,
      SLOT(OnCheckboxStateChanged(int)));

    // Add the first 4 checkboxes to one column (vertical layout)
    if (index < 3)
    {
      vLayoutInternal1->addWidget(checkBox);
    }
    else if (index < 6)
    {
      vLayoutInternal2->addWidget(checkBox);
    }
    else
    {
      vLayoutInternal3->addWidget(checkBox);
    }

    m_termsCheckboxes.push_back(checkBox);

    ++index;
  }

  LPGPU2PPNewQtWidget(&m_explicitFrameTermCheckbox,
      kExplicitFrameTerminatorName);
  vLayoutInternal3->addWidget(m_explicitFrameTermCheckbox);
  connect(m_explicitFrameTermCheckbox, SIGNAL(stateChanged(int)), this,
    SLOT(OnExplicitCheckboxStateChanged(int)));

  SetEnabled(false);

  return PPFnStatus::success;
}

/// @brief Clear up and reset the object. Act like a dtor
/// @return Always PPFnStatus::success
PPFnStatus FrameTerminatorSelWidget::Terminate()
{
  if (m_headerLabel)
  {
    delete m_headerLabel;
    m_headerLabel = nullptr;
  }

  for (auto *el : m_termsCheckboxes)
  {
    if(el)
    {
      delete el;
    }
  }
  m_termsCheckboxes.clear();

  if (m_explicitFrameTermCheckbox)
  {
    delete m_explicitFrameTermCheckbox;
    m_explicitFrameTermCheckbox = nullptr;
  }

  return PPFnStatus::success;
}

/// @brief Enable or disable the checkboxes which control the frame
///        terminator when not of type explicit
/// @param isEnabled Wheter to enable or disable the widgets
/// @return Always PPFnStatus::success
PPFnStatus FrameTerminatorSelWidget::SetEnabledBitfieldCheckboxes(
    bool isEnabled)
{
  // Loop over all checkboxes and set
  for (auto &cb : m_termsCheckboxes)
  {
    cb->setEnabled(isEnabled);
  }

  return PPFnStatus::success;
}

/// @brief Enable or disable all the widgets contained inside this widget
/// @param isEnabled Wheter to enable or disable the widgets
/// @return Always PPFnStatus::success
PPFnStatus FrameTerminatorSelWidget::SetEnabled(bool isEnabled)
{
  if (isEnabled == m_isWholeWidgetEnabled)
  {
    return PPFnStatus::success;
  }

  m_headerLabel->setEnabled(isEnabled);

  SetEnabledBitfieldCheckboxes(isEnabled);

  m_explicitFrameTermCheckbox->setEnabled(isEnabled);

  m_isWholeWidgetEnabled = isEnabled;

  return PPFnStatus::success;
}

/// @brief Check the checkboxes depending on the value set in the bitfield which
///        represents the user's selection in terms of what API calls to use as
///        frame terminators
/// @param bitfield The bitfield to use to set the checkboxes
/// @return PPFnStatus::success if success, PPFnStatus::failure if failure
PPFnStatus FrameTerminatorSelWidget::SetCheckboxesFromBitfield(
    gtUInt32 bitfield)
{
  if (m_termsCheckboxes.size() != kCallsList.size())
  {
    return PPFnStatus::failure;
  }

  // Unpack bitfield and set checkboxes
  size_t index = 0;
  for (const auto &el : kCallsList)
  {
    m_termsCheckboxes[index]->setChecked(
        static_cast<bool>(bitfield & el.bitfieldVal));

    ++index;
  }

  return PPFnStatus::success;
}

/// @brief Get the bitfield representation of the user selection
/// @param bitfield The returned bitfield
/// @return Always PPFnStatus::success
PPFnStatus FrameTerminatorSelWidget::GetBitfieldFromCheckboxes(
    gtUInt32 &bitfield) const
{
  bitfield = m_bitfield;

  return PPFnStatus::success;
}

/// @brief Called when a checkbox is changed. Updates the internal cached
///        bitfield representation of the user's selection
/// @param state The new state of the checkbox
void FrameTerminatorSelWidget::OnExplicitCheckboxStateChanged(
    int state)
{
  // Get who sent the event
  auto *pSender = qobject_cast<QCheckBox *>(sender());
  if (!pSender)
  {
    return;
  }

  if (pSender == m_explicitFrameTermCheckbox)
  {
    if (state == Qt::Checked)
    {
      SetEnabledBitfieldCheckboxes(false);
    }
    else
    {
      SetEnabledBitfieldCheckboxes(true);
    }
  }
}

/// @brief Called when a checkbox (non explicit frame terminator)
///        is changed. Updates the internal cached
///        bitfield representation of the user's selection
/// @param state The new state of the checkbox
void FrameTerminatorSelWidget::OnCheckboxStateChanged(int state)
{
  // Get who sent the event
  auto *pSender = qobject_cast<QCheckBox *>(sender());
  if (!pSender)
  {
    return;
  }

  // Get its bitfield value
  gtUInt32 bitfieldVal =
    pSender->property(
        kCheckBoxBitfieldPropertyName.toStdString().c_str()).toUInt();

  // Depending on the state, either remove the bitfield or add it
  if (state == Qt::Checked)
  {
    m_bitfield |= bitfieldVal;
  }
  else
  {
    m_bitfield &= (~bitfieldVal);
  }
}

/// @brief Uncheck all the checkboxes
/// @return Always PPFnStatus::success
PPFnStatus FrameTerminatorSelWidget::ResetCheckboxes()
{
  for (auto *cb : m_termsCheckboxes)
  {
    cb->setChecked(false);
  }

  m_explicitFrameTermCheckbox->setChecked(false);

  return PPFnStatus::success;
}

/// @brief Return whether the explicit frame terminator widget is checked or 
///        not
/// @return True if the widget is checked, false if it is not
bool FrameTerminatorSelWidget::IsExplicitFrameTermEnabled() const
{
  return (m_explicitFrameTermCheckbox->checkState() == Qt::Checked);
}

/// @brief Set whether the explicit terminator checkbox is checked or not
/// @param isChecked Whether it should be checked or unchecked
/// @return Always PPFnStatus::success
PPFnStatus FrameTerminatorSelWidget::SetExplicitFrameTermChecked(
    bool isChecked)
{
  m_explicitFrameTermCheckbox->setChecked(isChecked);

  return PPFnStatus::success;
}

} // namespace lpgpu2
