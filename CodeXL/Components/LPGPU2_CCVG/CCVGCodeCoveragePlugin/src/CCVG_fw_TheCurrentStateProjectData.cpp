// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTheCurrentStateProjectData implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Std:
#include <memory>    // for std::unique_ptr
#include <algorithm> // for std::find

// TinyXml:
#include <tinyxml.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

// Declarations:
namespace ccvg { namespace cmd { extern status ExecuteCmdFilesReadAll(); }}

namespace ccvg {
namespace fw {

// Instantiations:
const gtString                                            CCVGFWTheCurrentStateProjectData::ms_xmlProjDataName = L"ProjData";
const CCVGFWTheCurrentStateProjectData::SettingsXMLTagIDs CCVGFWTheCurrentStateProjectData::ms_xmlTag;

/// @brief  Class constructor. No work is done here by this class. 
CCVGFWTheCurrentStateProjectData::NotifyOnCcvgItemDataChanged::NotifyOnCcvgItemDataChanged()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGFWTheCurrentStateProjectData::NotifyOnCcvgItemDataChanged::~NotifyOnCcvgItemDataChanged()
{
}

/// @brief  Class constructor. No work is done here by this class. 
CCVGFWTheCurrentStateProjectData::NotifyOnProjectDataChanged::NotifyOnProjectDataChanged()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGFWTheCurrentStateProjectData::NotifyOnProjectDataChanged::~NotifyOnProjectDataChanged()
{
}

/// @brief  Class constructor. No work is done here by this class. 
CCVGFWTheCurrentStateProjectData::NotifyOnProjectDataSaved::NotifyOnProjectDataSaved()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGFWTheCurrentStateProjectData::NotifyOnProjectDataSaved::~NotifyOnProjectDataSaved()
{
}

/// @brief Class constructor. No work done here.
CCVGFWTheCurrentStateProjectData::CCVGItem::CCVGItem()
: m_bItemIsGood(false)
, m_bFileHaveCcvgr(false)
, m_bFileHaveKc(false)
, m_bFileHaveCcvgd(false)
, m_bFileHaveCcvge(false)
, m_eStateDataCcvge(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataCcvgd(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataKc(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataCcvgr(EnumCCVGItemDataState::kItemDataState_InternalNoData)
{
}

/// @brief Class constructor. No work done here.
/// @param[in] vbItemIsGood True = all item infor is ok, false = one or more 
///            parts missing or incoherrent.
/// @param[in] vFileName The CCVG 3 files' common file name.
/// @param[in] vbFileHaveCcvgr True = Results file present, false = missing.
/// @param[in] vbFileHaveKc True = Results file present, false = missing.
/// @param[in] vbFileHaveCcvgd True = Results file present, false = missing.
/// @param[in] vbFileHaveCcvge True = Results file present, false = missing.
CCVGFWTheCurrentStateProjectData::CCVGItem::CCVGItem(bool vbItemIsGood, const gtString &vFileName, bool vbFileHaveCcvgr, bool vbFileHaveKc, bool vbFileHaveCcvgd, bool vbFileHaveCcvge)
: m_bItemIsGood(vbItemIsGood)
, m_fileName(vFileName)
, m_bFileHaveCcvgr(vbFileHaveCcvgr)
, m_bFileHaveKc(vbFileHaveKc)
, m_bFileHaveCcvgd(vbFileHaveCcvgd)
, m_bFileHaveCcvge(vbFileHaveCcvge)
, m_eStateDataCcvge(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataCcvgd(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataKc(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataCcvgr(EnumCCVGItemDataState::kItemDataState_InternalNoData)
{
}

/// @brief Class destructor.
CCVGFWTheCurrentStateProjectData::CCVGItem::~CCVGItem()
{
  // Tidy up
  Clear();
}

/// @brief Class copy constructor. No work done here.
/// @param[in] vRhs The object to copy from.
CCVGFWTheCurrentStateProjectData::CCVGItem::CCVGItem(const CCVGItem &vRhs)
: m_bItemIsGood(false)
, m_bFileHaveCcvgr(false)
, m_bFileHaveKc(false)
, m_bFileHaveCcvgd(false)
, m_bFileHaveCcvge(false)
, m_eStateDataCcvge(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataCcvgd(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataKc(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataCcvgr(EnumCCVGItemDataState::kItemDataState_InternalNoData)
{
  Copy(vRhs);
}

/// @brief Class move constructor. No work done here.
/// @param[in] vRhs The object to copy or move from.
CCVGFWTheCurrentStateProjectData::CCVGItem::CCVGItem(CCVGItem &&vRhs)
: m_bItemIsGood(false)
, m_bFileHaveCcvgr(false)
, m_bFileHaveKc(false)
, m_bFileHaveCcvgd(false)
, m_bFileHaveCcvge(false)
, m_eStateDataCcvge(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataCcvgd(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataKc(EnumCCVGItemDataState::kItemDataState_InternalNoData)
, m_eStateDataCcvgr(EnumCCVGItemDataState::kItemDataState_InternalNoData)
{
  Move(std::move(vRhs));
}

/// @brief Class assignment operator copy.
/// @param[in] vRhs The object to copy from.
CCVGFWTheCurrentStateProjectData::CCVGItem& CCVGFWTheCurrentStateProjectData::CCVGItem::operator= (const CCVGItem &vRhs)
{
  Copy(vRhs);
  return *this;
}

/// @brief Class assignment operator move.
/// @param[in] vRhs The object to copy or move from.
CCVGFWTheCurrentStateProjectData::CCVGItem& CCVGFWTheCurrentStateProjectData::CCVGItem::operator= (CCVGItem &&vRhs)
{
  Move(std::move(vRhs));
  return *this;
}

/// @brief Helper function to copy *this object's members to itself.
void CCVGFWTheCurrentStateProjectData::CCVGItem::Copy(const CCVGItem &vRhs)
{
  if (this == &vRhs)
  {
    return;
  }
  m_bItemIsGood = vRhs.m_bItemIsGood;
  m_fileName = vRhs.m_fileName;
  m_bFileHaveCcvgr = vRhs.m_bFileHaveCcvgr;
  m_bFileHaveKc = vRhs.m_bFileHaveKc;
  m_bFileHaveCcvgd = vRhs.m_bFileHaveCcvgd;
  m_bFileHaveCcvge = vRhs.m_bFileHaveCcvge;
  m_eStateDataCcvge = vRhs.m_eStateDataCcvge;
  m_eStateDataCcvgd = vRhs.m_eStateDataCcvgd;
  m_eStateDataKc = vRhs.m_eStateDataKc;
  m_eStateDataCcvgr = vRhs.m_eStateDataCcvgr;
}

/// @brief Helper function to move *this object's members to itself.
void CCVGFWTheCurrentStateProjectData::CCVGItem::Move(CCVGItem &&vRhs)
{
  if (this == &vRhs)
  {
    return;
  }
  m_bItemIsGood = std::move(vRhs.m_bItemIsGood);
  m_fileName = std::move(vRhs.m_fileName);
  m_bFileHaveCcvgr = std::move(vRhs.m_bFileHaveCcvgr);
  m_bFileHaveKc = std::move(vRhs.m_bFileHaveKc);
  m_bFileHaveCcvgd = std::move(vRhs.m_bFileHaveCcvgd);
  m_bFileHaveCcvge = std::move(vRhs.m_bFileHaveCcvge);
  m_eStateDataCcvge = std::move(vRhs.m_eStateDataCcvge);
  m_eStateDataCcvgd = std::move(vRhs.m_eStateDataCcvgd);
  m_eStateDataKc = std::move(vRhs.m_eStateDataKc);
  m_eStateDataCcvgr = std::move(vRhs.m_eStateDataCcvgr);
}

/// @brief Not equals operator.
/// @param[in] vRhs The other object.
/// @return bool True = not equal, false = equal.
bool CCVGFWTheCurrentStateProjectData::CCVGItem::operator!= (const CCVGItem &vRhs) const
{ 
  if (this == &vRhs)
  {
    return false;
  }

  bool bSame = (m_bItemIsGood == vRhs.m_bItemIsGood);
  bSame = bSame && (m_fileName == vRhs.m_fileName);
  bSame = bSame && (m_bFileHaveCcvgr == vRhs.m_bFileHaveCcvgr);
  bSame = bSame && (m_bFileHaveKc == vRhs.m_bFileHaveKc);
  bSame = bSame && (m_bFileHaveCcvgd == vRhs.m_bFileHaveCcvgd);
  bSame = bSame && (m_bFileHaveCcvge == vRhs.m_bFileHaveCcvge);
  bSame = bSame && (m_eStateDataCcvge == vRhs.m_eStateDataCcvge);
  bSame = bSame && (m_eStateDataCcvgd == vRhs.m_eStateDataCcvgd);
  bSame = bSame && (m_eStateDataKc == vRhs.m_eStateDataKc);
  bSame = bSame && (m_eStateDataCcvgr == vRhs.m_eStateDataCcvgr);
  return !bSame;
}

/// @brief Clear or reset *this objects values back to default.
void CCVGFWTheCurrentStateProjectData::CCVGItem::Clear()
{
  m_bItemIsGood = false;
  m_fileName.makeEmpty();
  m_bFileHaveCcvgr = false;
  m_bFileHaveKc = false;
  m_bFileHaveCcvgd = false;
  m_bFileHaveCcvge = false;
  m_eStateDataCcvge = EnumCCVGItemDataState::kItemDataState_InternalNoData;
  m_eStateDataCcvgd = EnumCCVGItemDataState::kItemDataState_InternalNoData;
  m_eStateDataKc = EnumCCVGItemDataState::kItemDataState_InternalNoData;
  m_eStateDataCcvgr = EnumCCVGItemDataState::kItemDataState_InternalNoData;
}

/// @brief User has changed 'Driver file is required' project setting. This
///        function used to change individual items.
/// @param[in] vbYes True = Driver file is required, false = not required.
void CCVGFWTheCurrentStateProjectData::CCVGItem::SetCcvdRequired(bool vbYes)
{
  if (vbYes)
  {
    m_bItemIsGood = m_bFileHaveCcvgr && m_bFileHaveKc && m_bFileHaveCcvgd;
  }
  else
  {
    m_bItemIsGood = m_bFileHaveCcvgr && m_bFileHaveKc;
  }
}

/// @brief Class constructor. No work is done here by this class.
CCVGFWTheCurrentStateProjectData::SettingsXMLTagIDs::SettingsXMLTagIDs()
: m_ccvgItem(L"CCVGItem")
, m_itemIsGoodFlag(L"ItemIsGoodFlag")
, m_fileName(L"ItemFileName")
, m_fileHaveCcvgrFlag(L"ItemHaveCcvgrFlag")
, m_fileHaveKcFlag(L"ItemHaveKcFlag")
, m_fileHaveCcvgdFlag(L"ItemHaveCcvgdFlag")
, m_fileHaveCcvgeFlag(L"ItemHaveCcvgeFlag")
{
}

/// @brief Class constructor. No work done here.
CCVGFWTheCurrentStateProjectData::CCVGFWTheCurrentStateProjectData()
: m_bDoNotNotifyDataChanged(false)
, m_bDoNotNotifyDataSaved(false)
, m_bDoNotNotifyOnCcvgItemDataChanged(false)
{
}

/// @brief Class destructor. Tidy up release resources used by *this instance.
CCVGFWTheCurrentStateProjectData::~CCVGFWTheCurrentStateProjectData()
{
  m_bDoNotNotifyDataChanged = true;
  m_bDoNotNotifyDataSaved = true;
  m_bDoNotNotifyOnCcvgItemDataChanged = true;

  // Tide up release resources
  ItemsClear();
  m_vecNotifyOnProjectDataChanged.clear();
  m_vecNotifyOnProjectDataSaved.clear();
  m_vecNotifyOnCcvgItemDataChanged.clear();
}

/// @brief Pass the container of items to the caller to read or write to. The
///        contents of the contain are temporary until the ItemsCommit() 
///        function is called upon which the items are moved to and internal
///        container used for persistence of the project's data.
/// @return VecItems_t& Reference to container.
CCVGFWTheCurrentStateProjectData::VecItems_t& CCVGFWTheCurrentStateProjectData::ItemsGetThenCommit()
{
  return m_vecItemsTemp;
}

/// @brief Pass the container of items to the caller to read.
/// @return VecItems_t& Reference to container.
const CCVGFWTheCurrentStateProjectData::MapItems_t& CCVGFWTheCurrentStateProjectData::ItemsGet() const
{
  return m_mapItemsProjPreviousLoad;
}

/// @brief Retrieve the CCVGItem with the specified ID.
/// @param[in] vMapKeyFilename Map key is the filename.
/// @return CCVGItem* CCVGItem object pointer or NULL is not found.
const CCVGFWTheCurrentStateProjectData::CCVGItem* CCVGFWTheCurrentStateProjectData::ItemsGet(const gtString &vMapKeyFilename) const
{
  const MapItems_t::const_iterator it = m_mapItemsProjPreviousLoad.find(vMapKeyFilename);
  if (it != m_mapItemsProjPreviousLoad.cend())
  {
    const CCVGItem &rItemExist((*it).second);
    return &rItemExist;
  }

  return nullptr;
}

/// @brief Commit (which means move) temporary container of items to an
///        internal container used for persistence of the project's data.
///        If item mapped by key filename does not exist already add a new
///        element to the map otherwise update element with latest data
///        attributes.
/// @return status Success = all ok, failure = check error description.
status CCVGFWTheCurrentStateProjectData::ItemsCommit()
{
  const size_t nPrevItems = m_mapItemsProjPreviousLoad.size();
  bool bChange = false;
  MapItems_t mapItems;
  MapItemsState_t mapItemsChangeState;
  for (const CCVGItem &rItem : m_vecItemsTemp)
  {
    const gtString &rFilename(rItem.GetFileName());
    if (rFilename.isEmpty())
    {
      continue;
    }

    EnumCCVGItemChangeStateFromPrevLoad eState = EnumCCVGItemChangeStateFromPrevLoad::kChangeState_Removed;
    MapItems_t::iterator itPrev = m_mapItemsProjPreviousLoad.find(rFilename);
    if (itPrev == m_mapItemsProjPreviousLoad.cend())
    {
      eState = EnumCCVGItemChangeStateFromPrevLoad::kChangeState_New;
      bChange = true;
    }
    else
    {
      const CCVGItem &rItemPrev((*itPrev).second);
      if (rItem != rItemPrev)
      {
        eState = EnumCCVGItemChangeStateFromPrevLoad::kChangeState_Changed;
        m_mapItemsProjPreviousLoad.erase(itPrev);
        bChange = true;
      }
      else
      {
        eState = EnumCCVGItemChangeStateFromPrevLoad::kChangeState_None;
        m_mapItemsProjPreviousLoad.erase(itPrev);
      }
    }
    const MapPairItemsState_t pair(rFilename, eState);
    mapItemsChangeState.insert(pair);
    const MapPairItems_t pair2(rFilename, std::move(rItem));
    mapItems.insert(pair2);
  }

  // Mark CCVGItems that have to be removed
  const EnumCCVGItemChangeStateFromPrevLoad eState = EnumCCVGItemChangeStateFromPrevLoad::kChangeState_Removed;
  for (const auto it : m_mapItemsProjPreviousLoad)
  {
    const gtString &rFileName(it.first);
    const MapPairItemsState_t pair(rFileName, eState);
    mapItemsChangeState.insert(pair);
    bChange = true;
  }

  const size_t nItems = mapItems.size();
  const bool bSame = (nPrevItems == nItems) && !bChange;
   
  m_mapItemsProjPreviousLoad = std::move(mapItems);
  m_mapItemsChangeStateFromPrevLoad = std::move(mapItemsChangeState);
  m_vecItemsTemp.clear();

  if (!bSame)
  {
    NotifyProjectDataChanged();
  }

  const status status = UpdateChkProjectDataCCVGItemsDataChangedAndNotify();

  return status;
}

/// @brief Having sorting through the CCVGItems objects and determining their 
///        new state now go through the CCVGItemsData object, those that have 
///        had their file(s) read and shown in a view, and check whether any
///        data has changed. If so send out notifications to listeners.
/// @return status Success = all ok, failure = an error occurred check error
///         description for this object.
status CCVGFWTheCurrentStateProjectData::UpdateChkProjectDataCCVGItemsDataChangedAndNotify()
{
  CCVGuint errCount = 0;
  status status = success;
  for (const auto &it : m_mapItemsChangeStateFromPrevLoad)
  {
    const gtString &rFileName(it.first);
    const EnumCCVGItemChangeStateFromPrevLoad eState = it.second;
    switch (eState)
    {
    case EnumCCVGItemChangeStateFromPrevLoad::kChangeState_New:
      // Do nothing, this is loaded elsewhere like the cmd wnd src open (JIT load)
      break;
    case EnumCCVGItemChangeStateFromPrevLoad::kChangeState_Changed:
      status = ItemsDataReReadFile(rFileName);
      errCount += (status == success) ? 0 : 1;
      if (status == success)
      {
        ItemsDataChangedAndNotify(rFileName);
      }
      break;
    case EnumCCVGItemChangeStateFromPrevLoad::kChangeState_Removed:
      {
        bool bItemDeleted = false;
        const bool bIgnoreSubItemInUse = true;
        status = ItemsDataDeleteAndNotify(rFileName, bIgnoreSubItemInUse, bItemDeleted);
        errCount += (status == success) ? 0 : 1;
      }
      break;
    case EnumCCVGItemChangeStateFromPrevLoad::kChangeState_None:
      // CCVGItem itself may not have changed but check items' data
      // as file contents may have changed so update any views
      status = ItemsDataReReadFile(rFileName);
      errCount += (status == success) ? 0 : 1;
      if (status == success)
      {
        ItemsDataChangedAndNotify(rFileName);
      }
      break;
    }
  }

  return (errCount == 0) ? success : failure;
}

/// @brief Remove all the CCVGItems' data from this current project. This 
///        includes all the sub-item data objects belonging to a CCVGITemData
///        object.
///        Data is only added to *this object, cached, on a just in time 
///        basis i.e., when the user looks at a source file for viewing, so it
///        is likely the item's data does not exist here.
/// @return bool True = deleted, false = not found.
void CCVGFWTheCurrentStateProjectData::ItemsDataDeleteAll()
{
  for (auto &it : m_mapItemsIdToItemsData)
  {
    CCVGFWTheCurrentStateProjectDataCcvg *pData = it.second;
    delete pData;
  }
  m_mapItemsIdToItemsData.clear();

  // No notification needed as done at project level
}

/// @brief Retrieve the state of each CCVGItem compared to the previous load.
///        Each item can have a state indicating; no change, new item, changed,
///        or removed since reading the result, source and drivers folders.
/// @return MapItemsState_t& Map of CCVGItems' change state.
const CCVGFWTheCurrentStateProjectData::MapItemsState_t& CCVGFWTheCurrentStateProjectData::ItemsChangeStateFromPrevLoad() const
{
  return m_mapItemsChangeStateFromPrevLoad;
}

/// @brief Clear or releases all the CCVG items and associate CCVGItemData 
///        data from this CCVG project. Also clear all temporary and 
///        committed containers of items.
void CCVGFWTheCurrentStateProjectData::ItemsClear()
{
  m_mapItemsProjPreviousLoad.clear();
  m_vecItemsTemp.clear();
  m_mapItemsChangeStateFromPrevLoad.clear();

  // Clear out CCVGItems' file data amongst other things
  ItemsDataDeleteAll();

  NotifyProjectDataChanged();
}

/// @brief Clear or releases all the CCVG items and associate CCVGItemData 
///        data from this CCVG project. Also clear all temporary and 
///        committed containers of items.
void CCVGFWTheCurrentStateProjectData::ItemsClearAndNotify()
{
  ItemsClear();

  NotifyProjectDataChanged();
}

/// @brief Retrieve the empty state of the CCVGItems container.
bool CCVGFWTheCurrentStateProjectData::ItemsEmpty() const
{
  return m_mapItemsProjPreviousLoad.empty();
}

/// @brief Retrieve the number CCVGItems in the current CodeXL project.
CCVGuint CCVGFWTheCurrentStateProjectData::ItemsCount() const
{
  return static_cast<CCVGuint>(m_mapItemsProjPreviousLoad.size());
}

/// @brief  Static function. Retrieve the CCVG project settings name or XML 
///         node ID. Used in the XML DOM node identification for CCVG 
///         project setting information.
/// @return gtString& Wide char text.
const gtString& CCVGFWTheCurrentStateProjectData::GetExtnProjDataName()
{
  return ms_xmlProjDataName;
}

/// @brief      Save project data and settings into a XML string. 
/// @param[out] vProjectAsXMLString XML formatted settings string.
/// @return     status True = success, false = failure.
status CCVGFWTheCurrentStateProjectData::GetXMLDataString(gtString &vProjectAsXMLString)
{
  GT_UNREFERENCED_PARAMETER(vProjectAsXMLString);
  const wchar_t *pXMLNodeName = ms_xmlProjDataName.asCharArray();
  vProjectAsXMLString.appendFormattedString(L"<%ls>", pXMLNodeName);
  const status bOk = WriteItemsToXMLString(vProjectAsXMLString);
  vProjectAsXMLString.appendFormattedString(L"/<%ls>", pXMLNodeName);

  return bOk && NotifyProjectDataSaved();
}

/// @brief      Load project data and settings from an existing project if CCVG
///             enabled for the current project. Read the 
///             XML string extracter CCVG working data and settings.
/// @param[in]  vProjectAsXMLString XML formatted data string.
/// @param[in]  vMainNode CCVG project XML top node.
/// @return     status True = success, false = failure.
status CCVGFWTheCurrentStateProjectData::SetDataFromXMLString(const gtString &vProjectAsXMLString, const TiXmlNode &vMainNode)
{
  GT_UNREFERENCED_PARAMETER(vProjectAsXMLString);
  
  ItemsClear();

  status bOk = true;
  bool bFound = false;
  for (const TiXmlElement *pNode = vMainNode.FirstChildElement(); pNode != nullptr; pNode = pNode->NextSiblingElement())
  {
    const gtString nodeName(gtString().fromASCIIString(pNode->Value()));
    if (nodeName == ms_xmlProjDataName)
    {
      bFound = true;
      bOk = ReadItemsFromXMLString(vProjectAsXMLString, *pNode);
      if (bOk && !ItemsEmpty())
      {
        bOk = RefreshExecuteCmdFilesReadAll();
      }
      break;
    }
  }

  const status bOk2 = NotifyProjectDataChanged();

  bOk = bOk && bOk2 && bFound;

  return bOk;
}

/// @brief Write CCVG item, the information that comes together make up an CCVG
///        set of 3 files; results, source and driver, to the CodeXL XML 
///        project.
/// @param[out] vProjectAsXMLString XML formatted data string.
/// @return     status True = success, false = failure.
status CCVGFWTheCurrentStateProjectData::WriteItemsToXMLString(gtString &vProjectAsXMLString)
{
  const wchar_t *pCCVGItem = ms_xmlTag.GetCCVGItem().asCharArray();
  for (const auto &it : m_mapItemsProjPreviousLoad)
  {
    const CCVGItem &rItem(it.second);
    vProjectAsXMLString.appendFormattedString(L"<%ls>", pCCVGItem);
    afUtils::addFieldToXML(vProjectAsXMLString, ms_xmlTag.GetitemIsGoodFlag(), rItem.GetIsGood());
    afUtils::addFieldToXML(vProjectAsXMLString, ms_xmlTag.GetFileName(), rItem.GetFileName());
    afUtils::addFieldToXML(vProjectAsXMLString, ms_xmlTag.GetFileHaveCcvgrFlag(), rItem.IsPresentCcvgr());
    afUtils::addFieldToXML(vProjectAsXMLString, ms_xmlTag.GetFileHaveKcFlag(), rItem.IsPresentKc());
    afUtils::addFieldToXML(vProjectAsXMLString, ms_xmlTag.GetFileHaveCcvgdFlag(), rItem.IsPresentCcvgd());
    afUtils::addFieldToXML(vProjectAsXMLString, ms_xmlTag.GetFileHaveCcvgeFlag(), rItem.IsPresentCcvge());
    vProjectAsXMLString.appendFormattedString(L"</%ls>", pCCVGItem);
  }

  return success;
}

/// @brief Read a CCVG item, the information that comes together make up an CCVG
///        set of 3 files; results, source and driver, from the CodeXL XML 
///        project.
/// @param[in] vProjectAsXMLString XML formatted data string.
/// @param[in] vMainNode Root XML node for CCVG project data.
/// @return     status True = success, false = failure.
status CCVGFWTheCurrentStateProjectData::ReadItemsFromXMLString(const gtString &vProjectAsXMLString, const TiXmlNode &vMainNode)
{
  GT_UNREFERENCED_PARAMETER(vProjectAsXMLString);

  const EnumCCVGItemChangeStateFromPrevLoad eState = EnumCCVGItemChangeStateFromPrevLoad::kChangeState_New;
  const gtString &rCCVGItem(ms_xmlTag.GetCCVGItem());
  const TiXmlElement *pRoot = vMainNode.FirstChildElement();
  for (const TiXmlElement *pNode = pRoot; pNode != nullptr; pNode = pNode->NextSiblingElement())
  {
    gtString name;
    name.fromASCIIString(pNode->Value());
    if (rCCVGItem == name)
    {
      TiXmlElement *pN = const_cast<TiXmlElement *>(pNode);
      bool bItemIsGood;
      gtString fileName;
      bool bFileHaveCcvgr;
      bool bFileHaveKc;
      bool bFileHaveCcvgd;
      bool bFileHaveCcvge;
      afUtils::getFieldFromXML(*pN, ms_xmlTag.GetitemIsGoodFlag(), bItemIsGood);
      afUtils::getFieldFromXML(*pN, ms_xmlTag.GetFileName(), fileName);
      afUtils::getFieldFromXML(*pN, ms_xmlTag.GetFileHaveCcvgrFlag(), bFileHaveCcvgr);
      afUtils::getFieldFromXML(*pN, ms_xmlTag.GetFileHaveKcFlag(), bFileHaveKc);
      afUtils::getFieldFromXML(*pN, ms_xmlTag.GetFileHaveCcvgdFlag(), bFileHaveCcvgd);
      afUtils::getFieldFromXML(*pN, ms_xmlTag.GetFileHaveCcvgeFlag(), bFileHaveCcvge);
      const CCVGItem item(bItemIsGood, fileName, bFileHaveCcvgr, bFileHaveKc, bFileHaveCcvgd, bFileHaveCcvge);
      const MapPairItems_t pair(fileName, item);
      m_mapItemsProjPreviousLoad.insert(pair);

      const MapPairItemsState_t pair2(fileName, eState);
      m_mapItemsChangeStateFromPrevLoad.insert(pair2);
    }
  }

  return success;
}

/// @brief User has changed 'Driver file is required' project setting. This
///        function used to change all the items in the project.
/// @param[in] vbYes True = Driver file is required, false = not required.
void CCVGFWTheCurrentStateProjectData::ItemsSetCcvgdRequired(bool vbYes)
{
  for (auto it : m_mapItemsProjPreviousLoad)
  {
    CCVGItem &rItem(it.second);
    rItem.SetCcvdRequired(vbYes);
  }
}

/// @brief Register with *this class for notifications of when some part of the
///        project data has been changed.
/// @param[in] NotifyOnProjectDataChanged& Object to notify.
/// @return status success = registration ok, false = error no registration.
status CCVGFWTheCurrentStateProjectData::RegisterForNotificationOnProjectDataChanged(const NotifyOnProjectDataChanged &vObj)
{
  m_vecNotifyOnProjectDataChanged.push_back(const_cast<NotifyOnProjectDataChanged *>(&vObj));
  return success;
}

/// @brief Notify registered listeners that some part the project data has 
///        changed. Internally to *this object it can disable the notification 
///        for example on object destruction.
/// @return status success = all notifications ok, false = one or more 
///        stated they failed talk of carrying out the notification.
status CCVGFWTheCurrentStateProjectData::NotifyProjectDataChanged()
{
  if (m_bDoNotNotifyDataChanged)
  {
    return success;
  }
  
  CCVGuint errCount = 0;
  for (NotifyOnProjectDataChanged *pObj : m_vecNotifyOnProjectDataChanged)
  {
    errCount += (pObj->OnNotifyProjectDataChanged() == success) ? 0 : 1;
  }

  return ((errCount == 0) ? success : failure);
}

/// @brief Register with *this class for notifications of when the project
///        data has been saved to the CodeXL XML project data.
/// @param[in] NotifyOnProjectDataChanged& Object to notify.
/// @return status success = registration ok, false = error no registration.
status CCVGFWTheCurrentStateProjectData::RegisterForNotificationOnProjectDataSaved(const NotifyOnProjectDataSaved &vObj)
{
  m_vecNotifyOnProjectDataSaved.push_back(const_cast<NotifyOnProjectDataSaved *>(&vObj));
  return success;
}

/// @brief Notify registered listeners that some part the project data has 
///        been saved to the CodeXL XML project data. Internally to *this 
///        object it can disable the notification for example on object 
///        destruction.
/// @return status success = all notifications ok, false = one or more 
///        stated they failed talk of carrying out the notification.
status CCVGFWTheCurrentStateProjectData::NotifyProjectDataSaved()
{
  if (m_bDoNotNotifyDataSaved)
  {
    return success;
  }

  CCVGuint errCount = 0;
  for (NotifyOnProjectDataSaved *pObj : m_vecNotifyOnProjectDataSaved)
  {
    errCount += (pObj->OnNotifyProjectDataSaved() == success) ? 0 : 1;
  }

  return ((errCount == 0) ? success : failure);
}

/// @brief Register with *this class for notifications of when one or 
///        CCVGItems' data has changed.
/// @param[in] NotifyOnCcvgItemDataChanged& Object to notify.
/// @return status success = registration ok, false = error no registration.
status CCVGFWTheCurrentStateProjectData::RegisterForNotificationOnCcvgItemDataChanged(const NotifyOnCcvgItemDataChanged &vObj)
{
  m_vecNotifyOnCcvgItemDataChanged.push_back(const_cast<NotifyOnCcvgItemDataChanged *>(&vObj));
  return success;
}

/// @brief Unregister with *this class notifications for when CCVGItem data
///        has been changed.
/// @param[in] NotifyOnCcvgItemDataChanged& Object to unregister.
/// @return status success = registration ok, false = error no registration.
status CCVGFWTheCurrentStateProjectData::UnRegisterForNotificationOnCcvgItemDataChanged(const NotifyOnCcvgItemDataChanged &vObj)
{
  const VecNotifyOnCcvgItemDataChanged_t::iterator it = std::find(m_vecNotifyOnCcvgItemDataChanged.begin(), m_vecNotifyOnCcvgItemDataChanged.end(), &vObj);
  if (it != m_vecNotifyOnCcvgItemDataChanged.end())
  {
    m_vecNotifyOnCcvgItemDataChanged.erase(it);
  }
  return success;
}

/// @brief Notify registered listeners that a CCVGItem's data has 
///        been changed. Internally to *this object it can disable the 
///        notification for example on object destruction.
/// @param[in] vrItemId CCVGItem ID.
/// @return status success = all notifications ok, false = one or more 
///        stated they failed talk of carrying out the notification.
status CCVGFWTheCurrentStateProjectData::NotifyCcvgItemDataChanged(const gtString &vrItemId)
{
  if (m_bDoNotNotifyOnCcvgItemDataChanged)
  {
    return success;
  }

  CCVGuint errCount = 0;
  for (NotifyOnCcvgItemDataChanged *pObj : m_vecNotifyOnCcvgItemDataChanged)
  {
    errCount += (pObj->OnNotifyCcvgItemDataChanged(vrItemId) == success) ? 0 : 1;
  }

  return ((errCount == 0) ? success : failure);
}

/// @brief Setup and execute command to scan the code coverage source 
///        directories and read in files.
status CCVGFWTheCurrentStateProjectData::RefreshExecuteCmdFilesReadAll()
{
  return cmd::ExecuteCmdFilesReadAll();
}

/// @brief Remove the CCVGItem's data and all its sub-items object of type X
///        from this current project. 
///        Data is only added to *this object, cached, on a just in time basis
///        i.e., when the user looks at a source file for viewing, so it is 
///        likely the item's data does not exist here.
/// @param[in]  vrItemId CCVGItem's ID. 
/// @param[in]  vbIgnoreSubItemInUse True = force delete sub-item, false = 
///             skip deletion.
/// @param[out] vrwbDeleted True = deleted, false = not found.
/// @return status Success = all ok, failure = one or more errors during 
///         notification to listeners.
status CCVGFWTheCurrentStateProjectData::ItemsDataDeleteAndNotify(const gtString &vrItemId, bool vbIgnoreSubItemInUse, bool &vrwbDeleted)
{
  vrwbDeleted = false;

  status status = success;
  const MapItemsData_t::iterator it = m_mapItemsIdToItemsData.find(vrItemId);
  if (it != m_mapItemsIdToItemsData.end())
  {
    CCVGFWTheCurrentStateProjectDataCcvg *pData = (*it).second;
    if (!vbIgnoreSubItemInUse && pData->GetInUseSubItemAny())
    {
      return success;
    }

    delete pData;
    m_mapItemsIdToItemsData.erase(it);

    status = NotifyCcvgItemDataChanged(vrItemId); // For example refresh of files read in directories
    //  Close a view of the data does not delete the data from the project
    vrwbDeleted = true;
  }

  return status;
}

/// @brief Remove the CCVGItem's data and all its sub-items object of type X
///        from this current project. 
///        Data is only added to *this object, cached, on a just in time basis
///        i.e., when the user looks at a source file for viewing, so it is 
///        likely the item's data does not exist here.
/// @param[in]  vrItemId CCVGItem's ID. 
/// @param[in]  vbIgnoreSubItemInUse True = force delete sub-item, false = 
///             skip deletion.
/// @param[out] vrwbDeleted True = deleted, false = not found.
/// @return status Success = all ok, failure = one or more errors during 
///         notification to listeners.
status CCVGFWTheCurrentStateProjectData::ItemsDataDelete(const gtString &vrItemId, bool vbIgnoreSubItemInUse, bool &vrwbDeleted)
{
  vrwbDeleted = false;

  status status = success;
  const MapItemsData_t::iterator it = m_mapItemsIdToItemsData.find(vrItemId);
  if (it != m_mapItemsIdToItemsData.end())
  {
    CCVGFWTheCurrentStateProjectDataCcvg *pData = (*it).second;
    if (!vbIgnoreSubItemInUse && pData->GetInUseSubItemAny())
    {
      return success;
    }

    delete pData;
    m_mapItemsIdToItemsData.erase(it);
    vrwbDeleted = true;
  }

  return status;
}

/// @brief Remove the CCVGItem's sub-items data object of type X
///        from this current project irrspective of whether any sub-item data
///        is in used. If none of the sub-items are no longer in use then
///        also delete the CCVGItem itself.
///        Data is only added to *this object, cached, on a just in time basis
///        i.e., when the user looks at a source file for viewing, so it is 
///        likely the item's data does not exist here.
/// @param[in]  vrItemId CCVGItem's ID. 
/// @param[in]  veType Enumeration of the type of sub-item object data.
/// @param[in]  vbIgnoreSubItemInUse True = force delete sub-item, false = 
///             skip deletion.
/// @param[out] vrwbDeleted True = deleted sub-item, false = in use.
/// @param[out] vrwbDeletedParent True = deleted CCVGItemData, false = in use.
/// @return status Success = all ok, failure = one or more errors during 
///         notification to listeners.
status CCVGFWTheCurrentStateProjectData::ItemsDataDeleteSubItem(const gtString &vrItemId, EnumCCVGItemDataSubObjType veType, bool vbIgnoreSubItemInUse, bool &vrwbDeleted, bool &vrwbDeletedParent)
{
  vrwbDeleted = false;
  vrwbDeletedParent = false;

  status status = success;
  const MapItemsData_t::iterator it = m_mapItemsIdToItemsData.find(vrItemId);
  if (it != m_mapItemsIdToItemsData.end())
  {
    CCVGFWTheCurrentStateProjectDataCcvg *pData = (*it).second;
    if (!vbIgnoreSubItemInUse)
    {
      bool bInUse = false;
      status = pData->GetInUseSubItem(veType, bInUse);
      if (status == failure)
      {
        const gtString errMsg = pData->ErrorGetDescription();
        return ErrorSet(errMsg);
      }
      
      if (bInUse)
      {
        return success;
      }
      
      status = pData->ClrData(veType);
      if (status == failure)
      {
        const gtString errMsg = pData->ErrorGetDescription();
        return ErrorSet(errMsg);
      }

      vrwbDeleted = true;
      
      if (pData->GetInUseSubItemAny())
      {
        return success;
      }
    }

    delete pData;
    m_mapItemsIdToItemsData.erase(it);
    vrwbDeletedParent = true;
  }

  return status;
}

/// @brief Remove the CCVGItem's sub-items data object of type X
///        from this current project irrspective of whether any sub-item data
///        is in used. If none of the sub-items are no longer in use then
///        also delete the CCVGItem itself.
///        Data is only added to *this object, cached, on a just in time basis
///        i.e., when the user looks at a source file for viewing, so it is 
///        likely the item's data does not exist here.
/// @param[in]  vrItemId CCVGItem's ID. 
/// @param[in]  veType Enumeration of the type of sub-item object data.
/// @param[in]  vbIgnoreSubItemInUse True = force delete sub-item, false = 
///             skip deletion.
/// @param[out] vrwbDeleted True = deleted sub-item, false = in use.
/// @param[out] vrwbDeletedParent True = deleted CCVGItemData, false = in use.
/// @return status Success = all ok, failure = one or more errors during 
///         notification to listeners.
status CCVGFWTheCurrentStateProjectData::ItemsDataDeleteSubItemAndNotify(const gtString &vrItemId, EnumCCVGItemDataSubObjType veType, bool vbIgnoreSubItemInUse, bool &vrwbDeleted, bool &vrwbDeletedParent)
{
  vrwbDeleted = false;
  vrwbDeletedParent = false;

  status status = success;
  const MapItemsData_t::iterator it = m_mapItemsIdToItemsData.find(vrItemId);
  if (it != m_mapItemsIdToItemsData.end())
  {
    CCVGFWTheCurrentStateProjectDataCcvg *pData = (*it).second;
    if (!vbIgnoreSubItemInUse)
    {
      bool bInUse = false;
      status = pData->GetInUseSubItem(veType, bInUse);
      if (status == failure)
      {
        const gtString errMsg = pData->ErrorGetDescription();
        return ErrorSet(errMsg);
      }

      if (bInUse)
      {
        return success;
      }

      status = pData->ClrData(veType);
      if (status == failure)
      {
        const gtString errMsg = pData->ErrorGetDescription();
        return ErrorSet(errMsg);
      }

      vrwbDeleted = true;
      status = NotifyCcvgItemDataChanged(vrItemId);

      if (pData->GetInUseSubItemAny())
      {
        return success;
      }
    }

    delete pData;
    m_mapItemsIdToItemsData.erase(it);
    vrwbDeletedParent = true;
  }

  return status;
}

/// @brief Re-read the all the file(s) data for the specificed CCVGItemData 
///        object. Re-reading checks the data put in a temporary buffer with
///        existing previous data and updates the data change flag if the 
///        latest file read is different. The container m_mapItemsIdToItemsData
///        contains only items that have been requested by the user to view the
///        contents.
/// @return status Success = all ok, failure = error occurred check error
///          description.
status CCVGFWTheCurrentStateProjectData::ItemsDataReReadFile(const gtString &vrItemId)
{
  status status = success;

  const MapItemsData_t::iterator it = m_mapItemsIdToItemsData.find(vrItemId);
  if (it != m_mapItemsIdToItemsData.end())
  {
    CCVGFWTheCurrentStateProjectDataCcvg *pData = (*it).second;
    status = pData->ChkDataChanged();
  }

  return status;
}

/// @brief Iterate all the CCVGItemData object to check if their data has 
///        changed. Notification is sent out to all registered listeners.
///        The container m_mapItemsIdToItemsData contains only items that 
///        have been requested by the user to view the contents.
/// @return status Success = all ok, failure = error occurred check error
///          description.
status CCVGFWTheCurrentStateProjectData::ItemsDataChangedAndNotify(const gtString &vrItemId, bool vbDoChkForChangeB4Notification /* = true */)
{
  status status = success;
  const MapItemsData_t::iterator it = m_mapItemsIdToItemsData.find(vrItemId);
  if (it != m_mapItemsIdToItemsData.end())
  {
    if (vbDoChkForChangeB4Notification)
    {
      CCVGFWTheCurrentStateProjectDataCcvg *pData = (*it).second;
      if(pData->HasDataChanged())
      {
        pData->ClrDataChangedFlags();
        status = NotifyCcvgItemDataChanged(vrItemId); // For example update contents of a source view 
        return status;
      }
    }
    else
    {
      // Assume data has changed without doubt
      status = NotifyCcvgItemDataChanged(vrItemId); // For example update contents of a source view 
    }
  }

  return status;
}

/// @brief Create a new CCVGITtemData object. The containers with are filled in
///        later. A data object holds the file data from the results, source 
///        or drivers files. Note not all three made hold data but at least one
///        for this object to be created.
/// @return status Success = all ok, failure = error occurred check error
///          description.
status CCVGFWTheCurrentStateProjectData::ItemsDataNew(const gtString &vrItemId, CCVGFWTheCurrentStateProjectDataCcvg *&vpData)
{
  if (vrItemId.isEmpty())
  {
    return ErrorSet(CCVGRSRC(KIDS_CCVG_STR_fw_projectData_err_invalidCCVGItemId));
  }

  const MapItemsData_t::const_iterator it = m_mapItemsIdToItemsData.find(vrItemId);
  if (it != m_mapItemsIdToItemsData.end())
  {
    const wchar_t *pErrMsg = CCVGRSRC(KIDS_CCVG_STR_fw_projectData_err_dupicateCCVGItemId);
    const wchar_t *pItemId = vrItemId.asCharArray();
    gtString errMsg;
    errMsg.appendFormattedString(pErrMsg, pItemId);
    return ErrorSet(errMsg);
  }

  CCVGFWTheCurrentStateProjectDataCcvg *pData = new (std::nothrow) CCVGFWTheCurrentStateProjectDataCcvg(vrItemId);
  if (pData == nullptr)
  {
    const wchar_t *pErrMsg = CCVGRSRC(KIDS_CCVG_STR_fw_projectData_err_failCreateCCVGItemData);
    const wchar_t *pItemId = vrItemId.asCharArray();
    gtString errMsg;
    errMsg.appendFormattedString(pErrMsg, pItemId);
    return ErrorSet(errMsg);
  }

  MapPairItemsData_t pair(vrItemId, pData);
  m_mapItemsIdToItemsData.insert(pair);
  vpData = pData;

  // No notification broad cast needed 

  return success;
}

/// @brief Retrieve the CCVGItemData object for the specified CCVGItem ID. If
///        the object does not exist the object is created and returned.
/// @return status Success = all ok, failure = error occurred check error
///          description.
status CCVGFWTheCurrentStateProjectData::ItemsDataGet(const gtString &vrItemId, CCVGFWTheCurrentStateProjectDataCcvg *&vpData)
{
  const MapItemsData_t::const_iterator it = m_mapItemsIdToItemsData.find(vrItemId);
  if (it != m_mapItemsIdToItemsData.end())
  {
    vpData = (*it).second;
    return success;
  }

  return ItemsDataNew(vrItemId, vpData);
}

} // namespace fw
} // namespace ccvg
