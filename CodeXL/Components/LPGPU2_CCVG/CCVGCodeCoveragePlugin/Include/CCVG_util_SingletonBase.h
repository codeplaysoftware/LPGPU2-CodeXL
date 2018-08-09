// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGUtilSingletonBase interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_UTIL_SINGLETONBASE_H_INCLUDE
#define LPGPU2_CCVG_UTIL_SINGLETONBASE_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGFnStatus.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>

/// @brief    Base class for the singleton pattern.
///
/// Usage util::ISingleton base class:
///   class CCVGDerivedClass : public ISingleton<CCVGDerivedClass>
///   {
///     friend ISingleton<CCVGDerivedClass>;
///
///     // Overridden:
///     public:
///       // From util::ISingleton
///       status Initialise() override;
///       status Shutdown() override;
///   };
///
/// @warning  Derived class must specify util::ISingleton<> as a friend class.
/// @see      None.
/// @date     16/08/2016.
/// @author   Illya Rudkin.
// clang-format off
template<typename T> class ISingleton
{
// Statics:
public:
  /// @brief  Get the Package singletion instance.
  /// @return T& Reference to instance.
  static T& Instance()
  {
    // This will fail if the derived class has not declared itself to be a 
    // friend of util::ISingleton
    static T instance;
    return instance;
  }

// Methods:
public:
  /* ctor */
  ISingleton()
  : m_refCount(0)
  , m_bBeenInitialised(false)
  , m_bBeenShutdown(false)
  {}

// Overrideable:
public:
  virtual ccvg::fnstatus::status Initialise() = 0;
  virtual ccvg::fnstatus::status Shutdown() = 0;
  //
  /* dtor */ virtual ~ISingleton() {}

// Attributes:
protected:
  // *this object's state data
  CCVGuint  m_refCount;
  bool      m_bBeenInitialised;  // True = yes this has been initialised, false = not yet
  bool      m_bBeenShutdown;     // True = yes this has been shutdown already, false = not yet
};
// clang-format on

#endif // LPGPU2_CCVG_UTIL_SINGLETONBASE_H_INCLUDE