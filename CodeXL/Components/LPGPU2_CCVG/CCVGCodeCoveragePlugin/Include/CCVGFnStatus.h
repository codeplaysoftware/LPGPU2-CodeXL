// Copyright (C) 2002-2017 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief General Code Coverage class. Functional status and error handling
///        for all objects and functions in *this project.
///
/// There is generally no exception handling code in *this project. All functions
/// unless very simple return a function status which is either 'success' or 
/// or 'failure'. On failure the calling code, from the lowest level back up
/// to the API level, can and should query the object for its error 
/// description(s).
/// The classes and definition in this file are meant to be used in other
/// memory manager of this ilk.
///
/// ccvg::fnstatus::status typedef
///
/// @copyright
/// Copyright (C) 2002-2017 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FNSTATUS_H_INCLUDE
#define LPGPU2_CCVG_FNSTATUS_H_INCLUDE

namespace ccvg {

/// @brief  Function operational status. Functions return 'success' or 
///         'failure' and the task they carry out.
/// @warning None.
/// @see     None.
/// @req_id  None
/// @author  Illya Rudkin.
namespace fnstatus {
  const bool success = true;
  const bool failure = false;
  using status = bool;
} // namespace status

} // namespace ccvg

#endif // LPGPU2_CCVG_FNSTATUS_H_INCLUDE
