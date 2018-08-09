// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWMDIViewMgr interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_MDIVIEWMGR_H_INCLUDE
#define LPGPU2_CCVG_FW_MDIVIEWMGR_H_INCLUDE

// Std:
#include <map>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_SingletonBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewEnumTypes.h>

// Declarations:
class QWidget;

namespace ccvg {
namespace fw {

// Declarations:
class CCVGFWMDIViewFactory;
class CCVGFWMDIViewBase;

/// @brief    CCVG execution mode MDI and dock view object manager.
///           CCVGFWMDIViewCreator is the bridging facade through which
///           the CCVG view handling is integrated into the CodeXL view
///           framework.
/// @warning  None.
/// @see      CCVGFWMDIViewFactory, 
///           CCVGFWMDIViewCreator,
///           CCVGFWMDIViewBase
/// @date     16/03/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWMDIViewMgr final
: public CCVGClassCmnBase
, public ISingleton<CCVGFWMDIViewMgr>
{
  friend ISingleton<CCVGFWMDIViewMgr>;
  
  // Classes:
public:
  /// @brief CCVG views inherit this interface.
  class IView
  {
  public:
    virtual status Initialise() = 0;
    virtual status Shutdown() = 0;

    /* dtor */ virtual ~IView() {}; // Objs are ref'ed and deleted by this base class
  };

// Methods:
public:
  ~CCVGFWMDIViewMgr();
  //
  status Initialise();
  status Shutdown();
  //
  status              ViewCreate(const gtString &rViewId, EnumCCVGMDIViewType veViewType, QWidget *vpQParent, ::QWidget *&vpViewWgt, CCVGFWMDIViewBase *&vpView);
  bool                ViewIsExist(const gtString &rViewId);
  status              ViewDelete(const gtString &rViewId);
  void                ViewDeleteAll();
  ::QWidget*          ViewGetWidget(const gtString &rViewId);
  CCVGFWMDIViewBase*  ViewGet(const gtString &rViewId);
  status              ViewClose(const gtString &rViewId);
  CCVGFWMDIViewBase*  ViewGetSubClass(QWidget &vrForThisWgt) const;

// Typedefs:
private:
  using MapCCVGItemIdToViewWgt_t = std::map<gtString, ::QWidget*>;
  using MapPairCCVGItemIdToViewWgt_t = std::pair<gtString, ::QWidget*>;
  using MapCCVGItemIdToView_t = std::map<gtString, CCVGFWMDIViewBase*>;
  using MapPairCCVGItemIdToView_t = std::pair<gtString, CCVGFWMDIViewBase*>;
  using MapViewWgtToView_t = std::map<::QWidget*, CCVGFWMDIViewBase*>;
  using MapPairViewWgtToView_t = std::pair<::QWidget*, CCVGFWMDIViewBase*>;

// Methods:
private:
  // Singleton enforcement
  CCVGFWMDIViewMgr();
  CCVGFWMDIViewMgr(const CCVGFWMDIViewMgr&) = delete;
  CCVGFWMDIViewMgr(CCVGFWMDIViewMgr&&) = delete;
  CCVGFWMDIViewMgr& operator= (const CCVGFWMDIViewMgr&) = delete;
  CCVGFWMDIViewMgr& operator= (CCVGFWMDIViewMgr&&) = delete;
  //
  status ViewAdd(const gtString &rViewId, QWidget &vNewViewWgt, CCVGFWMDIViewBase &NewView);
  bool   IsValidViewId(const gtString &vViewId);

// Attributes:
private:
  CCVGFWMDIViewFactory      *m_pMDIViewFactory;
  MapCCVGItemIdToViewWgt_t  *m_mapMapCCVGItemIdToViewWgt;
  MapCCVGItemIdToView_t     *m_mapMapCCVGItemIdToView;
  MapViewWgtToView_t        *m_mapMapViewWgtToView;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_MDIVIEWMGR_H_INCLUDE