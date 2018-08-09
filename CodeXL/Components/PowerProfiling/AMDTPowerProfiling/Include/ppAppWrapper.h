//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppAppWrapper.h
///
//==================================================================================

//------------------------------ ppAppWrapper.h ------------------------------

#ifndef __PPAPPWRAPPER_H
#define __PPAPPWRAPPER_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
//++AT:LPGPU2
#include <AMDTPowerProfilingMidTier/include/LPGPU2_AndroidRemoteProtocolBuildDefinitions.h>
//--AT:LPGPU2

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/ppMDIViewCreator.h>

// Forward declarations:
//++KB: LPGPU2
//++AT:LPGPU2
#if LPGPU2_ANDROID_REMOTE_PROTOCOL_COMPILE_IN == 1
class ppProjectLPGPU2SettingsExtension;
#else
class ppProjectSettingsExtension;
#endif
//--AT:LPGPU2
//--KB: LPGPU2
class ppMDIViewCreator;
class ppMenuActionsExecutor;

class PP_API ppAppWrapper
{
public:

    static ppAppWrapper& instance();
    int CheckValidity(gtString& errString);
    void initialize();
    void initializeIndependentWidgets();

    ppMDIViewCreator* MDIViewCreator() const { return m_psMDIViewCreator; };


    // marks if the prerequisite of this plugin were met needs access
    static bool s_loadEnabled;

protected:

    // Do not allow the use of my default constructor:
    ppAppWrapper();

    static ppAppWrapper* m_spMySingleInstance;

//++KB: LPGPU2
//++AT:LPGPU2
    //// Contains the project settings extension
#if LPGPU2_ANDROID_REMOTE_PROTOCOL_COMPILE_IN == 1
    static ppProjectLPGPU2SettingsExtension* m_psProjectLPGPU2SettingsExtension;
#else
    static ppProjectSettingsExtension* m_psProjectSettingsExtension;
#endif
//--AT:LPGPU2
//--KB: LPGPU2

    // Contains the views creator
    static ppMDIViewCreator* m_psMDIViewCreator;

    // Contains the main menu actions creator
    static ppMenuActionsExecutor* m_spActionsCreator;
};

extern "C"
{
    // check validity of the plugin:
    int PP_API CheckValidity(gtString& errString);

    // initialize function for this wrapper:
    void PP_API initialize();

    // Initialize other items after main window creation:
    void PP_API initializeIndependentWidgets();
};


#endif //__PPAPPWRAPPER_H

