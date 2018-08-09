// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// Each and every Qt widget takes a unique string text identifier. This file
/// contains all CCVG's widget indentifiers. Identifiers are not the same as
/// as user facing text such as a button's text though they can be shared. 
/// Generally for user facing text should be found in and retrieve from
/// Package CCVGResourcesString.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

namespace ccvg {
namespace gui {

// History management tags. These tags are used for widgets and actions that
// support history management. afBrowseAction + afLineEdit.
const char *g_arrayWgtHistMgrTag[] =
{
  "CCVGCoverageResultsFilesDirPath",      // 0
  "CCVGCoverageSourceFilesDirPath",       // 1
  "CCVGCoverageDriverFilesDirPath",       // 2
  "CCVGCoverageExecuteableFilesDirPath",  // 3
};

} // namespace gui
} // namespace ccvg 
