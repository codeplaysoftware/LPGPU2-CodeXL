// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGItemDataExecuteable implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Standard: (for hex formatting of byte array)
#include <iomanip>
#include <sstream>

// Qt:
#include <QString>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataExecuteable.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFileRead.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectDataCcvg.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMCovMapReader.h>

// Framework
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

// On Windows, include both executable formats. On Linux, only ElfFile works.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#include <AMDTExecutableFormat/inc/PeFile.h>
#endif
#include <AMDTExecutableFormat/inc/ElfFile.h>

// constants
static constexpr const char*    ms_constCovMapHeaderWindows = ".lcovmap";
static constexpr const wchar_t* ms_constCovMapHeaderWindowsW = L".lcovmap";
static constexpr const char*    ms_constCovMapHeaderLinux = "__llvm_covmap";
static constexpr const wchar_t* ms_constCovMapHeaderLinuxW = L"__llvm_covmap";
static constexpr CCVGuint       ms_constHexCharsPerLine = 16;
static constexpr CCVGuint       ms_constHexCharsPerWord = 4; 

namespace ccvg {
namespace fw {
  

// @brief Class constructor. No work done here.
CCVGFWCCVGItemDataExecuteable::CCVGFWCCVGItemDataExecuteable(CCVGFWTheCurrentStateProjectDataCcvg &vpParent)
: CCVGFWCCVGItemDataBase(vpParent)
{
  m_constStrObjId = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailExecuteable);
  m_eConstThisCCVGItemDataSubObjType = EnumCCVGItemDataSubObjType::kkCCVGItemDataSubObjType_fileDataExecuteable;
}

/// @brief Overrideable. Class destructor. Tidy up release resources used by 
///        *this instance.
CCVGFWCCVGItemDataExecuteable::~CCVGFWCCVGItemDataExecuteable()
{
  // Tidy up
  Clear();
}

/// @brief Class copy constructor. No work done here.
/// @param[in] vRhs The object to copy from.
CCVGFWCCVGItemDataExecuteable::CCVGFWCCVGItemDataExecuteable(const CCVGFWCCVGItemDataExecuteable &vRhs)
: CCVGFWCCVGItemDataBase(vRhs)
{
  Copy(vRhs);
}

/// @brief Class move constructor. No work done here.
/// @param[in] vRhs The object to copy or move from.
CCVGFWCCVGItemDataExecuteable::CCVGFWCCVGItemDataExecuteable(CCVGFWCCVGItemDataExecuteable &&vRhs)
: CCVGFWCCVGItemDataBase(std::move(vRhs))
{
  Move(std::move(vRhs));
}

/// @brief Class assignment operator copy.
/// @param[in] vRhs The object to copy from.
CCVGFWCCVGItemDataExecuteable& CCVGFWCCVGItemDataExecuteable::operator= (const CCVGFWCCVGItemDataExecuteable &vRhs)
{
  CCVGFWCCVGItemDataBase::operator= (vRhs);
  Copy(vRhs);
  return *this;
}

/// @brief Class assignment operator move.
/// @param[in] vRhs The object to copy or move from.
CCVGFWCCVGItemDataExecuteable& CCVGFWCCVGItemDataExecuteable::operator= (CCVGFWCCVGItemDataExecuteable &&vRhs)
{
  CCVGFWCCVGItemDataBase::operator= (std::move(vRhs));
  Move(std::move(vRhs));
  return *this;
}

/// @brief Overridden. Helper function to copy *this object's members to 
///        itself.
void CCVGFWCCVGItemDataExecuteable::Copy(const CCVGFWCCVGItemDataExecuteable &vRhs)
{
  if (this == &vRhs)
  {
    return;
  }
  
  // ToDo: put attributes here
  //m_dummyDriver = vRhs.m_dummyDriver;
}

/// @brief Overridden. Helper function to move *this object's members to 
///        itself.
void CCVGFWCCVGItemDataExecuteable::Move(CCVGFWCCVGItemDataExecuteable &&vRhs)
{
  if (this == &vRhs)
  {
    return;
  }
  
  // ToDo: put attributes here
  // m_dummyDriver = std::move(vRhs.m_dummyDriver);
}

/// @brief Not equals operator. The comparison is for all *this object's
///        attributes and those of the base class.
/// @param[in] vRhs The other object.
/// @return bool True = not equal, false = equal.
bool CCVGFWCCVGItemDataExecuteable::operator!= (const CCVGFWCCVGItemDataExecuteable &vRhs) const
{
  bool bSame = CCVGFWCCVGItemDataBase::operator== (vRhs);
  //bSame = bSame && (m_dummyDriver == vRhs.m_dummyDriver);
  return !bSame;
}

/// @brief Equals operator. The comparison is for all *this object's
///        attributes and those of the base class.
/// @param[in] vRhs The other object.
/// @return bool True = equal, false = not equal.
bool CCVGFWCCVGItemDataExecuteable::operator== (const CCVGFWCCVGItemDataExecuteable &vRhs) const
{
  bool bSame = CCVGFWCCVGItemDataBase::operator== (vRhs);
  //bSame = bSame && (m_dummyDriver == vRhs.m_dummyDriver);
  return bSame;
}

/// @brief Overridden. Clear, release or reset *this objects' attributes to 
///        default.
void CCVGFWCCVGItemDataExecuteable::Clear()
{
  // ToDo: put attributes here
  // m_dummyDriver = 0;
  m_pCovMapBytes = nullptr;
}

/// @brief Retrieve the coverage map reader associated with this executable file
const CCVGFWLLVMCovMapReader &CCVGFWCCVGItemDataExecuteable::GetCovMapReader() const
{
  return m_covMapReader;
}

/// @brief *this object represent a phyiscal file. Re-read the file and check
///        if the data with has changed.
/// @return status Success = all ok, failure = see error description in 
///         CCVGFWTheCurrentStateProjectDataCcvg object.
status CCVGFWCCVGItemDataExecuteable::ReReadFileData()
{
  const bool bStoreDataInTempBuffer = true;
  return CCVGFWCCVGItemDataBase::ReReadFileData(cmd::CCVGCmdFileRead::FileDataType::kFileDataType_Executeable, bStoreDataInTempBuffer);
}

// @brief Initialise this object's AMDT ExecutableFile object and process the
//        data in the relevant segment.
status CCVGFWCCVGItemDataExecuteable::CreateExecutable(const gtString &vrFileName, const gtString &vrFileDirPath)
{
  gtString filePathFull;
  filePathFull.appendFormattedString(L"%ls/%ls", vrFileDirPath.asCharArray(), vrFileName.asCharArray());

  try
  {
    m_executableFile = std::unique_ptr<ExecutableFile>(ExecutableFile::Open(filePathFull.asCharArray()));
  }
  catch (...)
  {
    m_executableFile = nullptr;
  }

  if (m_executableFile == nullptr)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemDataExecuteable_err_allocateExecutable);
    gtString errMsg(pErrMsg);
    return ErrorSet(errMsg);
  }

  bool bExeOpen = m_executableFile->Open();
  if (!bExeOpen)
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemDataExecuteable_err_createAMDTExecutable);
    gtString errMsg(pErrMsg);
    return ErrorSet(errMsg);
  }

  // Find out if the executable is a PeFile or ElfFile
  const bool bIsElfFile = dynamic_cast<ElfFile*>(m_executableFile.get()) != nullptr;  

  // Select the appropriate header names for the executable format
  const auto *covMapHeader = bIsElfFile ? ms_constCovMapHeaderLinux : ms_constCovMapHeaderWindows;
  const auto *covMapHeaderW = bIsElfFile ? ms_constCovMapHeaderLinuxW : ms_constCovMapHeaderWindowsW;
  
  // Check for the appropriate coverage mapping section in the binary, and
  // retrieve the pointer to its data.
  const CCVGuint uCovMapSection = m_executableFile->LookupSectionIndex(covMapHeader);
  if (uCovMapSection == m_executableFile->GetSectionsCount())
  {
    const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemDataExecuteable_err_readCovMapSegment);
    gtString errMsg;
    errMsg.appendFormattedString(pErrMsg, covMapHeaderW, filePathFull);
    return ErrorSet(errMsg);
  }
  m_pCovMapBytes = m_executableFile->GetSectionBytes(uCovMapSection);
  
  // Display the coverage mapping segment's data in the executable file view
  // as a formatted hex dump
  std::wstringstream ss;
  for (CCVGuint i = 0; i < m_executableFile->GetSectionSize(uCovMapSection); i++)
  {
    ss << std::setfill(L'0') << std::setw(2) << std::hex << (CCVGuint) m_pCovMapBytes[i];
    if (i % ms_constHexCharsPerWord == ms_constHexCharsPerWord - 1)
    {
      ss << L' ';
    }
    if (i % ms_constHexCharsPerLine == ms_constHexCharsPerLine - 1)
    {
      ss << std::endl;
    }
  }

  // Parse the contents of the coverage map segment
  if (m_covMapReader.ReadHeader(m_pCovMapBytes) == success)
  {
    // Display the parsed contents, using a QString to provide easy conversion
    // between wchar_t* and char* strings.
    ss << m_covMapReader.GetInfoStream().str();
    ClrFileContent();
    auto contentString = QString::fromWCharArray(ss.str().c_str());
    return SetFileContent(contentString.toLatin1().data(), contentString.size() + 1);    
  }
  else
  {
    return failure;
  }
}

} // namespace fw
} // namespace ccvg
