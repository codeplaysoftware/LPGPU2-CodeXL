// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2InputOutput project is a collection of classes to allow
///         data to be imported/exported to/from CodeXL. The goal is to
///         have an independent library that can be used across all projects
///         that need access to external data.
///
/// CSVFileWriter interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_IO_CSVROW_H_INCLUDE
#define LPGPU2_IO_CSVROW_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <LPGPU2InputOutput/Include/LPGPU2_io_CSVFileBase.h>
#include <LPGPU2InputOutput/Include/LPGPU2_io_CSVRow.h>
#include <LPGPU2InputOutput/Include/LPGPU2_io_FnStatus.h>

namespace lpgpu2 {
namespace io {

using namespace fnstatus;

/// @brief      This is used to write CSV files. It allows flushing the data
///             periodically in case the contents are to big to store in memory.
/// @warning    All the CSVRow objects returned from CSVFileWriter::AddRow()
///             are managed and to not need to be deleted.
/// @date       12/09/2017
/// @author     Thales Sabino   
// clang-format off
class CSVFileWriter : public CSVFileBase
{
// Methods:
public:
    explicit CSVFileWriter(const osFilePath &vFilePath, bool vbOverride);
    virtual ~CSVFileWriter();

    status Flush();
    status Close();

    status AddColumn(const gtString &vColumnName);
    status AddColumns(const gtVector<gtString> &vColumnNames);
    status AddHeader(const gtString &vHeaderName);

    std::shared_ptr<CSVRow> AddRow();

// Methods
private:
    status WriteHeaderAndColumnRow(std::wofstream &vCsvFileStream) const;
    status WriteRows(std::wofstream &vCsvFileStream) const;

// Attributes:
private:
    osFilePath      m_filePath;
    gtSet<gtString> m_columnSet;
    bool            m_bOverrideExistingFile;
    bool            m_bWrittenHeaderAndColumnRow = false;
    bool            m_bIsFileClosed = false;
};
// clang-format on

} // namesapce io
} // namespace lpgpu2

#endif // LPGPU2_IO_CSVROW_H_INCLUDE