// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// Global utility or helper functions.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_Functions.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

/// @brief    Create CCVG version text string. The string contains the CCVG's
///           long name followed by the CCVG version.
/// @warning  None.
/// @see      None.
/// @date     09/12/2016.
/// @author   Illya Rudkin.
/// @return   gtString Version string.
gtString CCVGUtilGetVersionText()
{
  const wchar_t *pCCVGName = CCVGRSRC(kIDS_CCVG_STR_AppNameLong);
  const wchar_t *pCCVGVersion = nullptr;
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
  pCCVGVersion = CCVGRSRC(kIDS_CCVG_STR_VersionDescriptionDebug);
#else 
  pCCVGVersion = CCVGRSRC(kIDS_CCVG_STR_VersionDescription);
#endif // AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

  gtString text;
  text.appendFormattedString(L"%ls %ls", pCCVGName, pCCVGVersion);

  return text;
}
