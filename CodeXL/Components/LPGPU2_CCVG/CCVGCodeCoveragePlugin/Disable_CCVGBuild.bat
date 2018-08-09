@echo off

echo Disabling compilation of CCVGCodeCoveragePlugin
:: timeout /t 5

(
    
    echo // Copyright ^(C^) 2002-2018 Codeplay Software Limited. All Rights Reserved.
    echo.
    echo /// @file
    echo ///
    echo /// @brief CodeXL execution mode Code Coverage. LPGPU2 ^(== LP2^) Code Coverage 
    echo ///        DLL plugin to sit along side other DLL plugins ^(execution modes^)  
    echo ///        such as GPUProfilng or CPUProfiling.
    echo.
    echo /// Generated build definitions options. Enables/Disables the compilation of
    echo /// the CCVGCodeCoveragePlugin
    echo ///
    echo /// @copyright
    echo /// Copyright ^(C^) 2002-2018 Codeplay Software Limited. All Rights Reserved.
    echo.
    echo // #define LPGPU2_CCVG_COMPILE_IN

) > %~dp0Include/CCVGProjectBuildDefinitions_Generated.h