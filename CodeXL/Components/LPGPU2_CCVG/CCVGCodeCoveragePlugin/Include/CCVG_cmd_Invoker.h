// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdInvoker interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_INVOKER_H_INCLUDE
#define LPGPU2_CCVG_CMD_INVOKER_H_INCLUDE

// STL:
#include <stack>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_SingletonBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Params.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdIDs.h>

namespace ccvg {
namespace cmd {

// Declarations:
class CCVGCmdBase;
class CCVGCmdFactory;

/// @brief    CCVG command Invoker. Singleton class.
///
/// The Invoker implements the command pattern design. There are two main tasks
/// for a command; action command Execute(), followed by the command's Undo() 
/// function. When a command finishes is Execute() function it returns to the 
/// Invoker.
/// Should the user, another command or script wish to undo a command the 
/// Invoker calls the command's Undo() function. The Invoker takes ownership of
/// the commands created which means it is the object to delete them when a 
/// command is no longer required.
/// The Invoker executes command in the main thread. Commands are not run in 
/// another thread.
/// Example usage:
///   CCVGCmdInvoker &rCmdInvoker = CCVGCmdInvoker::Instance();
///   CCVGCmdParams *pCmdParams = nullptr;
///   status = rCmdInvoker.CmdGetParams(CmdID::kProjectNew, pCmdParams);
///   CCVGCmdParameter *pParamProjName = pCmdParams->GetFirst();
///   pParamProjName->param.Set<gtString>(L"CCVG First Project");
///   status = rCmdInvoker.CmdExecute();
///   status = rCmdInvoker.CmdUndo();
/// Sub commands are commands executed (and undone) from another command.
/// sub commands are not put on the undo stack. They are kept by the parent
/// command for when undone is required. When undone, passed to the Invoker,
/// they are not deleted. The parent responsibility to delete its sub commands.
/// Sub commands are created by the Invoker and the parent get the sub command
/// pointer to store internally to the parent command.
///
/// @warning  None.
/// @see      None.
/// @date     17/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGCmdInvoker final
: public CCVGClassCmnBase
, public ISingleton<CCVGCmdInvoker>
{
  friend ISingleton<CCVGCmdInvoker>;

// Classes:
public:
  class ICmd
  {
  public:
    virtual status          Initialise() = 0;
    virtual status          Shutdown() = 0;
    virtual status          Execute() = 0;
    virtual status          Undo() = 0;
    virtual bool            GetCmdFinishedTaskFlag() const = 0;
    virtual const gtString& GetCmdName() const = 0;
    virtual void            SetExecuteDone() = 0;
    virtual bool            GetExecuteDone() const = 0;
    virtual bool            GetIsUndoableFlag() const = 0;

    /* dtor */ virtual ~ICmd() {}; // Objs are ref'ed and deleted by this base class
  };

// Methods:
public:
  status    CmdSetDisableUndoFunctionality();
  status    CmdInit(CmdID vkCmdID);
  status    CmdGetParams(CmdID vkCmdID, CCVGCmdParams *&vpParams);
  status    CmdExecute();
  status    CmdUndo();
  status    CmdUndoSubCmd(CCVGCmdBase &vSubCmd);
  gtString  CmdGetNameAboutToExecute() const;
  status    CmdHandleError(status vFnStatus, bool vbExceptionCaught, const gtString &vrCmdName);
  //
  bool HaveCommandsToUndo() const;
  //
  template<class T>
  bool CmdGetSubCmd(T *&vpSubCmd);

// Overridden:
public:
  virtual ~CCVGCmdInvoker() override;
  // From ISingleton
  virtual status Initialise() override;
  virtual status Shutdown() override;

// Typedefs:
private:
  using StackCmdObjs_t = std::stack<CCVGCmdBase *>;

// Methods:
private:
  // Singleton enforcement
  CCVGCmdInvoker();
  CCVGCmdInvoker(const CCVGCmdInvoker&) = delete;
  CCVGCmdInvoker(CCVGCmdInvoker&&) = delete;
  CCVGCmdInvoker& operator= (const CCVGCmdInvoker&) = delete;
  CCVGCmdInvoker& operator= (CCVGCmdInvoker&&) = delete;
  //
  status ClearCmds();
  status LogCommand(const CCVGCmdBase &vCmd);
  status CmdUndoPrivate(CCVGCmdBase &vSubCmd);
  
// Attributes:
private:
  CCVGCmdFactory *m_pCmdFactory;
  CCVGCmdBase    *m_pCmdCurrent;
  CCVGCmdBase    *m_pCmdSubOfAnotherCmd;
  StackCmdObjs_t  m_stackCmdObjsUndo;   // Stack of commands that have been executed now awaiting undo
  bool            m_bHaveCmdsToUndo;    // True = Yes command stack has at least one command, false = stack is empty
  bool            m_bCmdUndoDisabled;   // True = No command undo fns are called, false = enabled
  bool            m_bCmdUndoOperating;  // True = At least 1 cmd has executed, false = no command executed yet
  const gtString  m_constTxtNoCurrentCmdATM;
};
// clang-format on

/// @brief      Retrieve current sub command (of another command).
/// @param[in]  T The command class.
/// @param[out] vpSubCmd The current sub command, NULL = no current sub command.
/// @return     bool True = a command, false = Invalid do not use.
template<class T>
bool CCVGCmdInvoker::CmdGetSubCmd(T *&vpSubCmd)
{
  if (m_pCmdSubOfAnotherCmd != nullptr)
  {
    vpSubCmd = static_cast<T *>(m_pCmdSubOfAnotherCmd);
    return success;
  }
  else
  {
    vpSubCmd = nullptr;
    return failure;
  }
}

} // namespace cmd
} // namespace ccvg

#endif // LPGPU2_CCVG_CMD_INVOKER_H_INCLUDE