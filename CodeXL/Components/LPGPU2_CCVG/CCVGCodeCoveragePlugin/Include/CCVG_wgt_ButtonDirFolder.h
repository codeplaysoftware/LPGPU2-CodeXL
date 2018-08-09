// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGWGTButtonDirFolder interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_WGT_BUTTONDIRFOLDER_H_INCLUDE
#define LPGPU2_CCVG_WGT_BUTTONDIRFOLDER_H_INCLUDE

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/Include/afBrowseAction.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_wgt_ButtonDirFolder.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_WidgetStringConstants.h>

// Definitions:
constexpr int AF_NEW_PROJECT_BUTTON_STANDARD_SIZE = 21; // Copied in afNewProjectDialof.cpp

// Declarations:
class afBrowseAction;
namespace ccvg { namespace gui { extern const char* g_arrayWgtHistMgrTag[]; }}

/// @brief    CCVG directory folder button widget comprised of a QToolButton
///           and CodeXL afBrowseAction together. The user clicks a button
///           displaying a folder icon to then open a folder dialog.
///           On creating an instance of this class a QToolbutton and action
///           are created. They are not deleted by *this class on *this class
///           destruction, but by the widget it was added too (addWidget()).
/// @param    HISTMGRTAGIDID History management unique ID tag for afBrowseAction. 
/// @warning  None.
/// @see      None.
/// @date     16/12/2016.
/// @author   Illya Rudkin.
// clang-format off
template<int HISTMGRTAGID>
class CCVGWGTButtonDirFolder final
{
// Methods:
public:
  CCVGWGTButtonDirFolder();
  CCVGWGTButtonDirFolder(const CCVGWGTButtonDirFolder&) = delete;
  CCVGWGTButtonDirFolder& operator=(const CCVGWGTButtonDirFolder&) = delete;
  CCVGWGTButtonDirFolder(CCVGWGTButtonDirFolder&&) = delete;
  CCVGWGTButtonDirFolder& operator=(CCVGWGTButtonDirFolder&&) = delete;

  ~CCVGWGTButtonDirFolder();
  QToolButton*    GetButton() const;
  afBrowseAction* GetAction() const;
  void            SetTooltip(const QString &vTxt);

// Attributes:
private:
  QToolButton     *m_pFolderPathButton; 
  afBrowseAction  *m_pFolderPathButtonAction;
  mutable bool     m_ownObjQToolButton;       // True = *this owns it, false = 3rd party owns the object
  mutable bool     m_ownObjBrowseAction;      // True = *this owns it, false = 3rd party owns the object
};
// clang-format on

/// @brief Class constructor. 
template<int HISTMGRTAGID>
CCVGWGTButtonDirFolder<HISTMGRTAGID>::CCVGWGTButtonDirFolder()
: m_pFolderPathButton(nullptr)
, m_pFolderPathButtonAction(nullptr)
, m_ownObjQToolButton(true)
, m_ownObjBrowseAction(true)
{
  m_pFolderPathButton = new QToolButton;
  m_pFolderPathButtonAction = new afBrowseAction(ccvg::gui::g_arrayWgtHistMgrTag[HISTMGRTAGID]);

  const int buttonDim = static_cast<int>(acScalePixelSizeToDisplayDPI(AF_NEW_PROJECT_BUTTON_STANDARD_SIZE));
  const QSize buttonSize(buttonDim, buttonDim);

  m_pFolderPathButton->setMaximumSize(buttonSize);
  m_pFolderPathButton->setMinimumSize(buttonSize);
  m_pFolderPathButton->setDefaultAction(m_pFolderPathButtonAction);
  m_pFolderPathButtonAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
}

/// @brief  Class destructor. Tidy up release resources used by 
///        *this instance.
template<int HISTMGRTAGID>
CCVGWGTButtonDirFolder<HISTMGRTAGID>::~CCVGWGTButtonDirFolder()
{
  if (m_ownObjQToolButton && m_pFolderPathButton != nullptr)
  {
    delete m_pFolderPathButton;
  }
  m_pFolderPathButton = nullptr;
  
  if (m_ownObjBrowseAction)
  {
    delete m_pFolderPathButtonAction;
  }
  m_pFolderPathButtonAction = nullptr;
}

/// @brief  Retrieve the directory folder button object. The caller takes
///        ownership of the object to destroy.
/// @return QToolButton* The button object.
template<int HISTMGRTAGID>
QToolButton* CCVGWGTButtonDirFolder<HISTMGRTAGID>::GetButton() const
{
  m_ownObjQToolButton = false;
  return m_pFolderPathButton;
}

/// @brief     Set the tooltip text to be display for folder directory button.
/// @param[in] vTxt Tooltip text.
template<int HISTMGRTAGID>
void CCVGWGTButtonDirFolder<HISTMGRTAGID>::SetTooltip(const QString &vTxt)
{
  m_pFolderPathButtonAction->setToolTip(vTxt);
}

/// @brief  Retrieve the browse action object associated with the directory 
///         folder button object. The caller takes ownership of the object to
///         destroy.
/// @return afBrowseAction* The browse action object.
template<int HISTMGRTAGID>
afBrowseAction* CCVGWGTButtonDirFolder<HISTMGRTAGID>::GetAction() const
{
  m_ownObjBrowseAction = false;
  return m_pFolderPathButtonAction;
}

#endif // LPGPU2_CCVG_WGT_BUTTONDIRFOLDER_H_INCLUDE