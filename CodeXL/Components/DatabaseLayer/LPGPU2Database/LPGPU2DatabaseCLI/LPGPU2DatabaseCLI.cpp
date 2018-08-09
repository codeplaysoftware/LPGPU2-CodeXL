// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  This project is a demo application that aims to show how to use 
///         the DatabaseLayer projects to read/write and extend the CodeXL profiling 
///         database format.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// STL:
#include <iostream>
#include <chrono>
#include <numeric>

// Boost:
#include <boost/program_options.hpp>

// AMDTDBaseTools: 
#include <AMDTBaseTools/Include/gtString.h>

// AMDTOSWrappers:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osTimeInterval.h>

// AMDTDbAdapter:
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>

// LPGPU2Database
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAccessor.h>

// Local:
#include "AssertionHandler.h"
#include "TraceDBDump.h"

#include <sqlite3.h>

/// @brief  This accessor is just a way to generate the database using a dump
///         of the existing trace database file. The dump is located in the file
///         TraceDBDump.h. This class just execute each statement of the dump
///         to write information on the database
class DumpDBAccessor : public lpgpu2::db::LPGPU2DatabaseAccessor
{
public:
    virtual bool CreateProfilingDatabase(const gtString& databaseFilepath, gtUInt64 profileType) override
    {
        bool bReturn = false;

        GT_IF_WITH_ASSERT_EX(LPGPU2DatabaseAccessor::CreateProfilingDatabase(databaseFilepath, profileType), L"Cannot create profiling database")
        {
            sqlite3* pDbWriteConnection = GetDatabaseWriteConnection();

            GT_IF_WITH_ASSERT_EX(pDbWriteConnection != nullptr, L"Invalid database write connection")
            {
                bReturn = true;
                std::for_each(std::begin(TRACE_DB_DUMP), std::end(TRACE_DB_DUMP), [&](const char* stmt)
                {
                    sqlite3_stmt* pStmt = nullptr;

                    std::string stmtStr(stmt);
                    int rc = sqlite3_prepare_v2(pDbWriteConnection, stmtStr.data(), stmtStr.size(), &pStmt, nullptr);

                    if (SQLITE_OK == rc)
                    {
                        rc = sqlite3_step(pStmt);
                        bReturn &= (rc == SQLITE_DONE);
                    }

                    sqlite3_finalize(pStmt);
                });
            }
        }

        return bReturn;
    }
};

class DumpDBAdapter : public lpgpu2::db::LPGPU2DatabaseAdapter
{
public:
    virtual bool Initialise() override
    {
        bool bReturn = false;

        m_pDbAccessor = new (std::nothrow) DumpDBAccessor{};

        GT_IF_WITH_ASSERT_EX(m_pDbAccessor != nullptr, L"Cannot allocate DumpDBAccessor")
        {
            bReturn = true;
        }

        return bReturn;
    }
};


// Type definition to correct a case mistake in the CodeXL framework
using AmdtProfileDbAdapter = amdtProfileDbAdapter;

// Alias for Adapters shared_ptr's
using AmdtProfileDbAdapterPtr = std::shared_ptr<AmdtProfileDbAdapter>;
using TraceDatabaseAdapterPtr = std::shared_ptr<lpgpu2::db::LPGPU2DatabaseAdapter>;

/// @brief                          Returns the AMDTProfileMode based on the database filepath extension
/// @param[in]  DatabaseFilePath    The database file path to be analyzed
/// @return     AMDTProfileMode     The profile mode based on the database file extension
AMDTProfileMode GetDatabaseProfileMode(const osFilePath& databaseFilePath)
{
    gtString CPUPFileExtention;
    gtString PWRPFileExtension;
    CPUP_DB_FILE_EXTENSION.getSubString(1, CPUP_DB_FILE_EXTENSION.length(), CPUPFileExtention);
    PWRP_DB_FILE_EXTENSION.getSubString(1, PWRP_DB_FILE_EXTENSION.length(), PWRPFileExtension);

    if (databaseFilePath.IsMatchingExtension(CPUPFileExtention))
    {
        return AMDT_PROFILE_MODE_TIMELINE;
    }
    else if (databaseFilePath.IsMatchingExtension(PWRPFileExtension))
    {
        return AMDT_PROFILE_MODE_AGGREGATION;
    }
    else
    {
        return AMDT_PROFILE_MODE_NONE;
    }
}

/// @brief                          Creates a database adapter. Opens and tries to prepare the database
/// @param[in]  DatabaseFilePath    The database file path to be opened
/// @param[in]  ProfileMode         The profile mode to be used to prepare the database
/// @param[in]  bIsReadOnly         Specifies if the database will be opened in read only mode
TraceDatabaseAdapterPtr CreateDbAdapter(const osFilePath& databaseFilePath, AMDTProfileMode profileMode, bool bCreateDatabase, bool bIsReadOnly)
{
    TraceDatabaseAdapterPtr pDbAdapter = nullptr;

    if (bCreateDatabase)
    {
        // pDbAdapter = std::make_shared<lpgpu2::db::LPGPU2DatabaseAdapter>(); // Use this line to create an empty database
        pDbAdapter = std::make_shared<DumpDBAdapter>(); // Use this line to fill the database with TraceDBDump.h content
        GT_IF_WITH_ASSERT_EX(pDbAdapter->Initialise(), L"Cannot initialize database adapter")
        {
            GT_IF_WITH_ASSERT_EX(pDbAdapter->CreateDb(databaseFilePath.asString(), profileMode), pDbAdapter->GetErrorMessage().asCharArray())
            {
            }
    else
    {
        return nullptr;
    }
        }
    }
    else
    {
        pDbAdapter = std::make_shared<lpgpu2::db::LPGPU2DatabaseAdapter>();
        GT_IF_WITH_ASSERT_EX(pDbAdapter->OpenDb(databaseFilePath.asString(), profileMode, bIsReadOnly), pDbAdapter->GetErrorMessage().asCharArray())
        {
        }
        else
        {
            return nullptr;
        }
    }

    if (profileMode == AMDT_PROFILE_MODE_AGGREGATION)
    {
        // Need to call PrepareDb in the AGGREGATION mode
        GT_ASSERT_EX(pDbAdapter->PrepareDb(), L"Cannot prepare database");
    }

    return pDbAdapter;
}

/// @brief                  Validates a SamplingTimeRange object.
/// @param      timeRange   SamplingTimeRange object to be validate
/// @return     bool        True if timeRange is valid, both start and end
///                         must be greater than 0 and end should be greater
///                         than the start
constexpr bool IsTimeRangeValid(const SamplingTimeRange& timeRange)
{
    return timeRange.m_fromTime >= 0 && timeRange.m_toTime > timeRange.m_fromTime;
}

/// @brief Output stream operator for gtString's
std::wostream& operator<< (std::wostream& out, const gtString& str)
{
    out << str.asCharArray();
    return out;
}

/// @brief Output stream operator for osFilePath's
std::wostream& operator<< (std::wostream& out, const osFilePath& filePath)
{
    out << filePath.asString();
    return out;
}

/// @brief              Prints to wcout a Key and Value pair in the format "Key : Value"
/// @param[in]  Key     Any object that can be printed to the standard output
/// @param[in]  Value   Any object that can be printed to the standard output
template<typename KeyType, typename ValueType>
void PrintKeyValuePair(KeyType key, ValueType value)
{
    std::wcout << key << L" : " << value << std::endl;
}

/// @brief                      Prints to wcout any pair complaint object
/// @param[in]  KeyValuePair    Any object that has .first and .second members
template<typename KeyValuePairType>
void PrintKeyValuePair(KeyValuePairType keyValuePair)
{
    PrintKeyValuePair(keyValuePair.first, keyValuePair.second);
}

/// @brief              Prints a console session title with nice formating
/// @param[in]  Title   The title of the session
void PrintSessionHeader(const gtString& title)
{
    std::wcerr << L"----------------------------------------------------" << std::endl;
    std::wcerr << L"    " << title.asCharArray() << std::endl;
    std::wcerr << L"----------------------------------------------------" << std::endl;
}

/// @brief  Prints a console session footer that matches the PrintSessionHeader formating
void PrintSessionFooter()
{
    std::wcerr << L"----------------------------------------------------" << std::endl;
    std::wcerr << std::endl;
}

/// @brief                  Prints a session with Title and Content
///                         The Content must be a callable that prints information to the default output
///                         It should, preferably, use the PrintKeyValuePair functions
/// @param[in]  Title       The title of the session
/// @param[in]  Content     A callable object (can be a lambda function) that prints information to the
///                         standard output
template<typename ContentFunction>
void PrintSessionInfo(const gtString& title, ContentFunction content)
{
    PrintSessionHeader(title);
    content();
    PrintSessionFooter();
}

/// @brief                          Prints general information about the database
/// @param[in]  pDbAdapter          Pointer to the database adapter
/// @param[in]  DatabaseFilePath    Database file path. This will be printed to the output
void PrintDatabaseFileInfo(AmdtProfileDbAdapterPtr pDbAdapter, const osFilePath& databaseFilePath)
{
    PrintSessionInfo(L"Database Info", [&]
    {
        int DbVersion = 0;
        GT_IF_WITH_ASSERT_EX(pDbAdapter->GetDbVersion(DbVersion), L"Cannot get database version")
        {
            PrintKeyValuePair(L"Database File Path", databaseFilePath);
            PrintKeyValuePair(L"Database Version", DbVersion);
            PrintKeyValuePair(L"Supported Db Version", pDbAdapter->GetSupportedDbVersion());
        }
    });
}


/// @brief                          Prints information about the profiling session info
/// @param[in]  pDbAdapter          Pointer to the database adapter
void PrintDatabaseSessionInfo(AmdtProfileDbAdapterPtr pDbAdapter)
{
    PrintSessionInfo(L"Session Info", [&]
    {
        gtVector<std::pair<gtString, gtString>> SessionInfoKeyValueVec;
        GT_IF_WITH_ASSERT_EX(pDbAdapter->GetAllSessionInfo(SessionInfoKeyValueVec), L"Cannot get Database Session Info")
        {
            for (const auto& SessionInfoKeyValuePair : SessionInfoKeyValueVec)
            {
                PrintKeyValuePair(SessionInfoKeyValuePair);
            }
        }
    });
}

/// @brief                  Tries to read the CPU topology information from the database
/// @param[in]  pDbAdapter  Pointer to the database adapter
void PrintDatabaseCpuTopology(AmdtProfileDbAdapterPtr pDbAdapter)
{
    PrintSessionInfo(L"CPU Topology", [&]
    {
        AMDTCpuTopologyVec CpuTopologyVec;
        GT_IF_WITH_ASSERT_EX(pDbAdapter->GetCpuTopology(CpuTopologyVec), L"Cannot get CPU Topology Information")
        {
            for (const auto& CpuTopology : CpuTopologyVec)
            {
                PrintKeyValuePair(L"CoreID", CpuTopology.m_coreId);
                PrintKeyValuePair(L"ProcessorID", CpuTopology.m_processorId);
                PrintKeyValuePair(L"Num Node ID", CpuTopology.m_numaNodeId);
            }
        }
    });
}

/// @brief                  Prints a list with all the counters available in the database
/// @param[in]  pDbAdapter  Pointer to the database adapter
void PrintDatabaseCountersNames(AmdtProfileDbAdapterPtr pDbAdapter)
{
    PrintSessionInfo(L"Counter Names", [&]
    {
        gtMap<gtString, int> CounterNamesMap;
        GT_IF_WITH_ASSERT_EX(pDbAdapter->GetCounterNames(CounterNamesMap), L"Cannot get counter names")
        {
            for (const auto& CounterNamePair : CounterNamesMap)
            {
                auto CounterName = CounterNamePair.first;
                auto CounterId = CounterNamePair.second;

                PrintKeyValuePair(CounterName, CounterId);
            }
        }
    });
}

/// @ brief                     Prints detailed information about each of the performance counters 
/// @param[in]  pDbAdapter      Pointer to the database adapter
/// @param[in]  SamplingRange   The range that will be used to query for sample values
/// @param[in]  MaxSamples      Within the range only prints this maximum number of samples
void PrintDatabasePerformanceCountersDetails(AmdtProfileDbAdapterPtr pDbAdapter, SamplingTimeRange& samplingRange, int maxSamples)
{
    PrintSessionInfo(L"Performance Counters", [&]
    {
        gtMap<int, AMDTProfileCounterDesc> CounterDetails;
        GT_IF_WITH_ASSERT_EX(pDbAdapter->GetCountersDescription(CounterDetails), L"Cannot get counters details")
        {
            for (const auto& CounterDetailPair : CounterDetails)
            {
                const auto& CounterDesc = CounterDetailPair.second;

                PrintKeyValuePair(L"Name", CounterDesc.m_name);
                PrintKeyValuePair(L"ID", CounterDesc.m_id);
                PrintKeyValuePair(L"HW Event ID", CounterDesc.m_hwEventId);
                PrintKeyValuePair(L"Device ID", CounterDesc.m_deviceId);
                PrintKeyValuePair(L"Abbrev", CounterDesc.m_abbrev);
                PrintKeyValuePair(L"Description", CounterDesc.m_description);
                PrintKeyValuePair(L"Type", CounterDesc.m_type);
                PrintKeyValuePair(L"TypeStr", CounterDesc.m_typeStr);
                PrintKeyValuePair(L"Category", CounterDesc.m_category);
                PrintKeyValuePair(L"CategoryStr", CounterDesc.m_categoryStr);
                PrintKeyValuePair(L"Unit", CounterDesc.m_unit);
                PrintKeyValuePair(L"UnitStr", CounterDesc.m_unitStr);

                const auto CounterIdAsInt = static_cast<int>(CounterDesc.m_id);

                gtMap<int, int> OverallNumberOfSamplesPerCounter;
                GT_IF_WITH_ASSERT_EX(pDbAdapter->GetOverallNubmerOfSamples({ CounterIdAsInt }, OverallNumberOfSamplesPerCounter), L"Cannot get overall number of samples")
                {
                    PrintKeyValuePair(L"\tOverall # Of Samples", OverallNumberOfSamplesPerCounter[CounterIdAsInt]);
                }

                gtMap<int, int> NumberOfSamplesPerCounter;
                GT_IF_WITH_ASSERT_EX(pDbAdapter->GetSampleCountByCounterId({ CounterIdAsInt }, NumberOfSamplesPerCounter), L"Cannot get sample count")
                {
                    PrintKeyValuePair(L"\tSample Count", NumberOfSamplesPerCounter[CounterIdAsInt]);
                }

                gtMap<int, double> SamplesGroupPerCounter;
                GT_IF_WITH_ASSERT_EX(pDbAdapter->GetSamplesGroupByCounterId({ CounterIdAsInt }, SamplesGroupPerCounter), L"Cannot get samples groups")
                {
                    PrintKeyValuePair(L"\tSamples Group", SamplesGroupPerCounter[CounterIdAsInt]);
                }

                if (IsTimeRangeValid(samplingRange))
                {
                    double RangeMinValue = 0.0;
                    double RangeMaxValue = 0.0;
                    GT_IF_WITH_ASSERT_EX(pDbAdapter->GetGlobalMinMaxValuesPerCounters({ CounterIdAsInt }, samplingRange, RangeMinValue, RangeMaxValue), L"Cannot get min max values per counter")
                    {
                        PrintKeyValuePair(L"\tMin Value", RangeMinValue);
                        PrintKeyValuePair(L"\tMax Value", RangeMaxValue);
                    }

                    gtMap<int, gtVector<SampledValue>> SampledValuesPerCounter;
                    GT_IF_WITH_ASSERT_EX(pDbAdapter->GetSampledValuesByRange({ CounterIdAsInt }, samplingRange, SampledValuesPerCounter), L"Cannot get sampled values by counter")
                    {
                        const auto& SampledValues = SampledValuesPerCounter[CounterIdAsInt];
                        PrintKeyValuePair("\tNumber of samples in range", SampledValues.size());

                        using SampledValueVecSizeType = decltype(SampledValuesPerCounter)::size_type;
                        const auto MaxSamplesWithCorrectType = static_cast<SampledValueVecSizeType>(maxSamples);
                        for (SampledValueVecSizeType SampleIndex = 0; SampleIndex < SampledValues.size() && SampleIndex < MaxSamplesWithCorrectType; ++SampleIndex)
                        {
                            const auto& Sample = SampledValues[SampleIndex];
                            PrintKeyValuePair(Sample.m_sampleTime, Sample.m_sampleValue);
                        }
                    }
                }

                PrintKeyValuePair(L"--", L"--");
            }
        }
    });
}

/// @brief                      Prints the sampling interval used in the profiling session
/// @param[in]  pDbAdapter      Pointer to the database adapter
void PrintDatabaseSessionSamplingInterval(AmdtProfileDbAdapterPtr pDbAdapter)
{
    PrintSessionInfo(L"Sampling Interval", [&]
    {
        unsigned int SamplingInterval = 0;
        GT_IF_WITH_ASSERT_EX(pDbAdapter->GetSessionSamplingIntervalMs(SamplingInterval), L"Cannot get sampling interval")
        {
            PrintKeyValuePair(L"Sampling Interval", SamplingInterval);
        }
    });
}

/// @brief                  Prints information from the database's trace table
/// @param[in]  pDbAdapter  Pointer to the database adapter
/// @param[in]  CallName    Name of the function to filter. It can be an empty string (no filter)
/// @param[in]  MaxSamples  Max number of trace information to print
void PrintDatabaseTraceInfo(AmdtProfileDbAdapterPtr pDbAdapter, const gtString& callName, int maxSamples)
{
    auto pTraceDbAdaper = std::dynamic_pointer_cast<lpgpu2::db::LPGPU2DatabaseAdapter>(pDbAdapter);

    GT_IF_WITH_ASSERT_EX(pTraceDbAdaper != nullptr, L"pDbAdater cannot be casted to TraceDatabaseAdapterPtr")
    {
        PrintSessionInfo(L"OpenGL Tracing Info", [&]
        {
            gtVector<lpgpu2::db::ProfileTraceInfo> OpenGLTraces;
            if (callName.isEmpty())
            {
                GT_ASSERT_EX(pTraceDbAdaper->GetProfileTraceInfo(OpenGLTraces), L"Cannot get trace info");
            }
            else
            {
                GT_ASSERT_EX(pTraceDbAdaper->GetProfileTraceInfoByCallName(callName, OpenGLTraces), L"Cannot get trace info");
            }

            if (!OpenGLTraces.empty())
            {
                using OpenGLTracesSizeType = decltype(OpenGLTraces)::size_type;
                const auto MaxSamplesWithCorrectType = static_cast<OpenGLTracesSizeType>(maxSamples);

                for (OpenGLTracesSizeType TraceIndex = 0; TraceIndex < OpenGLTraces.size() && TraceIndex < MaxSamplesWithCorrectType; ++TraceIndex)
                {
                    const auto& Trace = OpenGLTraces[TraceIndex];

                    PrintKeyValuePair(L"Trace ID", Trace.m_traceId);
                    PrintKeyValuePair(L"Call Name", Trace.m_callName);
                    PrintKeyValuePair(L"Cpu Start", Trace.m_cpuStart);
                    PrintKeyValuePair(L"Cpu End", Trace.m_cpuEnd);
                    PrintKeyValuePair(L"Frame", Trace.m_frameNum);
                    PrintKeyValuePair(L"Draw", Trace.m_drawNum);
                    PrintKeyValuePair(L"API ID", Trace.m_apiId);
                    PrintKeyValuePair(L"Param Offset", Trace.m_parameterOffset);
                    PrintKeyValuePair(L"PAram Length", Trace.m_parameterLength);
                    PrintKeyValuePair(L"--", L"--");
                }
            }
        });
    }
}

/// @brief                      Prints the total range of sampling information
/// @param[in]  pDbAdapter      Pointer to the database adapter
void PrintDatabaseTimeRange(AmdtProfileDbAdapterPtr pDbAdapter)
{
    PrintSessionInfo(L"Session Time Range", [&]
    {
        SamplingTimeRange TimeRange{ 0, 0 };
        GT_IF_WITH_ASSERT_EX(pDbAdapter->GetSessionTimeRange(TimeRange), L"Cannot get session time range (s)")
        {
            const std::chrono::milliseconds ElapsedTimeMilliSeconds(TimeRange.m_toTime - TimeRange.m_fromTime);
            const auto StartTimeInSeconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(TimeRange.m_fromTime));
            const auto EndTimeInSeconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(TimeRange.m_toTime));
            const auto ElpasedTimeSeconds = std::chrono::duration_cast<std::chrono::seconds>(ElapsedTimeMilliSeconds);
            const auto ElapsedTimeMinutes = std::chrono::duration_cast<std::chrono::minutes>(ElapsedTimeMilliSeconds);
            const auto ElapsedTimeHours = std::chrono::duration_cast<std::chrono::hours>(ElapsedTimeMilliSeconds);

            PrintKeyValuePair(L"Range Start Time (s)", StartTimeInSeconds.count());
            PrintKeyValuePair(L"Range End Time (s)", EndTimeInSeconds.count());
            PrintKeyValuePair(L"Elapsed Time (ms)", ElapsedTimeMilliSeconds.count());
            PrintKeyValuePair(L"Elapsed Time (s)", ElpasedTimeSeconds.count());
            PrintKeyValuePair(L"Elapsed Time (m)", ElapsedTimeMinutes.count());
            PrintKeyValuePair(L"Elapsed Time (h)", ElapsedTimeHours.count());
        }
    });
}

/// @brief                              Orchestrate the printing of information (just call the other printing functions
/// @param[in]  pDbAdapter              Pointer to the database adapter
/// @param[in]  DatabaseFilePath        Database file path to be printed on the screen
/// @param[in]  SamplingRange           Sampling range passed to PrintPerformanceCountersDetails
/// @param[in]  bPrintCounterDetails    Enables/Disables printing of detailed counter information
/// @param[in]  MaxSamples              The max number of samples passed to PrintPerformanceCounterDetails
/// @param[in]  CallName                Name of the function to query in the database's trace table
void PrintDatabaseInformation(TraceDatabaseAdapterPtr pDbAdapter,
                              const osFilePath& databaseFilePath,
                              SamplingTimeRange& samplingRange,
                              bool bPrintCounterDetails,
                              int maxSamples,
                              const gtString& callName)
{
    PrintDatabaseFileInfo(pDbAdapter, databaseFilePath);
    PrintDatabaseCpuTopology(pDbAdapter);
    PrintDatabaseCountersNames(pDbAdapter);

    if (bPrintCounterDetails)
    {
        PrintDatabasePerformanceCountersDetails(pDbAdapter, samplingRange, maxSamples);
    }

    PrintDatabaseTraceInfo(pDbAdapter, callName, maxSamples);

    PrintDatabaseSessionSamplingInterval(pDbAdapter);
    PrintDatabaseTimeRange(pDbAdapter);
}

/// @brief                  Generates fake data for the database
/// @param[in] pDbAdapter   Pointer to the database adapter
void GenerateDatabaseData(TraceDatabaseAdapterPtr pDbAdapter)
{
    const auto bIsLPGPU2Session = true;
    pDbAdapter->SetLPGPU2TargetDevice(bIsLPGPU2Session);

    SamplingTimeRange samplingTimeRange{ 0, 0 };
    pDbAdapter->GetSessionTimeRange(samplingTimeRange);

    if (!IsTimeRangeValid(samplingTimeRange))
    {
        return;
    }

    const auto samplingDuration = samplingTimeRange.m_toTime - samplingTimeRange.m_fromTime;

    // Generate Regions of Interest evenly spread across the sampling region
    const auto numberOfRegions = 10;
    const auto regionSize = static_cast<gtUInt64>(214.0 / numberOfRegions);
    for (auto iRegionId = 1; iRegionId < numberOfRegions; ++iRegionId)
    {
        lpgpu2::db::RegionOfInterest region;
        region.m_regionId = iRegionId;
        region.m_frameStart = iRegionId * regionSize;
        region.m_frameEnd = region.m_frameStart + regionSize - 1;
        region.m_degreeOfInterest = iRegionId / static_cast<gtFloat32>(numberOfRegions);

        //genRegionsOfInterest.push_back(region);
        pDbAdapter->InsertRegionOfInterest(region);
    }

    const std::array<gtString, 6> frameDelimiters =
    {
        L"clFinish",
        L"clFlush",
        L"vkQueuePresentKHR",
        L"eglSwapBuffers",
        L"glFlush",
        L"glFinish"
    };

    // Generate Call Summaries related to the regions of interest
    for (gtUInt32 iRegionId = 1; iRegionId < numberOfRegions; ++iRegionId)
    {
        lpgpu2::db::CallSummary summary;
        summary.m_regionId = iRegionId;
        summary.m_callName = frameDelimiters[iRegionId % frameDelimiters.size()];
        summary.m_timesCalled = iRegionId * 10;
        summary.m_totalTime = iRegionId * 12;

        pDbAdapter->InsertCallSummary(summary);
    }

    // Generate Calls per Type summary
    for (gtUInt32 iRegionId = 1; iRegionId < numberOfRegions; ++iRegionId)
    {
        const gtFloat32 callPercentage = 1.0f / 7.0f;

        lpgpu2::db::CallsPerType callsPerType;
        callsPerType.m_regionId = iRegionId;
        callsPerType.m_bindPercentage = callPercentage;
        callsPerType.m_drawPercentage = callPercentage;
        callsPerType.m_setupPercentage = callPercentage;
        callsPerType.m_waitPercentage = callPercentage;
        callsPerType.m_errorPercentage = callPercentage;
        callsPerType.m_delimiterPercentage = callPercentage;
        callsPerType.m_otherPercentage = callPercentage;

        pDbAdapter->InsertCallsPerType(callsPerType);
    }

    // Generate counter weights
    gtMap<gtString, int> counterNames;
    pDbAdapter->GetCounterNames(counterNames);
    for (const auto& counterNameIdPair : counterNames)
    {
        const auto& counterName = counterNameIdPair.first;
        lpgpu2::db::CounterWeight counterWeight;
        counterWeight.m_counterName = counterName;
        counterWeight.m_weightValue = counterNameIdPair.second * 0.5f;
        pDbAdapter->InsertCounterWeight(counterWeight);
    }

    // Generate shaders samples
    lpgpu2::db::ShaderTrace shaderTrace;
    shaderTrace.m_shaderId = 1;
    shaderTrace.m_apiId = lpgpu2::db::EShaderTraceAPIId::kGL;
    shaderTrace.m_frameNum = 0;
    shaderTrace.m_drawNum = 116;
    shaderTrace.m_shaderType = 35633;    

    shaderTrace.m_cpuTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::hours{ 1 }).count();
    shaderTrace.m_srcCode = 
        L"attribute highp vec3    myVertex; \n"
        "attribute highp vec3    myNormal; \n"
        "attribute mediump vec2  myUV; \n"
        "attribute mediump vec4  myBone; ";
    shaderTrace.m_asmCode =
        L"push rbp \n"
        "mov rbp, rsp \n"
        "mov DWORD PTR[rbp - 4], edi \n"
        "mov eax, DWORD PTR[rbp - 4] \n"
        "imul eax, DWORD PTR[rbp - 4] \n"
        "pop rbp \n"
        "ret";
    pDbAdapter->InsertShaderTrace(shaderTrace);
    
    // Generate some shader to asm
    lpgpu2::db::ShaderToAsm shaderToAsm;
    shaderToAsm.m_shaderId = 1;
    shaderToAsm.m_apiId = lpgpu2::db::EShaderTraceAPIId::kGL;
    shaderToAsm.m_shaderLine = 2;
    shaderToAsm.m_asmStartLine = 1;
    shaderToAsm.m_asmEndLine = 3;
    pDbAdapter->InsertShaderToAsm(shaderToAsm);

    shaderToAsm.m_shaderLine = 3;
    shaderToAsm.m_asmStartLine = 4;
    shaderToAsm.m_asmEndLine = 4;
    pDbAdapter->InsertShaderToAsm(shaderToAsm);

    shaderToAsm.m_shaderLine = 4;
    shaderToAsm.m_asmStartLine = 5;
    shaderToAsm.m_asmEndLine = 7;
    pDbAdapter->InsertShaderToAsm(shaderToAsm);

    // Generate some shader asm stats
    lpgpu2::db::ShaderAsmStat shaderAsmStat;
    shaderAsmStat.m_shaderId = 1;    
    shaderAsmStat.m_apiId = lpgpu2::db::EShaderTraceAPIId::kGL;
    
    const auto numAsmLines = shaderTrace.m_asmCode.count('\n') + 1;

    for (gtUInt64 iAsmLine = 1; iAsmLine < numAsmLines; ++iAsmLine)
    {
        shaderAsmStat.m_asmLine = iAsmLine;
        shaderAsmStat.m_percentage = 100.0f / iAsmLine;

        pDbAdapter->InsertShaderAsmStat(shaderAsmStat);
    }    
}

/// @brief                      Function used to check that of 'for_what' is specified, then
///                             'required_option' is specified too.
/// @param  variablesMap        Variables map with options parsed from the command line
/// @param  forWhat             For what option to check the dependency
/// @param  requiredOption      Which option is required
/// @return bool                True if the dependency was satisfied and false otherwise
bool CheckOptionDependency(const boost::program_options::variables_map& variablesMap, const char* forWhat, const char* requiredOption)
{
    if (variablesMap.count(forWhat) && !variablesMap[forWhat].defaulted())
    {
        if (variablesMap.count(requiredOption) == 0 || variablesMap[requiredOption].defaulted())
        {
            std::cerr << "Option '" << forWhat << "' requires option '" << requiredOption << "'." << std::endl;
            return false;
        }
    }

    return true;
}


/// @brief                  Function used to check that 'option1' and 'option2' are not specified at the same time
/// @param  variablesMap    Variables map with options parsed from the command line
/// @param  option1         First option to check for a conflict
/// @param  option2         Second option to check for a conflict
/// @return bool            True if 'option1' and 'option2' are conflicting and false otherwise
bool ConflicitingOptions(const boost::program_options::variables_map& variablesMap, const char* option1, const char* option2)
{
    if (variablesMap.count(option1) &&
        !variablesMap[option1].defaulted() &&
        !variablesMap[option2].defaulted())
    {
        std::cerr << "Conflicting options '" << option1 << "' and '" << option2 << "'." << std::endl;
        return true;
    }

    return false;
}

// Initializes the Logger or this will crash on Linux
static void InitLogger(osDebugLogSeverity initialiSeverity)
{
    osDebugLog& theDebugLog = osDebugLog::instance();

    theDebugLog.initialize(L"LPGPU2DatabaseCLI.log");
    theDebugLog.setLoggedSeverity(initialiSeverity);
}

/// @brief  Entry point of the application
int main(int argc, const char* argv[])
{
    InitLogger(OS_DEBUG_LOG_ERROR);

    // Defines the default values that the user can set via command line
    std::string inputDatabaseFilePath;      // The database file path to read/write
    int startTimeInSeconds = -1;            // The start time of the range to query for sample values
    int endTimeInSeconds = -1;              // The end time of the range to query for sample values
    std::string profileModeStr;             // The profile mode that is going to be used to open the database
    bool bPrintCounterDetails = false;      // Print detailed counter information
    int maxSamples = 10;                    // Max number of samples to print for each counter
    std::string functionNameStr;            // Name of the function to query in the trace table    
    bool bCreateDatabase = false;           // If true creates a new database with the information on TraceDBDump.h file      
    bool bGenerateData = false;         // Generates a table called 'help' with fake data        
    std::string outputCSVFolderName;

    // Define the command line args parser using boost
    boost::program_options::variables_map optionsMap;
    boost::program_options::options_description options("Allowed options");

    // NOTE: The indentation of this piece of code is messy because of Visual Studio        
    options.add_options()
        ("help", "Shows this help message")
        (
            "database",
            boost::program_options::value<std::string>(&inputDatabaseFilePath)
            ->value_name("FilePath"),
            "SQLite database file path"
            )
            (
                "mode",
                boost::program_options::value<std::string>(&profileModeStr)
                ->value_name("Mode")
                ->default_value("timeline"),
                "Mode to open the database file. Mode can be\n"
                "   - timeline\n"
                "   - aggregate\n"
                )
                (
                    "start-time",
                    boost::program_options::value<int>(&startTimeInSeconds)
                    ->value_name("Seconds")
                    ->default_value(-1),
                    "The start time of the range to query for sample values"
                    )
                    (
                        "end-time",
                        boost::program_options::value<int>(&endTimeInSeconds)
                        ->value_name("Seconds")
                        ->default_value(-1),
                        "The end time of the range to query for sample values"
                        )
                        (
                            "counter-details",
                            boost::program_options::bool_switch(&bPrintCounterDetails)
                            ->default_value(false),
                            "Print counter details"
                            )
                            (
                                "max-samples",
                                boost::program_options::value<int>(&maxSamples)
                                ->default_value(10),
                                "Max number of samples to print per counter"
                                )
                                (
                                    "function",
                                    boost::program_options::value<std::string>(&functionNameStr)
                                    ->value_name("Name")
                                    ->default_value(""),
                                    "Name of the function to query in the database's traces table"
                                    )
                                    (
                                        "create-database",
                                        boost::program_options::bool_switch(&bCreateDatabase)
                                        ->default_value(false),
                                        "If the database should be generated"
                                        )
                                        (
                                            "generate-data",
                                            boost::program_options::bool_switch(&bGenerateData)
                                            ->default_value(false),
                                            "Generates a table called 'help' with fake data"
                                            )
                                            (
                                                "export-csv",
                                                boost::program_options::value<std::string>(&outputCSVFolderName),
                                                "Exports the database to a group of CSV files"
                                                );

    try
    {
        auto parsedOptions = boost::program_options::parse_command_line(argc, argv, options);
        boost::program_options::store(parsedOptions, optionsMap);
        boost::program_options::notify(optionsMap);

        CheckOptionDependency(optionsMap, "generate-data", "create-database");
        CheckOptionDependency(optionsMap, "export-csv", "database");
    }
    catch (const std::exception& exception)
    {
        PrintKeyValuePair(L"ERROR", exception.what());
        std::cout << options << std::endl;
        return EXIT_FAILURE;
    }

    if (optionsMap.count("help") || argc == 1)
    {
        std::cout << options << std::endl;
        return EXIT_SUCCESS;
    }

    if (inputDatabaseFilePath.empty())
    {
        PrintKeyValuePair(L"ERROR", "Database file path was not set");
        return EXIT_FAILURE;
    }

    // Sets the database file path
    osFilePath databaseFilepath;
    gtString databaseFilePathStr;
    databaseFilePathStr.fromUtf8String(inputDatabaseFilePath);
    databaseFilepath.setFullPathFromString(databaseFilePathStr);

    if (bCreateDatabase && databaseFilepath.exists())
    {
        osFile databaseFile(databaseFilepath);
        if (!databaseFile.deleteFile())
        {
            PrintKeyValuePair(L"ERROR", L"Cannot delete database file");
            return EXIT_FAILURE;
        }
    }

    if (!bCreateDatabase && !databaseFilepath.exists())
    {
        PrintKeyValuePair(L"ERROR", L"Database file does not exists");
        return EXIT_FAILURE;
    }

    // Defines the actual assertion handler that is going to be registered
    AssertionHandler theAssertionHandler;

    // Register our custom assertion handler for assertions caught with calls to GT_IF_WITH_ASSERT_EX and GT_ASSERT_EX
    gtRegisterAssertionFailureHandler(&theAssertionHandler);

    // Select the profile mode based on the user input    
    AMDTProfileMode profileMode;
    if (profileModeStr.empty())
    {
        PrintKeyValuePair(L"ERROR", "Database profile mode was not set");
        return EXIT_FAILURE;
    }
    else
    {
        auto profileModeStr = optionsMap["mode"].as<std::string>();
        const std::map<std::string, AMDTProfileMode> availableProfileModes =
        {
            { "timeline", AMDT_PROFILE_MODE_TIMELINE },
            { "aggregation", AMDT_PROFILE_MODE_AGGREGATION }
        };

        auto profileModeSearch = availableProfileModes.find(profileModeStr);

        if (profileModeSearch != availableProfileModes.end())
        {
            profileMode = profileModeSearch->second;
        }
        else
        {
            PrintKeyValuePair(L"ERROR", L"Profile mode not supported");
            return EXIT_FAILURE;
        }
    }

    // Creates the database adapter to actually operate on the database
    const bool bIsDbReadOnly = !bCreateDatabase;
    auto pDbAdapter = CreateDbAdapter(databaseFilepath, profileMode, bCreateDatabase, bIsDbReadOnly);

    if (!pDbAdapter)
    {
        return EXIT_FAILURE;
    }

    if (!outputCSVFolderName.empty())
    {
        gtString outputCSVFolderNameStr;
        outputCSVFolderNameStr << outputCSVFolderName.data();

        const auto bOverrideExistingFiles = true;
        const auto fileNamePrefix = L"";
        GT_IF_WITH_ASSERT_EX(pDbAdapter->ExportToCSV(outputCSVFolderNameStr, fileNamePrefix, bOverrideExistingFiles), pDbAdapter->GetErrorMessage().asCharArray())
        {
            PrintKeyValuePair(L"SUCCESS", "Database content was exported to CSV");
            PrintKeyValuePair(L"Output folder", outputCSVFolderNameStr);
            return EXIT_SUCCESS;
        }
    }

    if (bIsDbReadOnly)
    {
        // Time conversion since the database stores information in milliseconds
        const auto startTimeInMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(startTimeInSeconds));
        const auto endTimeInMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(endTimeInSeconds));
        SamplingTimeRange samplingRange{ static_cast<gtInt64>(startTimeInMilliseconds.count()), static_cast<gtInt64>(endTimeInMilliseconds.count()) };

        gtString functionName;
        functionName << functionNameStr.data();

        // Reads information about the database    
        PrintDatabaseInformation(pDbAdapter, databaseFilepath, samplingRange, bPrintCounterDetails, maxSamples, functionName);

        PrintKeyValuePair(L"SUCCESS", L"Finish reading the database");

        {
            gtVector<lpgpu2::db::StackTraceParameter> traceParams;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetStackTraceParameters(traceParams), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& p : traceParams)
                {
                    PrintKeyValuePair("TypeDefDescription", p.m_typeDefDescription);

                    for (auto f : p.m_binaryParams)
                    {
                        std::cout << f;
                    }
                    std::cout << std::endl;
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Call names for trace in range");
            gtVector<lpgpu2::db::ProfileTraceInfo> traceInfo;
            const gtUInt64 rangeStart = 318;
            const gtUInt64 rangeEnd = 672;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetProfileTraceInfoInRange(rangeStart, rangeEnd, traceInfo), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& traceInfoEntry : traceInfo)
                {
                    PrintKeyValuePair(L"CallName: ", traceInfoEntry.m_callName);
                    PrintKeyValuePair(L"Cpu Start", traceInfoEntry.m_cpuStart);
                    PrintKeyValuePair(L"Cpu End", traceInfoEntry.m_cpuEnd);
                    PrintKeyValuePair(L"--", L"--");
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Unique Quantized Times");
            gtVector<gtUInt64> uniqueQuantizedTimes;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetUniqueQuantizedTimesFromSamples(uniqueQuantizedTimes), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& quantizedTime : uniqueQuantizedTimes)
                {
                    PrintKeyValuePair(L"Quantized Time", quantizedTime);
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Unique Quantized Times in Range");
            gtVector<gtUInt64> uniqueQuantizedTimes;
            const gtUInt64 rangeStart = 200000;
            const gtUInt64 rangeEnd = 400000;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetUniqueQuantizedTimesFromSamplesInRange(rangeStart, rangeEnd, uniqueQuantizedTimes), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& quantizedTime : uniqueQuantizedTimes)
                {
                    PrintKeyValuePair(L"Quantized Time", quantizedTime);
                }
            }
        }
        {
            gtUInt64 numThreads;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetNumThreadsFromTraceParameters(numThreads), pDbAdapter->GetErrorMessage().asCharArray())
            {
                PrintKeyValuePair(L"Number Threads", numThreads);
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Average Sample Values");
            gtVector<PPSampleData> avgSampleValues;
            gtString counterCategory = L"CPU_LOAD";
            const gtUInt32 rangeStart = 0;
            const gtUInt32 rangeEnd = 1000000;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetAverageSamplesByCounterCategoryInRange(counterCategory, rangeStart, rangeEnd, avgSampleValues), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& sample : avgSampleValues)
                {
                    PrintKeyValuePair(L"counterId", sample.m_counterID);
                    PrintKeyValuePair(L"quantizedTimeMs", sample.m_quantizedTime);
                    PrintKeyValuePair(L"sampleValue", sample.m_sampleValue);
                    PrintKeyValuePair(L"--", L"--");
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Maximum Sample Values");
            gtVector<PPSampleData> maxSampleValues;
            gtString counterCategory = L"CPU_LOAD";
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetMaxSampleValuesByCounterCategory(counterCategory, maxSampleValues), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& sample : maxSampleValues)
                {
                    PrintKeyValuePair(L"counterId", sample.m_counterID);
                    PrintKeyValuePair(L"quantizedTimeMs", sample.m_quantizedTime);
                    PrintKeyValuePair(L"sampleValue", sample.m_sampleValue);
                    PrintKeyValuePair(L"--", L"--");
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Unique FrameNums");
            gtVector<gtUInt64> uniqueFrameNums;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetUniqueProfileTraceFrameNumbers(uniqueFrameNums), pDbAdapter->GetErrorMessage().asCharArray())
            {
                PrintKeyValuePair("Number of unique frameNums", uniqueFrameNums.size());
                for (auto frameNum : uniqueFrameNums)
                {
                    std::cout << frameNum << " ";
                }
                std::cout << std::endl;
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Samples for Counter Name: FPS");
            gtVector<PPSampleData> sampleValues;
            const gtString counterName = L"FPS";
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetSamplesForCounterByName(counterName, sampleValues), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& sample : sampleValues)
                {
                    PrintKeyValuePair(L"counterId", sample.m_counterID);
                    PrintKeyValuePair(L"quantizedTimeMs", sample.m_quantizedTime);
                    PrintKeyValuePair(L"sampleValue", sample.m_sampleValue);
                    PrintKeyValuePair(L"--", L"--");
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"StackTraceParameters for category 'FrameDelimiter' and API name 'OpenGL'");
            gtVector<lpgpu2::db::StackTraceParameter> stackTraceParams;
            const gtString callCategory = L"FrameDelimiter";
            const gtString apiName = L"OpenGL";
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetStackTraceParametersByCallCategoryAndAPIName(callCategory, apiName, stackTraceParams), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& p : stackTraceParams)
                {
                    PrintKeyValuePair("TypeDefDescription", p.m_typeDefDescription);

                    for (auto f : p.m_binaryParams)
                    {
                        std::cout << f;
                    }
                    std::cout << std::endl;
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"StackTraceParameters for category 'Shader'");
            gtVector<lpgpu2::db::StackTraceParameter> stackTraceParams;
            const gtString callCategory = L"Shader";
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetStackTraceParametersByCallCategory(callCategory, stackTraceParams), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& p : stackTraceParams)
                {
                    PrintKeyValuePair("TypeDefDescription", p.m_typeDefDescription);

                    for (auto f : p.m_binaryParams)
                    {
                        std::cout << f;
                    }
                    std::cout << std::endl;
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"ProfileTraceInfo for category 'FrameDelimiter' and API name 'OpenGL'");
            gtVector<lpgpu2::db::ProfileTraceInfo> traceInfo;
            const gtString callCategory = L"FrameDelimiter";
            const gtString apiName = L"OpenGL";
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetProfileTraceInfoByCallCategoryAndAPIName(callCategory, apiName, traceInfo), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& traceInfoEntry : traceInfo)
                {
                    PrintKeyValuePair(L"CallName: ", traceInfoEntry.m_callName);
                    PrintKeyValuePair(L"Cpu Start", traceInfoEntry.m_cpuStart);
                    PrintKeyValuePair(L"Cpu End", traceInfoEntry.m_cpuEnd);
                    PrintKeyValuePair(L"--", L"--");
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"StackTraceParams for FrameNum and DrawNum");
            gtVector<lpgpu2::db::StackTraceParameter> traceParams;
            const gtUInt32 frameNum = 10;
            const gtUInt32 drawNum = 42;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetStackTraceParametersByFrameAndDrawNumber(frameNum, drawNum, traceParams), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& p : traceParams)
                {
                    PrintKeyValuePair("TypeDefDescription", p.m_typeDefDescription);

                    for (auto f : p.m_binaryParams)
                    {
                        std::cout << f;
                    }
                    std::cout << std::endl;
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Weighted sum for counter with samples in range");
            const gtUInt32 rangeStart = 200000;
            const gtUInt32 rangeEnd = 4000000;
            const gtString counterName = L"Battery Power";
            gtFloat32 weightedSum;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetWeightedSumForCounterByNameInRange(counterName, rangeStart, rangeEnd, weightedSum), pDbAdapter->GetErrorMessage().asCharArray())
            {
                PrintKeyValuePair(L"Weighed Sum", weightedSum);
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Profile Trace info for call category with num calls above threshold");
            const gtString callCategory = L"Draw";
            const gtUInt32 threshold = 0;
            gtVector<lpgpu2::db::ProfileTraceInfo> traceInfo;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetProfileTraceInfoByCallCategoryWithNumberOfCallsAboveThreshold(callCategory, threshold, traceInfo), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& traceInfoEntry : traceInfo)
                {
                    PrintKeyValuePair(L"CallName: ", traceInfoEntry.m_callName);
                    PrintKeyValuePair(L"Cpu Start", traceInfoEntry.m_cpuStart);
                    PrintKeyValuePair(L"Cpu End", traceInfoEntry.m_cpuEnd);
                    PrintKeyValuePair(L"--", L"--");
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Counter Weight for Counter with Name");
            const gtString counterName = L"FPS";
            lpgpu2::db::CounterWeight counterWeight;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetCounterWeightForCounterByName(counterName, counterWeight), pDbAdapter->GetErrorMessage().asCharArray())
            {
                PrintKeyValuePair("Counter Name", counterWeight.m_counterName);
                PrintKeyValuePair("Counter Weight", counterWeight.m_weightValue);
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Counter Weight for Counter with Name");
            const gtString counterName = L"FPS";
            gtVector<PPSampleData> sampleValues;
            GT_IF_WITH_ASSERT_EX(pDbAdapter->GetWeightedSamplesForCounterByName(counterName, sampleValues), pDbAdapter->GetErrorMessage().asCharArray())
            {
                for (const auto& sample : sampleValues)
                {
                    PrintKeyValuePair(L"counterId", sample.m_counterID);
                    PrintKeyValuePair(L"quantizedTimeMs", sample.m_quantizedTime);
                    PrintKeyValuePair(L"sampleValue", sample.m_sampleValue);
                    PrintKeyValuePair(L"--", L"--");
                }
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Get Profile Trace Info By Call name");
            gtVector<lpgpu2::db::ProfileTraceInfo> traceInfo;
            pDbAdapter->GetProfileTraceInfoByCallName(L"eglGetProcAddress", traceInfo);
            for (const auto& traceInfoEntry : traceInfo)
            {
                PrintKeyValuePair(L"CallName: ", traceInfoEntry.m_callName);
                PrintKeyValuePair(L"Cpu Start", traceInfoEntry.m_cpuStart);
                PrintKeyValuePair(L"Cpu End", traceInfoEntry.m_cpuEnd);
                PrintKeyValuePair(L"--", L"--");
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Get Profile Trace Info by API Name");
            gtVector<lpgpu2::db::ProfileTraceInfo> traceInfo;
            pDbAdapter->GetProfileTraceInfoByAPIName(L"OpenGLES", traceInfo);
            for (const auto& traceInfoEntry : traceInfo)
            {
                PrintKeyValuePair(L"CallName: ", traceInfoEntry.m_callName);
                PrintKeyValuePair(L"Cpu Start", traceInfoEntry.m_cpuStart);
                PrintKeyValuePair(L"Cpu End", traceInfoEntry.m_cpuEnd);
                PrintKeyValuePair(L"--", L"--");
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Get Shader Traces by APIId");
            gtVector<lpgpu2::db::ShaderTrace> shaderTraces;
            pDbAdapter->GetShaderTracesByAPIID(lpgpu2::db::EShaderTraceAPIId::kGLES2, shaderTraces);
            for (const auto& shaderTrace : shaderTraces)
            {
                PrintKeyValuePair(L"FrameNum", shaderTrace.m_frameNum);
                PrintKeyValuePair(L"Draw Num", shaderTrace.m_drawNum);
                PrintKeyValuePair(L"API ID", static_cast<gtUInt32>(shaderTrace.m_apiId));
                PrintKeyValuePair(L"Text", shaderTrace.m_srcCode);
                PrintKeyValuePair(L"--", L"--");
            }
        }
        {
            PrintKeyValuePair(L"----------------------------------------", L"Get ShaderStats");
            gtVector<lpgpu2::db::ShaderAsmStat> shaderStats;
            const gtUInt64 shaderId = 1;
            pDbAdapter->GetShaderAsmStatsByShaderId(shaderId, shaderStats);
            for (const auto& shaderStat : shaderStats)
            {
                PrintKeyValuePair(L"ShaderId", shaderStat.m_shaderId);
                PrintKeyValuePair(L"apiId", static_cast<gtUInt32>(shaderStat.m_apiId));
                PrintKeyValuePair(L"ShaderLine", shaderStat.m_asmLine);
                PrintKeyValuePair(L"Percentage", shaderStat.m_percentage);
                PrintKeyValuePair(L"--", L"--");
            }
        }
    }
    else
    {
        if (bGenerateData)
        {
            GenerateDatabaseData(pDbAdapter);
        }

#if 0
        {
            // Function to write a power estimation value
            lpgpu2::db::EstimatedPowerSample pwrSample;
            pwrSample.m_modelId = 1;
            pwrSample.m_unit = L"My Unit";
            pwrSample.m_estimatedPower = 1234.30f;
            pwrSample.m_sampleTime = 42;
            pwrSample.m_componentId = 50;
            GT_ASSERT(pDbAdapter->InsertEstimatedPowerSample(pwrSample));

            // Function to write a power component
            lpgpu2::db::EstimatedPowerComponent pwrComponent;
            pwrComponent.m_componentId = 1;
            pwrComponent.m_componentName = L"Component Name";
            pwrComponent.m_deviceId = 1;
            GT_ASSERT(pDbAdapter->InsertEstimatedPowerComponent(pwrComponent));

            // Function to write a ProfileTraceInfo
            lpgpu2::db::ProfileTraceInfo traceInfo;
            traceInfo.m_apiId = 123;
            traceInfo.m_callName = L"Call name";
            traceInfo.m_cpuStart = 42;
            traceInfo.m_cpuEnd = 420;
            traceInfo.m_drawNum = 123;
            traceInfo.m_frameNum = 1234;
            traceInfo.m_parameterLength = 30;
            GT_ASSERT(pDbAdapter->InsertProfileTraceInfo(traceInfo));

            // Function to write a Stack Trace Info
            lpgpu2::db::StackTraceInfo stackTrace;
            stackTrace.m_baseAddr = 0x12345;
            stackTrace.m_drawNum = 42000;
            stackTrace.m_frameNum = 42;
            stackTrace.m_fileName = L"filename.glsl";
            stackTrace.m_symbolAddr = 0x12345;
            stackTrace.m_symbolName = L"My Symbol";
            GT_ASSERT(pDbAdapter->InsertStackTrace(stackTrace));

            // Function to write an annotation
            lpgpu2::db::Annotation annotation;
            annotation.m_annotationType = 42;
            annotation.m_cpuStartTime = 42;
            annotation.m_cpuEndTime = 420;
            annotation.m_drawNum = 42;
            annotation.m_frameNum = 42;
            annotation.m_text = L"Annotation text";
            GT_ASSERT(pDbAdapter->InsertAnnotation(annotation));

            // Function to write a shader trace
            lpgpu2::db::ShaderTrace shaderTrace;
            shaderTrace.m_shaderId = 1;
            shaderTrace.m_apiId = lpgpu2::db::EShaderTraceAPIId::kALL;
            shaderTrace.m_cpuTime = 420;
            shaderTrace.m_drawNum = 4200;
            shaderTrace.m_frameNum = 4200;
            shaderTrace.m_shaderType = 1;
            shaderTrace.m_srcCode = L"My shader trace text";            
            GT_ASSERT(pDbAdapter->InsertShaderTrace(shaderTrace));

            shaderTrace.m_shaderId = 4;
            GT_ASSERT(pDbAdapter->UpdateShaderTrace(lpgpu2::db::EShaderMetadataType::kSourceFromMainShaderDataPacket, shaderTrace));

            shaderTrace.m_srcCode = L"My updated shader trace src code";
            shaderTrace.m_asmCode = L"My updated asm code";
            const std::string debugStr = "DEBUG";
            const std::string binaryStr = "BINARY";
            shaderTrace.m_debug.insert(shaderTrace.m_debug.begin(), debugStr.begin(), debugStr.end());
            shaderTrace.m_binary.insert(shaderTrace.m_binary.begin(), binaryStr.begin(), binaryStr.end());

            GT_ASSERT(pDbAdapter->UpdateShaderTrace(lpgpu2::db::EShaderMetadataType::kSource, shaderTrace));
            GT_ASSERT(pDbAdapter->UpdateShaderTrace(lpgpu2::db::EShaderMetadataType::kAsm, shaderTrace));
            GT_ASSERT(pDbAdapter->UpdateShaderTrace(lpgpu2::db::EShaderMetadataType::kBinary, shaderTrace));
            GT_ASSERT(pDbAdapter->UpdateShaderTrace(lpgpu2::db::EShaderMetadataType::kDebug, shaderTrace));

            // Function to insert a shader stat
            lpgpu2::db::ShaderAsmStat shaderStat;
            shaderStat.m_shaderId = 1;
            shaderStat.m_asmLine = 10;
            shaderStat.m_percentage = 123.123f;
            GT_ASSERT(pDbAdapter->InsertShaderAsmStat(shaderStat));

            shaderStat.m_asmLine = 20;
            shaderStat.m_percentage = 222.0f;
            GT_ASSERT(pDbAdapter->InsertShaderAsmStat(shaderStat));
            GT_ASSERT(pDbAdapter->InsertShaderAsmStat(shaderStat));

            shaderStat.m_asmLine = 10;
            GT_ASSERT(pDbAdapter->InsertShaderAsmStat(shaderStat));
            
            gtVector<lpgpu2::db::ShaderAsmStat> shaderStats;
            shaderStats.push_back({ 1, lpgpu2::db::EShaderTraceAPIId::kGL, 1, 0.0f });
            shaderStats.push_back({ 3, lpgpu2::db::EShaderTraceAPIId::kGL, 1, 0.0f });
            GT_ASSERT(pDbAdapter->UpdateShaderAsmStats(shaderStats));

            lpgpu2::db::ShaderToAsm shaderToAsm;
            shaderToAsm.m_shaderId = 1;
            shaderToAsm.m_apiId = lpgpu2::db::EShaderTraceAPIId::kGL;
            shaderToAsm.m_shaderLine = 10;
            shaderToAsm.m_asmStartLine = 3;
            shaderToAsm.m_asmEndLine = 5;
            GT_ASSERT(pDbAdapter->InsertShaderToAsm(shaderToAsm));

            // Function to write a GPU timer event
            lpgpu2::db::GPUTimer gpuTimer;
            gpuTimer.m_drawNum = 42;
            gpuTimer.m_frameNum = 420;
            gpuTimer.m_time = 12334;
            gpuTimer.m_type = 2;
            GT_ASSERT(pDbAdapter->InsertGPUTimer(gpuTimer));

            // Function to write a counter weight
            lpgpu2::db::CounterWeight cw;
            cw.m_counterName = L"My counter name";
            cw.m_weightValue = 42.0;
            GT_ASSERT(pDbAdapter->InsertCounterWeight(cw));

            // Function to write a stack trace parameter
            lpgpu2::db::StackTraceParameter traceParam;
            traceParam.m_frameNum = 10;
            traceParam.m_drawNum = 42;
            traceParam.m_threadId = 100;
            traceParam.m_typeDefDescription = L"Typedef description";

            gtString binaryParams = L"Test of writing blob";
            const gtUByte* rawBinaryParams = reinterpret_cast<const gtUByte*>(binaryParams.asASCIICharArray());

            traceParam.m_binaryParams.assign(rawBinaryParams, rawBinaryParams + binaryParams.length());
            traceParam.m_binaryReturn.assign(rawBinaryParams, rawBinaryParams + binaryParams.length());
            GT_ASSERT(pDbAdapter->InsertStackTraceParameter(traceParam));

            // Function to write a region of interest
            lpgpu2::db::RegionOfInterest regOfInterest;
            regOfInterest.m_regionId = 1;
            regOfInterest.m_frameStart = 100;
            regOfInterest.m_frameEnd = 200;
            regOfInterest.m_degreeOfInterest = 42.2f;
            GT_ASSERT(pDbAdapter->InsertRegionOfInterest(regOfInterest));

            // Function to insert a call summary
            lpgpu2::db::CallSummary callSummary;
            callSummary.m_regionId = 1; // RegionId must exist in regions of interest table
            callSummary.m_callName = L"My Call Name";
            callSummary.m_timesCalled = 42;
            callSummary.m_totalTime = 42;
            GT_ASSERT(pDbAdapter->InsertCallSummary(callSummary));

            // Function to insert a CallsPerType entry
            lpgpu2::db::CallsPerType callsPerType;
            callsPerType.m_regionId = 1;
            callsPerType.m_bindPercentage = 1;
            callsPerType.m_delimiterPercentage = 1;
            callsPerType.m_drawPercentage = 1;
            callsPerType.m_errorPercentage = 1;
            callsPerType.m_otherPercentage = 1;
            callsPerType.m_setupPercentage = 1;
            callsPerType.m_waitPercentage = 1;
            GT_ASSERT(pDbAdapter->InsertCallsPerType(callsPerType));            
        }
#endif
    }

    PrintKeyValuePair(L"File path", databaseFilepath.asString());

    // Unregistered the assertion handler
    gtUnRegisterAssertionFailureHandler(&theAssertionHandler);

    GT_ASSERT(pDbAdapter->CloseDb());

    return EXIT_SUCCESS;
}
