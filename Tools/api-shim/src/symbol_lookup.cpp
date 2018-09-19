/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include <dlfcn.h>

#include "logger.h"

void* OpenLibrary(const char*path)
{
  void* lib = NULL;

#if defined(INTERPOSER_EXPLICITLY_LINKED_FROM_APP)
  // RTLD_NOW; resolve all symbols in the shared object before dlopen returns
  lib = dlopen(path, RTLD_NOW);
#else
  // RTLD_LOCAL; symbols of the shared library are not available for subsequently loaded library
  // RTLD_LAZY; resolve symbols as code that references is executed
  lib = dlopen(path, RTLD_LOCAL | RTLD_LAZY);
#endif

  if(!lib)
    Log(LogError, "Failed to load library :%s\n", path);

  return lib;
}

void* LookupSymbol(void* library, const char* name)
{
  void* rv = NULL;

  Log(LogVerbose, "Resolving symbol:%s\n", name);

#if !defined(INTERPOSER_EXPLICITLY_LINKED_FROM_APP)
  // try RTLD_NEXT first; This loads from the next library in the link order
  // (ie it doesnt search libraries before this one in the link order)
  rv = dlsym(RTLD_NEXT, name);
  if ( rv )
  {
    Log(LogVerbose, "Resolved RTLD_NEXT :%s\n", name);
    return ( rv );
  }
#endif

  // otherwise use the given library
  rv = dlsym(library, name);

  if ( rv )
  {
    Log(LogVerbose, "Resolved from library :%s\n", name);
    return ( rv );
  }

  if(!rv)
  {
    Log(LogError, "Unresolved :%s\n", name);
  }

  return rv;
}
