// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Feedback engine is an add on to LPGPU2Database Layer that
///         allows execution of custom operations on the database regarding
///         crunching of Android Profiling data.
///
/// Defines the class LPGPU2FeedbackEnginePreProcessor for including in the
/// database the data required for the actual feedback analysis.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.
#ifndef LPGPU2_DB_FEEDBACK_ENGINE_PRE_PROCESSOR_H_INCLUDE
#define LPGPU2_DB_FEEDBACK_ENGINE_PRE_PROCESSOR_H_INCLUDE

// LPGPU2Database:
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>

// STL:
#include <string>

namespace lpgpu2 {
namespace db {    

/// @brief      The FeedbackEngine pre processor is responsible for populating 
///             the database with any information required for the actual feedback
///             engine.
/// @warning    None.
/// @see        LPGPU2FeedbackEngine
/// @author     Thales Sabino.
// clang-format off
class LPGPU2FeedbackEnginePreProcessor
{
// Methods:
public:
    explicit LPGPU2FeedbackEnginePreProcessor(LPGPU2DatabaseAdapter &vDbAdapter);
    virtual ~LPGPU2FeedbackEnginePreProcessor();

    const gtString& GetErrorMessage() const;

    status Run();

// Methods:
private:
    status InsertCallsCategories();
    status InsertCounterCategories();

    status InsertCallsType(std::vector<std::string> &calls, gtString category, gtString api);
    status InsertCounterType(std::vector<std::string> &counters, gtString category);
    
    status InsertFrameDelimitersCalls();
    status InsertDrawTypeCalls();
    status InsertBindTypeCalls();
    status InsertWaitTypeCalls();
    status InsertSetupTypeCalls();
    status InsertErrorTypeCalls();
    status InsertCompileTypeCalls();
    status InsertComputeTypeCalls();
    status InsertDataTransferTypeCalls();
    status InsertStateTypeCalls();
    status InsertFrameTypeCalls();
    status InsertTextureTypeCalls();
    status InsertPipelineTypeCalls();
    status InsertDescriptorTypeCalls();
    status InsertBufferTypeCalls();
    status InsertMemoryTypeCalls();
    status InsertBlockTypeCalls();
    status InsertShaderTypeCalls();
    status InsertOtherTypeCalls();

// Attributes:
private:
    LPGPU2DatabaseAdapter &m_dbAdapter;
    gtString m_errorMessage;
};
// clang-format on


} // namespace db
} // namespace lpgpu2

#endif // LPGPU2_DB_FEEDBACK_ENGINE_PRE_PROCESSOR_H_INCLUDE
