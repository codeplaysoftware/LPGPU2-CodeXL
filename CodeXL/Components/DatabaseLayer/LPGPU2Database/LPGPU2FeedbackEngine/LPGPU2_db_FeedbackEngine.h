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
#ifndef LPGPU2_DB_FEEDBACKENGINE_H_INCLUDE
#define LPGPU2_DB_FEEDBACKENGINE_H_INCLUDE

// Local:
#include <LPGPU2Database/LPGPU2FeedbackEngine/LPGPU2_db_FeedbackEngineAPIDefs.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>

#include <functional>

#define PP_STR_FeedbackEngineScriptsFolder "LPGPU2_scripts"
#define PP_STR_FeedbackEngineScriptsFolder_W L"LPGPU2_scripts"
#define PP_STR_FeedbackEngineScriptsExt_W L"*.lua"

class osFilePath;

namespace lpgpu2 {
namespace db {

// Structure to return the available feedback options.
struct AvailableFeedback
{
    gtString scriptName;         // Name to display in UI
    gtString scriptFunction;     // function name in the script to call
    gtString scriptCatagory;     // In the UI, we can group the scripts into categories e.g. OpenGL, Power, etc
    gtString inputName;          // Optional, may be empty. The script may need an input, e.g. "FPS Threshold"
    gtString inputDescription;   // Optional, may be empty. e.g. "Regions will contain frame with an FPS below this threshold"
    gtUInt32 inputDefault;       // Optional. Default value for the input
    gtUInt32 inputMin;           // Optional. Minimum value for the input
    gtUInt32 inputMax;           // Optional. Maximum value for the input
};

// Structure to configure the feedback engine run
struct FeedbackOption
{
    gtString          functionName;
    gtVector<gtInt32> functionArg;
};

/// @brief      Entry point for processing the Android Power Profiling capture
///             data. This class will analyze the current database using the
///             interface LPGPU2DatabaseAdapter for reading and writing.
///             
/// @warning    None.
/// @see        LPGPU2DatabaseAdapter
/// @author     Thales Sabino.
// clang-format off
class LPGPU2_FEEDBACKENGINE_API LPGPU2FeedbackEngine
{
    // Declarations:
public:
    using OutputMsgCallbackFunc = std::function<void(const std::string&)>;

    // Methods:
public:
    explicit LPGPU2FeedbackEngine(LPGPU2DatabaseAdapter &vDbAdapter);
    virtual ~LPGPU2FeedbackEngine();

    // Error handling.
    const gtString& GetErrorMessage() const;

    // Database processing
    status Run(const gtString &vScriptName, const gtVector<FeedbackOption> &vFeedbackOptions);    

    status GetAvailableFeedbackOptions(const osFilePath &vScriptFilename, gtVector <AvailableFeedback> &vFeedbackOptions) const;
    status GetFeedbackScriptVersion(const osFilePath &vScriptFileName, gtString &vScriptVersion) const;

    void SetOutputCallbackFunc(OutputMsgCallbackFunc vOutputCallbackFunc);

    static status GetAvailableFeedbackScripts(gtVector<gtString> &vAvailableFeedbackScripts);

    // Attributes:
private:
    LPGPU2DatabaseAdapter &m_dbAdapter;        //!< Reference to the database adapter used for interacting with the database.
    gtString m_errorMessage;                   //!< Error message resulting from any problems when processing the database.
    OutputMsgCallbackFunc m_outputCallFunc = nullptr; //!< Function that will be called when the feedback engine wants to print to CodeXL output window.
};
// clang-format on

} // namespace db
} // namespace lpgpu2

#endif // LPGPU2_DB_FEEDBACKENGINE_H_INCLUDE
