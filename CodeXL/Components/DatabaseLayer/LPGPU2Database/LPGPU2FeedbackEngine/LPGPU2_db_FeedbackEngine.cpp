// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Feedback engine is an add on to LPGPU2Database Layer that
///         allows execution of custom operations on the database regarding
///         crunching of Android Profiling data.
///
/// Feedback Engine main API. This is the entry point for triggering database
/// post-processing operations.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <LPGPU2Database/LPGPU2FeedbackEngine/LPGPU2_db_FeedbackEngine.h>
#include <LPGPU2Database/LPGPU2FeedbackEngine/LPGPU2_db_FeedbackEnginePreProcessor.h>
#include <LPGPU2Database/LPGPU2FeedbackEngine/LPGPU2_db_FeedbackLuaBindings.h>

#include <AMDTOSWrappers/Include/osDirectory.h>

namespace lpgpu2 {
namespace db {

/// @brief   Class constructor, no work is done here apart from storing the
///          reference to LPGPU2DatabaseAdapter class.
/// @param[in] vDbAdapter 
LPGPU2FeedbackEngine::LPGPU2FeedbackEngine(LPGPU2DatabaseAdapter &vDbAdapter)
    : m_dbAdapter(vDbAdapter)
{
}

/// @brief   Class destructor, no work is done here for now.
LPGPU2FeedbackEngine::~LPGPU2FeedbackEngine()
{
}

/// @brief  Returns the current error message, is any.
/// @returns gtString   The current error message.
const gtString& LPGPU2FeedbackEngine::GetErrorMessage() const
{
    return m_errorMessage;
}


/// @brief              Triggers the processing of the database to generate the
///                     feedback result to be displayed to the user.
/// @returns status     success = all the processing was completed successfully,
///                     failure = an error has occurred during the process of
///                     the database.
status LPGPU2FeedbackEngine::Run(const gtString &vScriptName, const gtVector<FeedbackOption> &vFeedbackOptions)
{
    auto bReturn = failure;

    // Trigger the processing of the database here
    // store any error in m_errorMessage
    // return success or failure depending on the result

    // Generates extra data for the analysis
    LPGPU2FeedbackEnginePreProcessor feedbackPreProcessor{ m_dbAdapter };
    bReturn = feedbackPreProcessor.Run();

    if (bReturn == failure)
    {
        m_errorMessage = feedbackPreProcessor.GetErrorMessage();
    }

    osFilePath scriptFullPath{ vScriptName };

    if (scriptFullPath.exists())
    {
        // Delete previous feedback from the database
        bReturn = m_dbAdapter.DeleteFeedbacks();        

        if (bReturn == success)
        {
            LuaBinding lua{ m_dbAdapter };
            lua.SetOutputCallbackFunc(m_outputCallFunc);

            // Run the script for each feedback option.
            for (const auto& feedbackConfig : vFeedbackOptions)
            {
                // Needs a copy the data here in order to call RunScript
                const std::string functionName = feedbackConfig.functionName.asASCIICharArray();
                auto functionArgs = feedbackConfig.functionArg;

                lua.RunScript(scriptFullPath.asString().asASCIICharArray(), functionName, functionArgs.size(), functionArgs.data());
            }            
        }        
    }

    return bReturn;
}

/// @brief                       Return all the available feedback options from the specified script.
///                              Each option is defined as a struct that contains the option id, display 
///                              name and default value, if any.
/// @param[in]  vScriptFilePath  The script filename as a string. This is the script name with the .lua extension.
///                              Note that the script must be a file inside osFilePath::OS_CODEXL_BINARIES_PATH.
/// @param[out] vFeedbackOptions The list of feedback options as returned by the script.
/// @return     status           success: The feedback option list was retrieved successfully from the script,
///                              failure: The script was not found or something wrong happened while retrieving
///                                       the options from the script.
status LPGPU2FeedbackEngine::GetAvailableFeedbackOptions(const osFilePath &vScriptFilePath, gtVector<AvailableFeedback> &vFeedbackOptions) const
{
    auto bReturn = failure;

    if (vScriptFilePath.exists())
    {
        // Generates extra data for the analysis
        LPGPU2FeedbackEnginePreProcessor feedbackPreProcessor{ m_dbAdapter };
        feedbackPreProcessor.Run();

        LuaBinding lua{ m_dbAdapter };
        lua.SetOutputCallbackFunc(m_outputCallFunc);
        bReturn = lua.GetAvailableFeedbackOptions(vScriptFilePath.asString(), vFeedbackOptions);
    }

    return bReturn;
}

/// @brief                       Return the script version defined by the Lua function getScriptVersion().
/// @param[in]  vScriptFilePath  The path of the script to get the version from.
/// @param[out] vScriptVersion   The gtString where the version will be written.
/// @return     status           success: The version string was read from the specified script,
///                              failure: An error has occurred while reading the script version.
status LPGPU2FeedbackEngine::GetFeedbackScriptVersion(const osFilePath &vScriptFilePath, gtString &vScriptVersion) const
{
    auto bReturn = failure;

    if (vScriptFilePath.exists())
    {
        LuaBinding lua{ m_dbAdapter };
        lua.SetOutputCallbackFunc(m_outputCallFunc);
        
        lua.GetFeedbackScriptVersion(vScriptFilePath.asString(), vScriptVersion);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                      Sets a callback function in the feedback script that will be called
///                             for every output made by the engine. This can be used to redirect the 
///                             output to a log file or to be displayed on the GUI.
/// @param[in] vOutputCallback  The output callback function to be called by the feedback script.
void LPGPU2FeedbackEngine::SetOutputCallbackFunc(OutputMsgCallbackFunc vOutputCallbackFunc)
{
    m_outputCallFunc = vOutputCallbackFunc;
}

/// @brief List the available feedback scripts in the folder LPGPU2_scripts.
/// @param[out] vAvailableFeedbackScripts The list of available scripts.
/// @returns status     success = at least one script was found,
///                     failure = no available scripts.
status LPGPU2FeedbackEngine::GetAvailableFeedbackScripts(gtVector<gtString> &vAvailableFeedbackScripts)
{
    auto bReturn = failure;

    osFilePath scriptsFolderPath{ osFilePath::OS_CODEXL_BINARIES_PATH };
    scriptsFolderPath.appendSubDirectory(PP_STR_FeedbackEngineScriptsFolder_W);
    if (scriptsFolderPath.exists())
    {
        osDirectory scriptsDir{ scriptsFolderPath };

        gtList<osFilePath> availableScripts;
        const auto bCleanOutput = false;
        scriptsDir.getContainedFilePaths(PP_STR_FeedbackEngineScriptsExt_W, osDirectory::SORT_BY_NAME_ASCENDING, availableScripts, bCleanOutput);

        for (const auto &scriptFilePath : availableScripts)
        {
            gtString scriptFileNameAndExtension;
            scriptFilePath.getFileNameAndExtension(scriptFileNameAndExtension);

            vAvailableFeedbackScripts.push_back(scriptFileNameAndExtension);
            bReturn = success;
        }
    }

    return bReturn;
}

} // namespace db
} // namespace lpgpu2
