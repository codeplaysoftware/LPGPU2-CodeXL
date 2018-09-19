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
  lib = dlopen(path, RTLD_NOW);
#else
  lib = dlopen(path, RTLD_LOCAL | RTLD_LAZY);
#endif

  if(lib)
    Log(LogVerbose, "Loaded library :%s\n", path);
  else
    Log(LogVerbose, "Failed to load library :%s\n", path);

  return lib;
}

void* LookupSymbol(void* library, const char* name)
{
  void* rv = NULL;

#if !defined(INTERPOSER_EXPLICITLY_LINKED_FROM_APP)

  // try RTLD_NEXT first; This loads from the next library in the link order
  // (ie it doesnt search libraries before this one in the link order)
  Log(LogVerbose, "Resolving symbol RTLD_NEXT:%s\n", name);
  rv = dlsym(RTLD_NEXT, name);

  if ( rv )
  {
    // typedef struct {
    //   const char *dli_fname;  /* Pathname of shared object that
    //                              contains address */
    //   void       *dli_fbase;  /* Base address at which shared
    //                              object is loaded */
    //   const char *dli_sname;  /* Name of symbol whose definition
    //                              overlaps addr */
    //   void       *dli_saddr;  /* Exact address of symbol named
    //                              in dli_sname */
    // } Dl_info;
    Dl_info info;
    Log(LogVerbose, "Resolving symbol address\n");
    int err = dladdr(rv, &info);
    if(0 != err)
    {
      Log(LogVerbose, "Resolved RTLD_NEXT :%s (%s)\n", name, info.dli_fname);
    }
    else
    {
      Log(LogError, "Resolved RTLD_NEXT :%s. Cannot get resolution info.\n", name);
    }

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
  else
  {
    if( library )
    {
      Log(LogVerbose, "  dlsym dlerror :%s (%s)\n", dlerror(), name);
    }
  }

  // otherwise use eglGetProcAddress
  if(!original_eglGetProcAddress)
  {
    Log(LogError, "eglGetProcAddress is missing. Some functionality may fail.");
  }
  else
  {
    rv = (void*)original_eglGetProcAddress(name);
  }

  if ( rv )
  {
    Log(LogVerbose, "Resolved from eglGetProcAddress :%s\n", name);
    return ( rv );
  }

  if(!rv)
  {
    Log(LogVerbose, "Unresolved :%s\n", name);
  }

  return rv;
}
