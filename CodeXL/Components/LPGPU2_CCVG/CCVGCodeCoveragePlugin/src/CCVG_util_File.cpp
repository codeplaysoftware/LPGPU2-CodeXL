// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// Global utility or helper functions.
/// CCVGUtilFile implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// std:
#include <cstdio>   // For max file name length
#include <limits.h> // For max file path length

// Infrastructure:
#include <AMDTOSWrappers/Include/osFile.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_File.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

/// @brief Class constructor. No work is done here by this class.
CCVGUtilFile::CCVGUtilFile()
: m_pBuffer(nullptr)
, m_bThisOwnsTheReadBuffer(false)
, m_bHaveValidFileNameAndPath(false)
, m_constMaxFileNameLength(FILENAME_MAX)
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
, m_constMaxFileDirPathLength(PATH_MAX)
#else
, m_constMaxFileDirPathLength(_MAX_PATH)
#endif
, m_bHaveValidFileName(false)
, m_bHaveValidFileNamePath(false)
{
}

/// @brief Class destructor. Tidy up release resources used by *this instance.
CCVGUtilFile::~CCVGUtilFile()
{
  if (m_bThisOwnsTheReadBuffer && (m_pBuffer != nullptr))
  {
    delete[] m_pBuffer;
  }
  m_pBuffer = nullptr;
}

/// @brief Retrieve the file's directory location path.
/// @return gtString& The path text.
const gtString& CCVGUtilFile::GetFileNamePath() const
{
  return m_fileNamePath;
}

/// @brief Specify the file's name.
/// @param[in] vFileName File's name.
/// @return status Success = file name accepted, failure = file name not valid.
status CCVGUtilFile::SetFileName(const gtString &vFileName)
{
  m_bHaveValidFileName = false;

  if (vFileName.isEmpty())
  {
    gtString errMsg;
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_util_file_err_fileNameInvalid);
    errMsg.appendFormattedString(pTxt, m_fileName.asCharArray(), m_fileDirPath.asCharArray());
    return ErrorSet(errMsg);
  }

  if (vFileName.length() >= static_cast<int>(m_constMaxFileNameLength))
  {
    gtString errMsg;
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_util_file_err_fileNameToLong);
    errMsg.appendFormattedString(pTxt, m_fileName.asCharArray(), m_constMaxFileNameLength);
    return ErrorSet(errMsg);
  }

  m_bHaveValidFileName = true;
  m_fileName = vFileName;
  m_bHaveValidFileNameAndPath = m_bHaveValidFileNamePath && m_bHaveValidFileName;
  if (m_bHaveValidFileNameAndPath)
  {
    m_fileNamePath = FormFileNamePath();
  }

  return success;
}

/// @brief Specify the file's directory path location.
/// @param[in] vFileName File's path.
/// @return status Success = file path accepted, failure = file path not valid.
status CCVGUtilFile::SetFilePath(const gtString &vFileDirPath)
{
  m_bHaveValidFileNamePath = false;

  if (vFileDirPath.isEmpty())
  {
    gtString errMsg;
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_util_file_err_fileDirPathInvalid);
    errMsg.appendFormattedString(pTxt, m_fileName.asCharArray(), m_fileDirPath.asCharArray());
    return ErrorSet(errMsg);
  }

  if (vFileDirPath.length() >= static_cast<int>(m_constMaxFileDirPathLength))
  {
    gtString errMsg;
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_util_file_err_fileNameToLong);
    errMsg.appendFormattedString(pTxt, m_fileName.asCharArray(), m_constMaxFileDirPathLength);
    return ErrorSet(errMsg);
  }

  m_bHaveValidFileNamePath = true;
  m_fileDirPath = vFileDirPath;
  m_bHaveValidFileNameAndPath = m_bHaveValidFileNamePath && m_bHaveValidFileName;
  if (m_bHaveValidFileNameAndPath)
  {
    m_fileNamePath = FormFileNamePath();
  }

  return success;
}

/// @brief Retrieve the file's name and directory location. This path is a 
///        the path with the file name appended using functions SetFilePath()
///        and SetFileName() respectively.
/// @return gtString Path text.
gtString CCVGUtilFile::FormFileNamePath() const
{
  return gtString().appendFormattedString(L"%ls/%ls", m_fileDirPath.asCharArray(), m_fileName.asCharArray());
}

/// @brief Retrieve the file's content size (specified previously using 
///        functions SetFilePath() and SetFileName()). This function must be
///        called after using functions SetFilePath() and SetFileName()
///        otherwise an error will occur.
/// @param[out] vwBytes Size in bytes.
/// @return status Success = file size valid, failure = previous functions not
///          called or error locating the file or reading the file.
status CCVGUtilFile::GetFileSizeBytes(size_t &vwBytes) 
{
  vwBytes = 0;
  if (!m_bHaveValidFileNameAndPath)
  {
    gtString errMsg;
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_util_file_err_fileNamePathInvalid);
    errMsg.appendFormattedString(pTxt, m_fileName.asCharArray(), m_fileDirPath.asCharArray());
    return ErrorSet(errMsg);
  }
 
  osFile file(m_fileNamePath);
  unsigned long fileSize = 0;
  const bool bOk = file.getSize(fileSize);
  if (!bOk)
  {
    gtString errMsg;
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_util_file_err_readFailed);
    errMsg.appendFormattedString(pTxt, m_fileNamePath.asCharArray());
    return ErrorSet(errMsg);
  }

  vwBytes = static_cast<size_t>(fileSize);

  return success;
}

/// @brief Determine if the files exists.
/// @return bool True = yes exists, false = does not exist.
bool CCVGUtilFile::FileExists() const
{
  osFile file(m_fileNamePath);
  return file.exists();
}

/// @brief Retrieve the file's contents (specified previously using 
///        functions SetFilePath() and SetFileName()). This function must be
///        called after using functions SetFilePath() and SetFileName()
///        otherwise an error will occur. The contents are placed int the 
///        buffer provided by the client.
/// @param[in] vpBufferArray Client provided allocated memory.
/// @param[in] vBufferSizeBytes Size in bytes of the client's buffer.
/// @param[out] vrwReadBytes Number of characters read from the file.
/// @param[in] vbTextMode True = open file ASCII mode, false = binary mode.
/// @param[in] vbOwnsBuffer True = *this class delete buffer on going out of 
///            scope, false = Client manages their allocation. Should an error
///            occur during this function the ownership passes back to the 
///            client. (Dflt = true)
/// @return status Success = file read ok, failure = read failed.
status CCVGUtilFile::FileRead(char *&vpBufferArray, size_t vBufferSizeBytes, size_t &vrwReadBytes, bool vbTextMode, bool vbOwnsBuffer /*= true*/)
{
  vrwReadBytes = 0;
  
  if (!m_bHaveValidFileNameAndPath)
  {
    gtString errMsg;
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_util_file_err_fileNamePathInvalid);
    errMsg.appendFormattedString(pTxt, m_fileName.asCharArray(), m_fileDirPath.asCharArray());
    return ErrorSet(errMsg);
  }
  if (vpBufferArray == nullptr)
  {
    gtString errMsg;
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_util_file_err_invalidBufferSize);
    errMsg.appendFormattedString(pTxt, vBufferSizeBytes);
    return ErrorSet(errMsg);
  }

  const osChannel::osChannelType eReadMode = vbTextMode ? osFile::OS_ASCII_TEXT_CHANNEL : osFile::OS_BINARY_CHANNEL;
  osFile file(m_fileNamePath);
  bool bOk = file.open(eReadMode, osFile::OS_OPEN_TO_READ);
  if(!bOk)
  {
    gtString errMsg;
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_util_file_err_openFailed);
    errMsg.appendFormattedString(pTxt, m_fileNamePath.asCharArray());
    return ErrorSet(errMsg);
  }
  gtSize_t dataRead = 0;
  bOk = file.readAvailableData(vpBufferArray, vBufferSizeBytes, dataRead);
  if (!bOk)
  {
    file.close();
    gtString errMsg;
    const wchar_t *pTxt = CCVGRSRC(kIDS_CCVG_STR_util_file_err_readFailed);
    errMsg.appendFormattedString(pTxt, m_fileNamePath.asCharArray());
    return ErrorSet(errMsg);
  }

  if (vbTextMode)
  {
    // Add null at the end of the buffer making sure it is null terminated
    vpBufferArray[dataRead] = 0;
  }

  m_bThisOwnsTheReadBuffer = vbOwnsBuffer;
  if (m_bThisOwnsTheReadBuffer)
  {
    m_pBuffer = vpBufferArray;
  }
  vrwReadBytes = dataRead;

  file.close();

  return success;
}
