// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2InputOutput project is a collection of classes to allow
///         data to be imported/exported to/from CodeXL. The goal is to
///         have an independent library that can be used across all projects
///         that need access to external data.
///
/// Define return status for functions with the LPGPU2InputOutput project.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_IO_FNSTATUS_H_INCLUDE
#define LPGPU2_IO_FNSTATUS_H_INCLUDE

namespace lpgpu2 {
namespace io {

/// @brief  Function operational status. Functions return 'success' or 
///         'failure' and the task they carry out.
/// @warning None.
/// @see     None.
/// @req_id  None
/// @author  Thales Sabino.
namespace fnstatus {

const bool success = true;
const bool failure = false;
using status = bool;

} // namespace status
} // namespace io
} // namespace lpgpu2

#endif // LPGPU2_IO_FNSTATUS_H_INCLUDE
