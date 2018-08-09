// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2InputOutput project is a collection of classes to allow
///         data to be imported/exported to/from CodeXL. The goal is to
///         have an independent library that can be used across all projects
///         that need access to external data.
///
/// CSVFileBase implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <LPGPU2InputOutput/Include/LPGPU2_io_CSVFileBase.h>

namespace lpgpu2 {
namespace io {

/// @brief  Returns the error message in case any method call fails.
/// @return gtString    The error message of the last unsuccessful call.
const gtString& CSVFileBase::GetErrorMessage() const
{
    return m_errorMessage;
}

/// @brief  Sets vSeparator as the new separator to be written to the actual files.
/// @param[in] vSeparator   New character to be used as CSV separator.
/// @return    status       Always returns success.
status CSVFileBase::SetSeparator(wchar_t vSeparator)
{
    m_cSeparator = vSeparator;
    return success;
}

/// @brief Sets vCommentChar as the new comment character to be written to the
///        actual files.
/// @param[in]  vCommentChar  New character to be used as CSV comments
/// @return     status        Always returns success.   
status CSVFileBase::SetCommentChar(wchar_t vCommentChar)
{
    m_cCommentChar = vCommentChar;
    return success;
}

/// @brief                  Sets vMessage as the new error message for this class.
/// @param[in]  vMessage    The message to set as en error.
void CSVFileBase::SetErrorMessage(const gtString &vMessage)
{
    if (!vMessage.isEmpty())
    {
        m_errorMessage = vMessage;
    }    
}

} // namespace io
} // namespace lpgpu2