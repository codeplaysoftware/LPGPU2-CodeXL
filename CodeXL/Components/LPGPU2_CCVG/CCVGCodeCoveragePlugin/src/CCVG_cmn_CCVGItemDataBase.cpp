// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGItemDataBase implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// std:
#include <cstring>   // For std::memcmp, std::memcpy

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectDataCcvg.h>

// Declarations:
namespace ccvg { namespace cmd { extern status ExecuteCmdReadFile(const gtString &vrFileName, CCVGCmdFileRead::FileDataType veFileType, bool bStoreDataInTempBuffer); } }


namespace ccvg {
namespace fw {

constexpr size_t constExprFileContentBufferSizeBytesMax = 10 * 1024 * 1024;

// @brief Class constructor. No work done here.
CCVGFWCCVGItemDataBase::CCVGFWCCVGItemDataBase(CCVGFWTheCurrentStateProjectDataCcvg &vpParent)
: m_pParent(&vpParent)
, m_pFileContentBufferTempForComparison(nullptr)
, m_pFileContentBuffer(nullptr)
, m_fileContentBufferSizeBytes(0)
, m_fileContentBufferTempForComparisonSizeBytes(0)
, m_fileContentBufferSizeBytesMax(constExprFileContentBufferSizeBytesMax)
, m_bFileContentValid(true)
, m_bFileContentChanged(false)
, m_constStrObjId(L"CCVGFWCCVGItemDataBase")
, m_nThisSubItemInUseCnt(0)
, m_eConstThisCCVGItemDataSubObjType(EnumCCVGItemDataSubObjType::kCCVGItemDataSubObjType_invalid)
{
}

/// @brief Overrideable. Class destructor. Tidy up release resources used by 
///        *this instance.
CCVGFWCCVGItemDataBase::~CCVGFWCCVGItemDataBase()
{
  // Tidy up
  Clear();
}

/// @brief Class copy constructor. No work done here. The move can fail without 
///        notification.
/// @param[in] vRhs The object to copy from.
CCVGFWCCVGItemDataBase::CCVGFWCCVGItemDataBase(const CCVGFWCCVGItemDataBase &vRhs)
: CCVGClassCmnBase(vRhs)
, m_pParent(nullptr)
, m_pFileContentBuffer(nullptr)
, m_fileContentBufferSizeBytes(0)
, m_fileContentBufferSizeBytesMax(constExprFileContentBufferSizeBytesMax)
, m_bFileContentChanged(false)
{
  Copy(vRhs);
}

/// @brief Class move constructor. No work done here. The move can fail without 
///        notification.
/// @param[in] vRhs The object to copy or move from.
CCVGFWCCVGItemDataBase::CCVGFWCCVGItemDataBase(CCVGFWCCVGItemDataBase &&vRhs)
: CCVGClassCmnBase(std::move(vRhs))
, m_pParent(nullptr)
, m_pFileContentBuffer(nullptr)
, m_fileContentBufferSizeBytes(0)
, m_fileContentBufferSizeBytesMax(constExprFileContentBufferSizeBytesMax)
, m_bFileContentChanged(false)
{
  Move(std::move(vRhs));
}

/// @brief Class assignment operator copy. The copy can fail without 
///        notification.
///        *this object's m_bFileContentChanged flag is updated to true
///        if the new data is different from before the copy operation.
/// @param[in] vRhs The object to copy from.
CCVGFWCCVGItemDataBase& CCVGFWCCVGItemDataBase::operator= (const CCVGFWCCVGItemDataBase &vRhs)
{
  CCVGClassCmnBase::operator= (vRhs);
  Copy(vRhs);
  return *this;
}

/// @brief Class assignment operator move. The move can fail without 
///        notification.
///        *this object's m_bFileContentChanged flag is updated to true
///        if the new data is different from before the move operation.
///        The vRhs other object will have its m_bFileContentChanged flag
///        set to true to indicate that object has changed.
/// @param[in] vRhs The object to copy or move from.
CCVGFWCCVGItemDataBase& CCVGFWCCVGItemDataBase::operator= (CCVGFWCCVGItemDataBase &&vRhs)
{
  CCVGClassCmnBase::operator= (std::move(vRhs));
  Move(std::move(vRhs));
  return *this;
}

/// @brief Helper function to copy *this object's members to itself.
///        The copy can fail without notification.
///        *this object's m_bFileContentChanged flag is updated to true
///        if the new data is different from before the copy operation.
void CCVGFWCCVGItemDataBase::Copy(const CCVGFWCCVGItemDataBase &vRhs)
{
  if (this == &vRhs)
  {
    return;
  }

  m_pParent = vRhs.m_pParent;
  m_bFileContentChanged = vRhs.m_bFileContentChanged;
  FileContentBufferCopy(vRhs);
  m_nThisSubItemInUseCnt = vRhs.m_nThisSubItemInUseCnt;
}

/// @brief Helper function to move *this object's members to itself.
///        *this object's m_bFileContentChanged flag is updated to true
///        if the new data is different from before the move operation.
///        The vRhs other object will have its m_bFileContentChanged flag
///        set to true to indicate that object has changed.
void CCVGFWCCVGItemDataBase::Move(CCVGFWCCVGItemDataBase &&vRhs)
{
  if (this == &vRhs)
  {
    return;
  }
  
  // Note: Check need return vRhs to default condition, not empty
  m_pParent = vRhs.m_pParent; // Do not clear
  m_bFileContentChanged = vRhs.m_bFileContentChanged;
  vRhs.m_bFileContentChanged = false; // Reset
  FileContentBufferMove(vRhs);
  m_nThisSubItemInUseCnt = vRhs.m_nThisSubItemInUseCnt;
  vRhs.m_nThisSubItemInUseCnt = 0; // Reset
}

/// @brief Not equals operator. The comparison is for all *this object's
///        attributes and those of the base class.
/// @param[in] vRhs The other object.
/// @return bool True = not equal, false = equal.
bool CCVGFWCCVGItemDataBase::operator!= (const CCVGFWCCVGItemDataBase &vRhs) const
{
  bool bSame = CCVGClassCmnBase::operator== (vRhs);
  // Attribute m_pParent does not apply here;
  bSame = bSame && (m_fileContentBufferSizeBytes == vRhs.m_fileContentBufferSizeBytes);
  bSame = bSame && (m_bFileContentChanged == vRhs.m_bFileContentChanged);
  if (bSame)
  {
    FileContentBufferCompareSame(&vRhs.m_pFileContentBuffer[0], vRhs.m_fileContentBufferSizeBytes, bSame);
  }
  bSame = bSame && (m_nThisSubItemInUseCnt == vRhs.m_nThisSubItemInUseCnt);

  return !bSame;
}

/// @brief Equals operator. The comparison is for all *this object's
///        attributes and those of the base class.
/// @param[in] vRhs The other object.
/// @return bool True = Equal, false = not equal.
bool CCVGFWCCVGItemDataBase::operator== (const CCVGFWCCVGItemDataBase &vRhs) const
{
  bool bSame = CCVGClassCmnBase::operator== (vRhs);
  // Attribute m_pParent does not apply here;
  bSame = bSame && (m_fileContentBufferSizeBytes == vRhs.m_fileContentBufferSizeBytes);
  bSame = bSame && (m_bFileContentChanged == vRhs.m_bFileContentChanged);
  if (bSame)
  {
    FileContentBufferCompareSame(&vRhs.m_pFileContentBuffer[0], vRhs.m_fileContentBufferSizeBytes, bSame);
  }
  bSame = bSame && (m_nThisSubItemInUseCnt == vRhs.m_nThisSubItemInUseCnt);
  
  return bSame;
}

/// @brief Clear, release or reset *this objects' attributes to default.
void CCVGFWCCVGItemDataBase::Clear()
{
  m_nThisSubItemInUseCnt = 0;
  ClrFileContent();
  
  // Explicitly empty all containers:
  m_vecHighlights.clear();
  m_mapLineToVecSummaries.clear();
}

/// @brief Destroy and recreate the file data buffer to the specified new size.
/// @param[in] vSizeBytes Size of the buffer in bytes.
/// @return status Success = buffer created, failure = creation failed see 
///         *this object for error description.
status CCVGFWCCVGItemDataBase::FileContentBufferCreate(size_t vSizeBytes)
{
  FileContentBufferDestroy();

  if (vSizeBytes == 0)
  {
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferSizeZero);
    const wchar_t *pId(m_pParent->GetIdCCVGItemFileName().asCharArray());
    gtString errMsg;
    errMsg.appendFormattedString(pTxt, pId);
    return ErrorSet(errMsg);
  }

  if (m_fileContentBufferSizeBytesMax < vSizeBytes)
  {
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferMaxSizeExceeded);
    const wchar_t *pId(m_pParent->GetIdCCVGItemFileName().asCharArray());
    gtString errMsg;
    errMsg.appendFormattedString(pTxt, pId, m_fileContentBufferSizeBytesMax);
    return ErrorSet(errMsg);
  }

  const size_t realBufferSize = vSizeBytes;
  char *pBuffer = new (std::nothrow) char[realBufferSize];
  if (pBuffer == nullptr)
  {
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferAllocFail);
    const wchar_t *pId(m_pParent->GetIdCCVGItemFileName().asCharArray());
    gtString errMsg;
    errMsg.appendFormattedString(pTxt, pId, realBufferSize);
    return ErrorSet(errMsg);
  }

  m_pFileContentBuffer = pBuffer;
  m_fileContentBufferSizeBytes = realBufferSize;

  return success;
}

/// @brief Retrieve a pointer to *this object's file data.
/// @return char * Pointer to internal data buffer, NULL = *this object not
///         not assigned any data yet.
const char * const CCVGFWCCVGItemDataBase::FileContentBufferGet() const
{
  return m_pFileContentBuffer;
}

/// @brief Compare *this object's data with that of another data buffer.
///        Comparision at byte level.
/// @param[in]  vpOtherBuffer Pointer the other buffer.
/// @param[in]  vOtherBufferSizeBytes Size of the buffer in bytes.
/// @parem[out] vrwbResult True = Is the same, false = not same data.
/// @return status Success = comparison ok, failure = comparision failed.
status CCVGFWCCVGItemDataBase::FileContentBufferCompareSame(const char * const vpOtherBuffer, size_t vOtherBufferSizeBytes, bool &vrwbResult) const
{
  if ((m_pFileContentBuffer == nullptr) && (vpOtherBuffer == nullptr))
  {
    vrwbResult = true;
    return success;
  }
  if ((vOtherBufferSizeBytes == 0) && (m_fileContentBufferSizeBytes == 0))
  {
    vrwbResult = true;
    return success;
  }
  if (((m_pFileContentBuffer == nullptr) && (vpOtherBuffer != nullptr)) ||
    ((m_pFileContentBuffer != nullptr) && (vpOtherBuffer == nullptr)))
  {
    vrwbResult = false;
    return success;
  }
  if (vOtherBufferSizeBytes != m_fileContentBufferSizeBytes )
  {
    vrwbResult = false;
    return success;
  }
  const int result = std::memcmp((const void *)m_pFileContentBuffer, (const void *)vpOtherBuffer, m_fileContentBufferSizeBytes);

  vrwbResult = (result == 0);
  
  return success;
}

/// @brief Copy the contents of the other container into *this object's
///        data buffer.
///        *this object's m_bFileContentChanged flag is updated to true
///        if the new data is different from before the copy operation.
/// @param[in]  vRhs Copy from that object.
/// @return status Success = copy ok, failure = copy failed see *this object's
///         error message.
status CCVGFWCCVGItemDataBase::FileContentBufferCopy(const CCVGFWCCVGItemDataBase &vRhs)
{
  if (vRhs.m_fileContentBufferSizeBytes >= m_fileContentBufferSizeBytesMax)
  {
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferMaxSizeExceeded);
    const wchar_t *pId(m_pParent->GetIdCCVGItemFileName().asCharArray());
    gtString errMsg;
    errMsg.appendFormattedString(pTxt, pId, m_fileContentBufferSizeBytesMax);
    return ErrorSet(errMsg);
  }
  
  if (*this == vRhs)
  {
    return success;
  }

  FileContentBufferDestroy();

  if (vRhs.m_fileContentBufferSizeBytes == 0)
  {
    return success;
  }

  if (FileContentBufferCreate(vRhs.m_fileContentBufferSizeBytes) == failure)
  {
    return failure;
  }

  std::memcpy((void *)&m_pFileContentBuffer[0], (const void *)&vRhs.m_pFileContentBuffer[0], vRhs.m_fileContentBufferSizeBytes);
  m_fileContentBufferSizeBytes = vRhs.m_fileContentBufferSizeBytes;
  m_bFileContentChanged = true;

  return success;
}

/// @brief Move the contents of the other container into *this object's
///        data buffer. The other object is set to default. 
///        *this object's m_bFileContentChanged flag is updated to true
///        if the new data is different from before the move operation.
///        The vRhs other object will have its m_bFileContentChanged flag
///        set to true to indicate that object has changed.
/// @param[in]  vRhs Move from that object.
/// @return status Success = move ok, failure = move failed see *this object's
///         error message.
status CCVGFWCCVGItemDataBase::FileContentBufferMove(CCVGFWCCVGItemDataBase &vRhs)
{
  if (vRhs.m_fileContentBufferSizeBytes >= m_fileContentBufferSizeBytesMax)
  {
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferMaxSizeExceeded);
    const wchar_t *pId(m_pParent->GetIdCCVGItemFileName().asCharArray());
    gtString errMsg;
    errMsg.appendFormattedString(pTxt, pId, m_fileContentBufferSizeBytesMax);
    return ErrorSet(errMsg);
  }

  const bool bSame = (*this == vRhs);
  
  FileContentBufferDestroy();

  if (vRhs.m_fileContentBufferSizeBytes == 0)
  {
    return success;
  }

  m_pFileContentBuffer = &vRhs.m_pFileContentBuffer[0];
  m_fileContentBufferSizeBytes = vRhs.m_fileContentBufferSizeBytes;
  m_bFileContentChanged = !bSame;
  
  vRhs.m_pFileContentBuffer = nullptr;
  vRhs.m_fileContentBufferSizeBytes = 0;
  vRhs.m_bFileContentChanged = true;

  return success;
}

/// @brief Clear the data for *this object and reset to default.
void CCVGFWCCVGItemDataBase::FileContentBufferDestroy()
{
  if (m_pFileContentBuffer != nullptr)
  {
    delete[] m_pFileContentBuffer;
    m_pFileContentBuffer = nullptr;
    m_fileContentBufferSizeBytes = 0;
    m_bFileContentChanged = true;
  }
}

/// @brief Copy the contents of the other data buffer into *this object's
///        data buffer. to indicate that *this sub-item now needs to be
///        monitored for changed call function SetThisItemIUse().
/// @param[in]  vpData Source buffer to copy from.
/// @param[in]  vDataSizeBytes Size of the data to copy in bytes.
/// @return status Success = copy ok, failure = copy failed see *this object's
///         error message.
status CCVGFWCCVGItemDataBase::SetFileContent(const char * const vpData, size_t vDataSizeBytes)
{
  if (vDataSizeBytes >= m_fileContentBufferSizeBytesMax)
  {
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferMaxSizeExceeded);
    const wchar_t *pId(m_pParent->GetIdCCVGItemFileName().asCharArray());
    gtString errMsg;
    errMsg.appendFormattedString(pTxt, pId, m_fileContentBufferSizeBytesMax);
    return ErrorSet(errMsg);
  }

  bool bSame = false;
  if(FileContentBufferCompareSame(vpData, vDataSizeBytes, bSame) == failure)
  {
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferMaxSizeExceeded);
    const wchar_t *pId(m_pParent->GetIdCCVGItemFileName().asCharArray());
    gtString errMsg;
    errMsg.appendFormattedString(pTxt, pId, m_fileContentBufferSizeBytesMax);
    return ErrorSet(errMsg);
  }
  if (bSame)
  {
    return success;
  }

  FileContentBufferDestroy();

  if (vDataSizeBytes == 0)
  {
    return success;
  }

  if (FileContentBufferCreate(vDataSizeBytes) == failure)
  {
    return failure;
  }

  std::memcpy((void *)&m_pFileContentBuffer[0], (const void *)&vpData[0], vDataSizeBytes);
  m_fileContentBufferSizeBytes = vDataSizeBytes;
  m_bFileContentChanged = true;

  return success;
}

/// @brief Retrieve a pointer to *this object's file data.
/// @return char* Pointer or NULL is not filled with data.
const char * const CCVGFWCCVGItemDataBase::GetFileContent() const
{
  return FileContentBufferGet();
}

/// @brief Retrieve *this object's file data size.
/// @return size_t Size bytes.
size_t CCVGFWCCVGItemDataBase::GetFileContentSizeBytes() const
{
  return m_fileContentBufferSizeBytes;
}

/// @brief Determine if *this object's file content data hase changed in any 
///        because of calling *this class's following functions:
///         SetFileContent()
///         Operator copy assignment
///         Operator move assignment
///         Clear()
/// @return bool True = data changed, false = no change.
bool CCVGFWCCVGItemDataBase::HasFileContentChanged() const
{
  if (m_nThisSubItemInUseCnt > 0)
  {
    return m_bFileContentChanged;
  }
  return false;
}

/// @brief Set this object's flag for whether the content is valid (ie. has been
///        or can be successfully parsed)
void CCVGFWCCVGItemDataBase::SetFileContentValid(bool vbFileContentValid)
{
  m_bFileContentValid = vbFileContentValid;
}

/// @brief Clear the file content data flag indicating it has changed in any 
///        because of calling *this class's following functions:
///         SetFileContent()
///         Operator copy assignment
///         Operator move assignment
///         Clear()
void CCVGFWCCVGItemDataBase::ClrFileContentChangedFlag()
{
  m_nThisSubItemInUseCnt = 0;
}

/// @brief Clear the temp data buffer for *this object and reset to default.
void CCVGFWCCVGItemDataBase::FileContentBufferTempForComparisonDestroy()
{
  if (m_pFileContentBufferTempForComparison != nullptr)
  {
    delete[] m_pFileContentBufferTempForComparison;
    m_pFileContentBufferTempForComparison = nullptr;
    m_fileContentBufferTempForComparisonSizeBytes = 0;
  }
}

/// @brief Destroy and recreate the temporary file data comparison buffer to 
///        the specified new size.
/// @param[in] vSizeBytes Size of the buffer in bytes.
/// @return status Success = buffer created, failure = creation failed see 
///         *this object for error description.
status CCVGFWCCVGItemDataBase::FileContentBufferTempForComparisonCreate(size_t vSizeBytes)
{
  FileContentBufferTempForComparisonDestroy();

  if (vSizeBytes == 0)
  {
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferSizeZero);
    const wchar_t *pId(m_pParent->GetIdCCVGItemFileName().asCharArray());
    gtString errMsg;
    errMsg.appendFormattedString(pTxt, pId);
    return ErrorSet(errMsg);
  }

  if (m_fileContentBufferSizeBytesMax < vSizeBytes)
  {
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferMaxSizeExceeded);
    const wchar_t *pId(m_pParent->GetIdCCVGItemFileName().asCharArray());
    gtString errMsg;
    errMsg.appendFormattedString(pTxt, pId, m_fileContentBufferSizeBytesMax);
    return ErrorSet(errMsg);
  }

  const size_t realBufferSize = vSizeBytes;
  char *pBuffer = new (std::nothrow) char[realBufferSize];
  if (pBuffer == nullptr)
  {
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferAllocFail);
    const wchar_t *pId(m_pParent->GetIdCCVGItemFileName().asCharArray());
    gtString errMsg;
    errMsg.appendFormattedString(pTxt, pId, realBufferSize);
    return ErrorSet(errMsg);
  }

  m_pFileContentBufferTempForComparison = pBuffer;
  m_fileContentBufferTempForComparisonSizeBytes = realBufferSize;

  return success;
}

/// @brief Retrieve a pointer to *this CCVGItemData's temperory file data (just
///        read from the file).
/// @return char* A valid pointer if file data re-read, NULL = reread complete 
///        or never re-read.
const char * const CCVGFWCCVGItemDataBase::FileContentBufferTempForComparisonGet() const
{
  return m_pFileContentBufferTempForComparison;
}

/// @brief Retrieve the size of *this CCVGItemData's temperory file data (just
///        read from the file) buffer.
/// @return size_t Size in bytes.
size_t CCVGFWCCVGItemDataBase::FileContentBufferTempForComparisonGetSizeBytes() const
{
  return m_fileContentBufferTempForComparisonSizeBytes;
}

/// @brief Copy data from another source into *this object's temporary file
///        data buffer for later comparision with existing file data hold in
///        this object.
/// @param[in] char* The source buffer to copy from.
/// @param[in] size_t The size in bytes of the data to copy.
/// @return status Success = buffer created and filled, failure = see error
///         for *this object.
status CCVGFWCCVGItemDataBase::FileContentBufferTempForComparisonSet(const char * const vpOtherBuffer, size_t vOtherBufferSizeBytes)
{
  FileContentBufferTempForComparisonDestroy();

  if ((vpOtherBuffer == nullptr) || (vOtherBufferSizeBytes == 0))
  {
    return success;
  }

  if (failure == FileContentBufferTempForComparisonCreate(vOtherBufferSizeBytes))
  {
    return failure;
  }

  std::memcpy((void *) &m_pFileContentBufferTempForComparison[0], (const void *) vpOtherBuffer, vOtherBufferSizeBytes);
  m_fileContentBufferTempForComparisonSizeBytes = vOtherBufferSizeBytes;
 
  return success;
}

/// @brief Compare the temporary holding data buffer with the permanent data
///        buffer contents to see if different in anyway. The flag 
///        m_bFileContentChanged is set depending on the result.
void CCVGFWCCVGItemDataBase::FileContentBufferTempForComparisonDoCmp() 
{
  bool bSame = true;
  FileContentBufferCompareSame(m_pFileContentBufferTempForComparison, m_fileContentBufferTempForComparisonSizeBytes, bSame);
  m_bFileContentChanged = !bSame;
}

/// @brief Transfer the temporary holding data buffer contents to the permanent
///         data buffer.
/// @return status Success = transfered ok, failure = error occurred to error
///         text.
status CCVGFWCCVGItemDataBase::FileContentBufferTempForComparisonTransfer()
{
  if (m_fileContentBufferTempForComparisonSizeBytes >= m_fileContentBufferSizeBytesMax)
  {
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_fileBufferMaxSizeExceeded);
    const wchar_t *pId(m_pParent->GetIdCCVGItemFileName().asCharArray());
    gtString errMsg;
    errMsg.appendFormattedString(pTxt, pId, m_fileContentBufferSizeBytesMax);
    return ErrorSet(errMsg);
  }

  FileContentBufferDestroy();

  if (m_fileContentBufferTempForComparisonSizeBytes == 0)
  {
    return success;
  }

  if (FileContentBufferCreate(m_fileContentBufferTempForComparisonSizeBytes) == failure)
  {
    return failure;
  }

  m_pFileContentBuffer = m_pFileContentBufferTempForComparison;
  m_pFileContentBufferTempForComparison = nullptr;
  m_fileContentBufferTempForComparisonSizeBytes = 0;
  //m_bFileContentChanged  not required

  return success;
}

/// @brief Re-read the *this object's file's data again and check if the 
///        contents have changed since the last file read. The in use counter
///        m_nThisSubItemInUseCnt is checked and if >0 the check does not
///        occur. The flag m_bFileContentChanged is updated to reflect if 
///        the data has changed at all.
/// @param[in] FileDataType Enumeration of the file type to read.
/// @param[in] bool True = Store data in temporary comparision buffer,
///            false = store data in the permanent  buffer.
/// @return status Success = Read data ok, failure = error occurred.
status CCVGFWCCVGItemDataBase::ReReadFileData(cmd::CCVGCmdFileRead::FileDataType veFileType, bool vbStoreDataInTempBuffer)
{
  if (!(m_nThisSubItemInUseCnt > 0))
  {
    return success;
  }

  const gtString &rCCVGItemId(m_pParent->GetIdCCVGItemFileName());
  const status status = ExecuteCmdReadFile(rCCVGItemId, veFileType, vbStoreDataInTempBuffer);
  if (status == success)
  {
    FileContentBufferTempForComparisonDoCmp();
    if (m_bFileContentChanged)
    {
      FileContentBufferTempForComparisonTransfer();
    }
    return success;
  }
  
  const wchar_t *pErrObj = GetThisObjTypeString().asCharArray();
  return ErrorSet(pErrObj);
}

/// @brief Retrieve the name of  *this CCVGItem which it represent. The name
///        is used to form error message text.
/// @return gtString& Text.
const gtString& CCVGFWCCVGItemDataBase::GetThisObjTypeString() const
{
  return m_constStrObjId;
}

/// @brief Retrieve the enumeration CCVGItemData sub-item type for *this 
///        object.
EnumCCVGItemDataSubObjType CCVGFWCCVGItemDataBase::GetThisObjType() const
{
  return m_eConstThisCCVGItemDataSubObjType;
}

/// @brief *this object is in use. File data is present (read from a file) and
///        needs to refreshed, checked or maintained. When not in use *this
///        object is affected ignored.
/// param[in] vbYes True = in use, false = not in use.
void CCVGFWCCVGItemDataBase::SetInUse(bool vbYes)
{
  m_nThisSubItemInUseCnt = vbYes ? ++m_nThisSubItemInUseCnt : --m_nThisSubItemInUseCnt;
}

/// @brief Check if *this object is in use. When not in use *this
///        object is affected ignored.
/// param[in] vbYes True = in use, false = not in use.
bool CCVGFWCCVGItemDataBase::GetInUse() const
{
  return (m_nThisSubItemInUseCnt > 0);
}

/// @brief Empty *this CCVGItemData sub-item of its data. Example the user has
///        closed a view displaying the was contents of *this object.
void CCVGFWCCVGItemDataBase::ClrFileContent()
{
  FileContentBufferDestroy();
  FileContentBufferTempForComparisonDestroy();
}

/// @brief Construct a new CCVGFWCCVGHighlight at the back of the item's
///        vector of highlights
/// param[in] CCVGuint Number of characters into the text that the highlight
///           begins
/// param[in] CCVGuint Length of the highlight (number of characters)
/// param[in] CCVGFWCCVGHighlightType Enum specifying what the highlight represents
status CCVGFWCCVGItemDataBase::AddHighlight(CCVGuint vStartPos, CCVGuint vLength, CCVGFWCCVGHighlightType veHighlightType)
{
  try
  {
    m_vecHighlights.emplace_back(vStartPos, vLength, veHighlightType);
  }
  catch (...)
  {
    return failure;
  }
  return success;
}

/// @brief Retrieve the vector of highlights generated for this item
/// @return std::vector<CCVGFWCCVGHighlight>& highlights
const std::vector<CCVGFWCCVGHighlight>& CCVGFWCCVGItemDataBase::GetHighlights() const
{
  return m_vecHighlights;
}

/// @brief Retrieve the map of line numbers to a vector of function summaries for that line.
/// @return Map from line number to vector of summaries.
const CCVGFWCCVGItemDataBase::MapLineToVecSummaries_t &CCVGFWCCVGItemDataBase::GetMapLineToVecSummaries() const
{
  return m_mapLineToVecSummaries;
}

} // namespace fw
} // namespace ccvg
