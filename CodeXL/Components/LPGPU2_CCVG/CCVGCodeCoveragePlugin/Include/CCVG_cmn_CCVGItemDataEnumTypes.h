// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGItemDataEnumTypes interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_CCVGITEMDATAENUMTYPES_H_INCLUDE
#define LPGPU2_CCVG_FW_CCVGITEMDATAENUMTYPES_H_INCLUDE

namespace ccvg {
namespace fw {

/// @brief Enumeration of the types of data objects, sub data items, a 
///        CCVGItemData object encapsulates. Use by higher level entities
///        to be able to identify and work with a specific sub item.
/// @see      CCVGFWTheCurrentStateProjectData, CCVGItems, 
///           CCVGFWTheCurrentStateProjectDataCcvg, CCVGFWCCVGItemDataBase.
/// @date     14/03/2017.
/// @author   Illya Rudkin.
// clang-format off
enum class EnumCCVGItemDataSubObjType
{
  kCCVGItemDataSubObjType_invalid = 0,
  kCCVGItemDataSubObjType_fileDataResults,
  kCCVGItemDataSubObjType_fileDataSource,
  kCCVGItemDataSubObjType_fileDataDrivers, 
  kkCCVGItemDataSubObjType_fileDataExecuteable
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_CCVGITEMDATAENUMTYPES_H_INCLUDE