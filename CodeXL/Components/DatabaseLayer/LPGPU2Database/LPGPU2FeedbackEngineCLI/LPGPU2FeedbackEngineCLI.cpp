// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  This is an command line application used to trigger the SAMS
///         feedback engine for generating feedback information based on
///         Android Power Profiling data collected using CodeXL.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIAssertionFailureHandler.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Boost:
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
//#include <boost/property_tree/xml_parser.hpp>

// STL:
#include <iostream>

// LPGPU2 Database Layer
#include <LPGPU2Database/LPGPU2FeedbackEngine/LPGPU2_db_FeedbackEngine.h>

// For the assertion handler
#include <LPGPU2Database/LPGPU2DatabaseCLI/AssertionHandler.h>

// Initializes the Logger or this will crash on Linux
static void InitLogger(osDebugLogSeverity initialiSeverity)
{
    osDebugLog& theDebugLog = osDebugLog::instance();

    theDebugLog.initialize(L"LPGPU2FeedbackEngineCLI.log");
    theDebugLog.setLoggedSeverity(initialiSeverity);
}

int main(int argc, char* argv[])
{
    // Initialises the CodeXL logger. It is optional on Windows but required on Linux machines.
    InitLogger(OS_DEBUG_LOG_ERROR);

    std::string inputDatabaseFilePath;
    std::string inputFeedbackScriptFilePath;
    bool bGetFeedbackOptions = false;
    std::string inputOptionsFile;

    boost::program_options::variables_map optionsMap;
    boost::program_options::options_description options("Allowed options");

    options.add_options()
        ("help", "Shows this help message")
        ("database",
         boost::program_options::value<std::string>(&inputDatabaseFilePath)->value_name("DBFilePath")->required(),
         "SQLite database file path")
         ("script",
          boost::program_options::value<std::string>(&inputFeedbackScriptFilePath)->value_name("FeedbackScript"),
          "Feedback script to be used")
          ("get-options",
           boost::program_options::bool_switch(&bGetFeedbackOptions)->default_value(false),
           "Use this to get the available feedback options from the script")
           ("options-file",
            boost::program_options::value<std::string>(&inputOptionsFile)->value_name("OptionsFile"),
            "File to output the script options or to read the options to run the script. Use --get-options to extract the available options");

    try
    {
        auto parsedOptions = boost::program_options::parse_command_line(argc, argv, options);
        boost::program_options::store(parsedOptions, optionsMap);
        boost::program_options::notify(optionsMap);
    }
    catch (const std::exception& exception)
    {
        std::cerr << "ERROR: " << exception.what() << std::endl;
        std::cout << options << std::endl;
        return EXIT_FAILURE;
    }

    if (optionsMap.count("help") || argc == 1)
    {
        std::cout << options << std::endl;
        return EXIT_SUCCESS;
    }

    osFilePath databaseFilePath;
    gtString databaseFilePathStr;
    databaseFilePathStr.fromUtf8String(inputDatabaseFilePath);
    databaseFilePath.setFullPathFromString(databaseFilePathStr);

    osFilePath scriptFilePath;
    gtString scriptFilePathStr;
    scriptFilePathStr.fromUtf8String(inputFeedbackScriptFilePath);
    scriptFilePath.setFullPathFromString(scriptFilePathStr);

    if (!databaseFilePath.exists())
    {
        std::cerr << "ERROR: File not found: " << databaseFilePathStr.asASCIICharArray() << std::endl;
        return EXIT_FAILURE;
    }

    if (!scriptFilePath.exists())
    {
        std::cerr << "ERROR: File not found: " << scriptFilePathStr.asASCIICharArray() << std::endl;
        return EXIT_FAILURE;
    }

    // Defines the actual assertion handler that is going to be registered
    AssertionHandler theAssertionHandler;

    // Register our custom assertion handler for assertions caught with calls to GT_IF_WITH_ASSERT_EX and GT_ASSERT_EX
    gtRegisterAssertionFailureHandler(&theAssertionHandler);

    // Create the database adapter
    lpgpu2::db::LPGPU2DatabaseAdapter dbAdapter;

    // The initialise call is used to handle errors
    if (!dbAdapter.Initialise())
    {
        std::cerr << "ERROR: Cannot initialise database adapter" << std::endl;
        std::cerr << dbAdapter.GetErrorMessage().asASCIICharArray() << std::endl;
        return EXIT_FAILURE;
    }

    // Opens the database file
    const bool bReadOnly = false;
    if (!dbAdapter.OpenDb(databaseFilePathStr, AMDT_PROFILE_MODE_TIMELINE, bReadOnly))
    {
        std::cerr << "ERROR: Cannot open the database file" << std::endl;
        std::cerr << dbAdapter.GetErrorMessage().asASCIICharArray() << std::endl;
        return EXIT_FAILURE;
    }

    if (bGetFeedbackOptions)
    {
        lpgpu2::db::LPGPU2FeedbackEngine feedbackEngine{ dbAdapter };

        gtVector<lpgpu2::db::AvailableFeedback> availableFeedbacks;
        GT_IF_WITH_ASSERT_EX(feedbackEngine.GetAvailableFeedbackOptions(scriptFilePathStr, availableFeedbacks) == lpgpu2::db::success, feedbackEngine.GetErrorMessage().asCharArray())
        {
            boost::property_tree::ptree tree;

            for (const auto& feedback : availableFeedbacks)
            {
                boost::property_tree::ptree c;
                
                c.put("scriptName", feedback.scriptName.asASCIICharArray());
                c.put("scriptFunction", feedback.scriptFunction.asASCIICharArray());
                c.put("scriptCategory", feedback.scriptCatagory.asASCIICharArray());
                c.put("inputName", feedback.inputName.asASCIICharArray());
                c.put("inputDescription", feedback.inputDescription.asASCIICharArray());
                c.put("inputDefault", feedback.inputDefault);
                c.put("inputValue", feedback.inputDefault);
                c.put("run", false);

                tree.push_back({ "", c });
            }

            boost::property_tree::write_json(inputOptionsFile, tree);
        }

        return EXIT_SUCCESS;
    }

    osFilePath optionsFilePath;
    gtString optionsFilePathStr;
    optionsFilePathStr.fromUtf8String(inputOptionsFile);
    optionsFilePath.setFullPathFromString(optionsFilePathStr);

    if (!optionsFilePath.exists())
    {
        std::cerr << "ERROR: File not found: " << optionsFilePathStr.asASCIICharArray() << std::endl;
        return EXIT_FAILURE;
    }

    // Read the options file
    boost::property_tree::ptree tree;
    boost::property_tree::read_json(optionsFilePathStr.asASCIICharArray(), tree);

    gtVector<lpgpu2::db::FeedbackOption> feedbackOptions;

    for (const auto &inputOption : tree.get_child(""))
    {
        lpgpu2::db::FeedbackOption feedbackOption;
        const auto &option = inputOption.second;

        const auto bShouldRun = option.get<bool>("run");
        if (bShouldRun)
        {
            feedbackOption.functionName.fromASCIIString(option.get<std::string>("scriptFunction").data());
            const auto feedbackInputName = option.get<std::string>("inputName");
            if (!feedbackInputName.empty())
            {
                const auto inputValue = option.get<gtInt32>("inputValue");
                feedbackOption.functionArg = { inputValue };
            }

            feedbackOptions.push_back(feedbackOption);
        }
    }

    // Create the feedback engine with the database adapter
    lpgpu2::db::LPGPU2FeedbackEngine feedbackEngine{ dbAdapter };

    // Run the feedback engine on the database handled by the dbAdapter
    GT_ASSERT_EX(feedbackEngine.Run(scriptFilePathStr, feedbackOptions), feedbackEngine.GetErrorMessage().asCharArray());

    // Closes the database and checks for any errors. This is not required but good practice.
    GT_ASSERT_EX(dbAdapter.CloseDb(), dbAdapter.GetErrorMessage().asCharArray());

    // Unregister the assertion handler
    gtUnRegisterAssertionFailureHandler(&theAssertionHandler);

    return EXIT_SUCCESS;
}