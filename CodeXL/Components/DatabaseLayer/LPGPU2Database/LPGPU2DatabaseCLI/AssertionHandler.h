// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  This project is a demo application that aims to show how to use 
///         the DatabaseLayer projects to read/write and extend the CodeXL profiling 
///         database format.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.
#ifndef LPGPU2_DB_ASSERTION_HANDLER_H_INCLUDE
#define LPGPU2_DB_ASSERTION_HANDLER_H_INCLUDE

// STL:
#include <iostream>

// AMDTBaseTools:
#include <AMDTBaseTools/Include/gtIAssertionFailureHandler.h>


/// @brief      Custom assertion handler to print assertion info on the console
/// @warning    None.
/// @see        gtIAssertionFailureHandler
/// @data       08/08/2017
/// @author     Thales Sabino
class AssertionHandler final : public gtIAssertionFailureHandler
{
public:
    // From gtIAssertionFailureHandler
    virtual void onAssertionFailure(const wchar_t* functionName, const wchar_t* fileName, int lineNumber, const wchar_t* message) override
    {
        std::wcerr << std::endl;
        std::wcerr << L"----------------------------------------------------" << std::endl;
        std::wcerr << L"Assertion Failure" << std::endl;
        std::wcerr << L"----------------------------------------------------" << std::endl;
        std::wcerr << L"File    : " << fileName << std::endl;
        std::wcerr << L"Function: " << functionName << std::endl;
        std::wcerr << L"Line    : " << lineNumber << std::endl;
        std::wcerr << L"Message : " << message << std::endl;
        std::wcerr << L"----------------------------------------------------" << std::endl;
        std::wcerr << std::endl;
    }
};

#endif // LPGPU2_DB_ASSERTION_HANDLER_H_INCLUDE
