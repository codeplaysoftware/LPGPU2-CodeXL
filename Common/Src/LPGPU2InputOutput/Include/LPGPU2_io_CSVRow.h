// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2InputOutput project is a collection of classes to allow
///         data to be imported/exported to/from CodeXL. The goal is to
///         have an independent library that can be used across all projects
///         that need access to external data.
///
/// CSVRow interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_IO_CSV_ROW_H_INCLUDE
#define LPGPU2_IO_CSV_ROW_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <LPGPU2InputOutput/Include/LPGPU2_io_FnStatus.h>

namespace lpgpu2 {
namespace io {

using namespace fnstatus;

/// @brief      This represents a single row of a CSV file. It has
///             methods for setting and getting the data of the row.
/// @warning    This class should be instanciated directly. Use
///             lpgpu2::io::CSVFileWriter::addRow() instead.
/// @see        lpgpu2::io::CSVFileWriter::addRow().
/// @date       08/08/2017
/// @author     Thales Sabino
// clang-format off
class CSVRow
{
// Methods:
public:
    explicit CSVRow(const gtVector<gtString> &vHeader);

    // Disable copy and move of this object
    CSVRow(const CSVRow&) = delete;
    CSVRow(CSVRow&&) = delete;
    const CSVRow& operator= (const CSVRow&) = delete;

    // Accessor methods    
    status SetRowData(const gtString &vColumn, const gtString &vValue);
    status SetRowData(const gtVector<gtString> &vRowData);
    status GetRowData(const gtString &vColumn, gtString &vValue) const;

// Attributes:
private:
    gtMap<gtString, gtString> m_rowData;
    const gtVector<gtString> &m_header;
};
// clang-format on

} // namespace io
} // namespace lpgpu2

#endif // LPGPU2_IO_CSV_ROW_H_INCLUDE