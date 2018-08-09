// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2InputOutput project is a collection of classes to allow
///         data to be imported/exported to/from CodeXL. The goal is to
///         have an independent library that can be used across all projects
///         that need access to external data.
///
/// CSVFileBase interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_IO_CSV_FILE_BASE_H_INCLUDE
#define LPGPU2_IO_CSV_FILE_BASE_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>

// STL: for std::shared_ptr
#include <memory>

// Local:
#include <LPGPU2InputOutput/Include/LPGPU2_io_FnStatus.h>

namespace lpgpu2 {
namespace io {

using namespace fnstatus;

// Declarations:
class CSVRow;

/// @brief   This is the base class for CSV file manipulation. It contains the
///          definition of a CSV file as well as the data itself. This is a base
///          class that CSV readers/writers must inherit.
/// @warning This class should not be used directly. Use of its child classes.
/// @date    12/09/2017
/// @author  Thales Sabino
// clang-format off
class CSVFileBase
{
// Methods:
public:

    const gtString& GetErrorMessage() const;

    status SetSeparator(wchar_t vSeparator);
    status SetCommentChar(wchar_t vCommentChar);    

// Methods:
protected:
    void SetErrorMessage(const gtString &vMessage);

// Attributes:
protected:
    wchar_t                             m_cSeparator = L',';
    wchar_t                             m_cCommentChar = L'#';
    gtVector<gtString>                  m_columns;
    gtVector<std::shared_ptr<CSVRow>>   m_rows;
    gtVector<gtString>                  m_headers;

    gtString                            m_errorMessage;
};
// clang-format on

} // namespace io
} // namespace lpgpu2

#endif // LPGPU2_IO_CSV_FILE_BASE_H_INCLUDE