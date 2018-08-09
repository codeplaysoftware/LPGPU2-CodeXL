// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppInfoWidget.cpp
///
/// LPGPU2ppInfoWidget definitions.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/LPGPU2ppInfoWidget.h>

#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
using lpgpu2::PPFnStatus;

#include <QGroupBox>
#include <QVBoxLayout>

/// @brief Ctor. No work done here.
LPGPU2ppInfoWidget::LPGPU2ppInfoWidget(const QString& vTitle, const QString& vMsg, QWidget* vpParent)
: QWidget(vpParent)
, m_title(vTitle)
, m_msg(vMsg)
{
}

/// @brief Dtor.
LPGPU2ppInfoWidget::~LPGPU2ppInfoWidget()
{
  Shutdown();
}

/// @brief Set up and display the components for this widget.
PPFnStatus LPGPU2ppInfoWidget::Initialise()
{
  QVBoxLayout* pMainLayout = nullptr;
  LPGPU2PPNewQtWidget(&pMainLayout, this);
  
  QWidget* pTitleWidget;
  LPGPU2PPNewQtWidget(&pTitleWidget, this);
  QHBoxLayout* pTitleLayout;
  LPGPU2PPNewQtWidget(&pTitleLayout, pTitleWidget);
  
  QLabel* pIconLabel;
  LPGPU2PPNewQtWidget(&pIconLabel, "");
  QPixmap infoIcon;
  acSetIconInPixmap(infoIcon, AC_ICON_WARNING_INFO);
  pIconLabel->setPixmap(infoIcon);
  pIconLabel->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
  pTitleLayout->addWidget(pIconLabel);
  
  QLabel* pTitleLabel;
  LPGPU2PPNewQtWidget(&pTitleLabel, m_title);
  QFont font = pTitleLabel->font();
  font.setBold(true);
  pTitleLabel->setFont(font);
  pTitleLabel->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
  pTitleWidget->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
  pTitleLayout->addWidget(pTitleLabel);
  
  pMainLayout->addWidget(pTitleWidget);
  
  QLabel* pMessageLabel;
  LPGPU2PPNewQtWidget(&pMessageLabel, m_msg);
  pMessageLabel->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
  pMainLayout->addWidget(pMessageLabel);
  
  setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));

  return PPFnStatus::success;
}

PPFnStatus LPGPU2ppInfoWidget::Shutdown()
{
  return PPFnStatus::success;
}



