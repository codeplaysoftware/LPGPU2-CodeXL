// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWMDIViewFactory interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_MDIVIEWFACTORY_H_INCLUDE
#define LPGPU2_CCVG_FW_MDIVIEWFACTORY_H_INCLUDE

// STL:
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
class CCVGFWMDIViewBase;

/// @brief    CCVG's view object creator.
/// @warning  None.
/// @see      CCVGFWMDIViewMgr, 
///           CCVGFWMDIViewCreator,
///           CCVGFWMDIViewBase.
/// @date     16/03/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWMDIViewFactory final
: public CCVGClassCmnBase
, public ISingleton<CCVGFWMDIViewFactory>
{
  friend ISingleton<CCVGFWMDIViewFactory>;
  
// Typedefs:
public:
  typedef CCVGFWMDIViewBase *(*ViewCreatorFnPtr)();

// Classes:
public:
  /// @brief CCVG views inherit this interface.
  class IFactory
  {
  public:
    virtual const gtString&     GetViewId() const = 0;
    virtual gtString            GetViewName() const = 0;
    virtual EnumCCVGMDIViewType GetViewType() const = 0;
    virtual ViewCreatorFnPtr    GetCreatorFn() const = 0;
    virtual ::QWidget*          GetWidget() = 0;
    /* virtual CCVGFWMDIViewBase* CreateSelf() = 0    // Not possible as require a static creator
                                                      // function in the command class, here for
                                                      // awareness.
    */

    /* dtor */ virtual ~IFactory() {}; // Objs are ref'ed and deleted by this base class
  };

// Methods:
public:
  ~CCVGFWMDIViewFactory();
  //
  status Initialise();
  status Shutdown();
  //
  status ViewRegister(EnumCCVGMDIViewType vkViewType, ViewCreatorFnPtr vCmdCreateFn, const gtString &vrViewName);
  status ViewCreate(const gtString &vViewid, EnumCCVGMDIViewType vkViewType, ::QWidget *&vpViewWgt, CCVGFWMDIViewBase *&vpView);

// Typedefs:
private:
  using MapViewTypeToViewCreatorFn_t = std::map<EnumCCVGMDIViewType, ViewCreatorFnPtr>;
  using MapPairViewTypeToViewCreatorFn_t = std::pair<EnumCCVGMDIViewType, ViewCreatorFnPtr>;

// Methods:
private:
  // Singleton enforcement
  CCVGFWMDIViewFactory();
  CCVGFWMDIViewFactory(const CCVGFWMDIViewFactory&) = delete;
  CCVGFWMDIViewFactory(CCVGFWMDIViewFactory&&) = delete;
  CCVGFWMDIViewFactory& operator= (const CCVGFWMDIViewFactory&) = delete;
  CCVGFWMDIViewFactory& operator= (CCVGFWMDIViewFactory&&) = delete;
  // 
  bool IsValidViewId(const gtString &vViewId);
  bool IsValidViewType(EnumCCVGMDIViewType vkViewType);
  bool HaveViewTypeAlreadyRegistered(EnumCCVGMDIViewType vkViewType) const;

  // Attributes:
private:
  MapViewTypeToViewCreatorFn_t *m_mapViewTypeToViewCreatorFn;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_MDIVIEWFACTORY_H_INCLUDE