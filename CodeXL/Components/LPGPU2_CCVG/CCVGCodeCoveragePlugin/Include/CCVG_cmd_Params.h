// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdParams interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDPARAMS_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDPARAMS_H_INCLUDE

// STL:
#include <list>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Parameter.h>

namespace ccvg {
namespace cmd {

/// @brief    CCVG command parameters container. Hold zero to many
///           parameters. Should a command have parameters the client
///           asks the command for *this container. The client then
///           iterates the parameters contained within assigning or
///           reading data for each.
/// @warning  None.
/// @see      cmd::Invoker, cmd::Parameters.
/// @date     18/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdParams final
{
// Typedefs:
private:
  using ListCmdParam_t = std::list<CCVGCmdParameter *>;
  
// Methods:
public:
  CCVGCmdParams();
  CCVGCmdParams(ListCmdParam_t &vParameters);
  ~CCVGCmdParams();
  //
  CCVGCmdParameter* GetFirst();
  CCVGCmdParameter* GetNext();
  CCVGuint          GetCount() const;
  CCVGCmdParameter* GetParam(CCVGuint vParamID) const;
  bool              IsEmpty() const;

// Attributes:
private:
  ListCmdParam_t          *m_pListCmdParameters;
  ListCmdParam_t::iterator m_listIterator;
};
// clang-format on

} // namespace cmd
} //  namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDPARAMS_H_INCLUDE