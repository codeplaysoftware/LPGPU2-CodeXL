// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGItemDataBase interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_CCVGITEMDATABASE_H_INCLUDE
#define LPGPU2_CCVG_FW_CCVGITEMDATABASE_H_INCLUDE

// Std:
#include <vector>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFileRead.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGHighlight.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMFunctionSummary.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataEnumTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>

namespace ccvg {
namespace fw {

// Declarations:
class CCVGFWTheCurrentStateProjectDataCcvg;


/// @brief    CCVGFWCCVGItemDataBase is base class. The class represents data
///           read from either a code coverage results file, the source
///           for a kernel file or the source code for a driver source code
///           file. It is know as the CCVGItemData sub-item. CCVGItemData
///           must not contain duplicate sub-item types.
///           *this object has two buffers, one for permanent Project
///           data and temp buffer only for the use of comparing if the data
///           in anyway has changed. If changed it set an internal flag to
///           be read by the Project Data level. For displaying annotated sources,
///           it also contains a vector of CCVGHighlights to apply to the text,
///           and a map of line numbers to inline function summaries.
/// @see      CCVGFWTheCurrentStateProjectData, CCVGItems, 
///           CCVGFWTheCurrentStateProjectDataCcvg
/// @date     16/02/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWCCVGItemDataBase
: public CCVGClassCmnBase
{
// Typedef:
public:
  using VecCCVGHighlight_t        = std::vector<CCVGFWCCVGHighlight>;
  using VecSummary_t              = std::vector<CCVGFWLLVMFunctionSummary>;
  using MapLineToVecSummaries_t   = std::map<gtUInt32, VecSummary_t>;
  using PairLineAndVecSummaries_t = std::pair<gtUInt32, VecSummary_t>;

// Methods:
public:
  CCVGFWCCVGItemDataBase(CCVGFWTheCurrentStateProjectDataCcvg &vpParent);
  CCVGFWCCVGItemDataBase(const CCVGFWCCVGItemDataBase &vRhs);
  CCVGFWCCVGItemDataBase(CCVGFWCCVGItemDataBase &&vRhs);
  CCVGFWCCVGItemDataBase& operator= (const CCVGFWCCVGItemDataBase &vRhs);
  CCVGFWCCVGItemDataBase& operator= (CCVGFWCCVGItemDataBase &&vRhs);
  bool                    operator!= (const CCVGFWCCVGItemDataBase &vRhs) const;
  bool                    operator== (const CCVGFWCCVGItemDataBase &vRhs) const;
  //
  void SetInUse(bool vbYes);
  bool GetInUse() const;
  //
  status              SetFileContent(const char * const vpData, size_t vDataSizeBytes);
  const char * const  GetFileContent() const;
  size_t              GetFileContentSizeBytes() const;
  bool                HasFileContentChanged() const;
  void                ClrFileContentChangedFlag();
  void                ClrFileContent();
  void                SetFileContentValid(bool vbFileContentValid);
  //
  status                      FileContentBufferTempForComparisonSet(const char * const vpOtherBuffer, size_t vOtherBufferSizeBytes);
  const gtString&             GetThisObjTypeString() const;
  EnumCCVGItemDataSubObjType  GetThisObjType() const;
  //
  status                                  AddHighlight(CCVGuint vStartPos, CCVGuint vLength, CCVGFWCCVGHighlightType veHighlightType);
  const VecCCVGHighlight_t&               GetHighlights() const;
  const MapLineToVecSummaries_t&          GetMapLineToVecSummaries() const;

// Overrideable:
public:
  virtual ~CCVGFWCCVGItemDataBase();
  virtual void Clear();

// Methods:
protected:
  void Copy(const CCVGFWCCVGItemDataBase &vRhs);
  void Move(CCVGFWCCVGItemDataBase &&vRhs);
  //
  status              FileContentBufferCreate(size_t vSizeBytes);
  const char * const  FileContentBufferGet() const;
  status              FileContentBufferCompareSame(const char * const vpOtherBuffer, size_t vOtherBufferSizeBytes, bool &vrwbResult) const;
  status              FileContentBufferCopy(const CCVGFWCCVGItemDataBase &vRhs);
  status              FileContentBufferMove(CCVGFWCCVGItemDataBase &vRhs);
  void                FileContentBufferDestroy();
  status              FileContentBufferTempForComparisonCreate(size_t vSizeBytes);
  void                FileContentBufferTempForComparisonDestroy();
  const char * const  FileContentBufferTempForComparisonGet() const;
  size_t              FileContentBufferTempForComparisonGetSizeBytes() const;
  void                FileContentBufferTempForComparisonDoCmp();
  status              FileContentBufferTempForComparisonTransfer();
  status              ReReadFileData(cmd::CCVGCmdFileRead::FileDataType veFileType, bool vbStoreDataInTempBuffer);

// Attributes:
protected:
  EnumCCVGItemDataSubObjType            m_eConstThisCCVGItemDataSubObjType;
  CCVGFWTheCurrentStateProjectDataCcvg *m_pParent;
  char                                 *m_pFileContentBufferTempForComparison;
  char                                 *m_pFileContentBuffer;
  size_t                                m_fileContentBufferSizeBytes;
  size_t                                m_fileContentBufferTempForComparisonSizeBytes;
  const size_t                          m_fileContentBufferSizeBytesMax;
  bool                                  m_bFileContentChanged;  // True = yes changed, false = same.
  bool                                  m_bFileContentValid;
  gtString                              m_constStrObjId;
  CCVGuint                              m_nThisSubItemInUseCnt; // >0 = File data present here, ==0 = data is not present and does not need to be checked
  VecCCVGHighlight_t                    m_vecHighlights;
  MapLineToVecSummaries_t               m_mapLineToVecSummaries;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_CCVGITEMDATABASE_H_INCLUDE