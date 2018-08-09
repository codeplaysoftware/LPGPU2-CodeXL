// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVG resources handler for image an icons. For more efficient use of 
/// resources and the sharing of them the resources object hands out resources
/// on being given a resource ID.  
///
/// CCVGResourcesIcon  implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Qt:
#include <qwidget.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesIcon.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

// Instantiations:
CCVGIconResource CCVGResourcesIcon::ms_arrayIconResources[];
CCVGIconResource CCVGResourcesIcon::ms_thisErrorGetFailedIcon;

/// @brief  Package initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGResourcesIcon::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised) 
  {
    return success;
  }

  status ok = CreateResource(ms_thisErrorGetFailedIcon, kIDS_CCVG_ICON_err_internal_fail, AC_ICON_EMPTY);
  ok = ok && CreateResource(kIDS_CCVG_ICON_CCVGExecutionMode, AC_ICON_CCVG_MODE);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeRootNode_SetFilesGood, AC_ICON_CCVG_APPTREEROOTNODE_SETFILESGOOD);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeRootNode_SetFilesBad, AC_ICON_CCVG_APPTREEROOTNODE_SETFILESBAD);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeCCVGItem, AC_ICON_CCVG_APPTREECCVGITEM);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeCCVGItem_Results, AC_ICON_CCVG_APPTREECCVGITEM_RESULTS);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeCCVGItem_Source, AC_ICON_CCVG_APPTREECCVGITEM_SOURCE);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeCCVGItem_Drivers, AC_ICON_CCVG_APPTREECCVGITEM_DRIVERS);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeCCVGItem_Executeable, AC_ICON_CCVG_APPTREECCVGITEM_EXECUTEABLE);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeCCVGItem_Bad, AC_ICON_CCVG_APPTREECCVGITEM_BAD);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeCCVGItem_Results_Bad, AC_ICON_CCVG_APPTREECCVGITEM_RESULTS_BAD);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeCCVGItem_Source_Bad, AC_ICON_CCVG_APPTREECCVGITEM_SOURCE_BAD);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeCCVGItem_Drivers_Bad, AC_ICON_CCVG_APPTREECCVGITEM_DRIVERS_BAD);
  ok = ok && CreateResource(kIDS_CCVG_ICON_AppTreeCCVGItem_Executeable_Bad, AC_ICON_CCVG_APPTREECCVGITEM_EXECUTEABLE_BAD);

  m_bBeenInitialised = (ok == success);
  return ok;
}

/// @brief  Package shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGResourcesIcon::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0)) 
  {
    return success;
  }

  const status ok = DestroyResourceAll();

  m_bBeenShutdown = true;
  return ok;
}

/// @brief  Class constructor. No work is done here.
CCVGResourcesIcon::CCVGResourcesIcon()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGResourcesIcon::~CCVGResourcesIcon()
{
  Shutdown();
}

/// @brief Retrieve the resource structure represented by the resource ID.
/// @param[in]   veGlobalIconId Unique resource ID.
/// @param[out]  vRsrc Pointer the resource structure holding the resource.
/// @return status success = function succeeded in its task,
///                failure = function failed in its task.
status CCVGResourcesIcon::GetIconFromResource(CCVGuint veGlobalIconId, const CCVGIconResource *&vRsrc)
{
  if (veGlobalIconId >= kIDS_CCVG_ICON_resource_count)
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_icon_err_id_not_recognised));
  }
     
  const CCVGIconResource &rResource = ms_arrayIconResources[veGlobalIconId];
  if (rResource.m_resourceId != veGlobalIconId)
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_icon_err_id_mismatch));
  }

  vRsrc = &rResource;

  return success;
}

/// @brief Retrieve the resource structure represented by the resource ID.
///        Debug assert is performed should the ID not be recognised. If in
///        release build the default error resource structure in returned
///        containing a error message in the case of a string resource or
///        a dummy resource object.
/// @param[in] veGlobalIconId Unique resource ID.
/// @return    CCVGStringResource& Reference to object.
const CCVGIconResource& CCVGResourcesIcon::GetResource(CCVGuint veGlobalIconId)
{
  const CCVGIconResource *pRsrc = nullptr;
  const bool bFound = GetIconFromResource(veGlobalIconId, pRsrc);
  GT_ASSERT_EX(bFound, CCVGRSRC(kIDS_resource_icon_err_id_not_recognised));
  if (pRsrc != nullptr)
  {
    return *pRsrc;
  }

  return ms_thisErrorGetFailedIcon;
}

/// @brief Determine if the resource structure represented by the resource ID
///        is present and recognised.
/// @param[in] veGlobalIconId Unique resource ID.
/// @return    bool True = ID is valid, false = not valid.
bool CCVGResourcesIcon::HasIconResource(CCVGuint veGlobalIconId)
{
  const CCVGIconResource *pRsrc = nullptr;
  return GetIconFromResource(veGlobalIconId, pRsrc);
}

/// @brief  For a specific icon resource object assign the specified icon 
///         image retrieved from CodeXL image of all icons.
/// @param[in] vrRsrc A CCVG con resource object.
/// @param[in] veGlobalIconId A CCVG enumeration icon resource ID.
/// @param[in] vCodeXLId A CodeXL enumeration icon resource ID.
/// @return status Success = task completed task ok, failure = task failed.
status CCVGResourcesIcon::CreateResource(CCVGIconResource &vrRsrc, CCVGuint veGlobalIconId, acIconId vCodeXLId)
{
  if (veGlobalIconId >= kIDS_CCVG_ICON_resource_count)
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_icon_err_id_not_recognised));
  }
  QPixmap *pPixmap = new (std::nothrow) QPixmap;
  if (pPixmap == nullptr)
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_icon_err_create_failed));
  }
  if(!acSetIconInPixmap(*pPixmap, vCodeXLId))
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_icon_err_create_failed));
  }
  
  vrRsrc.m_resourceId = kIDS_CCVG_ICON_err_internal_fail;
  vrRsrc.m_pPixmap = pPixmap;

  return success;
}

/// @brief  For a specific icon resource object located internally in a
///         container assign the specified icon image retrieved from CodeXL
///         image of all icons. 
/// @param[in] veGlobalIconId A CCVG enumeration icon resource ID.
/// @param[in] vCodeXLId A CodeXL enumeration icon resource ID.
/// @return status Success = task completed task ok, failure = task failed.
status CCVGResourcesIcon::CreateResource(CCVGuint veGlobalIconId, acIconId vCodeXLId)
{
  if (veGlobalIconId >= kIDS_CCVG_ICON_resource_count)
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_icon_err_id_not_recognised));
  }
  QPixmap *pPixmap = new (std::nothrow) QPixmap;
  if (pPixmap == nullptr)
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_icon_err_create_failed));
  }
  if (!acSetIconInPixmap(*pPixmap, vCodeXLId))
  {
    return ErrorSet(CCVGRSRC(kIDS_resource_icon_err_create_failed));
  }

  CCVGIconResource &rRsrc = ms_arrayIconResources[veGlobalIconId];
  rRsrc.m_resourceId = veGlobalIconId;
  rRsrc.m_pPixmap = pPixmap;

  return success;
}

/// @brief  Destroy the  icon pixmap object for the specified resource object.
/// @param[inout] vrRsrc A icon resource object.
/// @return status Success = task completed task ok, failure = task failed.
status CCVGResourcesIcon::DestroyResource(CCVGIconResource &vrRsrc)
{
  if (vrRsrc.m_pPixmap != nullptr)
  {
    delete vrRsrc.m_pPixmap;
    vrRsrc.m_pPixmap = nullptr;
  }

  return success;
}

/// @brief  Iterate all icon resources and destroy each icon pixmap object.
/// @return status Success = task completed task ok, failure = task failed.
status CCVGResourcesIcon::DestroyResourceAll()
{
  status ok = DestroyResource(ms_thisErrorGetFailedIcon);

  CCVGuint errCount = 0;
  for (CCVGuint i = 0; i < kIconResourceItemCount; i++)
  {
    if (failure == DestroyResource(ms_arrayIconResources[i]))
    {
      errCount++;
    }
  }
  ok = ok && (errCount == 0);

  return ok;
}