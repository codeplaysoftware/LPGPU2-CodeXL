// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVG resources handler for text messages. For more efficient use of 
/// resources and the sharing of them the resources object hands out resources
/// on being given a resource ID. If necessary when appropriate the resources 
/// for the case of text messages could handle language locality. 
/// CCVG resource handler is a Package. Packages in CCVG are classes or a set
/// classes (rare), probaby singletons, that carry out one task only. Packages 
/// initialise other Packages in their Initialise() function that need in 
/// order to operate themselves. They are also responsible to to shutdown
/// those packages they have initiated.
///
/// CCVGStringResource   interface
/// CCVGResourcesString  interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_RESOURCECOLOR_H_INCLUDE
#define LPGPU2_CCVG_RESOURCECOLOR_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_SingletonBase.h>

// Qt:
 #include <QColor> 

/// @Brief Common Resources Color type resource IDs for the CCVG.
///        All the IDs must be unique.
enum {
  kIDS_CCVG_COL_Highlight_Covered = 0,
  kIDS_CCVG_COL_Highlight_Uncovered,
  kIDS_CCVG_COL_Highlight_BranchTakenNone,
  kIDS_CCVG_COL_Highlight_BranchTakenOne,
  kIDS_CCVG_COL_Highlight_BranchTakenBoth,
  kIDS_resource_err_internal_default,
  kIDS_CCVG_COL_resource_count
};

/// @Brief A resource structure. Common Resources Color type for the CCVG.  
// clang_format off
struct CCVGColorResource {
  CCVGuint       m_resourceId;  // Resource ID
  QColor         m_color;       // Qt Color
};
// clang_format on

/// @Brief Common color resources handler class for the CCVG.  
///        Singleton pattern. CCVGResourcesColor is Package.
///
/// The handler is the one of the very first objects to be setup. Its
/// primary use here is to hand out color resources.
/// All the resources are statically defined at compile time.
///
/// @warning  None.
/// @see      None.
/// @date     01/09/2017.
/// @author   Callum Fare.
// clang_format off
class CCVGResourcesColor final 
: public CCVGClassCmnBase
, public ISingleton<CCVGResourcesColor>
{
  friend ISingleton<CCVGResourcesColor>;

// Enums:
private:
enum 
{
  kColorResourceItemCount = kIDS_CCVG_COL_resource_count
};

// Methods:
public:
  const CCVGColorResource& GetResource(CCVGuint vId);
  bool                     HasColorResource(CCVGuint vId);

// Overridden:
public:
  virtual ~CCVGResourcesColor() override;
  // From util::ISingleton
  virtual status Initialise() override;
  virtual status Shutdown() override;

// Methods:
private:
   // Singleton enforcement
   CCVGResourcesColor();
   CCVGResourcesColor(const CCVGResourcesColor&) = delete;
   CCVGResourcesColor(CCVGResourcesColor&&) = delete;
   CCVGResourcesColor& operator= (const CCVGResourcesColor&) = delete;
   CCVGResourcesColor& operator= (CCVGResourcesColor&&) = delete;
   //
   bool GetColorFromResource(CCVGuint vId, const CCVGColorResource *&vRsrc);

// Attributes:
private:
  static CCVGColorResource ms_arrayColorResources[kColorResourceItemCount + 1];
  static CCVGColorResource ms_thisErrorGetFailedColor;
};
// clang-format on

/// @brief CCVG Resource retrieval shortcut macro for text resources
#define CCVGRSRCCOL(x) CCVGResourcesColor::Instance().GetResource(x).m_color

#endif // LPGPU2_CCVG_RESOURCECOLOR_H_INCLUDE