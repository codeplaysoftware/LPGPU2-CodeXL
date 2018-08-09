// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdFactory interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_FACTORY_H_INCLUDE
#define LPGPU2_CCVG_CMD_FACTORY_H_INCLUDE

// STL:
#include <map>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_SingletonBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdIDs.h>

namespace ccvg {
namespace cmd {

// Declarations:
class CCVGCmdParams;
class CCVGCmdBase;

/// @brief    CCVG command factory.
///           Singleton class.
/// @warning  None.
/// @see      cmd::Invoker, cmd::CmdIDs.
/// @date     19/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdFactory final
: public CCVGClassCmnBase
, public ISingleton<CCVGCmdFactory>
{
  friend ISingleton<CCVGCmdFactory>;

// Typedefs:
public:
  typedef CCVGCmdBase *(*CmdCreatorFnPtr)();

// Classes:
public:
  class IFactory
  {
  public:
    virtual CmdID           GetCmdID() const = 0;
    virtual CmdCreatorFnPtr GetCmdCreatorFn() const = 0;
    /* virtual CCVGCmdBase* CreateSelf() = 0 */ // Not possible as require a static creator
                                                // function in the command class, here for
                                                // awareness.
    /* dtor */ virtual ~IFactory() {}; // Objs are ref'ed and deleted by this base class
  };

// Methods:
public:
  status CmdRegister(CmdID vkCmdID, CmdCreatorFnPtr vCmdCreateFn, const gtString &vrCmdName);
  status CmdCreate(CmdID vkCmdID, CCVGCmdBase *&vpCmd);

// Overridden:
public:
  virtual ~CCVGCmdFactory() override;
  // From ISingleton
  virtual status Initialise() override;
  virtual status Shutdown() override;

// Typedefs:
private:
  using MapCmdIdToCmdCreatorFn_t = std::map<CmdID, CmdCreatorFnPtr>;
  using MapPairCmdIdToCmdCreatorFn_t = std::pair<CmdID, CmdCreatorFnPtr>;
  
// Methods:
private:
  // Singleton enforcement
  CCVGCmdFactory();
  CCVGCmdFactory(const CCVGCmdFactory&) = delete;
  CCVGCmdFactory(CCVGCmdFactory&&) = delete;
  CCVGCmdFactory& operator= (const CCVGCmdFactory&) = delete;
  CCVGCmdFactory& operator= (CCVGCmdFactory&&) = delete;
  // 
  bool IsValidCmdID(CmdID vkCmdID) const;
  bool HaveCmdIDAlreadyRegistered(CmdID vkCmdID) const;

// Attributes:
private:
  MapCmdIdToCmdCreatorFn_t m_mapCmdIdToCmdCreatorfn;
};
// clang-format on

} // namespace cmd
} //  namespace ccvg

#endif // LPGPU2_CCVG_CMD_FACTORY_H_INCLUDE