// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTheCurrentStateProjectData interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_THECURRENTSTATEPROJECTDATA_H_INCLUDE
#define LPGPU2_CCVG_FW_THECURRENTSTATEPROJECTDATA_H_INCLUDE

// Std:
#include <vector>
#include <map>

// Framework:
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectDataCcvg.h>

// Declarations:
class TiXmlNode;

namespace ccvg {
namespace fw {

/// @brief    CCVGFWTheCurrentStateProjectData handles the CCVG's part of the
///           current loaded project. It load and saves the CCVG files data,
///           their state, to XML. It also manages and validates the data
///           being held. Like CCVGFWTheCurrentStateProjectSettings *this 
///           class works for saving loaded data through the overloaded
///           XML get and set project settings functions.
///           CCVG's project data can be lost should a project with CCVG
///           data be loaded into CodeXL where the CCVG plugin is not
///           built in to it (enabled). Data is written back out to the 
///           project files from all present enabled plugins. Since CCVG is
///           not there to write its bit then the CCVG data is not 
///           recorded. Loading the project into CodeXL with CCVG enabled
///           will show a project where CCVG data is not present.
///           This behaviour is the same for any plugin which is not built in
///           at the time.
/// @see      CCVGFWTheCurrentStateProjectSettings, CCVG_gui_ProjectSettings.
/// @date     22/12/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWTheCurrentStateProjectData final
: public CCVGClassCmnBase
{
// Enumerations:
public:
  /// @ brief Enumeration of how the CCVGItem has changed since the last 
  ///         project open or all files refresh (action). Used by CCVG to
  ///         allow the App Tree Hander to update its tree items for 
  ///         example.
  enum class EnumCCVGItemChangeStateFromPrevLoad
  {
    kChangeState_None,
    kChangeState_New,
    kChangeState_Changed,
    kChangeState_Removed
  };

// Structs:
public:
  /// @brief CCVGItem holds information and status on the 3 files; Results, 
  ///        kernel source file and the driver source file. The 3 make an item.
  ///        There are many items held in a container. 
  ///        File contents (the data) is held seperately in *this class to keep
  ///        CCVGItem lightweight.
  struct CCVGItem final
  {
  // Enumerations:
  public:
    /// @brief Enumeration of the CCVGItem's data (plural - data for each file)
    ///        state during the lifetime of a project open to closed.
    enum class EnumCCVGItemDataState
    {
      kItemDataState_InternalNoData,    // Only exists in the file need to read file
      kItemDataState_InternalNew,       // Not exist as a file (Not in use here for reference only)
      kItemDataState_InternalChanged,   // So now change contents of the file (Not in use here for reference only)
      kItemDataState_InternalRemoved,   // So now delete file (Not in use here for reference only)
      kItemDataState_InternalDoRefresh, // Re-read the file required. An interim state until refresh carried out
      kItemDataState_Internal           // Read from file not changed or not changed since last refresh
    };

  // Members:
  public:
    CCVGItem();
    CCVGItem(bool vbItemIsGood, const gtString &vFileName, bool vbFileHaveCcvgr, bool vbFileHaveKc, bool vbFileHaveCcvgd, bool vbFileHaveCcvge);
    ~CCVGItem();
    CCVGItem(const CCVGItem &vRhs);
    CCVGItem(CCVGItem &&vRhs);
    CCVGItem& operator= (const CCVGItem &vRhs);
    CCVGItem& operator= (CCVGItem &&vRhs);
    bool      operator!= (const CCVGItem &vRhs) const;
    //
    void Clear();
    //
    // Accessor function item attributes
    bool                  GetIsGood() const { return m_bItemIsGood; }
    void                  SetIsGood(bool vbYes) { m_bItemIsGood = vbYes; }
    const gtString&       GetFileName() const { return m_fileName; }
    status                SetFileName(const gtString &vFileName) { m_fileName = vFileName; return success; }
    bool                  IsPresentCcvgr() const { return m_bFileHaveCcvgr; }
    void                  SetPresentCcvgr(bool vbYes) { m_bFileHaveCcvgr = vbYes; }
    bool                  IsPresentKc() const { return m_bFileHaveKc; }
    void                  SetPresentKc(bool vbYes) { m_bFileHaveKc = vbYes; }
    bool                  IsPresentCcvgd() const { return m_bFileHaveCcvgd; }
    void                  SetPresentCcvgd(bool vbYes) { m_bFileHaveCcvgd = vbYes; }
    void                  SetCcvdRequired(bool vbYes);
    bool                  IsPresentCcvge() const { return m_bFileHaveCcvge; }
    void                  SetPresentCcvge(bool vbYes) { m_bFileHaveCcvge = vbYes; }
    void                  SetStateDataCcvge(EnumCCVGItemDataState veNewState) { m_eStateDataCcvge = veNewState; }
    EnumCCVGItemDataState GetStateDataCcvge() { return m_eStateDataCcvge; }
    void                  SetStateDataCcvd(EnumCCVGItemDataState veNewState) { m_eStateDataCcvgd = veNewState; }
    EnumCCVGItemDataState GetStateDataCcvd() { return m_eStateDataCcvgd; }
    void                  SetStateDataKc(EnumCCVGItemDataState veNewState) { m_eStateDataKc = veNewState; }
    EnumCCVGItemDataState GetStateDataKc() { return m_eStateDataKc; }
    void                  SetStateDataCcvgr(EnumCCVGItemDataState veNewState) { m_eStateDataCcvgr = veNewState; }
    EnumCCVGItemDataState GetStateDataCcvgr() { return m_eStateDataCcvgr; }

  // Methods:
  private:
    void Copy(const CCVGItem &vRhs);
    void Move(CCVGItem &&vRhs);

  // Attributes:
  private:
    bool                  m_bItemIsGood;        // True = All parts the item ok, false = part of *this item missing or incorrect
    gtString              m_fileName;           // The filename common to the results, source and driver files 3 some.
    bool                  m_bFileHaveCcvgr;     // Results file. True = present and valid, false = missing or invalid.
    bool                  m_bFileHaveKc;        // Kernel file. True = present and valid, false = missing or invalid.
    bool                  m_bFileHaveCcvgd;     // Driver file. True = present and valid, false = missing or invalid.
    bool                  m_bFileHaveCcvge;     // Executeable file. True = present and valid, false = missing or invalid.
    EnumCCVGItemDataState m_eStateDataCcvge;
    EnumCCVGItemDataState m_eStateDataCcvgd;
    EnumCCVGItemDataState m_eStateDataKc;
    EnumCCVGItemDataState m_eStateDataCcvgr;
  };

  /// @brief The data structure containing all CCVG operational parameters' XML
  ///        tag IDs. They each must be unique and not be empty.
  struct SettingsXMLTagIDs
  {
    friend CCVGFWTheCurrentStateProjectData;

  // Methods:
  public:
    SettingsXMLTagIDs();
    //
    const gtString& GetCCVGItem() const { return m_ccvgItem; }
    const gtString& GetitemIsGoodFlag() const { return m_itemIsGoodFlag; }
    const gtString& GetFileName() const { return m_fileName; }
    const gtString& GetFileHaveCcvgrFlag() const { return m_fileHaveCcvgrFlag; }
    const gtString& GetFileHaveKcFlag() const { return m_fileHaveKcFlag; }
    const gtString& GetFileHaveCcvgdFlag() const { return m_fileHaveCcvgdFlag; }
    const gtString& GetFileHaveCcvgeFlag() const { return m_fileHaveCcvgeFlag; }

  // Attributes:
  private:
    gtString m_ccvgItem;
    gtString m_itemIsGoodFlag;
    gtString m_fileName;
    gtString m_fileHaveCcvgrFlag;
    gtString m_fileHaveKcFlag;
    gtString m_fileHaveCcvgdFlag;
    gtString m_fileHaveCcvgeFlag;
    /* m_eStateDataCcvgd is not stored in the project */
    /* m_eStateDataKc is not stored in the project */
    /* m_eStateDataCcvgr is not stored in the project */
    /* m_eStateDataCcvge is not stored in the project */
  };

// Classes:
public:
  /// @brief Class to inherited by classes that wish to be notified of when
  ///        project data has been changed.
  class NotifyOnProjectDataChanged
  {
  // Methods:
  public:
    NotifyOnProjectDataChanged();

  // Overrideable:
  public:
    virtual ~NotifyOnProjectDataChanged();
    virtual status OnNotifyProjectDataChanged() = 0;
  };

  /// @brief Class to inherited by classes that wish to be notified of when
  ///        project data has been save to CodeXL XML project data.
  class NotifyOnProjectDataSaved
  {
  // Methods:
  public:
    NotifyOnProjectDataSaved();

  // Overrideable:
  public:
    virtual ~NotifyOnProjectDataSaved();
    virtual status OnNotifyProjectDataSaved() = 0;
  };

  /// @brief Class to inherited by classes that wish to be notified of when
  ///        something about CCVGItem data has been changed.
  class NotifyOnCcvgItemDataChanged
  {
  // Methods:
  public:
    NotifyOnCcvgItemDataChanged();

  // Overrideable:
  public:
    virtual ~NotifyOnCcvgItemDataChanged();
    virtual status OnNotifyCcvgItemDataChanged(const gtString &vItemId) = 0;
  };

// Typedefs:
public:
  using VecItems_t = std::vector<CCVGItem>;
  using MapItems_t = std::map<gtString, CCVGItem>;
  using MapPairItems_t = std::pair<gtString, CCVGItem>;
  using MapItemsState_t = std::map<gtString, EnumCCVGItemChangeStateFromPrevLoad>;
  using MapPairItemsState_t = std::pair<gtString, EnumCCVGItemChangeStateFromPrevLoad>;

// Statics:
public:
  static const gtString& GetExtnProjDataName();
 
// Methods:
public:
  CCVGFWTheCurrentStateProjectData();
  ~CCVGFWTheCurrentStateProjectData();
  //
  // Event notifications
  status RegisterForNotificationOnProjectDataChanged(const NotifyOnProjectDataChanged &vObj);
  status RegisterForNotificationOnProjectDataSaved(const NotifyOnProjectDataSaved &vObj);
  status RegisterForNotificationOnCcvgItemDataChanged(const NotifyOnCcvgItemDataChanged &vObj);
  status UnRegisterForNotificationOnCcvgItemDataChanged(const NotifyOnCcvgItemDataChanged &vObj);
  //
  // Load save project data
  status GetXMLDataString(gtString &vProjectAsXMLString);
  status SetDataFromXMLString(const gtString &vProjectAsXMLString, const TiXmlNode &vMainNode);
  //
  // Give access to temporary 3 files items container - used to them update 
  // (commit to) the internal container onces loaded and files checked
  CCVGuint                ItemsCount() const;
  VecItems_t&             ItemsGetThenCommit();
  const MapItems_t&       ItemsGet() const;
  const CCVGItem*         ItemsGet(const gtString &vMapKeyFilename) const;
  status                  ItemsCommit();
  void                    ItemsClear();
  void                    ItemsClearAndNotify();
  bool                    ItemsEmpty() const;
  void                    ItemsSetCcvgdRequired(bool vbYes);
  const MapItemsState_t&  ItemsChangeStateFromPrevLoad() const;
  //
  // CCVGItems' data
  status ItemsDataGet(const gtString &vrItemId, CCVGFWTheCurrentStateProjectDataCcvg *&vpData);
  status ItemsDataDeleteSubItem(const gtString &vrItemId, EnumCCVGItemDataSubObjType veType, bool vbIgnoreSubItemInUse, bool &vrwbDeleted, bool &vrwbDeletedParent);

// Typedef:
private:
  using VecNotifyOnCcvgItemDataChanged_t = std::vector<NotifyOnCcvgItemDataChanged *>;
  using VecNotifyOnProjectDataChanged_t = std::vector<NotifyOnProjectDataChanged *>;
  using VecNotifyOnProjectDataSaved_t = std::vector<NotifyOnProjectDataSaved *>;
  using MapItemsData_t = std::map<gtString, CCVGFWTheCurrentStateProjectDataCcvg *>;
  using MapPairItemsData_t = std::pair<gtString, CCVGFWTheCurrentStateProjectDataCcvg *>;

// Methods:
private:
  status WriteItemsToXMLString(gtString &vProjectAsXMLString);
  status ReadItemsFromXMLString(const gtString &vProjectAsXMLString, const TiXmlNode &vMainNode);
  status NotifyProjectDataChanged();
  status NotifyProjectDataSaved();
  status NotifyCcvgItemDataChanged(const gtString &vrItemId);
  status RefreshExecuteCmdFilesReadAll();
  //
  status ItemsDataDelete(const gtString &vrItemId, bool vbIgnoreSubItemInUse, bool &vrwbDeleted);
  status ItemsDataDeleteAndNotify(const gtString &vrItemId, bool vbIgnoreSubItemInUse, bool &vrwbDeleted);
  status ItemsDataDeleteSubItemAndNotify(const gtString &vrItemId, EnumCCVGItemDataSubObjType veType, bool vbIgnoreSubItemInUse, bool &vrwbDeleted, bool &vrwbDeletedParent);
  void   ItemsDataDeleteAll();
  status ItemsDataReReadFile(const gtString &vrItemId);
  status ItemsDataChangedAndNotify(const gtString &vrItemId, bool vbDoChkForChangeB4Notification = true);
  status ItemsDataNew(const gtString &vrItemId, CCVGFWTheCurrentStateProjectDataCcvg *&vpData);
  //
  status UpdateChkProjectDataCCVGItemsDataChangedAndNotify();

// Attributes:
private:
  static const gtString          ms_xmlProjDataName;
  static const SettingsXMLTagIDs ms_xmlTag;
  //
  // Project data
  MapItems_t      m_mapItemsProjPreviousLoad;        // Contains all the CCVGItems files (and state of them) read in a directory
  VecItems_t      m_vecItemsTemp;
  MapItemsState_t m_mapItemsChangeStateFromPrevLoad; // Contains all the CCVGItems files (and state of them) read in a directory
  MapItemsData_t  m_mapItemsIdToItemsData;           // JIT container, only contains CCVGItems' data that have been specifically
  //                                                    been loaded (file(s) opened by the user
  // Event notification
  bool                              m_bDoNotNotifyDataChanged;            // True = No notification, false = send notification
  bool                              m_bDoNotNotifyDataSaved;              // True = No notification, false = send notification
  bool                              m_bDoNotNotifyOnCcvgItemDataChanged;  // True = No notification, false = send notification
  VecNotifyOnProjectDataChanged_t   m_vecNotifyOnProjectDataChanged;
  VecNotifyOnProjectDataSaved_t     m_vecNotifyOnProjectDataSaved;
  VecNotifyOnCcvgItemDataChanged_t  m_vecNotifyOnCcvgItemDataChanged;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_THECURRENTSTATEPROJECTDATA_H_INCLUDE