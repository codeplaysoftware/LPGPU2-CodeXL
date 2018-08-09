// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVG resources handler for icons, pixmap, images. For more efficient use of 
/// resources and the sharing of them the resources object hands out resources
/// on being given a resource ID. 
/// CCVG resource handler is a Package. Packages in CCVG are classes or a set
/// classes (rare), probaby singletons, that carry out one task only. Packages 
/// initialise other Packages in their Initialise() function that need in 
/// order to operate themselves. They are also responsible to to shutdown
/// those packages they have initiated.
///
/// CCVGIconResource   interface
/// CCVGResourcesIcon  interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_RESOURCEICON_H_INCLUDE
#define LPGPU2_CCVG_RESOURCEICON_H_INCLUDE

// Infra:
#include <AMDTApplicationComponents/Include/acIcons.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_SingletonBase.h>

// Declarations:
class QPixmap;

/// @Brief Common Resources icon resource IDs for the CCVG.
///        All the IDs must be unique.
enum {
  kIDS_CCVG_ICON_err_internal_fail = 0,
  kIDS_CCVG_ICON_CCVGExecutionMode,
  kIDS_CCVG_ICON_AppTreeRootNode_SetFilesGood,
  kIDS_CCVG_ICON_AppTreeRootNode_SetFilesBad,
  kIDS_CCVG_ICON_AppTreeCCVGItem,
  kIDS_CCVG_ICON_AppTreeCCVGItem_Results,
  kIDS_CCVG_ICON_AppTreeCCVGItem_Source,
  kIDS_CCVG_ICON_AppTreeCCVGItem_Drivers,
  kIDS_CCVG_ICON_AppTreeCCVGItem_Executeable,
  kIDS_CCVG_ICON_AppTreeCCVGItem_Bad,
  kIDS_CCVG_ICON_AppTreeCCVGItem_Results_Bad,
  kIDS_CCVG_ICON_AppTreeCCVGItem_Source_Bad,
  kIDS_CCVG_ICON_AppTreeCCVGItem_Drivers_Bad,
  kIDS_CCVG_ICON_AppTreeCCVGItem_Executeable_Bad,
  kIDS_CCVG_ICON_resource_count
};

/// @Brief A resource structure. Common Resources icon type for the CCVG.  
// clang_format off
struct CCVGIconResource {
  CCVGuint  m_resourceId;  // Resource ID
  QPixmap  *m_pPixmap;     // Icon object
};
// clang_format on

/// @Brief Common icon resources handler class for the CCVG.  
///        Singleton pattern. CCVGResourcesIcon is Package.
///
/// The handler is the one of the very first objects to be setup. Its
/// primary use here is to hand out image or icon resources.
/// All the resources are statically defined at compile time.
///
/// @warning  None.
/// @see      None.
/// @date     24/01/2017.
/// @author   Illya Rudkin.
// clang_format off
class CCVGResourcesIcon final 
: public CCVGClassCmnBase
, public ISingleton<CCVGResourcesIcon>
{
  friend ISingleton<CCVGResourcesIcon>;

// Enums:
private:
enum 
{
  kIconResourceItemCount = kIDS_CCVG_ICON_resource_count
};

// Methods:
public:
  const CCVGIconResource& GetResource(CCVGuint vId);
  bool                    HasIconResource(CCVGuint vId);

// Overridden:
public:
  virtual ~CCVGResourcesIcon() override;
  // From util::ISingleton
  virtual status Initialise() override;
  virtual status Shutdown() override;

// Methods:
private:
  // Singleton enforcement
  CCVGResourcesIcon();
  CCVGResourcesIcon(const CCVGResourcesIcon&) = delete;
  CCVGResourcesIcon(CCVGResourcesIcon&&) = delete;
  CCVGResourcesIcon& operator= (const CCVGResourcesIcon&) = delete;
  CCVGResourcesIcon& operator= (CCVGResourcesIcon&&) = delete;
  //
  status GetIconFromResource(CCVGuint veGlobalIconId, const CCVGIconResource *&vRsrc);
  status CreateResource(CCVGIconResource &vrRsrc, CCVGuint veGlobalIconId, acIconId vCodeXLId);
  status CreateResource(CCVGuint veGlobalIconId, acIconId vCodeXLId);
  status DestroyResource(CCVGIconResource &vrRsrc);
  status DestroyResourceAll();

// Attributes:
private:
  static CCVGIconResource ms_arrayIconResources[kIconResourceItemCount + 1];
  static CCVGIconResource ms_thisErrorGetFailedIcon;
};
// clang-format on

/// @brief CCVG Resource retrieval shortcut macro for icon resources
#define CCVGRSRCICON(x) CCVGResourcesIcon::Instance().GetResource(x).m_pPixmap

#endif // LPGPU2_CCVG_RESOURCEICON_H_INCLUDE