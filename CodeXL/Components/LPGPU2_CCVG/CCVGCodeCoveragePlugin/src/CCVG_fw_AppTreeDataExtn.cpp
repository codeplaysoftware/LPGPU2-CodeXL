// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTreeDataExtn implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_AppTreeDataExtn.h>

namespace ccvg {
namespace fw {

/// @brief    Class constructor. No work is done here by this class.
CCVGFWTreeDataExtn::CCVGFWTreeDataExtn()
: m_bBeenInitialised(false)
, m_bBeenShutdown(false)
, m_pDataItem(nullptr)
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGFWTreeDataExtn::~CCVGFWTreeDataExtn()
{
  Shutdown();
}

/// @brief  Object initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWTreeDataExtn::Initialise()
{
  if (m_bBeenInitialised)
  {
    return success;
  }

  // Do work

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Object shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWTreeDataExtn::Shutdown()
{
  if (m_bBeenShutdown)
  {
    return success;
  }

  Clear();
  m_bBeenShutdown = true;

  return success;
}

/// @brief Reset *this object to new state.
void CCVGFWTreeDataExtn::Clear()
{
  m_projDataItemID = gtString();
  m_pDataItem = nullptr;
}

/// @brief Copy an object to *this object.
/// @param[in] vrData The other *this object.
void CCVGFWTreeDataExtn::Copy(const CCVGFWTreeDataExtn &vrData)
{
  m_projDataItemID = vrData.m_projDataItemID;
  m_pDataItem = vrData.m_pDataItem;
}

/// @brief Create the connection of *this object with its associate parts.
/// @param[in] vrDataItemID The key ID for this object which relates to other 
///            data parts.
/// @param[in] The project data item *this object associates with.
/// @return status Success = task complete, failure = task failed in its task.
status CCVGFWTreeDataExtn::Create(const gtString &vrDataItemID, const CCVGFWTheCurrentStateProjectData::CCVGItem &vDataItem)
{
  if (vrDataItemID.isEmpty())
  {
    return failure;
  }
  m_pDataItem = new (std::nothrow) CCVGFWTheCurrentStateProjectData::CCVGItem;
  if (m_pDataItem == nullptr)
  {
    return failure;
  }
  m_projDataItemID = vrDataItemID;
  *m_pDataItem = vDataItem;

  return success;
}

/// @brief Teardown *this object. Disconnect it with respect to its other
///        associate parts.
/// @return status Success = task complete, failure = task failed in its task.
status CCVGFWTreeDataExtn::Destroy()
{
  Clear();
  return success;
}

/// @brief Overridden. Compares *this with another like itself to see if they 
///        both have the same data.
/// @param[in] vpOtherItemData Another object or NULL.
/// @return bool True = All data matches, false = One or more attributes 
///         differ.
bool CCVGFWTreeDataExtn::isSameObject(afTreeDataExtension *vpOtherItemData) const
{
  bool bSame = false;

  if (vpOtherItemData != nullptr)
  {
    CCVGFWTreeDataExtn *pData = qobject_cast<CCVGFWTreeDataExtn *>(vpOtherItemData);
    if (pData != nullptr)
    {
      bSame = m_projDataItemID == pData->m_projDataItemID;
      bSame = bSame && (m_pDataItem == pData->m_pDataItem);
    }
  }

  return bSame;
}

/// @brief Overridden. Copy *this item's data to another item. If NULL passed in then
///        create clear/reset the data for the other object.
/// @param[in] vpOtherItemData Another object or NULL.
void CCVGFWTreeDataExtn::copyID(afTreeDataExtension *&vpOtherItemData) const
{
  if (vpOtherItemData == nullptr)
  {
    vpOtherItemData = new CCVGFWTreeDataExtn;
  }
  else
  {
    CCVGFWTreeDataExtn *pData = qobject_cast<CCVGFWTreeDataExtn *>(vpOtherItemData);
    GT_IF_WITH_ASSERT(pData != nullptr)
    {
      pData->m_projDataItemID = m_projDataItemID;
      pData->m_pDataItem = m_pDataItem;
    }
  }
}

/// @brief Retrieve the pointer to the CCVGItem object stored in the tree item.
/// @return CCVGItem* Pointer to object or NULL object not created for this 
///         tree item data extension.
CCVGFWTheCurrentStateProjectData::CCVGItem* CCVGFWTreeDataExtn::GetDataItem() const
{
  return m_pDataItem;
}

} // namespace fw
} //namespace ccvg
