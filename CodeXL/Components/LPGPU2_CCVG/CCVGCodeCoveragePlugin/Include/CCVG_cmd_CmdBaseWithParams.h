// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdBaseWithParams interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDBASEWITHPARAMS_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDBASEWITHPARAMS_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBase.h>

namespace ccvg {
namespace cmd {

/// @brief    CCVG commands' base class with parameters. Derived from 
///           CCVGCmdBase. CCVG commands with parameters can be derived from
///           *this base class. 
/// @warning  None.
/// @see      cmd::CmdBase.
/// @date     07/09/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdBaseWithParams 
: public CCVGCmdBase
{
// Methods:
public:
  CCVGCmdBaseWithParams();

// Overridden:
public:
  virtual ~CCVGCmdBaseWithParams() override;
  // From CCVGCmdBase
  virtual bool           GetHasParams() const override;
  virtual CCVGCmdParams& GetParams() override;

// Typedefs:
protected:
  using ListCmdParam_t = std::list<CCVGCmdParameter *>;

// Attributes:
protected:
  ListCmdParam_t   m_listCmdParameters;
  CCVGCmdParams    m_cmdParams;
};
// clang-format on

} // namespace cmd
} //  namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDBASEWITHPARAMS_H_INCLUDE