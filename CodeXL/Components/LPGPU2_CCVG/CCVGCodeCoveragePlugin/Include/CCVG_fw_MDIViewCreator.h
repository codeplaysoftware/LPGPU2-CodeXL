// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGMenuActionsExecutor interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_MDIVIEWCREATOR_H_INCLUDE
#define LPGPU2_CCVG_FW_MDIVIEWCREATOR_H_INCLUDE

// Infra:
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewEnumTypes.h>

// Declarations:
class QWidget;

namespace ccvg {
namespace fw {

// Declarations:
class  CCVGMDIGeneralActionsCreator;

/// @brief    CCVGMDIViewCreator derived from afQtViewCreatorAbstract. 
///           Concrete implementation for the MDI view(s). A class handling
///           the creation of the execution mode's views. A mode can have
///           number of views. There are two types of view; (Qt) dockable
///           views, (Qt) MDI views. Dockable views can be docked at the sides
///           of the client area or float. Dockable views are created 
///           automatically when CodeXL is opened. They can be hidden or shown
///           based on the active mode but they are not destroyed and created,
///           always exist. MDI views are only created when a specific document
///           is requested to be opened or created. MDI view only visible in 
///           the client area, normally as tab view panes (like the other in the
///           other modes). All views are derived from afQtViewCreatorAbstract.
///           Views are identified by an index. A plugin (an execution mode) 
///           can  have several creators or which each can create limitless 
///           views.
///           *this class is a fullfill the operation interface to allow it
///           fullfil CCVG's execution mode to integrate and create views
///           for the CodeXL application. It is also a facade to the CCVG's
///           view manager. 
/// @warning  None.
/// @see      CCVGFWMDIViewMgr.
/// @date     12/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGMDIViewCreator 
: public afQtViewCreatorAbstract
, public CCVGClassCmnBase
{
// Statics:
public:
  static const gtString& GetId();

// Methods:
public:
  CCVGMDIViewCreator();
  status Initialise();
  status Shutdown();

// Overridden:
public:
  // From afQtViewCreatorAbstract
  ~CCVGMDIViewCreator();

  // From afViewCreatorAbstract
  virtual void                              titleString(int vViewIndex, gtString &vViewTitle, gtString &vViewMenuCommand) override;
  virtual gtString                          associatedToolbar(int vViewIndex) override;
  virtual afViewCreatorAbstract::afViewType type(int vViewIndex) override;
  virtual int                               dockArea(int vViewIndex) override;
  virtual bool                              isDynamic() override;
  virtual const gtString                    CreatedMDIType() const override;
  virtual QDockWidget::DockWidgetFeatures   dockWidgetFeatures(int vViewIndex) override;
  virtual QSize                             initialSize(int vViewIndex) override;
  virtual bool                              visibility(int vViewIndex) override;
  virtual int                               amountOfViewTypes() override;
  virtual void                              handleTrigger(int vViewIndex, int vActionIndex) override;
  virtual void                              handleUiUpdate(int vViewIndex, int vActionIndex) override;
  virtual bool                              createViewContent(int vViewIndex, QWidget *&vpContentQWidget, QWidget *vpQParent) override;
  virtual bool                              displayExistingView(const apMDIViewCreateEvent &vMdiViewEvent) override;
  virtual bool                              getCurrentlyDisplayedFilePath(osFilePath &vFilePath) override;
  virtual bool                              onMDISubWindowClose(afQMdiSubWindow *vpMDISubWindow) override;
  virtual QPixmap *                         iconAsPixmap(int vViewIndex);

// Methods:
private:
  void      LogError(const wchar_t *vpErrMsg, const wchar_t *vpErrCntxt);
  bool      ViewIsExist(const gtString &rViewId);
  status    ViewDelete(const gtString &rViewId);
  void      ViewDeleteAll();
  QWidget*  ViewGet(const gtString &rViewId);
  status    ViewClose(const gtString &rViewId);

// Attributes:
private:
  const gtString m_debugThisObIdStr; // Used to ID *this class when stepping through for next loops
  //                                    of all CodeXL views.
  bool m_bBeenInitialised;  // True = yes this has been initialised, false = not yet
  bool m_bBeenShutdown;     // True = yes this has been shutdown already, false = not yet
  //
  CCVGMDIGeneralActionsCreator *m_pViewActionCreator;
  static gtString               ms_constThisCreatorId;
};
// clang-format on

} // namespace ccvg
} // namespace fw

#endif //LPGPU2_CCVG_FW_MDIVIEWCREATOR_H_INCLUDE
