// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTreeDataExtn interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Framework:
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>

#ifndef LPGPU2_CCVG_FW_APPTREEDATAEXTENSION_H_INCLUDE
#define LPGPU2_CCVG_FW_APPTREEDATAEXTENSION_H_INCLUDE

namespace ccvg {
namespace fw {

/// @brief    CCVGFWTreeDataExtn derived from AMDT's afTreeDataExtension. It
///           provides CCVG's program and data extension to otherwise generic
///           tree items. Item data for monitored objects in the tree.
///           The class contain basic information for the identification of
///           the object, and pointer to a more specific item data classes.
///           When object navigation tree is inherited, and a more expanded use
///           of the object data is needed, another type of object data should 
///           bed created, and be added as member to this class.
/// @warning  None.
/// @see      Class afApplicationTreeItemData.
/// @date     14/01/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWTreeDataExtn final 
: public afTreeDataExtension
, public CCVGClassCmnBase
{
  Q_OBJECT
    
// Methods:
public:
  CCVGFWTreeDataExtn();
  status Create(const gtString &vrDataItemID, const CCVGFWTheCurrentStateProjectData::CCVGItem &vDataItem);
  status Destroy();
  CCVGFWTheCurrentStateProjectData::CCVGItem* GetDataItem() const;

// Overridden:
public:
  // From afTreeDataExtension
  virtual      ~CCVGFWTreeDataExtn() override;
  virtual bool isSameObject(afTreeDataExtension *vpOtherItemData) const override;
  virtual void copyID(afTreeDataExtension *&vpOtherItemData) const override;

// Methods:
private:
  CCVGFWTreeDataExtn(const CCVGFWTreeDataExtn &) = delete;
  CCVGFWTreeDataExtn(CCVGFWTreeDataExtn &&) = delete;
  CCVGFWTreeDataExtn& operator=(const CCVGFWTreeDataExtn &) = delete;
  CCVGFWTreeDataExtn& operator==(CCVGFWTreeDataExtn &&) = delete;
  //
  void   Clear();
  void   Copy(const CCVGFWTreeDataExtn &vrData);
  status Initialise();
  status Shutdown();

// Attributes:
private:
  bool                                        m_bBeenInitialised;  // True = yes this has been initialised, false = not yet
  bool                                        m_bBeenShutdown;     // True = yes this has been shutdown already, false = not yet
  gtString                                    m_projDataItemID;
  CCVGFWTheCurrentStateProjectData::CCVGItem *m_pDataItem;
};
// clang-format on

} // namespace fw
}  //namespace ccvg

#endif // LPGPU2_CCVG_FW_APPTREEDATAEXTENSION_H_INCLUDE
