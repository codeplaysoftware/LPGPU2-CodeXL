// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Database is an extension to the CodeXL's Database Layer to
///         allow customisation of the CodeXL power profiling database.It
///         has the API to read and write the tables used by the LPGPU2
///         consortium
///
/// LPGPU2DatabaseAccessor interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.
#ifndef LPGPU2_DB_DATABASE_ACCESSOR_H_INCLUDE
#define LPGPU2_DB_DATABASE_ACCESSOR_H_INCLUDE

// AMDTProfilerDAL:
#include <AMDTProfilerDAL/include/AMDTDatabaseAccessor.h>

// Local:
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAPIDefs.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_FnStatus.h>

// Declarations:
struct sqlite3_stmt;

namespace lpgpu2 {
namespace db {

// Declarations:
class SQLiteDatabaseGuard;
class SQLiteStatementGuard;

using namespace fnstatus;

/// @brief      This is an extension of the AMDTProfileDAL::AmdtDatabaseAccessor 
///             to read/write information from the power profiling database. This
///             accessor makes available information of the LPGPU2 tables.
/// @warning    None.
/// @see        AMDTProfileDAL::AmdtDatabaseAccessor.
/// @author     Thales Sabino.
// clang-format off
class LPGPU2_DATABASE_API LPGPU2DatabaseAccessor : public AMDTProfilerDAL::AmdtDatabaseAccessor
{
    // Methods:
public:
    LPGPU2DatabaseAccessor();
    virtual ~LPGPU2DatabaseAccessor();

    // Overridden:
public:
    // From AMDTProfilerDAL::AmdtDatabaseAccessor    
    virtual bool CreateProfilingDatabase(const gtString& vDatabaseFilepath, gtUInt64 vProfileType) override;
    virtual bool OpenProfilingDatabase(const gtString& vDatabaseFilepath, gtUInt64 vProfileType, bool vbIsReadOnly = true) override;

    // Methods:
public:
    const gtString& GetErrorMessage() const;

    // Methods to read the SQLite database
    status GetAllSamples(gtVector<PPSampleData> &vSamples) const;
    status GetProfileTraceInfo(gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const;
    status GetProfileTraceInfoByCallName(const gtString& vCallName, gtVector<ProfileTraceInfo> &vProfileTraceInfo) const;
    status GetProfileTraceInfoMaxCpuEndTimeByFrameNumber(gtUInt64 vFrameNum, ProfileTraceInfo &vProfileTraceInfo) const;
    status GetProfileTraceInfoByCallCategoryWithNumberOfCallsAboveThreshold(const gtString &vCounterCategory, gtUInt64 vThreshold, gtVector<ProfileTraceInfo> &vProfileTraceInfo) const;
    status GetFirstTraceCallTimestamp(gtUInt64 &vCpuStartTimestamp) const;    
    status GetProfileTraceInfoInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<ProfileTraceInfo> &vProfileTraceInfo) const;
    status GetFrameTimeRange(gtUInt64 vFrameNum, gtUInt64 &vStartTime, gtUInt64 &vEndTime) const;
    status GetEstimatedPowerSamples(gtVector<EstimatedPowerSample>& vPwrSamples) const;
    status GetEstimatedPowerComponents(gtMap<gtUInt64, gtVector<EstimatedPowerComponent>> &vPwrComponentsByDevice) const;
    status GetAnnotations(gtVector<Annotation>& vAnnotations) const;
    status GetAnnotationById(gtUInt64 index, Annotation& vAnnotation) const;
    status GetAnnotationsInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<Annotation> &vAnnotations) const;
    status GetAnnotationsInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<Annotation> &vAnnotations) const;
    status GetGlobalAnnotations(gtVector<Annotation> &vAnnotations) const;
    status GetUserAnnotations(gtVector<Annotation> &vAnnotations) const;
    status GetGPUTimers(gtVector<GPUTimer>& vGpuTimers) const;
    status GetGPUTimersInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<GPUTimer> &vGpuTimers) const;
    status GetShaderTraces(gtVector<ShaderTrace> &vShaderTraces) const;    
    status GetShaderTracesInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<ShaderTrace> &vShaderTraces) const;
    status GetShaderTracesInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<ShaderTrace> &vShaderTraces) const;
    status GetShaderTracesByAPIId(EShaderTraceAPIId vApiId, gtVector<ShaderTrace> &vShaderTraces) const;    
    status GetShaderAsmStatsByShaderId(gtUInt64 vShaderId, gtVector<ShaderAsmStat> &vShaderStats) const;
    status GetShaderToAsmByShaderId(gtUInt64 vShaderId, gtVector<ShaderToAsm> &vShaderToAsm) const;
    status GetCounterWeights(gtVector<CounterWeight>& vCounterWeights) const;
    status GetCounterWeightForCounterByName(const gtString &vCounterName, CounterWeight &vCounterWeight) const;
    status GetWeightedSumForCounterByNameInRange(const gtString &vCounterName, gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtFloat32 &vWeightedSum) const;
    status GetWeightedSamplesForCounterByName(const gtString &vCounterName, gtVector<PPSampleData> &vSampleValues) const;
    status GetStackTraces(gtVector<StackTraceInfo>& vStackTraces) const;
    status GetStackTraceParameters(gtVector<StackTraceParameter>& vTraceParameters) const;
    status GetStackTraceParametersByCallCategoryAndAPIName(const gtString &vCallCategory, const gtString &vApiName, gtVector<StackTraceParameter> &vTraceParameters) const;
    status GetStackTraceParametersByCallCategory(const gtString &vCallCategory, gtVector<StackTraceParameter> &vTraceParameters) const;
    status GetStackTraceParametersByAPIName(const gtString &vApiName, gtVector<StackTraceParameter> &vTraceParameters) const;
    status GetNumThreadsFromTraceParameters(gtUInt64 &vNumThreads) const;
    status GetStackTraceParametersByFrameAndDrawNumber(gtUInt64 vFrameNum, gtUInt64 vDrawNum, gtVector<StackTraceParameter> &vTraceParameters) const;
    status GetRegionsOfInterest(gtVector<RegionOfInterest> &vRegionsOfInterest) const;
    status GetRegionOfInterestByRegionId(gtUInt64 vRegionId, RegionOfInterest &vRegionOfInterest) const;
    status GetAccumulatedCallSummaryTime(gtUInt64 &vAccumulatedTime) const;
    status GetCallSummaryForRegionById(gtUInt64 vRegionId, gtVector<CallSummary> &vCallSummaryList) const;
    status GetCallsPerTypeForRegionById(gtUInt64 vRegionId, CallsPerType &vCallsPerTypeList) const;
    status GetUniqueQuantizedTimesFromSamples(gtVector<gtUInt64> &vUniqueTimes) const;    
    status GetUniqueQuantizedTimesFromSamplesInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<gtUInt64> &vUniqueTimes) const;
    status GetUniqueQuantizedTimesFromSamplesByCategoryInRange(const gtString &vCounterCategory, gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<gtUInt64> &vUniqueTimes) const;
    status GetCallNamesByCategory(const gtString &vCategory, gtVector<CallCategory> &vCallCategories) const;
    status GetProfileTraceInfoByCallCategory(const gtString &vCallCategory, gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const;
    status GetProfileTraceInfoByAPIName(const gtString &vApiName, gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const;    
    status GetProfileTraceInfoByCallCategoryAndAPIName(const gtString &vCallCategory, const gtString &vApiName, gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const;
    status GetSamplesForCounterByName(const gtString& vCounterName, gtVector<PPSampleData> &vSampleValues) const;
    status GetAverageSamplesByCounterCategoryInRange(const gtString &vCounterCategory, gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<PPSampleData> &vSampleValues) const;
    status GetMaxSampleValuesByCounterCategory(const gtString &vCounterCategory, gtVector<PPSampleData> &vSampleValues) const;    
    status GetNearestSampleValueBeforeTime(gtUInt64 vTgtTime, gtInt32 vCounter, SampledValue& vrNearestSample) const;
    status GetNearestSampleValueAfterTime(gtUInt64 vTgtTime, gtInt32 vCounter, SampledValue& vrNearestSample) const;
    status GetAllDevicesAsVector(gtVector<Device> &devices) const;
    status GetEstimatedCounterFrequency(gtInt32 vCounter, double& vFreq) const;

    // Methods to write the SQLite database
    status InsertLPGPU2CounterSampleBlobs(const CounterSampleBlobsVec &blobs);
    status InsertProfileTraceInfo(const ProfileTraceInfo &vProfileTraceInfo);
    status InsertEstimatedPowerSample(const EstimatedPowerSample &vEstimatedPwrSample);
    status InsertEstimatedPowerComponent(const EstimatedPowerComponent &vEstimatedPwrComponent);
    status InsertStackTrace(const StackTraceInfo &vStackTraceInfo);
    status InsertStackTraceParameter(const StackTraceParameter &vStackTraceParam);
    status InsertAnnotation(const Annotation &vAnnotation);
    status InsertGPUTimer(const GPUTimer &vGpuTimer);
    status InsertShaderTrace(const ShaderTrace &vShaderTrace);
    status InsertShaderAsmStat(const ShaderAsmStat &vShaderStat);
    status InsertShaderToAsm(const ShaderToAsm &vShaderToAsm);
    status InsertCounterWeight(const CounterWeight &vCounterWeight);
    status InsertRegionOfInterest(const RegionOfInterest &vRegionOfInterest);
    status InsertCallSummary(const CallSummary &vCallSummary);
    status InsertCallsPerType(const CallsPerType &vCallsPerType);
    status InsertCallCategory(const CallCategory &vCallCategory);
    status InsertCounterCategory(const CounterCategory &vCounterCategory);

    status UpdateShaderTraceSource(const ShaderTrace &vShaderTrace);
    status UpdateShaderTraceAsm(const ShaderTrace &vShaderTrace);
    status UpdateShaderTraceBinary(const ShaderTrace &vShaderTrace);
    status UpdateShaderTraceDebug(const ShaderTrace &vShaderTrace);
    status UpdateShaderTraceFromMainShaderPacket(const ShaderTrace &vShaderTrace);

    status DeleteRegionsOfInterest();
    status DeleteAnnotations();
    status DeleteCallCategories();
    status DeleteCounterCategories();
    status DeleteShaderAsmStatsForShaderId(gtUInt64 vShaderId, EShaderTraceAPIId vApiId);

    bool IsTransactionActive() const;
    status BeginTransaction();        
    status EndTransaction();    
    status RollbackTransaction();

    // Methods to export the database
    bool ExportToCSV(const gtString &vOutputDirectoryName, const gtString &vFileNamePrefix, bool vbOverrideExistingFiles);

    // Methods:
private:
    // Methods to prepare the read statements
    status PrepareGetAllSamples();
    status PrepareGetProfileTraceInfoStatement();
    status PrepareGetProfileTraceInfoByCallNameStatement();
    status PrepareGetProfileTraceInfoMaxCpuEndForFrameNumStatement();
    status PrepareGetProfileTraceInfoForCallCategoryWithNumCallsAboveThreshold();
    status PrepareGetFirstTraceCallTimestampStatement();
    status PrepareGetCallNamesForRangeStatement();
    status PrepareGetFrameTimeRangeStatement();
    status PrepareGetEstimatedPowerSamplesStatement();
    status PrepareGetEstimatedPowerComponentsStatement();
    status PrepareGetAnnotationsStatement();
    status PrepareGetAnnotationByIdStatement();
    status PrepareGetAnnotationsInRangeStatement();
    status PrepareGetAnnotationsInFrameRangeStatement();
    status PrepareGetGlobalAnnotations();
    status PrepareGetUserAnnotations();
    status PrepareGetGPUTimersStatement();
    status PrepareGetGPUTimersForFrameNumInRangeStatement();
    status PrepareGetStackTracesStatement();
    status PrepareGetStackTraceParamsStatement();
    status PrepareGetStackTraceParamsForCallCategoryAndAPINameStatement();
    status PrepareGetNumThreadsFromTraceParamsStatement();
    status PrepareGetStackTraceParamsForFrameAndDrawNumStatement();
    status PrepareGetStackTraceParamsForCallCategoryStatment();
    status PrepareGetStackTraceParamsForAPINameStatement();
    status PrepareGetShaderTracesStatement();
    status PrepareGetShaderTracesByAPIIDStatement();
    status PrepareGetShaderTracesInRangeStatement();
    status PrepareGetShaderTracesInFrameRangeStatement();
    status PrepareGetShaderAsmStatByShaderId();
    status PrepareGetShaderToAsmByShaderId();
    status PrepareGetCounterWeightsStatement();
    status PrepareGetCounterWeightForCounterWithNameStatement();
    status PrepareGetWeightedSumForCounterInRangeStatement();
    status PrepareGetWeightedSamplesForCounterByNameStatement();
    status PrepareGetRegionsOfInterestStatement();
    status PrepareGetRegionOfInterestByIdStatement();
    status PrepareGetAccumulatedTimeFromCallSummaryStatement();
    status PrepareGetCallSummaryForRegionStatement();
    status PrepareGetCallsPerTypeForRegionStatement();
    status PrepareGetUniqueQuantizedTimesFromSamplesStatement();
    status PrepareGetUniqueQuantizedTimesFromSamplesInRangeStatement();
    status PrepareGetUniqueQuantizedTimesFromSamplesForCategoryInRange();
    status PrepareGetCallNamesForCategoryStatement();
    status PrepareGetProfileTraceInfoForCallCategoryStatement();
    status PrepareGetProfileTraceInfoForAPINameStatement();
    status PrepareGetProfileTraceInfoByCallCategoryAndAPINameStatement();
    status PrepareGetSampleValuesForCounterWithNameStatement();
    status PrepareGetAverageSampleValuesForCounterCategoryStatement();
    status PrepareGetMaxSampleValuesForCounterCategoryStatement();
    status PrepareGetNearestSampleValueBeforeTimeStatement();
    status PrepareGetNearestSampleValueAfterTimeStatement();
    status PrepareGetAllDevicesStatement();
    status PrepareGetFirstDifferenceForCounterStatement();

    // Methods to prepare the write statements
    status PrepareInsertLPGPU2CounterSampleBlob();
    status PrepareInsertProfileTraceInfoStatement();
    status PrepareInsertEstimatedPowerStatement();
    status PrepareInsertEstimatedPowerComponentStatement();
    status PrepareInsertStackTraceStatement();
    status PrepareInsertStackTraceParametersStatement();
    status PrepareInsertAnnotationStatement();
    status PrepareInsertGPUTimerStatement();
    status PrepareInsertShaderTraceStatement();
    status PrepareInsertShaderAsmStatStatement();
    status PrepareInsertShaderToAsmStatement();
    status PrepareInsertCounterWeightStatement();
    status PrepareInsertRegionOfInterestStatement();
    status PrepareInsertCallSummaryStatement();
    status PrepareInsertCallsPerTypeStatement();
    status PrepareInsertCallCategoryStatement();
    status PrepareInsertCounterCategoryStatement();    

    status PrepareDeleteRegionsOfInterestStatement();
    status PrepareDeleteAnnotationsStatement();
    status PrepareDeleteCallsCategoryStatement();
    status PrepareDeleteCountersCategoryStatement();
    status PrepareDeleteShaderStatsForShaderIdStatement();

    status PrepareUpdateShaderTraceSourceStatement();
    status PrepareUpdateShaderTraceAsmStatement();
    status PrepareUpdateShaderTraceBinaryStatement();
    status PrepareUpdateShaderTraceDebugStatement();
    status PrepareUpdateShaderTraceFromMainShaderStatement();

    status PrepareBeginTransactionStatement();
    status PrepareEndTransactionStatement();
    status PrepareRollbackTransactionStatement();

    // Methods to prepare all the read/write statements
    status PrepareReadStatements();
    status PrepareWriteStatements();

    // Error handling
    void AppendErrorMessage(const gtString &vMessage);
    status CheckForErrorAndReturn(const SQLiteDatabaseGuard& vDbGuard);
    status CheckForErrorAndReturn(const SQLiteStatementGuard& vStmtGuard);

    // Attributes:
private:
    // Pointers to prepared read statements
    sqlite3_stmt* m_pGetAllSamples                                            = nullptr;
    sqlite3_stmt* m_pGetProfileTraceInfoByCallNameStmt                        = nullptr;
    sqlite3_stmt* m_pGetProfileTraceInfoStmt                                  = nullptr;
    sqlite3_stmt* m_pGetProfileTraceInfoMaxCpuTimeForFrameNumStmt             = nullptr;
    sqlite3_stmt* m_pGetTracesForCallCategoryWithNumCallsAboveThresholdStmt   = nullptr;
    sqlite3_stmt* m_pGetFirstTraceCallStmt                                    = nullptr;
    sqlite3_stmt* m_pGetCallNamesInRangeStmt                                  = nullptr;
    sqlite3_stmt* m_pGetFrameTimeRangeStmt                                    = nullptr;
    sqlite3_stmt* m_pGetEstimatedPwrComponentsStmt                            = nullptr;
    sqlite3_stmt* m_pGetEstimatedPwrSamplesStmt                               = nullptr;
    sqlite3_stmt* m_pGetAnnotationsStmt                                       = nullptr;
    sqlite3_stmt* m_pGetAnnotationByIdStmt                                    = nullptr;
    sqlite3_stmt* m_pGetAnnotationsInRangeStmt                                = nullptr;
    sqlite3_stmt* m_pGetAnnotationsInFrameRangeStmt                           = nullptr;
    sqlite3_stmt* m_pGetGlobalAnnotations                                     = nullptr;
    sqlite3_stmt* m_pGetUserAnnotations                                       = nullptr;
    sqlite3_stmt* m_pGetGPUTimersStmt                                         = nullptr;
    sqlite3_stmt* m_pGetGPUTimersForFrameNumInRangeStmt                       = nullptr;
    sqlite3_stmt* m_pGetShaderTracesStmt                                      = nullptr;
    sqlite3_stmt* m_pGetShaderTracesByAPIIdStmt                               = nullptr;
    sqlite3_stmt* m_pGetStackTracesStmt                                       = nullptr;
    sqlite3_stmt* m_pGetShaderTracesInRangeStmt                               = nullptr;
    sqlite3_stmt* m_pGetShaderTracesInFrameRangeStmt                          = nullptr;
    sqlite3_stmt* m_pGetShaderStatByShaderIdStmt                              = nullptr;
    sqlite3_stmt* m_pGetShaderToAsmByShaderIdStmt                             = nullptr;
    sqlite3_stmt* m_pGetStackTraceParamsStmt                                  = nullptr;
    sqlite3_stmt* m_pGetStackTraceParamsForCallCategoryAndAPINameStmt         = nullptr;
    sqlite3_stmt* m_pGetStackTraceParamsForCallCategoryStmt                   = nullptr;
    sqlite3_stmt* m_pGetStackTraceParamsForAPINameStmt                        = nullptr;
    sqlite3_stmt* m_pGetNumThreadsFromTraceParamsStmt                         = nullptr;
    sqlite3_stmt* m_pGetStackTraceParamsForFrameAndDrawNumStmt                = nullptr;
    sqlite3_stmt* m_pGetCounterWeightsStmt                                    = nullptr;
    sqlite3_stmt* m_pGetCounterWeightForCounterByNameStmt                     = nullptr;
    sqlite3_stmt* m_pGetWeightedSumForCounterInRangeStmt                      = nullptr;
    sqlite3_stmt* m_pGetWeightedSamplesForCounterByNameStmt                   = nullptr;
    sqlite3_stmt* m_pGetRegionsOfInterestStmt                                 = nullptr;
    sqlite3_stmt* m_pGetRegionOfInterestByIdStmt                              = nullptr;
    sqlite3_stmt* m_pGetAccumulatedTimeFromCallSummaryStmt                    = nullptr;
    sqlite3_stmt* m_pGetCallSummaryForRegionStmt                              = nullptr;
    sqlite3_stmt* m_pGetCallsPerTypeForRegionStmt                             = nullptr;
    sqlite3_stmt* m_pGetUniqueQuantizedTimesFromSamplesStmt                   = nullptr;
    sqlite3_stmt* m_pGetUniqueQuantizedTimesFromSamplesInRangeStmt            = nullptr;
    sqlite3_stmt* m_pGetUniqueQuantizedTimesFromSamplesForCategoryInRangeStmt = nullptr;
    sqlite3_stmt* m_pGetCallNamesForCategoryStmt                              = nullptr;
    sqlite3_stmt* m_pGetProfileTraceInfoForCallCategoryStmt                   = nullptr;
    sqlite3_stmt* m_pGetProfileTraceInfoForAPINameStmt                        = nullptr;
    sqlite3_stmt* m_pGetProfileTraceInfoByCallCategoryAndAPINameStmt          = nullptr;
    sqlite3_stmt* m_pGetSampleValuesForCounterWithNameStmt                    = nullptr;
    sqlite3_stmt* m_pGetAverageSampleValuesForCounterCategoryStmt             = nullptr;
    sqlite3_stmt* m_pGetMaxSampleValuesForCounterCategoryStmt                 = nullptr;
    sqlite3_stmt* m_pGetNearestSampleValueBeforeTimeStmt                      = nullptr;
    sqlite3_stmt* m_pGetNearestSampleValueAfterTimeStmt                       = nullptr;
    sqlite3_stmt* m_pGetAllDevicesStmt                                        = nullptr;
    sqlite3_stmt* m_pGetFirstDiffForCounter                                   = nullptr;

    // Pointers to prepared write statements
    sqlite3_stmt* m_pInsertLPGPU2CounterSampleBlobStmt = nullptr;
    sqlite3_stmt* m_pInsertProfileInfoStmt             = nullptr;
    sqlite3_stmt* m_pInsertEstimatedPowerSampleStmt    = nullptr;
    sqlite3_stmt* m_pInsertEstimatedPowerComponentStmt = nullptr;
    sqlite3_stmt* m_pInsertAnnotationStmt              = nullptr;
    sqlite3_stmt* m_pInsertGPUTimerStmt                = nullptr;
    sqlite3_stmt* m_pInsertShaderTraceStmt             = nullptr;
    sqlite3_stmt* m_pInsertShaderStatStmt              = nullptr;
    sqlite3_stmt* m_pInsertShaderToAsmStmt             = nullptr;
    sqlite3_stmt* m_pInsertStackTraceStmt              = nullptr;
    sqlite3_stmt* m_pInsertStackTraceParameterStmt     = nullptr;
    sqlite3_stmt* m_pInsertCounterWeightStmt           = nullptr;
    sqlite3_stmt* m_pInsertRegionOfInterestStmt        = nullptr;
    sqlite3_stmt* m_pInsertCallSummaryStmt             = nullptr;
    sqlite3_stmt* m_pInsertCallsPerTypeStmt            = nullptr;
    sqlite3_stmt* m_pInsertCallCategoryStmt            = nullptr;
    sqlite3_stmt* m_pInsertCounterCategoryStmt         = nullptr;

    sqlite3_stmt* m_pDeleteRegionsOfInterestStmt      = nullptr;
    sqlite3_stmt* m_pDeleteAnnotationsStmt            = nullptr;
    sqlite3_stmt* m_pDeleteCallsCategoriesStmt        = nullptr;
    sqlite3_stmt* m_pDeleteCountersCategoriesStmt     = nullptr;
    sqlite3_stmt* m_pDeleteShaderStatsForShaderIdStmt = nullptr;

    sqlite3_stmt* m_pUpdateShaderTraceSourceStmt = nullptr;
    sqlite3_stmt* m_pUpdateShaderTraceAsmStmt = nullptr;
    sqlite3_stmt* m_pUpdateShaderTraceBinaryStmt = nullptr;
    sqlite3_stmt* m_pUpdateShaderTraceDebugStmt = nullptr;
    sqlite3_stmt* m_pUpdateShaderTraceFromMainShaderStmt = nullptr;

    sqlite3_stmt* m_pBeginTransactionStmt    = nullptr;
    sqlite3_stmt* m_pEndTransactionStmt      = nullptr;
    sqlite3_stmt* m_pRollbackTransactionStmt = nullptr;
    
    gtVector<sqlite3_stmt*> m_readStatements;
    gtVector<sqlite3_stmt*> m_writeStatements;

    // Store the error message
    gtString m_errorMsg;
};
// clang-format on

} // namespace db
} // namespace lpgpu2

#endif // LPGPU2_DB_DATABASE_ACCESSOR_H_INCLUDE
