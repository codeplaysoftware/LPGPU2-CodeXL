// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Database is an extension to the CodeXL's Database Layer to
///         allow customisation of the CodeXL power profiling database.It
///         has the API to read and write the tables used by the LPGPU2
///         consortium
///
/// Define return status for functions with the LPGPU2Database project.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_DB_FNSTATUS_H_INCLUDE
#define LPGPU2_DB_FNSTATUS_H_INCLUDE

/// @brief  Used to identify the return status of a function.
/// @see  DB stands for Database
/// @author  Thales Sabino
namespace lpgpu2 {
namespace db {

/// @brief  Function operational status. Functions return 'success' or
///         'failure' and the task they carry out.
/// @author Thales Sabino.
namespace fnstatus {

const bool success = true;
const bool failure = false;
using status = bool;

} // namespace fnstatus
} // namespace db
} // namespace lpgpu2

#endif // LPGPU2_DB_FNSTATUS_H_INCLUDE
