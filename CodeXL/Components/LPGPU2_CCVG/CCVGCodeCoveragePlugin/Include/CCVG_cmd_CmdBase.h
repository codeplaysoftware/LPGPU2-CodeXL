// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdBase interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDBASE_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDBASE_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Invoker.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Factory.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Params.h>

namespace ccvg {
namespace cmd {

/// @brief    CCVG commands' base class.
///
/// CCVG commands derive from *this base class. The Command Factory creates
/// command objects and passes them to the Commmand Invoker. The Invoker takes
/// ownership of the command successfully created and initialised. If
/// initialisation fails the command is deleted. Only the Invoker deletes the
/// command when the command has been finished with. Commands do not delete
/// themselves.
/// Commands carry out work in the main thread, they do not run in their own
/// thread.
/// In order for command classes to be operational they must be registered with
/// the Command Factory before hand.
/// Example usage:
/// CCVGCmdInvoker &rCmdInvoker = CCVGCmdInvoker::Instance();
/// status = rCmdInvoker.Initialise(); // Only required once
/// CVGCmdParams *pCmdParams = nullptr;
/// status = rCmdInvoker.CmdGetParams(CmdID::kProjectNew, pCmdParams);
/// CCVGCmdParameter *pParamProjName = pCmdParams->GetFirst();
/// pParamProjName->param.Set<gtString>(L"CCVG First Project");
/// status = rCmdInvoker.CmdExecute();
/// status = rCmdInvoker.CmdUndo();
/// status = rCmdInvoker.Shutdown(); // Only required once
///
/// @warning  None.
/// @see      cmd::Factory, cmd::Invoker, cmd::Params, cmd::CmdID, 
///           cmd::RegisterAll().
/// @date     19/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdBase
: public CCVGCmdFactory::IFactory
, public CCVGCmdInvoker::ICmd
, public CCVGClassCmnBase
{
// Methods:
public:
  CCVGCmdBase();
  void         SetParent(CCVGCmdBase &vCmdParent);
  CCVGCmdBase* GetParent() const;

// Overrideable:
public:
  virtual bool           GetHasParams() const = 0;
  virtual CCVGCmdParams& GetParams() = 0;

// Overridden:
public:
  virtual ~CCVGCmdBase() override;
  // From CCVGCmdFactory
  virtual CmdID GetCmdID() const override;
  // From CCVGCmdInvoker
  virtual status          Undo() override;
  virtual bool            GetCmdFinishedTaskFlag() const override;
  virtual const gtString& GetCmdName() const override;
  virtual void            SetExecuteDone() override;
  virtual bool            GetExecuteDone() const override;

// Attributes:
protected:
  // *this object's state data
  CCVGuint  m_refCount;
  bool      m_bBeenInitialised;  // True = yes this has been initialised, false = not yet
  bool      m_bBeenShutdown;     // True = yes this has been shutdown already, false = not yet
  //
  CmdID                           m_cmdID;
  gtString                        m_cmdName;
  CCVGCmdFactory::CmdCreatorFnPtr m_pSelfCreatorFn;
  CCVGCmdParams                   m_cmdParamsDummy;
  bool                            m_bIsExecuteDone; // True = Invoker called *this cmd's Execute(), false = not yet
  bool                            m_bFinishedTask;  // True = Yes either Execute() or Undo() complete, false = not finished/started
  CCVGCmdBase                    *m_pCmdParent;     // Some commands can be called by other commands
};
// clang-format on

} // namespace cmd
} //  namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDBASE_H_INCLUDE