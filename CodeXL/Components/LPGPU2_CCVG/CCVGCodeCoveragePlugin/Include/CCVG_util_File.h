// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// Global utility or helper functions.
/// CCVGUtilFile interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGFnStatus.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>

#ifndef LPGPU2_CCVG_UTIL_FILE_H_INCLUDE
#define LPGPU2_CCVG_UTIL_FILE_H_INCLUDE

/// @brief    Utility class that handles the reading and writing to a file.
///           *this class represents one file.
/// @warning  None.
/// @see      None.
/// @date     14/02/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGUtilFile final
: public CCVGClassCmnBase
{
// Methods:
public:
  CCVGUtilFile();
  ~CCVGUtilFile();
  status          SetFileName(const gtString &vFileName);
  status          SetFilePath(const gtString &vFileDirPath);
  status          GetFileSizeBytes(size_t &vwBytes);
  const gtString& GetFileNamePath() const;
  bool            FileExists() const;
  status          FileRead(char *&vpBufferArray, size_t vBufferSizeBytes, size_t &vrwReadBytes, bool vbTextMode, bool vbOwnsBuffer = true);

// Methods:
private:
  gtString FormFileNamePath() const;

// Attributes:
private:
  char     *m_pBuffer;                // Pointer to client's allocated buffer *this may delete if owns
  bool      m_bThisOwnsTheReadBuffer; // True = Yes owns and deletes on destruction, false = client owns buffer.
  gtString  m_fileName;
  gtString  m_fileDirPath;
  gtString  m_fileNamePath;
  bool      m_bHaveValidFileNameAndPath; // True = Yes valid, false = not valid
  bool      m_bHaveValidFileName;        // True = Yes valid, false = not valid
  bool      m_bHaveValidFileNamePath;    // True = Yes valid, false = not valid
  //
  const size_t m_constMaxFileNameLength;
  const size_t m_constMaxFileDirPathLength;
};
// clang-format on

#endif // LPGPU2_CCVG_UTIL_FILE_H_INCLUDE