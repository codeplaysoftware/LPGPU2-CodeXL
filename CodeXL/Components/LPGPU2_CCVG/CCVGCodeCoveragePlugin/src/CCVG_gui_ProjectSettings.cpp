// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGGUIProjectSettings implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afBrowseAction.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_ProjectSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_Functions.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_wgt_LineEdit.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_wgt_GroupBoxHideTooltipOverChild.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_wgt_ButtonDirFolder.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_CommandIDs.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdProjDataSetNoDriverFilesReq.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Params.h>

namespace ccvg {
namespace gui {

/// @brief  Class constructor. No work is done here by this class. 
CCVGGUIProjectSettings::NotifyOnProjectClose::NotifyOnProjectClose()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGGUIProjectSettings::NotifyOnProjectClose::~NotifyOnProjectClose()
{
}

/// @brief    Class constructor. No work is done here by this class. Base class
///           carries out work
/// param[in] vpParent - Qt widget object to *this.
CCVGGUIProjectSettings::CCVGGUIProjectSettings()
: m_bBeenInitialised(false)
, m_bBeenShutdown(false)
, m_bCCVGReadyToWork(false)
, m_pGlobalSettings(nullptr)
, m_pProjSettings(nullptr)
, m_pCCVGProjectEnabledFlagGrpBx(nullptr)
, m_pCCVGResultsFolderPathEdit(nullptr)
, m_pCCVGResultsFilesExtnEdit(nullptr)
, m_pCCVGSourceFolderPathEdit(nullptr)
, m_pCCVGSourceFilesExtnEdit(nullptr)
, m_pCCVGDriverGrpBx(nullptr)
, m_pCCVGDriverFolderPathEdit(nullptr)
, m_pCCVGDriverFilesExtnEdit(nullptr)
, m_pCCVGExecuteableFolderPathEdit(nullptr)
, m_pCCVGExecuteableFilesExtnEdit(nullptr)
, m_bDoNotNotifyProjectClose(false)
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGGUIProjectSettings::~CCVGGUIProjectSettings()
{
  Shutdown();
}

/// @brief  Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGGUIProjectSettings::Initialise()
{
  if (m_bBeenInitialised)
  {
    return success;
  }

  // Project settings are loaded into widgets in RestoreCurrentSettings()
  m_pGlobalSettings = &fw::CCVGFWTheCurrentState::Instance();
  m_pProjSettings = &m_pGlobalSettings->GetProjectSetting();
  m_bCCVGReadyToWork = m_pGlobalSettings->IsCCVGReadyToWork();
  
  m_bBeenInitialised = m_bCCVGReadyToWork ? InitialiseCCVGReadyToWork() : InitialiseCCVGNotReadyToWork();

  return m_bBeenInitialised;
}

/// @brief  Set up the CCVG project settings page of the new or current
///         project settings dialog.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGGUIProjectSettings::InitialiseCCVGNotReadyToWork()
{
  QVBoxLayout *pMainLayout = nullptr;
  QLabel *pLabelInfo = nullptr;
  QLabel *pLabelStretch = nullptr;
  QLabel *pLabelVersion = nullptr;
  try {
    pMainLayout = new QVBoxLayout;
    pLabelInfo = new QLabel;
    pLabelStretch = new QLabel;
    pLabelVersion = new QLabel;
  }
  catch (...)
  {
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_title));
  }

  const wchar_t *pInfo = CCVGRSRC(kIDS_CCVG_STR_gui_ProjectSettings_err_ccvg_not_readyToWork);
  pLabelInfo->setText(acGTStringToQString(pInfo));
  pMainLayout->addWidget(pLabelInfo);

  // Add the vertical expansion
  pLabelStretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  pMainLayout->addWidget(pLabelStretch);

  // Add CCVG version information
  pLabelVersion->setText(acGTStringToQString(CCVGUtilGetVersionText()));
  pMainLayout->addWidget(pLabelVersion);

  // Add widgets to layout:
  setLayout(pMainLayout);

  return success;
}

/// @brief  Set up the CCVG project settings page of the new or current
///         project settings dialog.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGGUIProjectSettings::InitialiseCCVGReadyToWork()
{
  bool bOk = true;
  QVBoxLayout *pMainLayout = nullptr;
  QLabel *pLabelStretch = nullptr;
  QLabel *pLabelVersion = nullptr;
  QVBoxLayout *pLayoutCCVGEnabledFlagGrpBx = nullptr;
  QLabel *pLabelFolderResults = nullptr;
  QLabel *pLabelFolderSource = nullptr;
  QLabel *pLabelFolderDriver = nullptr;
  QLabel *pLabelFolderExecuteable = nullptr;
  QGroupBox *pGrpBxCCVGResultFiles = nullptr;
  QGridLayout *pLayoutCCVGResultFiles = nullptr;
  QLabel *pLabelCCVGCaption = nullptr;
  QLabel *pLabelCCVGTxtHelp = nullptr;
  QLabel *pLabelRequiredFields = nullptr;
  QLabel *pLabelCCVGResultFiles = nullptr;
  QLabel *pLabelCCVGResultFilesExtn = nullptr;
  QGroupBox *pGrpBxCCVGSourceFiles = nullptr;
  QGridLayout *pLayoutCCVGSourceFiles = nullptr;
  QLabel *pLabelCCVGSourceFiles = nullptr;
  QLabel *pLabelCCVGSourceFilesExtn = nullptr;
  QGridLayout *pLayoutCCVGDriverFiles = nullptr;
  QLabel *pLabelCCVGDriverFiles = nullptr;
  QLabel *pLabelCCVGDriverFilesExtn = nullptr;
  QGroupBox *pGrpBxCCVGExecuteableFiles = nullptr;
  QGridLayout *pLayoutCCVGExecuteableFiles = nullptr;
  QLabel *pLabelCCVGExecuteableFiles = nullptr;
  QLabel *pLabelCCVGExecuteableFilesExtn = nullptr;
  QLabel *pLabelCCVGWarningDataLoss = nullptr;
  try {
    pMainLayout = new QVBoxLayout;
    pLabelCCVGCaption = new QLabel;
    pLabelCCVGTxtHelp = new QLabel;
    m_pCCVGProjectEnabledFlagGrpBx = new QGroupBox;
    pLayoutCCVGEnabledFlagGrpBx = new QVBoxLayout;
    pLabelRequiredFields = new QLabel;
    
    pLabelCCVGResultFiles = new QLabel;
    pGrpBxCCVGResultFiles = new QGroupBox;
    pLayoutCCVGResultFiles = new QGridLayout;
    pLabelFolderResults = new QLabel;
    m_pCCVGResultsFolderPathEdit = new CCVGWGTLineEdit;
    pLabelCCVGResultFilesExtn = new QLabel;
    m_pCCVGResultsFilesExtnEdit = new QLineEdit;

    pLabelCCVGSourceFiles = new QLabel;
    pGrpBxCCVGSourceFiles = new QGroupBox;
    pLayoutCCVGSourceFiles = new QGridLayout;
    pLabelFolderSource = new QLabel;
    m_pCCVGSourceFolderPathEdit = new CCVGWGTLineEdit;
    pLabelCCVGSourceFilesExtn = new QLabel;
    m_pCCVGSourceFilesExtnEdit = new QLineEdit;

    pLabelCCVGExecuteableFiles = new QLabel;
    pGrpBxCCVGExecuteableFiles = new QGroupBox;
    pLayoutCCVGExecuteableFiles = new QGridLayout;
    pLabelFolderExecuteable = new QLabel;
    m_pCCVGExecuteableFolderPathEdit = new CCVGWGTLineEdit;
    pLabelCCVGExecuteableFilesExtn = new QLabel;
    m_pCCVGExecuteableFilesExtnEdit = new QLineEdit;

    pLabelCCVGDriverFiles = new QLabel;
    m_pCCVGDriverGrpBx = new CCVGWGTGroupBoxHideTooltipOverChild;
    pLayoutCCVGDriverFiles = new QGridLayout;
    pLabelFolderDriver = new QLabel;
    m_pCCVGDriverFolderPathEdit = new CCVGWGTLineEdit;
    pLabelCCVGDriverFilesExtn = new QLabel;
    m_pCCVGDriverFilesExtnEdit = new QLineEdit;
    
    pLabelCCVGWarningDataLoss = new QLabel;

    pLabelStretch = new QLabel;
    pLabelVersion = new QLabel;
  }
  catch (...)
  {
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_title));
  }

  // Common attributes to some widgets
  const QString lineEditFontStyleForTxtTip("color: grey; font-style: italic");
  const QString grpBoxStyleBorder("QGroupBox{ border: 1px solid lightgray; border-radius: 3px; margin-top: 3ex};");
  const QString grpBoxStyleTitle("QGroupBox::title{ background-color: solid; subcontrol-origin: margin; subcontrol-position: top left; padding:0 0px}");
  const QString grpBoxStyle(grpBoxStyleTitle + grpBoxStyleBorder);

  // CCVG title for its property pages
  const wchar_t *pCaption = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_title);
  pLabelCCVGCaption->setText(acGTStringToQString(pCaption));
  pLabelCCVGCaption->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
  pMainLayout->addWidget(pLabelCCVGCaption); 

  // Label CCVG help text
  const wchar_t *pTextHelp = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_label_txtExplain3SetsOfFiles);
  pLabelCCVGTxtHelp->setText(acGTStringToQString(pTextHelp));
  pMainLayout->addWidget(pLabelCCVGTxtHelp);

  // User enable disable CCVG features/availability for current project
  const wchar_t *pTextCCVGEnabled = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_chkbx_txtEnabledDisablePluginForCurrentProj);
  m_pCCVGProjectEnabledFlagGrpBx->setTitle(acGTStringToQString(pTextCCVGEnabled));
  m_pCCVGProjectEnabledFlagGrpBx->setStyleSheet(grpBoxStyle);
  m_pCCVGProjectEnabledFlagGrpBx->setCheckable(true);
  m_pCCVGProjectEnabledFlagGrpBx->setChecked(false);

  // Label required fields
  const wchar_t *pTextRequiredFields = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_label_requiredFields);
  QFont fontRequiredFields;
  fontRequiredFields.setPointSize(7);
  pLabelRequiredFields->setText(acGTStringToQString(pTextRequiredFields));
  pLabelRequiredFields->setFont(fontRequiredFields);
  pLabelRequiredFields->setStyleSheet(lineEditFontStyleForTxtTip);
  pLayoutCCVGEnabledFlagGrpBx->addWidget(pLabelRequiredFields);

  // Code coverage result files folder fields
  int countRowCCVGResultsFilesGrpBx = 0;
  const wchar_t *pTextResults = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_label);
  pGrpBxCCVGResultFiles->setTitle(acGTStringToQString(pTextResults));
  pGrpBxCCVGResultFiles->setStyleSheet(grpBoxStyle);
  const wchar_t *pLabelFolderTxt = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_label_folder);
  const wchar_t *pCCVGResultsFolderPathEditTxtTip = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_editTxtTip);
  const wchar_t *pCCVGResultsFolderPathButtonTooltip = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_dirBtnTooltip);
  pLabelFolderResults->setText(acGTStringToQString(pLabelFolderTxt));
  m_pCCVGResultsFolderPathEdit->SetRealTextEntry("");
  m_pCCVGResultsFolderPathEdit->SetToolTip(acGTStringToQString(pCCVGResultsFolderPathEditTxtTip));
  m_pCCVGResultsFolderPathEdit->SetDisplayToolTip(true);
  m_CCVGBtnResultsFolder.SetTooltip(acGTStringToQString(pCCVGResultsFolderPathButtonTooltip));
  pLayoutCCVGResultFiles->setColumnMinimumWidth(2, 500);
  pLayoutCCVGResultFiles->addWidget(pLabelFolderResults, countRowCCVGResultsFilesGrpBx, 0);
  pLayoutCCVGResultFiles->addWidget(m_pCCVGResultsFolderPathEdit, countRowCCVGResultsFilesGrpBx, 1, 1, 2);
  pLayoutCCVGResultFiles->addWidget(m_CCVGBtnResultsFolder.GetButton(), countRowCCVGResultsFilesGrpBx, 3);
  countRowCCVGResultsFilesGrpBx++;
  const wchar_t *pCCVGResultsFileExtn = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_labelExtn);
  pLabelCCVGResultFilesExtn->setText(acGTStringToQString(pCCVGResultsFileExtn));
  pLayoutCCVGResultFiles->addWidget(pLabelCCVGResultFilesExtn, countRowCCVGResultsFilesGrpBx, 0);
  pLayoutCCVGResultFiles->addWidget(m_pCCVGResultsFilesExtnEdit, countRowCCVGResultsFilesGrpBx, 1);
  pGrpBxCCVGResultFiles->setLayout(pLayoutCCVGResultFiles);
  pLayoutCCVGEnabledFlagGrpBx->addWidget(pGrpBxCCVGResultFiles);

  // Code coverage kernel source files folder fields
  int countRowCCVGSourceFilesGrpBx = 0;
  const wchar_t *pTextSource = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_label);
  pGrpBxCCVGSourceFiles->setTitle(acGTStringToQString(pTextSource));
  pGrpBxCCVGSourceFiles->setStyleSheet(grpBoxStyle);
  const wchar_t *pCCVGSourceFolderPathEditTxtTip = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_editTxtTip);
  const wchar_t *pCCVGSourceFolderPathButtonTooltip = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_dirBtnTooltip);
  pLabelFolderSource->setText(acGTStringToQString(pLabelFolderTxt));
  m_pCCVGSourceFolderPathEdit->SetRealTextEntry("");
  m_pCCVGSourceFolderPathEdit->SetToolTip(acGTStringToQString(pCCVGSourceFolderPathEditTxtTip));
  m_pCCVGSourceFolderPathEdit->SetDisplayToolTip(true);
  m_CCVGBtnSourceFolder.SetTooltip(acGTStringToQString(pCCVGSourceFolderPathButtonTooltip));
  pLayoutCCVGSourceFiles->setColumnMinimumWidth(2, 500);
  pLayoutCCVGSourceFiles->addWidget(pLabelFolderSource, countRowCCVGSourceFilesGrpBx, 0);
  pLayoutCCVGSourceFiles->addWidget(m_pCCVGSourceFolderPathEdit, countRowCCVGSourceFilesGrpBx, 1, 1, 2);
  pLayoutCCVGSourceFiles->addWidget(m_CCVGBtnSourceFolder.GetButton(), countRowCCVGSourceFilesGrpBx, 3);
  countRowCCVGSourceFilesGrpBx++;
  const wchar_t *pCCVGSourceFileExtn = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_labelExtn);
  pLabelCCVGSourceFilesExtn->setText(acGTStringToQString(pCCVGSourceFileExtn));
  pLayoutCCVGSourceFiles->addWidget(pLabelCCVGSourceFilesExtn, countRowCCVGSourceFilesGrpBx, 0);
  pLayoutCCVGSourceFiles->addWidget(m_pCCVGSourceFilesExtnEdit, countRowCCVGSourceFilesGrpBx, 1);
  pGrpBxCCVGSourceFiles->setLayout(pLayoutCCVGSourceFiles);
  pLayoutCCVGEnabledFlagGrpBx->addWidget(pGrpBxCCVGSourceFiles);

  // Code coverage executeable files folder fields
  int countRowCCVGExecuteableFilesGrpBx = 0;
  const wchar_t *pTextExecuteable = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_label);
  pGrpBxCCVGExecuteableFiles->setTitle(acGTStringToQString(pTextExecuteable));
  pGrpBxCCVGExecuteableFiles->setStyleSheet(grpBoxStyle);
  const wchar_t *pCCVGExecuteableFolderPathEditTxtTip = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_editTxtTip);
  const wchar_t *pCCVGExecuteableFolderPathButtonTooltip = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_dirBtnTooltip);
  pLabelFolderExecuteable->setText(acGTStringToQString(pLabelFolderTxt));
  m_pCCVGExecuteableFolderPathEdit->SetRealTextEntry("");
  m_pCCVGExecuteableFolderPathEdit->SetToolTip(acGTStringToQString(pCCVGExecuteableFolderPathEditTxtTip));
  m_pCCVGExecuteableFolderPathEdit->SetDisplayToolTip(true);
  m_CCVGBtnExecuteableFolder.SetTooltip(acGTStringToQString(pCCVGExecuteableFolderPathButtonTooltip));
  pLayoutCCVGExecuteableFiles->setColumnMinimumWidth(2, 500);
  pLayoutCCVGExecuteableFiles->addWidget(pLabelFolderExecuteable, countRowCCVGExecuteableFilesGrpBx, 0);
  pLayoutCCVGExecuteableFiles->addWidget(m_pCCVGExecuteableFolderPathEdit, countRowCCVGExecuteableFilesGrpBx, 1, 1, 2);
  pLayoutCCVGExecuteableFiles->addWidget(m_CCVGBtnExecuteableFolder.GetButton(), countRowCCVGExecuteableFilesGrpBx, 3);
  countRowCCVGExecuteableFilesGrpBx++;
  const wchar_t *pCCVGExecuteableFileExtn = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_labelExtn);
  pLabelCCVGExecuteableFilesExtn->setText(acGTStringToQString(pCCVGExecuteableFileExtn));
  pLayoutCCVGExecuteableFiles->addWidget(pLabelCCVGExecuteableFilesExtn, countRowCCVGExecuteableFilesGrpBx, 0);
  pLayoutCCVGExecuteableFiles->addWidget(m_pCCVGExecuteableFilesExtnEdit, countRowCCVGExecuteableFilesGrpBx, 1);
  pGrpBxCCVGExecuteableFiles->setLayout(pLayoutCCVGExecuteableFiles);
  pLayoutCCVGEnabledFlagGrpBx->addWidget(pGrpBxCCVGExecuteableFiles);

  // Driver source files folder fields
  int countRowCCVGDriverFilesGrpBx = 0;
  const wchar_t *pTextDriver = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_label);
  const wchar_t *pDriverGrpBxTooltip = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_chkbx_mustHaveTooltip);
  m_pCCVGDriverGrpBx->setTitle(acGTStringToQString(pTextDriver));
  m_pCCVGDriverGrpBx->setStyleSheet(grpBoxStyle);
  m_pCCVGDriverGrpBx->setCheckable(true);
  m_pCCVGDriverGrpBx->setToolTip(acGTStringToQString(pDriverGrpBxTooltip));
  m_pCCVGDriverGrpBx->AddChild(*m_pCCVGDriverFolderPathEdit);
  m_pCCVGDriverGrpBx->AddChild(*m_pCCVGDriverFilesExtnEdit);
  const wchar_t *pCCVGDriverFolderPathEditTxtTip = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_editTxtTip);
  const wchar_t *pCCVGDriverFolderPathButtonTooltip = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_dirBtnTooltip);
  pLabelFolderDriver->setText(acGTStringToQString(pLabelFolderTxt));
  m_pCCVGDriverFolderPathEdit->SetRealTextEntry("");
  m_pCCVGDriverFolderPathEdit->SetToolTip(acGTStringToQString(pCCVGDriverFolderPathEditTxtTip));
  m_pCCVGDriverFolderPathEdit->SetDisplayToolTip(true);
  m_CCVGBtnDriverFolder.SetTooltip(acGTStringToQString(pCCVGDriverFolderPathButtonTooltip));
  pLayoutCCVGDriverFiles->setColumnMinimumWidth(2, 500);
  pLayoutCCVGDriverFiles->addWidget(pLabelFolderDriver, countRowCCVGDriverFilesGrpBx, 0);
  pLayoutCCVGDriverFiles->addWidget(m_pCCVGDriverFolderPathEdit, countRowCCVGDriverFilesGrpBx, 1, 1, 2);
  pLayoutCCVGDriverFiles->addWidget(m_CCVGBtnDriverFolder.GetButton(), countRowCCVGDriverFilesGrpBx, 3);
  countRowCCVGDriverFilesGrpBx++;
  const wchar_t *pCCVGDriverFileExtn = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_labelExtn);
  pLabelCCVGDriverFilesExtn->setText(acGTStringToQString(pCCVGDriverFileExtn));
  pLayoutCCVGDriverFiles->addWidget(pLabelCCVGDriverFilesExtn, countRowCCVGDriverFilesGrpBx, 0);
  pLayoutCCVGDriverFiles->addWidget(m_pCCVGDriverFilesExtnEdit, countRowCCVGDriverFilesGrpBx, 1);
  m_pCCVGDriverGrpBx->setLayout(pLayoutCCVGDriverFiles);
  pLayoutCCVGEnabledFlagGrpBx->addWidget(m_pCCVGDriverGrpBx);

  // Warning text CCVG loss of data and settings
  const wchar_t *pWarning = CCVGRSRC(kIDS_CCVG_STR_gui_ProjectSettings_warning_ccvg_loose_dataAndSettings);
  pLabelCCVGWarningDataLoss->setText(acGTStringToQString(pWarning));
  pLabelCCVGWarningDataLoss->setFont(fontRequiredFields);
  pLabelCCVGWarningDataLoss->setStyleSheet(lineEditFontStyleForTxtTip);
  pLayoutCCVGEnabledFlagGrpBx->addWidget(pLabelCCVGWarningDataLoss);

  // Add outer most group box to dailog layout
  m_pCCVGProjectEnabledFlagGrpBx->setLayout(pLayoutCCVGEnabledFlagGrpBx);
  pMainLayout->addWidget(m_pCCVGProjectEnabledFlagGrpBx); 

  // Add the vertical expansion
  pLabelStretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  pMainLayout->addWidget(pLabelStretch);

  // Add CCVG version information
  pLabelVersion->setText(acGTStringToQString(CCVGUtilGetVersionText()));
  pMainLayout->addWidget(pLabelVersion); 

  // Add widgets to layout:
  setLayout(pMainLayout);

  // Code coverage results widgets signal slot setup
  bOk = bOk && connect(m_CCVGBtnResultsFolder.GetAction(), SIGNAL(triggered()), this, SLOT(OnClickBtnGetCCVGResultFolder()));
  bOk = bOk && connect(m_CCVGBtnSourceFolder.GetAction(), SIGNAL(triggered()), this, SLOT(OnClickBtnGetCCVGSourceFolder()));
  bOk = bOk && connect(m_CCVGBtnDriverFolder.GetAction(), SIGNAL(triggered()), this, SLOT(OnClickBtnGetCCVGDriverFolder()));
  bOk = bOk && connect(m_pCCVGProjectEnabledFlagGrpBx, SIGNAL(clicked(bool)), this, SLOT(OnClickGrpBxProjectEnabledFlag(bool)));
  bOk = bOk && connect(m_CCVGBtnExecuteableFolder.GetAction(), SIGNAL(triggered()), this, SLOT(OnClickBtnGetCCVGExecuteableFolder()));

  return success;
}

/// @brief  Signal slot function. User choses to open a file dialog to locate
///         a specific file folder.
void CCVGGUIProjectSettings::OnClickBtnGetCCVGResultFolder()
{
  afApplicationCommands *pApplicationCommands = afApplicationCommands::instance();
  GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
  {
    afBrowseAction *pActionPrevFolder = m_CCVGBtnResultsFolder.GetAction();
    const wchar_t *pDlgCaption = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_results_folderDlgCaption);
    const QString dlgCaption(acGTStringToQString(pDlgCaption));
    QString folderPath(m_pCCVGResultsFolderPathEdit->GetRealTextEntry());
    const QString newFolderPath = pApplicationCommands->ShowFolderSelectionDialog(dlgCaption, folderPath, pActionPrevFolder);
    const bool bInValid = newFolderPath.isEmpty();
    m_pCCVGResultsFolderPathEdit->SetDisplayToolTip(bInValid);
    m_pCCVGResultsFolderPathEdit->SetRealTextEntry(bInValid ? QString() : newFolderPath);
  }
}

/// @brief  Signal slot function. User choses to open a file dialog to locate
///         a specific file folder.
void CCVGGUIProjectSettings::OnClickBtnGetCCVGSourceFolder()
{
  afApplicationCommands *pApplicationCommands = afApplicationCommands::instance();
  GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
  {
    afBrowseAction *pActionPrevFolder = m_CCVGBtnSourceFolder.GetAction();
    const wchar_t *pDlgCaption = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_source_folderDlgCaption);
    const QString dlgCaption(acGTStringToQString(pDlgCaption));
    QString folderPath(m_pCCVGSourceFolderPathEdit->GetRealTextEntry());
    const QString newFolderPath = pApplicationCommands->ShowFolderSelectionDialog(dlgCaption, folderPath, pActionPrevFolder);
    const bool bInValid = newFolderPath.isEmpty();
    m_pCCVGSourceFolderPathEdit->SetDisplayToolTip(bInValid);
    m_pCCVGSourceFolderPathEdit->SetRealTextEntry(bInValid ? QString() : newFolderPath);
  }
}

/// @brief  Signal slot function. User choses to open a file dialog to locate
///         a specific file folder.
void CCVGGUIProjectSettings::OnClickBtnGetCCVGDriverFolder()
{
  afApplicationCommands *pApplicationCommands = afApplicationCommands::instance();
  GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
  {
    afBrowseAction *pActionPrevFolder = m_CCVGBtnDriverFolder.GetAction();
    const wchar_t *pDlgCaption = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_driver_folderDlgCaption);
    const QString dlgCaption(acGTStringToQString(pDlgCaption));
    QString folderPath(m_pCCVGDriverFolderPathEdit->GetRealTextEntry());
    const QString newFolderPath = pApplicationCommands->ShowFolderSelectionDialog(dlgCaption, folderPath, pActionPrevFolder);
    const bool bInValid = newFolderPath.isEmpty();
    m_pCCVGDriverFolderPathEdit->SetDisplayToolTip(bInValid);
    m_pCCVGDriverFolderPathEdit->SetRealTextEntry(bInValid ? QString() : newFolderPath);
  }
}

/// @brief  Signal slot function. User choses to open a file dialog to locate
///         a specific file folder.
void CCVGGUIProjectSettings::OnClickBtnGetCCVGExecuteableFolder()
{
  afApplicationCommands *pApplicationCommands = afApplicationCommands::instance();
  GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
  {
    afBrowseAction *pActionPrevFolder = m_CCVGBtnExecuteableFolder.GetAction();
    const wchar_t *pDlgCaption = CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_CCVG_executeable_folderDlgCaption);
    const QString dlgCaption(acGTStringToQString(pDlgCaption));
    QString folderPath(m_pCCVGExecuteableFolderPathEdit->GetRealTextEntry());
    const QString newFolderPath = pApplicationCommands->ShowFolderSelectionDialog(dlgCaption, folderPath, pActionPrevFolder);
    const bool bInValid = newFolderPath.isEmpty();
    m_pCCVGExecuteableFolderPathEdit->SetDisplayToolTip(bInValid);
    m_pCCVGExecuteableFolderPathEdit->SetRealTextEntry(bInValid ? QString() : newFolderPath);
  }
}

/// @brief  Class shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGGUIProjectSettings::Shutdown()
{
  if (m_bBeenShutdown)
  {
    return m_bBeenShutdown;
  }

  m_bDoNotNotifyProjectClose = true;

  // Tidy up
  // Note Qt widgets added to a layout are owned and deleted by that layout

  m_vecNotifyOnProjectClose.clear();
  
  m_bBeenShutdown = true;

  return m_bBeenShutdown;
}

/// @brief Create the GUI widget that is reading the CCVG setting for CCVG.
void CCVGGUIProjectSettings::Initialize()
{
  // Do nothing - Initialise() replaces this function
}

/// @brief  Return the XML section extension name for the data to saved. This
///         is the top XML node under which all other CCVG data and settings
///         nodes existing.
/// @return gtString Unique text ID.
gtString CCVGGUIProjectSettings::ExtensionXMLString()
{
  return fw::CCVGFWTheCurrentStateProjectSettings::GetExtnName();
}

/// @brief Return the CCVG project settings extension display name (may contain
///        spaces). To be display in the Project Setting dialog left hand side. 
///        Returns the position of the settings page in the settings dialog.
///        Using a level representation where each new level is represented by
///        a comma i.e. "Analyze, Kernel/Shader Build Options, General &
///        optimisation".
/// @return gtString Unique text ID.
gtString CCVGGUIProjectSettings::ExtensionTreePathAsString()
{
  return CCVGRSRC(kIDS_CCVG_STR_project_settings_extn_display_name);
}

/// @brief      Save project data and settings into a XML string. 
/// @param[out] vProjectAsXMLString XML formatted settings string.
/// @return     bool True = success, false = failure.
bool CCVGGUIProjectSettings::GetXMLSettingsString(gtString &vProjectAsXMLString)
{
  status ok = m_pGlobalSettings->GetXMLProjSettingsAndProjDataString(vProjectAsXMLString);
  return (ok == success) ? true : false;
}

/// @brief      Load project data and settings from an existing project. Read the 
///             XML string extracter CCVG working data and settings.
/// @param[in]  vProjectAsXMLString XML formatted data string.
/// @return     bool True = success, false = failure.
bool CCVGGUIProjectSettings::SetSettingsFromXMLString(const gtString &vProjectAsXMLString)
{
  m_pGlobalSettings->SetProjectIsOpen(true);
  status ok = m_pGlobalSettings->SetProjSettingsAndProjDataFromXMLString(vProjectAsXMLString);
  ok = ok && (RestoreCurrentSettings() ? success : failure);
  return (ok == success) ? true : false;
}

/// @brief  Get the current project settings from the widgets, from it's 
///         controls, and store into the current project properties. The user
///         has clicked 'ok'.
///         This function also gets called on reloading a existing project.
///         However the parameters/settings/data not be read in from XML yet 
///         (!?) and so validation checks fail. So must return true for 
///         when loading.
/// @return bool True = success, false = failure.
bool CCVGGUIProjectSettings::SaveCurrentSettings()
{
  if (!m_bCCVGReadyToWork)
  {
    // Do nothing

    // Return true see above brief
    return true;
  }

  fw::CCVGFWTheCurrentStateProjectSettings::Settings currentSettings;
  CopyWidgetsToSettings(currentSettings);
  m_pProjSettings->SetSettingsWorking(currentSettings);
  m_pProjSettings->SetUserAndPCTheSameAsPreviousUseOfTheCodeXLProject();

  // This would get called on reloading a existing project, not click ok 
  // button, so stop unnecessary command execution (folder directory path
  // not allowed to  be empty on click ok button)
  if (!currentSettings.GetCcvgFolderDirResult().isEmpty())
  {
    const status ok = ExecuteCmdProjDataSetNoDriverFilesReq(currentSettings.GetCcvgDriverFilesMandatoryFlag());
    GT_UNREFERENCED_PARAMETER(ok);
  }

  // Return true see above brief
  return true;
}

/// @brief       Check current settings set in the widget are valid after the
///              user has clicked 'ok'. If false is returned the dialog will
///              not close.
/// @param[out]  vInvalidMessageStr Message to show the user what is wrong.
/// @return      bool True = all valid, false = one or more new settings are
///              invalid.
bool CCVGGUIProjectSettings::AreSettingsValid(gtString &vInvalidMessageStr)
{
  if (!m_bCCVGReadyToWork)
  {
    // Do nothing
    return true;
  }

  fw::CCVGFWTheCurrentStateProjectSettings::Settings currentSettings;
  CopyWidgetsToSettings(currentSettings);

  gtString errMsg;
  if (!m_pProjSettings->ChkSettingsWorkingValid(currentSettings, errMsg))
  {
    vInvalidMessageStr.appendFormattedString(L"%ls\n", CCVGRSRC(kIDS_CCVG_STR_execmode_session_type));
    vInvalidMessageStr += errMsg;
    return false;
  }

  m_pGlobalSettings->SetProjectIsOpen(true);

  return true;
}

/// @brief       Are current settings valid with other settings in other 
///              setting pages?
/// @param[out]  vInvalidMessageStr Error description of invalid setting.
/// @return      bool True = all valid, false = one or more new settings are
///              invalid.
bool CCVGGUIProjectSettings::AreSettingsValidSecondPass(gtString &vInvalidMessageStr)
{  
  GT_UNREFERENCED_PARAMETER(vInvalidMessageStr);  
  
  // Do not have to validate *this setting with other extension settings  
  return true;
}

/// @brief  Restore default project settings on a new project and existing 
///         project. For existing project SetSettingsFromXMLString() is called
///         later.
void CCVGGUIProjectSettings::RestoreDefaultProjectSettings()
{
  if (!m_bCCVGReadyToWork)
  {
    // Do nothing
    return;
  }

  const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rCurrentSettings(fw::CCVGFWTheCurrentStateProjectSettings::GetSettingsDefault());
  CopySettingsToWidgets(rCurrentSettings);
}

/// @brief  Load the current extension settings to the widget's controls. This 
///         function is called after the cancel button is clicked, and it is 
///         used to revert user settings changes. 
/// @return bool True = success, false = failure.
bool CCVGGUIProjectSettings::RestoreCurrentSettings() 
{
  if (!m_bCCVGReadyToWork)
  {
    // Do nothing
    return true;
  }

  const fw::CCVGFWTheCurrentStateProjectSettings::Settings &rCurrentSettings(m_pProjSettings->GetSettingsWorking());
  CopySettingsToWidgets(rCurrentSettings);

  m_pCCVGResultsFolderPathEdit->SetDisplayToolTip(rCurrentSettings.GetCcvgFolderDirResult().isEmpty());
  m_pCCVGSourceFolderPathEdit->SetDisplayToolTip(rCurrentSettings.GetCcvgFolderDirSource().isEmpty());
  m_pCCVGDriverFolderPathEdit->SetDisplayToolTip(rCurrentSettings.GetCcvgFolderDirDriver().isEmpty());
  m_pCCVGExecuteableFolderPathEdit->SetDisplayToolTip(rCurrentSettings.GetCcvgFolderDirExecuteable().isEmpty());

  return true;
}

/// @brief  Copy the current CCVG project settings to the appropriate widgets
///         which can then been changed by the user. 
/// @param[in] vSettings Data structure containing settings.
void CCVGGUIProjectSettings::CopySettingsToWidgets(const fw::CCVGFWTheCurrentStateProjectSettings::Settings &vSettings)
{ 
  const fw::CCVGFWTheCurrentState::Settings &currentWkSettings(m_pGlobalSettings->GetSettingsWorking());
  const bool bEnableAcrossAllProjs = currentWkSettings.GetCcvgAllProjectsEnabledFlag();
  const bool bSameUserAndPC = m_pProjSettings->IsUserAndPCTheSameAsPreviousUseOfTheCodeXLProject();
  const bool bUserEditedSettings = m_pProjSettings->GetUserEditedProjectSettingsOnThisComputer();
  const bool bEnabledPrjSettings = (bSameUserAndPC || bUserEditedSettings) && vSettings.GetCcvgProjectEnabledFlag();

  m_pCCVGProjectEnabledFlagGrpBx->setEnabled(bEnableAcrossAllProjs);
  m_pCCVGProjectEnabledFlagGrpBx->setChecked(bEnabledPrjSettings);
  m_pCCVGResultsFolderPathEdit->SetRealTextEntry(acGTStringToQString(vSettings.GetCcvgFolderDirResult()));
  m_pCCVGSourceFolderPathEdit->SetRealTextEntry(acGTStringToQString(vSettings.GetCcvgFolderDirSource()));
  m_pCCVGDriverFolderPathEdit->SetRealTextEntry(acGTStringToQString(vSettings.GetCcvgFolderDirDriver()));
  m_pCCVGExecuteableFolderPathEdit->SetRealTextEntry(acGTStringToQString(vSettings.GetCcvgFolderDirExecuteable()));
  m_pCCVGResultsFilesExtnEdit->setText(acGTStringToQString(vSettings.GetCcvgFilesExtnResult()));
  m_pCCVGSourceFilesExtnEdit->setText(acGTStringToQString(vSettings.GetCcvgFilesExtnSource()));
  m_pCCVGDriverFilesExtnEdit->setText(acGTStringToQString(vSettings.GetCcvgFilesExtnDriver()));
  m_pCCVGExecuteableFilesExtnEdit->setText(acGTStringToQString(vSettings.GetCcvgFilesExtnExecuteable()));
  m_pCCVGDriverGrpBx->setChecked(vSettings.GetCcvgDriverFilesMandatoryFlag());
}

/// @brief  Copy the settings from the GUI widgets to the  CCVG project 
///         settings data structure. 
/// @param[in] vSettings Data structure containing settings.
void CCVGGUIProjectSettings::CopyWidgetsToSettings(fw::CCVGFWTheCurrentStateProjectSettings::Settings &vCurrentSettings)
{
  gtString folderDirResult(acQStringToGTString(m_pCCVGResultsFolderPathEdit->GetRealTextEntry()));
  folderDirResult.trim();
  gtString folderDirSource(acQStringToGTString(m_pCCVGSourceFolderPathEdit->GetRealTextEntry()));
  folderDirSource.trim();
  gtString folderDirDriver(acQStringToGTString(m_pCCVGDriverFolderPathEdit->GetRealTextEntry()));
  folderDirDriver.trim();
  gtString folderDirExecuteable(acQStringToGTString(m_pCCVGExecuteableFolderPathEdit->GetRealTextEntry()));
  folderDirExecuteable.trim();
  gtString filesExtnResult(acQStringToGTString(m_pCCVGResultsFilesExtnEdit->text()));
  filesExtnResult.trim();
  gtString filesExtnSource(acQStringToGTString(m_pCCVGSourceFilesExtnEdit->text()));
  filesExtnSource.trim();
  gtString filesExtnDriver(acQStringToGTString(m_pCCVGDriverFilesExtnEdit->text()));
  filesExtnDriver.trim();
  gtString filesExtnExecuteable(acQStringToGTString(m_pCCVGExecuteableFilesExtnEdit->text()));
  filesExtnExecuteable.trim();

  vCurrentSettings.SetCcvgProjectEnabledFlag(m_pCCVGProjectEnabledFlagGrpBx->isChecked());
  vCurrentSettings.SetCcvgFolderDirResult(folderDirResult);
  vCurrentSettings.SetCcvgFolderDirSource(folderDirSource);
  vCurrentSettings.SetCcvgFolderDirDriver(folderDirDriver);
  vCurrentSettings.SetCcvgFolderDirExecuteable(folderDirExecuteable);
  vCurrentSettings.SetCcvgFilesExtnResult(filesExtnResult);
  vCurrentSettings.SetCcvgFilesExtnSource(filesExtnSource);
  vCurrentSettings.SetCcvgFilesExtnDriver(filesExtnDriver);
  vCurrentSettings.SetCcvgFilesExtnExecuteable(filesExtnExecuteable);
  vCurrentSettings.SetCcvgDriverFilesMandatoryFlag(m_pCCVGDriverGrpBx->isChecked());
}

/// @brief Overridden. Intercept widget showevent to update the GUI state of 
///        the CCVGProjectEnabledFlagGrpBx to reflect on the CCVG state 
///        'CCVG Enabled for all CodeXL projects' flag.
/// @param[in] vpEvent Qt event object.      
void CCVGGUIProjectSettings::showEvent(QShowEvent *vpEvent)
{
  afProjectSettingsExtension::showEvent(vpEvent);

  const bool bEnableAcrossAllProjs = m_pGlobalSettings->GetSettingsWorking().GetCcvgAllProjectsEnabledFlag();
  m_pCCVGProjectEnabledFlagGrpBx->setEnabled(bEnableAcrossAllProjs);
}

/// @brief On the user clicking the project groupbox record that the user has
///        enabled for project settings at least once for the current project
///        being used.
/// @param[in] vbChecked True = checked, false = unchecked.
void CCVGGUIProjectSettings::OnClickGrpBxProjectEnabledFlag(bool vbChecked)
{
  GT_UNREFERENCED_PARAMETER(vbChecked);

  m_pProjSettings->SetUserEditedProjectSettingsOnThisComputer(true);
}

/// @brief If the user has changed the project setting flag "Driver files are
///        required"for a set of Results files, then go through the current
///        project data and adjust the data to reflect this new requirement.
///        A CCVGItem structure holds the set of files attributes. A set of 
///        files can be a set of 3 (Results, Source and Drivers) or set of 2
///        (Results and Source files only).
/// @param[in] bool vbRequired True = Driver files required, false = not
///            required.
status CCVGGUIProjectSettings::ExecuteCmdProjDataSetNoDriverFilesReq(bool vbRequired)
{
  using namespace ccvg::cmd;
  CCVGCmdInvoker &rInvoker(CCVGCmdInvoker::Instance());
  CCVGCmdParams *pCmdParams = nullptr;
  fnstatus::status status = rInvoker.CmdGetParams(CmdID::kProjDataSetNoDriverFilesReq, pCmdParams);
  const gtString &rCmdName(rInvoker.CmdGetNameAboutToExecute());
  bool bCaughtException = false;
  if (status == success)
  {
    try
    {
      CCVGCmdParameter *pParamFlagDriverReq = pCmdParams->GetParam(CCVGCmdProjDataSetNoDriverFilesReq::kParamID_FlagDriversRequired);
      bool bOk = pParamFlagDriverReq->param.Set<bool>(vbRequired);
      bOk = bOk && (rInvoker.CmdExecute() == success);
      status = bOk;
    }
    catch (...)
    {
      bCaughtException = true;
      status = failure;
    }
  }
  rInvoker.CmdHandleError(status, bCaughtException, rCmdName);

  return status;
}

/// @brief Overridden. Notification from the afProjectManager that the project 
///        is now closed.
/// @return bool True = success, false = failure.
bool CCVGGUIProjectSettings::CloseCurrentProject()
{
  m_pGlobalSettings->SetProjectIsOpen(false);
  const status ok = NotifyProjectClosed();
  return (ok == success) ? true : false;
}

/// @brief Register with *this class for notifications of the current project 
///        is closed.
/// @param[in] NotifyOnProjectDataClose& Object to notify.
/// @return status success = registration ok, false = error no registration.
status CCVGGUIProjectSettings::RegisterForNotificationOnProjectClose(const NotifyOnProjectClose &vObj)
{
  m_vecNotifyOnProjectClose.push_back(const_cast<NotifyOnProjectClose *>(&vObj));
  return success;
}

/// @brief Notify registered listeners that the project has just been 
///        closed. Internally to *this object it can disable the notification
///        for example on object shutdown();
/// @return status success = all notifications ok, false = one or more 
///        stated they failed talk of carrying out the notifocation.
status CCVGGUIProjectSettings::NotifyProjectClosed()
{
  if (m_bDoNotNotifyProjectClose)
  {
    return success;
  }

  CCVGuint errCount = 0;
  for (NotifyOnProjectClose *pObj : m_vecNotifyOnProjectClose)
  {
    errCount += (pObj->OnNotifyClose() == success) ? 0 : 1;
  }

  return ((errCount == 0) ? success : failure);
}

} // namespace gui
}  //namespace ccvg
