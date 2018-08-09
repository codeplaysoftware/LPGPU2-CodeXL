// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGMdiViewId implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewId.h>

namespace ccvg {
namespace fw {

/// @brief Class constructor. 
CCVGMDIViewId::CCVGMDIViewId(const gtString &vCCVGItemId, const gtString &vSubItemType)
: m_strCCVGItemId(vCCVGItemId)
, m_strCCVGItemDataSubItemType(vSubItemType)
{
}

/// @brief Class constructor. 
/// @param[in] vViewId Text formatted ID formatted as 
///            "<CVGItemId>,<File type>".
CCVGMDIViewId::CCVGMDIViewId(const gtString &viewId)
{
  SplitViewId(viewId);
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGMDIViewId::~CCVGMDIViewId()
{
}

/// @brief Split the CCVGItem/view ID in to its parts to determine the 
///        handling context and the subject.
/// @param[in] vViewId Text formatted ID formatted as 
///            "<CVGItemId>,<File type>".
void CCVGMDIViewId::SplitViewId(const gtString &viewId)
{
  if (viewId.isEmpty())
  {
    return;
  }

  const int nPos = viewId.find(',');
  if (nPos == -1)
  {
    m_strCCVGItemId = viewId;
    m_strCCVGItemDataSubItemType.makeEmpty();
    return;
  }
  const int len = viewId.length();
  if (nPos == 0)
  {
    if (len == 1)
    {
      m_strCCVGItemId.makeEmpty();
      m_strCCVGItemDataSubItemType.makeEmpty();
      return;
    }

    m_strCCVGItemId.makeEmpty();
    viewId.getSubString(1, len, m_strCCVGItemDataSubItemType);
    return;
  }

  viewId.getSubString(0, nPos - 1, m_strCCVGItemId);
  viewId.getSubString(nPos + 1, len, m_strCCVGItemDataSubItemType);
}

/// @brief Retrieve the CCVGItem's unique ID associated with a view.
/// @return gtString& Text ID.
const gtString& CCVGMDIViewId::GetCCVGItemId() const
{
  return m_strCCVGItemId;
}

/// @brief Retrieve the CCVGItemData sub-item ID associated with a view.
/// @return gtString& Text ID.
const gtString& CCVGMDIViewId::GetCCVGItemDataSubItemType() const
{
  return m_strCCVGItemDataSubItemType;
}

/// @brief Retrieve the view's ID. It is a combination of the CCVGItem ID and
///        the CCVGItemData sub-item type.
const gtString CCVGMDIViewId::GetId() const
{
  gtString id;
  id.appendFormattedString(L"%ls,%ls", m_strCCVGItemId.asCharArray(), m_strCCVGItemDataSubItemType.asCharArray());
  return id;
}

// @brief Return the FileDataType enum representing the type specified by the
//        m_strCCVGItemDataSubItemType string
// @return FileDataType Enum representing sub-item type
const cmd::CCVGCmdFileRead::FileDataType CCVGMDIViewId::GetDataTypeEnum() const
{
  if (m_strCCVGItemDataSubItemType == L"Driver")
  {
    return cmd::CCVGCmdFileRead::kFileDataType_Driver;
  }
  else if (m_strCCVGItemDataSubItemType == L"Source")
  {
    return cmd::CCVGCmdFileRead::kFileDataType_Source;
  }
  else if (m_strCCVGItemDataSubItemType == L"Results")
  {
    return cmd::CCVGCmdFileRead::kFileDataType_Results;
  }
  else if (m_strCCVGItemDataSubItemType == L"Executeable")
  {
    return cmd::CCVGCmdFileRead::kFileDataType_Executeable;
  }
  else
  {
    return cmd::CCVGCmdFileRead::kFileDataType_None;
  }
}

} // namespace fw
} // namespace ccvg 
