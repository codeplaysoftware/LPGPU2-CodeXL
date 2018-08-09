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
///
/// CCVGResourcesColor  implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesColor.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

// Instantiations:
CCVGColorResource CCVGResourcesColor::ms_thisErrorGetFailedColor = {
  kIDS_resource_err_internal_default, QColor(255,255,255)
};

/// @Brief Color resoures used only for the CCVG code base.
CCVGColorResource CCVGResourcesColor::ms_arrayColorResources[] = {
  { kIDS_CCVG_COL_Highlight_Covered, QColor(150,255,125)},
  { kIDS_CCVG_COL_Highlight_Uncovered, QColor(255,150,150)},
  { kIDS_CCVG_COL_Highlight_BranchTakenNone, QColor(255,50,50)},
  { kIDS_CCVG_COL_Highlight_BranchTakenOne, QColor(255,255,50)},
  { kIDS_CCVG_COL_Highlight_BranchTakenBoth, QColor(0,150,0)}
};

/// @brief  Package initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGResourcesColor::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised) 
  {
    return success;
  }

  // Do nothing here

  m_bBeenInitialised = true;
  return success;
}

/// @brief  Package shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGResourcesColor::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0)) 
  {
    return success;
  }

  // Do nothing here

  m_bBeenShutdown = true;
  return success;
}

/// @brief  Class constructor. No work is done here.
CCVGResourcesColor::CCVGResourcesColor()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGResourcesColor::~CCVGResourcesColor()
{
  Shutdown();
}

/// @brief Retrieve the resource structure represented by the resource ID.
/// @param[in]   vId Unique resource ID.
/// @param[out]  vRsrc Pointer the resource structure holding the resource.
/// @return bool True = function succeeded in its task,
///              false = function failed in its task.
bool CCVGResourcesColor::GetColorFromResource(CCVGuint vId, const CCVGColorResource *&vRsrc)
{
  if (vId >= kColorResourceItemCount) 
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_err_id_not_recognised));
  }
     
  const CCVGColorResource &resource = ms_arrayColorResources[vId];
  if (resource.m_resourceId != vId) 
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_err_id_mismatch));
  }

  vRsrc = &resource;

  return true;
}

/// @brief Retrieve the resource structure represented by the resource ID.
///        Debug assert is performed should the ID not be recognised. If in
///        release build the default error resource structure in returned
///        containing a error message in the case of a string resource or
///        a dummy resource object.
/// @param[in] vId Unique resource ID.
/// @return    CCVGStringResource& Reference to object.
const CCVGColorResource& CCVGResourcesColor::GetResource(CCVGuint vId)
{
  const CCVGColorResource *pRsrc = nullptr;
  const bool bFound = GetColorFromResource(vId, pRsrc);
  GT_ASSERT_EX(bFound, CCVGRSRC(kIDS_resource_err_id_not_recognised));
  if (pRsrc != nullptr)
  {
    return *pRsrc;
  }

  // Default color if lookup fails:
  return ms_thisErrorGetFailedColor;
}

/// @brief Determine if the resource structure represented by the resource ID
///        is present and recognised.
/// @param[in] vId Unique resource ID.
/// @return    bool True = ID is valid, false = not valid.
bool CCVGResourcesColor::HasColorResource(CCVGuint vId)
{
  const CCVGColorResource *pRsrc = nullptr;
  return GetColorFromResource(vId, pRsrc);
}

