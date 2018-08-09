// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief The SYCL ATP File Parser.
///
/// SYCLAtpFilePart implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef SYCLATPFILEPART_H_INCLUDE
#define SYCLATPFILEPART_H_INCLUDE

// Infra:
#include "../Common/FileUtils.h"
#include "../sprofile/AtpFile.h"
#include "../Common/ATPFileUtils.h"
#include "../Common/StringUtils.h"

// Local:
#include "APIInfoManagerBase.h"
#include "SYCLAPIInfo.h"

// STL:
#include <sstream>

/// @brief    The SYCLAtpFilePart is responsible for reading/writing the ATP (Application Timeline Profile)
///           file for the SYCL part of the profile.
/// @warnings None.
class SYCLAtpFilePart : public IAtpFilePart, public IAtpFilePartParser, public BaseParser<SYCLAPIInfo>
{
// Methods
public:    
    SYCLAtpFilePart(const Config &vConfig, bool vbShouldReleaseMemory = false);
    ~SYCLAtpFilePart() override = default;

    // From IAtpFilePartParser
    bool Parse(std::istream &vIn, std::string &vOutErrorMsg) override;
    bool ParseHeader(const std::string &vStrKey, const std::string &vStrVal) override;

    // From IAtpFilePart
    void WriteHeaderSection(SP_fileStream &vSOut) override;
    bool WriteContentSection(SP_fileStream &vSOut, const std::string &vStrTmpFilePath, const std::string &vStrPID) override;    
};

#endif // SYCLATPFILEPART_H_INCLUDE
