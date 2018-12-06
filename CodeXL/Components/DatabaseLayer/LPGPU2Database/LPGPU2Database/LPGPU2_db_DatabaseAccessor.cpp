// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Database is an extension to the CodeXL's Database Layer to
///         allow customization of the CodeXL power profiling database.It
///         has the API to read and write the tables used by the LPGPU2
///         consortium
///
/// LPGPU2DatabaseAccessor implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// LPGPU2InputOutput
#include <LPGPU2InputOutput/Include/LPGPU2_io_CSVFileWriter.h>

// Local:
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAccessor.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseUtils.h>
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <algorithm>

// New tables to be created when creating a new database
const std::vector<std::string> gVecSqlCreateTableDbStatements =
{
    // This enables Foreign Keys constraints
    "PRAGMA foreign_keys = ON",

    "CREATE TABLE LPGPU2_traces (             "
    "   traceId         INTEGER NOT NULL,     "
    "   callName        TEXT,                 "
    "   cpuStart        INTEGER NOT NULL,     "
    "   cpuEnd          INTEGER NOT NULL,     "
    "   frameNum        INTEGER NOT NULL,     "
    "   drawNum         INTEGER NOT NULL,     "
    "   apiId           INTEGER NOT NULL,     "
    "   parameterOffset INTEGER NOT NULL,     "
    "   parameterLength INTEGER               "
    ");                                       ",

    // NOTE: These tables are meant to be used by TUB and THINK power estimation
    "CREATE TABLE LPGPU2_estimatedPower (sampleTime INTEGER NOT NULL, powerValue REAL, modelId INTEGER, unit TEXT, componentId INTEGER)",
    "CREATE TABLE LPGPU2_estimatedPowerComponents (componentId INTEGER NOT NULL, deviceId INTEGER NOT NULL, componentName TEXT)",

    // NOTE: These tables are meant to be used by Samsung's (SAMS) collector
    "CREATE TABLE LPGPU2_annotations (id INTEGER PRIMARY KEY, frameNum INTEGER, drawNum INTEGER, annotationType INTEGER, cpuStart INTEGER, cpuEnd INTEGER, text TEXT)",
    "CREATE TABLE LPGPU2_gpuTimer (type INTEGER, frameNum INTEGER, drawNum INTEGER, time INTEGER)",    
    "CREATE TABLE LPGPU2_stackTraces (frameNum INTEGER, drawNum INTEGER, baseAddr INTEGER, symbolAddr INTEGER, symbolName TEXT, filename TEXT)",
    "CREATE TABLE LPGPU2_traceParameters (frameNum INTEGER, drawNum INTEGER, threadId INTEGER, typedefDescription TEXT, binaryParams BLOB, paramsSize TEXT, binaryReturn BLOB, returnSize INTEGER, error INTEGER)",

    "CREATE TABLE LPGPU2_shader (     "
    "	shaderId INTEGER NOT NULL,    "
    "	apiId INTEGER NOT NULL,       "
    "	frameNum INTEGER,             " 
    "   drawNum INTEGER,              "
    "   shaderType INTEGER,           "
    "   cpuTime INTEGER,              "
    "   srcCode TEXT,                 "
    "   asmCode TEXT,                 "
    "   debug   BLOB,                 "  
    "   binary  BLOB,                 "
    "   PRIMARY KEY (shaderId, apiId) "
    ")                                ", 

    "CREATE TABLE LPGPU2_shaderAsmStats (             "
    "     shaderId INTEGER NOT NULL,                  "
    "     apiId INTEGER NOT NULL,                     "
    "     asmLine INTEGER,                            "
    "     percentage REAL,                            "
    "   FOREIGN KEY(shaderId, apiId)                  "
    "       REFERENCES LPGPU2_shader(shaderId, apiId) "
    "       ON DELETE CASCADE                         "
    ")                                                ",

    "CREATE TABLE LPGPU2_shaderToAsm (                "
    "   shaderId INTEGER NOT NULL,                    "
    "   apiId INTEGER NOT NULL,                       "
    "   shaderLine INTEGER NOT NULL,                  "
    "   asmStartLine INTEGER NOT NULL,                "
    "   asmEndLine INTEGER NOT NULL,                  "
    "   FOREIGN KEY(shaderId, apiId)                  "
    "       REFERENCES LPGPU2_shader(shaderId, apiId) "
    "       ON DELETE CASCADE                         "
    ")                                                ",

    // NOTE: These tables are meant to be used by Samsung's (SAMS) feedback analysis tool
    "CREATE TABLE LPGPU2_regionOfInterest(     "
    "   regionId          INTEGER PRIMARY KEY, "
    "   frameStart        INTEGER NOT NULL,    "
    "   frameEnd          INTEGER NOT NULL,    "
    "   degreeOfInterest  REAL NOT NULL        "
    ")                                         ",

    "CREATE TABLE LPGPU2_callSummary(                     "
    "   regionId            INTEGER NOT NULL,             "
    "   callName            TEXT INTEGER NULL,            "
    "   timesCalled         INTEGER NOT NULL,             "
    "   totalTime           INTEGER NOT NULL,             "
    "   PRIMARY KEY(regionId, callName),                  "
    "   FOREIGN KEY(regionId)                             "
    "       REFERENCES LPGPU2_regionOfInterest(regionId)  "
    "       ON DELETE CASCADE                             "
    ")                                                    ",

    "CREATE TABLE LPGPU2_callsPerType(                    "
    "    regionId               INTEGER PRIMARY KEY,      "
    "    drawPercentage         REAL NOT NULL,            "
    "    bindPercentage         REAL NOT NULL,            "
    "    setupPercentage        REAL NOT NULL,            "
    "    waitPercentage         REAL NOT NULL,            "
    "    errorPercentage        REAL NOT NULL,            "
    "    delimiterPercentage    REAL NOT NULL,            "
    "    otherPercentage        REAL NOT NULL,            "
    "    FOREIGN KEY(regionId)                            "
    "       REFERENCES LPGPU2_regionOfInterest(regionId)  "
    "       ON DELETE CASCADE                             "
    ")                                                    ",

    "CREATE TABLE LPGPU2_counterWeights (   "
    "   counterName TEXT,                   "
    "   weightValue REAL                    "
    ")                                      ",

    "CREATE TABLE LPGPU2_callsCategory(                "
    "    callId     INTEGER PRIMARY KEY AUTOINCREMENT, "
    "    callName   TEXT NOT NULL,                     "
    "    category   TEXT NOT NULL,                     "
    "    apiName    TEXT NOT NULL                      "
    ")                                                 ",

    "CREATE TABLE LPGPU2_countersCategory(  "
    "    counterId   INTEGER PRIMARY KEY,   "
    "    category    TEXT NOT NULL          "
    ")                                      ",

    "CREATE TABLE LPGPU2_samplesBlobs(  "
    "    LPGPU2Blob BLOB                "
    ")                                  ",

    // Create Indices to speedup slow queries.
    "CREATE INDEX LPGPU2_traces_frameNum_Index ON LPGPU2_traces (frameNum, drawNum)",
    "CREATE INDEX LPGPU2_traces_callName_Index ON LPGPU2_traces (callName)",
    "CREATE INDEX LPGPU2_traceParameters_frameNum ON LPGPU2_traceParameters (frameNum, drawNum)"
};

// SQL statements to read/write from the tables
#define SELECT_PROFILE_TRACE_SQL                                "SELECT traceId, callName, cpuStart, cpuEnd, frameNum, drawNum, apiId, parameterOffset, parameterLength FROM LPGPU2_traces"
#define SELECT_PROFILE_TRACE_BY_CALLNAME_SQL                    "SELECT traceId, callName, cpuStart, cpuEnd, frameNum, drawNum, apiId, parameterOffset, parameterLength FROM LPGPU2_traces WHERE callName = @callName"
#define SELECT_TRACE_MAX_CPUEND_TIME_FOR_FRAMENUM_SQL           "SELECT traceId, callName, cpuStart, MAX(cpuEnd), frameNum, drawNum, apiId, parameterOffset, parameterLength FROM LPGPU2_traces WHERE frameNum = @frameNum"
#define SELECT_FIRST_TRACE_CALL_TIMESTAMP_SQL                   "SELECT MIN(cpuStart) FROM LPGPU2_traces"
#define SELECT_FRAME_TIME_RANGE_SQL                             "SELECT MIN(cpuStart), MAX(cpuEnd) FROM LPGPU2_traces WHERE frameNum = @frameNum"
#define SELECT_CALLNAMES_FROM_TRACE_IN_RANGE_SQL                "SELECT traceId, callName, cpuStart, cpuEnd, frameNum, drawNum, apiId, parameterOffset, parameterLength FROM LPGPU2_traces WHERE cpuStart >= @cpuStart AND cpuEnd <= @cpuEnd ORDER BY cpuStart"
#define SELECT_ESTIMATED_POWER_SAMPLES_SQL                      "SELECT sampleTime, powerValue, modelId, unit, componentId FROM LPGPU2_estimatedPower"
#define SELECT_ESTIMATED_POWER_COMPONENTS_SQL                   "SELECT deviceId, componentId, componentName FROM LPGPU2_estimatedPowerComponents"
#define SELECT_ANNOTATIONS_SQL                                  "SELECT id, frameNum, drawNum, annotationType, cpuStart, cpuEnd, text FROM LPGPU2_annotations"
#define SELECT_ANNOTATION_BY_ID_SQL                             "SELECT frameNum, drawNum, annotationType, cpuStart, cpuEnd, text FROM LPGPU2_annotations WHERE id = @id"
#define SELECT_ANNOTATIONS_IN_RANGE_SQL                         "SELECT id, frameNum, drawNum, annotationType, cpuStart, cpuEnd, text FROM LPGPU2_annotations WHERE cpuStart >= @cpuStart AND cpuEnd <= @cpuEnd"
#define SELECT_ANNOTATIONS_IN_FRAME_RANGE_SQL                   "SELECT id, frameNum, drawNum, annotationType, cpuStart, cpuEnd, text FROM LPGPU2_annotations WHERE frameNum BETWEEN @frameStart AND @frameEnd"
#define SELECT_GLOBAL_ANNOTATIONS_SQL                           "SELECT id, frameNum, drawNum, annotationType, cpuStart, cpuEnd, text FROM LPGPU2_annotations WHERE annotationType BETWEEN @globalIDsStartInclusive AND @globalIDsEndInclusive"
#define SELECT_USER_ANNOTATIONS_SQL                             "SELECT id, frameNum, drawNum, annotationType, cpuStart, cpuEnd, text FROM LPGPU2_annotations WHERE annotationType BETWEEN @userIDsStartInclusive AND @userIDsEndInclusive"
#define SELECT_GPUTIMER_SQL                                     "SELECT type, frameNum, drawNum, time FROM LPGPU2_gpuTimer"
#define SELECT_GPUTIMER_FOR_FRAMENUM_IN_RANGE_SQL               "SELECT type, frameNum, drawNum, time FROM LPGPU2_gpuTimer WHERE frameNum BETWEEN @rangeStart AND @rangeEnd"
#define SELECT_SHADERTRACES_SQL                                 "SELECT shaderId, apiId, frameNum, drawNum, shaderType, cpuTime, srcCode, asmCode, debug, binary FROM LPGPU2_shader"
#define SELECT_SHADERTRACES_BY_APIID_SQL                        "SELECT shaderId, apiId, frameNum, drawNum, shaderType, cpuTime, srcCode, asmCode, debug, binary FROM LPGPU2_shader WHERE apiId = @apiId"
#define SELECT_SHADERTRACES_IN_RANGE_SQL                        "SELECT shaderId, apiId, frameNum, drawNum, shaderType, cpuTime, srcCode, asmCode, debug, binary text FROM LPGPU2_shader WHERE cpuTime BETWEEN @cpuStart AND @cpuEnd"
#define SELECT_SHADERTRACES_IN_FRAME_RANGE_SQL                  "SELECT shaderId, apiId, frameNum, drawNum, shaderType, cpuTime, srcCode, asmCode, debug, binary FROM LPGPU2_shader WHERE frameNum BETWEEN @frameStart AND @frameEnd"
#define SELECT_SHADER_ASM_STAT_BY_SHADER_ID_SQL                 "SELECT shaderId, apiId, asmLine, percentage FROM LPGPU2_shaderAsmStats WHERE shaderId = @shaderId"
#define SELECT_SHADER_TO_ASM_BY_SHADER_ID_SQL                   "SELECT shaderId, apiId, shaderLine, asmStartLine, asmEndLine FROM LPGPU2_shaderToAsm WHERE shaderId = @shaderId"
#define SELECT_STACKTRACES_SQL                                  "SELECT frameNum, drawNum, baseAddr, symbolAddr, symbolName, filename FROM LPGPU2_stackTraces"
#define SELECT_STACKTRACE_PARAMETERS_SQL                        "SELECT frameNum, drawNum, threadId, typedefDescription, binaryParams, paramsSize, binaryReturn, returnSize, error FROM LPGPU2_traceParameters"
#define SELECT_NUM_THREADS_FROM_STACKTRACE_PARAMS_SQL           "SELECT COUNT(DISTINCT threadId) FROM LPGPU2_traceParameters"
#define SELECT_TRACE_PARAMS_FOR_FRAME_AND_DRAW_NUM_SQL          "SELECT frameNum, drawNum, threadId, typedefDescription, binaryParams, paramsSize, binaryReturn, returnSize, error FROM LPGPU2_traceParameters WHERE frameNum = @frameNum AND drawNum = @drawNum"
#define SELECT_COUNTER_WEIGHTS_SQL                              "SELECT counterName, weightValue FROM LPGPU2_counterWeights"
#define SELECT_COUNTER_WEIGHT_FOR_COUNTER_SQL                   "SELECT counterName, weightValue FROM LPGPU2_counterWeights WHERE counterName = @counterName"
#define SELECT_REGIONS_OF_INTEREST_SQL                          "SELECT regionId, frameStart, frameEnd, degreeOfInterest FROM LPGPU2_regionOfInterest"
#define SELECT_REGION_OF_INTEREST_BY_ID_SQL                     "SELECT regionId, frameStart, frameEnd, degreeOfInterest FROM LPGPU2_regionOfInterest WHERE regionId = @regionId"
#define SELECT_ACCUMULATED_TIME_FROM_CALL_SUMMARY_SQL           "SELECT SUM(totalTime) FROM LPGPU2_callSummary"
#define SELECT_CALL_SUMMARY_FOR_REGION_SQL                      "SELECT regionId, callName, timesCalled, totalTime FROM LPGPU2_callSummary WHERE regionId = @regionId"
#define SELECT_CALLS_PER_TYPE_FOR_REGION_SQL                    "SELECT regionId, drawPercentage, bindPercentage, setupPercentage, waitPercentage, errorPercentage, delimiterPercentage, otherPercentage FROM LPGPU2_callsPerType WHERE regionId = @regionId"
#define SELECT_UNIQUE_QUANTIZED_TIMES_FROM_SAMPLES_SQL          "SELECT DISTINCT quantizedTimeMs FROM samples"
#define SELECT_UNIQUE_QUANTIZED_TIMES_FROM_SAMPLES_IN_RANGE_SQL "SELECT DISTINCT quantizedTimeMs FROM samples WHERE quantizedTimeMs BETWEEN @rangeStart AND @rangeEnd"

#define SELECT_UNIQUE_QUANTIZED_TIMES_FROM_SAMPLES_FOR_CATEGORY_IN_RANGE_SQL "SELECT DISTINCT quantizedTimeMs                                   "\
                                                                             "FROM samples                                                      "\
                                                                             "WHERE quantizedTimeMs BETWEEN @rangeStart AND @rangeEnd           "\
                                                                             "AND                                                               "\
                                                                             "counterId IN(                                                     "\
                                                                             "    SELECT counterId                                              "\
                                                                             "    FROM LPGPU2_countersCategory                                  "\
                                                                             "    WHERE LPGPU2_countersCategory.category = @counterCategory     "\
                                                                             ")                                                                 "

#define SELECT_CALLNAMES_FOR_CATEGORY_SQL                       "SELECT callName, category, apiName FROM LPGPU2_callsCategory WHERE category = @category"

#define SELECT_TRACES_FOR_CATEGORY_WITH_NUM_CALLS_ABOVE_THRESHOLD "SELECT traceId, callName, cpuStart, cpuEnd, frameNum, drawNum, apiId, parameterOffset, parameterLength "\
                                                                  "FROM LPGPU2_traces                                                                                     "\
                                                                  "WHERE callName IN (                                                                                    "\
                                                                  "    SELECT callName                                                                                    "\
                                                                  "    FROM LPGPU2_callsCategory                                                                          "\
                                                                  "    WHERE category = @callCategory                                                                     "\
                                                                  ")                                                                                                      "\
                                                                  "GROUP BY frameNum                                                                                      "\
                                                                  "HAVING COUNT(frameNum) > @threshold                                                                    "

#define SELECT_PROFILE_TRACE_FOR_CALL_CATEGORY_SQL     "SELECT traceId, callName, cpuStart, cpuEnd, frameNum, drawNum, apiId, parameterOffset, parameterLength "\
                                                       "FROM LPGPU2_traces                                                                                     "\
                                                       "WHERE LPGPU2_traces.callName IN (                                                                      "\
                                                       "   SELECT callName                                                                                     "\
                                                       "   FROM LPGPU2_callsCategory                                                                           "\
                                                       "   WHERE LPGPU2_callsCategory.category = @category                                                     "\
                                                       ")                                                                                                      "

#define SELECT_PROFILE_TRACE_FOR_API_NAME_SQL          "SELECT traceId, callName, cpuStart, cpuEnd, frameNum, drawNum, apiId, parameterOffset, parameterLength  "\
                                                       "FROM LPGPU2_traces                                                                                      "\
                                                       "WHERE LPGPU2_traces.callName IN (                                                                       "\
                                                        "   SELECT callName                                                                                     "\
                                                        "   FROM LPGPU2_callsCategory                                                                           "\
                                                        "   WHERE LPGPU2_callsCategory.apiName = @apiName                                                       "\
                                                        ")                                                                                                      "

#define SELECT_PROFILE_TRACE_FOR_CALL_TYPE_SQL         "SELECT traceId, callName, cpuStart, cpuEnd, frameNum, drawNum, apiId, parameterOffset, parameterLength  "\
                                                       "FROM LPGPU2_traces                                                                                      "\
                                                       "WHERE LPGPU2_traces.callName IN (                                                                       "\
                                                        "   SELECT DISTINCT callName                                                                            "\
                                                        "   FROM LPGPU2_callsCategory                                                                           "\
                                                        "   WHERE LPGPU2_callsCategory.apiName = @apiName                                                       "\
                                                        "           AND                                                                                         "\
                                                        "         LPGPU2_callsCategory.category = @callCategory                                                 "\
                                                        ")                                                                                                      "

#define SELECT_STACKTRACE_PARAMS_FOR_CALL_TYPE_SQL     "SELECT LPGPU2_traceParameters.frameNum,                           "\
                                                       "       LPGPU2_traceParameters.drawNum,                            "\
                                                       "       LPGPU2_traceParameters.threadId,                           "\
                                                       "       LPGPU2_traceParameters.typedefDescription,                 "\
                                                       "       LPGPU2_traceParameters.binaryParams,                       "\
                                                       "       LPGPU2_traceParameters.paramsSize,                         "\
                                                       "       LPGPU2_traceParameters.binaryReturn,                       "\
                                                       "       LPGPU2_traceParameters.returnSize,                         "\
                                                       "       LPGPU2_traceParameters.error                               "\
                                                       "FROM LPGPU2_traceParameters                                       "\
                                                       "JOIN LPGPU2_traces                                                "\
                                                       "ON	LPGPU2_traceParameters.frameNum = LPGPU2_traces.frameNum  "\
                                                       "    AND                                                           "\
                                                       "LPGPU2_traceParameters.drawNum = LPGPU2_traces.drawNum            "\
                                                       "WHERE LPGPU2_traces.callName IN(                                  "\
                                                       "    SELECT DISTINCT(callName)                                     "\
                                                       "    FROM LPGPU2_callsCategory                                     "\
                                                       "    WHERE LPGPU2_callsCategory.category = @callCategory           "\
                                                       "        AND                                                       "\
                                                       "    LPGPU2_callsCategory.apiName = @apiName                       "\
                                                       ")                                                                 "

#define SELECT_STACKTRACE_PARAMS_FOR_CALL_CATEGORY_SQL "SELECT LPGPU2_traceParameters.frameNum,                           "\
                                                       "       LPGPU2_traceParameters.drawNum,                            "\
                                                       "       LPGPU2_traceParameters.threadId,                           "\
                                                       "       LPGPU2_traceParameters.typedefDescription,                 "\
                                                       "       LPGPU2_traceParameters.binaryParams,                       "\
                                                       "       LPGPU2_traceParameters.paramsSize,                         "\
                                                       "       LPGPU2_traceParameters.binaryReturn,                       "\
                                                       "       LPGPU2_traceParameters.returnSize,                         "\
                                                       "       LPGPU2_traceParameters.error                               "\
                                                       "FROM LPGPU2_traceParameters                                       "\
                                                       "JOIN LPGPU2_traces                                                "\
                                                       "ON	LPGPU2_traceParameters.frameNum = LPGPU2_traces.frameNum  "\
                                                       "    AND                                                         "\
                                                       "LPGPU2_traceParameters.drawNum = LPGPU2_traces.drawNum          "\
                                                       "WHERE LPGPU2_traces.callName IN(                                "\
                                                       "    SELECT DISTINCT(callName)                                   "\
                                                       "    FROM LPGPU2_callsCategory                                   "\
                                                       "    WHERE LPGPU2_callsCategory.category = @callCategory         "\
                                                       ")                                                               "

#define SELECT_STACKTRACE_PARAMS_FOR_API_NAME_SQL      "SELECT LPGPU2_traceParameters.frameNum,                           "\
                                                       "       LPGPU2_traceParameters.drawNum,                            "\
                                                       "       LPGPU2_traceParameters.threadId,                           "\
                                                       "       LPGPU2_traceParameters.typedefDescription,                 "\
                                                       "       LPGPU2_traceParameters.binaryParams,                       "\
                                                       "       LPGPU2_traceParameters.paramsSize,                         "\
                                                       "       LPGPU2_traceParameters.binaryReturn,                       "\
                                                       "       LPGPU2_traceParameters.returnSize,                         "\
                                                       "       LPGPU2_traceParameters.error                               "\
                                                       "FROM LPGPU2_traceParameters                                       "\
                                                       "JOIN LPGPU2_traces                                                "\
                                                       "ON	LPGPU2_traceParameters.frameNum = LPGPU2_traces.frameNum  "\
                                                       "    AND                                                           "\
                                                       "LPGPU2_traceParameters.drawNum = LPGPU2_traces.drawNum            "\
                                                       "WHERE LPGPU2_traces.callName IN(                                  "\
                                                       "    SELECT DISTINCT(callName)                                     "\
                                                       "    FROM LPGPU2_callsCategory                                     "\
                                                       "    WHERE LPGPU2_callsCategory.apiName = @apiName                 "\
                                                       ")                                                                 "

#define SELECT_SAMPLES_FOR_COUNTER_WITH_NAME_SQL       "SELECT samples.counterId,                        "\
                                                       "       samples.quantizedTimeMs,                  "\
                                                       "       samples.sampledValue,                     "\
                                                       "       LPGPU2_samplesBlobs.LPGPU2Blob            "\
                                                       "FROM samples                                     "\
                                                       "INNER JOIN LPGPU2_samplesBlobs ON                "\
                                                       "     samples.ROWID = LPGPU2_samplesBlobs.ROWID   "\
                                                       "     AND                                         "\
                                                       "     counterId IN(                               "\
                                                       "         SELECT counterId                        "\
                                                       "         FROM counters                           "\
                                                       "         WHERE counterName = @counterName        "\
                                                       "     )                                           "
                                                                                                         
#define SELECT_AVERAGE_VALUES_FOR_COUNTER_CATEGORY_SQL "SELECT counterId, quantizedTimeMs, AVG(sampledValue), LPGPU2Blob  "\
                                                       "FROM samples, LPGPU2_samplesBlobs                                 "\
                                                       "WHERE counterId IN (                                              "\
                                                       "    SELECT counterId                                              "\
                                                       "    FROM LPGPU2_countersCategory                                  "\
                                                       "    WHERE category == @counterCategory                            "\
                                                       ")                                                                 "\
                                                       "AND quantizedTimeMs BETWEEN @rangeStart AND @rangeEnd             "\
                                                       "    AND samples.ROWID = LPGPU2_samplesBlobs.ROWID                 "\
                                                       "GROUP BY quantizedTimeMs                                          "

#define SELECT_MAX_VALUES_FOR_COUNTER_CATEGORY_SQL     "SELECT counterId, quantizedTimeMs, MAX(sampledValue), LPGPU2Blob   "\
                                                       "FROM samples, LPGPU2_samplesBlobs                                  "\
                                                       "WHERE samples.counterId IN (                                       "\
                                                       "   SELECT counterId                                                "\
                                                       "   FROM LPGPU2_countersCategory                                    "\
                                                       "   WHERE category = @counterCategory                               "\
                                                       ") AND samples.ROWID = LPGPU2_samplesBlobs.ROWID                    "\
                                                       "GROUP BY quantizedTimeMs                                           "
                                                       
#define SELECT_FIRST_VALUE_AFTER_TIME_FOR_COUNTER_SQL "SELECT sampledValue, MIN(quantizedTimeMs)               "\
                                                       "FROM samples                                           "\
                                                       "WHERE quantizedTimeMs >= @targetTime                   "\
                                                       "   AND counterId = @targetCounter                      "\
                                                       
#define SELECT_FIRST_VALUE_BEFORE_TIME_FOR_COUNTER_SQL  "SELECT sampledValue, MAX(quantizedTimeMs)             "\
                                                       "FROM samples                                           "\
                                                       "WHERE quantizedTimeMs <= @targetTime                   "\
                                                       "   AND counterId = @targetCounter                      "\

#define SELECT_WEIGHTED_SUM_FOR_COUNTER_IN_RANGE_SQL   "SELECT SUM(samples.sampledValue * LPGPU2_counterWeights.weightValue)"\
                                                       "FROM samples, LPGPU2_counterWeights                                 "\
                                                       "WHERE samples.quantizedTimeMs BETWEEN @rangeStart AND @rangeEnd     "\
                                                       "AND samples.counterId IN(                                           "\
                                                       "    SELECT counterId                                                "\
                                                       "    FROM counters                                                   "\
                                                       "    WHERE counterName = @counterName                                "\
                                                       ")                                                                   "

#define SELECT_WEIGHTED_SAMPLES_FOR_COUNTER_WITH_NAME_SQL "SELECT counterId, quantizedTimeMs, sampledValue * @counterWeight, LPGPU2Blob "\
                                                          "FROM samples,LPGPU2_samplesBlobs                                                     "\
                                                          "WHERE counterId = @counterId AND samples.ROWID = LPGPU2_samplesBlobs.ROWID ORDER BY samples.ROWID                                    "                                                      
#define SELECT_ALL_DEVICES_SQL "SELECT deviceId, deviceTypeId, deviceType, deviceName, deviceDescription FROM devices"
#define SELECT_ALL_SAMPLES_SQL "SELECT counterId, quantizedTimeMs, sampledValue, LPGPU2Blob "\
"FROM samples, LPGPU2_samplesBlobs                                                     "\
"WHERE samples.ROWID = LPGPU2_samplesBlobs.ROWID ORDER BY samples.ROWID"

#define SELECT_FIRST_DIFFERENCE_FOR_COUNTER_SQL "SELECT (SELECT quantizedTimeMs FROM samples WHERE counterId = @counterId LIMIT 1 OFFSET 1)"\
                                                "- (SELECT quantizedTimeMs FROM samples  WHERE counterId = @counterId LIMIT 1 OFFSET 0)"

#define INSERT_PROFILE_TRACE_INFO_SQL         "INSERT INTO LPGPU2_traces (traceId, callName, cpuStart, cpuEnd, frameNum, drawNum, apiId, parameterOffset, parameterLength) VALUES (@traceId, @callName, @cpuStart, @cpuEnd, @frameNum, @drawNum, @apiId, @parameterOffset, @parameterLength)"
#define INSERT_ESTIMATED_POWER_SAMPLE_SQL     "INSERT INTO LPGPU2_estimatedPower (sampleTime, powerValue, modelId, componentId, unit) VALUES(@sampleTime, @powerValue, @modelId, @componentId, @unit)"
#define INSERT_ESTIMATED_POWER_COMPONENT_SQL  "INSERT INTO LPGPU2_estimatedPowerComponents (componentId, deviceId, componentName) VALUES (@componentId, @deviceId, @componentName)"
#define INSERT_ANNOTATION_SQL                 "INSERT INTO LPGPU2_annotations (frameNum, drawNum, annotationType, cpuStart, cpuEnd, text) VALUES (@frameNum, @drawNum, @annotationType, @cpuStart, @cpuEnd, @text)"
#define INSERT_GPUTIMER_SQL                   "INSERT INTO LPGPU2_gpuTimer (type, frameNum, drawNum, time) VALUES (@type, @frameNum, @drawNum, @time)"
#define INSERT_SHADERTRACE_SQL                "INSERT INTO LPGPU2_shader (shaderId, apiId, frameNum, drawNum, shaderType, cpuTime, srcCode, asmCode, debug, binary) VALUES (@shaderId, @apiId, @frameNum, @drawNum, @shaderType, @cpuTime, @srcCode, @asmCode, @debug, @binary)"
#define INSERT_SHADER_STAT_SQL                "INSERT INTO LPGPU2_shaderAsmStats (shaderId, apiId, asmLine, percentage) VALUES (@shaderId, @apiId, @asmLine, @percentage)"
#define INSERT_SHADER_TO_ASM_SQL              "INSERT INTO LPGPU2_shaderToAsm (shaderId, apiId, shaderLine, asmStartLine, asmEndLine) VALUES (@shaderId, @apiId, @shaderLine, @asmStartLine, @asmEndLine)"
#define INSERT_STACKTRACE_SQL                 "INSERT INTO LPGPU2_stackTraces (frameNum, drawNum, baseAddr, symbolAddr, symbolName, filename) VALUES (@frameNum, @drawNum, @baseAddr, @symbolAddr, @symbolName, @filename)"
#define INSERT_STACKTRACE_PARAMS_SQL          "INSERT INTO LPGPU2_traceParameters (frameNum, drawNum, threadId, typedefDescription, binaryParams, paramsSize, binaryReturn, returnSize, error) VALUES (@frameNum, @drawNum, @threadId, @typedefDescription, @binaryParams, @paramsSize, @binaryReturn, @returnSize, @error)"
#define INSERT_COUNTER_WEIGHT_SQL             "INSERT INTO LPGPU2_counterWeights (counterName, weightValue) VALUES (@counterName, @weightValue)"
#define INSERT_REGION_OF_INTEREST_SQL         "INSERT INTO LPGPU2_regionOfInterest (regionId, frameStart, frameEnd, degreeOfInterest) VALUES (@regionId, @frameStart, @frameEnd, @degreeOfInterest)"
#define INSERT_CALL_SUMMARY_SQL               "INSERT INTO LPGPU2_callSummary (regionId, callName, timesCalled, totalTime) VALUES (@regionId, @callName, @timesCalled, @totalTime)"
#define INSERT_CALLS_PER_TYPE_SQL             "INSERT INTO LPGPU2_callsPerType (regionId, drawPercentage, bindPercentage, setupPercentage, waitPercentage, errorPercentage, delimiterPercentage, otherPercentage) "\
                                              "    VALUES (@regionId, @drawPercentage, @bindPercentage, @setupPercentage, @waitPercentage, @errorPercentage, @delimiterPercentage, @otherPercentage)"
#define INSERT_CALL_CATEGORY_SQL              "INSERT INTO LPGPU2_callsCategory (callName, category, apiName) VALUES (@callName, @category, @apiName)"
#define INSERT_COUNTER_CATEGORY_SQL           "INSERT INTO LPGPU2_countersCategory (counterId, category) VALUES (@counterId, @category)"
#define INSERT_LPGPU2_COUNTER_SAMPLE_BLOB_SQL "INSERT INTO LPGPU2_samplesBlobs (LPGPU2Blob) VALUES (@LPGPU2Blob)"

#define UPDATE_SHADERTRACE_SOURCE_SQL         "UPDATE LPGPU2_shader SET srcCode = @srcCode WHERE shaderId = @shaderId AND apiId = @apiId"
#define UPDATE_SHADERTRACE_ASM_SQL            "UPDATE LPGPU2_shader SET asmCode = @asmCode WHERE shaderId = @shaderId AND apiId = @apiId"
#define UPDATE_SHADERTRACE_BINARY_SQL         "UPDATE LPGPU2_shader SET binary = @binary WHERE shaderId = @shaderId AND apiId = @apiId"
#define UPDATE_SHADERTRACE_DEBUG_SQL          "UPDATE LPGPU2_shader SET debug = @debug WHERE shaderId = @shaderId AND apiId = @apiId"
#define UPDATE_SHADERTRACEFROMMAIN_SOURCE_SQL "UPDATE LPGPU2_shader SET srcCode = @srcCode, cpuTime = @cpuTime, frameNum = @frameNum, drawNum = @drawNum WHERE shaderId = @shaderId AND apiId = @apiId"

#define DELETE_REGIONS_OF_INTEREST_SQL        "DELETE FROM LPGPU2_regionOfInterest"
#define DELETE_ANNOTATIONS_SQL                "DELETE FROM LPGPU2_annotations"
#define DELETE_CALLS_CATEGORY_SQL             "DELETE FROM LPGPU2_callsCategory"
#define DELETE_COUNTERS_CATEGORY_SQL          "DELETE FROM LPGPU2_countersCategory"
#define DELETE_SHADER_STATS_BY_SHADER_ID_SQL  "DELETE FROM LPGPU2_shaderAsmStats WHERE shaderId = @shaderId AND apiId = @apiId"

#define BEGIN_TRANSACTION_SQL                 "BEGIN TRANSACTION"
#define COMMIT_TRANSACTION_SQL                "COMMIT TRANSACTION"
#define ROLLBACK_TRANSACTION_SQL              "ROLLBACK TRANSACTION"

namespace lpgpu2 {
namespace db {

/// @brief  Class constructor. No work is done here by this class.
LPGPU2DatabaseAccessor::LPGPU2DatabaseAccessor() 
: AmdtDatabaseAccessor()
{
}

/// @brief  Class destructor. All the statements used by this class are finalized.
LPGPU2DatabaseAccessor::~LPGPU2DatabaseAccessor()
{
    SQLiteDatabaseGuard readDbGuard{ GetDatabaseReadConnection() };
    if (readDbGuard.IsValid())
    {
        for(auto *stmt : m_readStatements)
        {
            readDbGuard.Finalize(stmt);
        }        
    }

    SQLiteDatabaseGuard writeDbGuard{ GetDatabaseWriteConnection() };
    if (writeDbGuard.IsValid())
    {
        for(auto *stmt : m_writeStatements)
        {
            writeDbGuard.Finalize(stmt);
        }
    }
}


/// @brief              Returns the error vMessage in case of failure in any 
///                     method call.
/// @brief  gtString    The error vMessage, if any.
const gtString& LPGPU2DatabaseAccessor::GetErrorMessage() const
{
    return m_errorMsg;
}

/// @brief                      Create a new database and prepare all the write
///                             statements to write data into the database.
/// @param[in]  vDatabaseName   File path of the database file to be created.
/// @param[in]  vProfileType    The mode to open the database, should be one of AMDTProfileMode.
/// @returns    status          true  = the database file was created,
///                             false = an error has occurred during the creation of the database file
status LPGPU2DatabaseAccessor::CreateProfilingDatabase(const gtString &vDatabaseName, gtUInt64 vProfileType)
{
    status bReturn = success;

    GT_IF_WITH_ASSERT_EX(AmdtDatabaseAccessor::CreateProfilingDatabase(vDatabaseName, vProfileType), L"Cannot create profiling database")
    {
        SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

        for (const auto& createStmt : gVecSqlCreateTableDbStatements)
        {
            SQLiteStatementGuard stmtGuard{ dbGuard.Prepare(createStmt) };
            bReturn &= stmtGuard.Exec();
            bReturn &= stmtGuard.Finalize();

            if (!bReturn)
            {
                AppendErrorMessage(stmtGuard.GetErrorMessage());
            }

        }

        bReturn &= PrepareReadStatements();
        bReturn &= PrepareWriteStatements();
    }
    else
    {
        AppendErrorMessage(L"Error: Cannot create profiling database");
    }

    return bReturn;
}

/// @brief                    Open an existing database and prepare all the
///                           statements for reading and/or writing. The
///                           database can be opened in read only mode.
/// @param[in]  vDatabaseName File path of the database file to be opened.
/// @param[in]  vProfileType  The mode to open the database, should be one 
///                           of AMDTProfileMode.
/// @param[in]  vbIsReadOnly  If true opens the database in read only mode, 
///                           no write statement is prepared.
/// @returns    status        success = the database file was opened,
///                           failure = an error has occurred during opening the database file.
status LPGPU2DatabaseAccessor::OpenProfilingDatabase(const gtString &vDatabaseName, gtUInt64 vProfileType, bool vbIsReadOnly)
{
    auto bReturn = failure;

    GT_IF_WITH_ASSERT_EX(AmdtDatabaseAccessor::OpenProfilingDatabase(vDatabaseName, vProfileType, vbIsReadOnly), L"Cannot open profiling database")
    {
        bReturn = PrepareReadStatements();

        if (!bReturn)
        { 
            AppendErrorMessage(L"Error: Cannot prepare read statements");      
        }

        if (!vbIsReadOnly)
        {
            bReturn &= PrepareWriteStatements();

            if (!bReturn)
            {
                AppendErrorMessage(L"Error: Cannot prepare write statements");
            }
        }
    }

    return bReturn;
}

/// @brief                        Get all the lines in the database's sample table.
/// @param[out] vSamples A list with PPSampleData objects representing a sample.
/// @returns   status             success = the information was retrieved,
///                               failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetAllSamples(gtVector<PPSampleData> &vSamples) const
{
  auto bReturn = failure;

  SQLiteStatementGuard stmtGuard{ m_pGetAllSamples };

  while (stmtGuard.Step())
  {
    // counterId, quantizedTimeMs, sampleValue * counterWeight, blob
    auto counterId = stmtGuard.Get<int>(0);
    auto quantizedTimeMs = stmtGuard.Get<int>(1);
    auto avgSampleValue = stmtGuard.Get<double>(2);
    auto blob = stmtGuard.Get<PPSampleData::BlobVec>(3);

    vSamples.emplace_back(quantizedTimeMs, counterId, avgSampleValue, std::move(blob));

    // If at least one item is pushed into the result vector returns true
    bReturn = success;
  }

  return bReturn;
}

/// @brief                        Get all the lines in the database's trace table.
/// @param[out] vProfileTraceInfo A list with ProfileTraceInfo objects representing a trace.
/// @returns   status             success = the information was retrieved,
///                               failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetProfileTraceInfo(gtVector<ProfileTraceInfo> &vProfileTraceInfo) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetProfileTraceInfoStmt };

    while (stmtGuard.Step())
    {
        ProfileTraceInfo traceInfo;
        traceInfo.m_traceId = stmtGuard.Get<gtUInt64>(0);
        traceInfo.m_callName = stmtGuard.Get<gtString>(1);
        traceInfo.m_cpuStart = stmtGuard.Get<gtUInt64>(2);
        traceInfo.m_cpuEnd = stmtGuard.Get<gtUInt64>(3);
        traceInfo.m_frameNum = stmtGuard.Get<gtUInt64>(4);
        traceInfo.m_drawNum = stmtGuard.Get<gtUInt64>(5);
        traceInfo.m_apiId = stmtGuard.Get<gtUInt64>(6);
        traceInfo.m_parameterOffset = stmtGuard.Get<gtUInt64>(7);
        traceInfo.m_parameterLength = stmtGuard.Get<gtUInt64>(8);

        vProfileTraceInfo.push_back(traceInfo);

        // If at least one item is pushed into the result vector returns true
        bReturn = success;
    }

    return bReturn;
}

/// @brief                        Get all the lines in the database's trace table that
///                               match the vCallName column with the vCallName.
/// @param[in]  vCallName         Name of the function to filter.
/// @param[out] vProfileTraceInfo A list with ProfileTraceInfo objects representing a trace.
/// @returns   status             success = the information was retrieved,
///                               failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetProfileTraceInfoByCallName(const gtString &vCallName, gtVector<ProfileTraceInfo> &vProfileTraceInfo) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetProfileTraceInfoByCallNameStmt };

    stmtGuard.Bind("@callName", vCallName);

    while (stmtGuard.Step())
    {
        ProfileTraceInfo traceInfo;
        traceInfo.m_traceId = stmtGuard.Get<gtUInt64>(0);
        traceInfo.m_callName = stmtGuard.Get<gtString>(1);
        traceInfo.m_cpuStart = stmtGuard.Get<gtUInt64>(2);
        traceInfo.m_cpuEnd = stmtGuard.Get<gtUInt64>(3);
        traceInfo.m_frameNum = stmtGuard.Get<gtUInt64>(4);
        traceInfo.m_drawNum = stmtGuard.Get<gtUInt64>(5);
        traceInfo.m_apiId = stmtGuard.Get<gtUInt64>(6);
        traceInfo.m_parameterOffset = stmtGuard.Get<gtUInt64>(7);
        traceInfo.m_parameterLength = stmtGuard.Get<gtUInt64>(8);

        vProfileTraceInfo.push_back(traceInfo);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                         Gets the ProfileTraceInfo with the maximum value for cpuEnd. 
///                                This will return a ProfileTraceInfo object that matches the search criteria.
/// @param[in]  vFrameNum          The frameNum to look for.
/// @param[out] vProfileTraceInfo  The ProfileTraceInfo with the max cpuEnd time.
/// @returns    status             success = the information was retrieved,
///                                failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetProfileTraceInfoMaxCpuEndTimeByFrameNumber(gtUInt64 vFrameNum, ProfileTraceInfo &vProfileTraceInfo) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetProfileTraceInfoMaxCpuTimeForFrameNumStmt };

    stmtGuard.Bind("@frameNum", vFrameNum);

    if (stmtGuard.Step())
    {
        vProfileTraceInfo.m_traceId = stmtGuard.Get<gtUInt64>(0);
        vProfileTraceInfo.m_callName = stmtGuard.Get<gtString>(1);
        vProfileTraceInfo.m_cpuStart = stmtGuard.Get<gtUInt64>(2);
        vProfileTraceInfo.m_cpuEnd = stmtGuard.Get<gtUInt64>(3);
        vProfileTraceInfo.m_frameNum = stmtGuard.Get<gtUInt64>(4);
        vProfileTraceInfo.m_drawNum = stmtGuard.Get<gtUInt64>(5);
        vProfileTraceInfo.m_apiId = stmtGuard.Get<gtUInt64>(6);
        vProfileTraceInfo.m_parameterOffset = stmtGuard.Get<gtUInt64>(7);
        vProfileTraceInfo.m_parameterLength = stmtGuard.Get<gtUInt64>(8);

        bReturn = success;
    }

    return bReturn;
}

status LPGPU2DatabaseAccessor::GetProfileTraceInfoByCallCategoryWithNumberOfCallsAboveThreshold(const gtString &vCallCategory, gtUInt64 vThreshold, gtVector<ProfileTraceInfo> &vProfileTraceInfo) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetTracesForCallCategoryWithNumCallsAboveThresholdStmt };

    stmtGuard.Bind("@callCategory", vCallCategory);
    stmtGuard.Bind("@threshold", vThreshold);

    while (stmtGuard.Step())
    {
        ProfileTraceInfo traceInfo;
        traceInfo.m_traceId = stmtGuard.Get<gtUInt64>(0);
        traceInfo.m_callName = stmtGuard.Get<gtString>(1);
        traceInfo.m_cpuStart = stmtGuard.Get<gtUInt64>(2);
        traceInfo.m_cpuEnd = stmtGuard.Get<gtUInt64>(3);
        traceInfo.m_frameNum = stmtGuard.Get<gtUInt64>(4);
        traceInfo.m_drawNum = stmtGuard.Get<gtUInt64>(5);
        traceInfo.m_apiId = stmtGuard.Get<gtUInt64>(6);
        traceInfo.m_parameterOffset = stmtGuard.Get<gtUInt64>(7);
        traceInfo.m_parameterLength = stmtGuard.Get<gtUInt64>(8);

        vProfileTraceInfo.push_back(traceInfo);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                         Get the first timestamp available from the LPGPU2_traces table
/// @param[out] vCpuStartTimestamp The first timestamp available
/// @returns    status             success = the information was retrieved,
///                                failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetFirstTraceCallTimestamp(gtUInt64 &vCpuStartTimestamp) const
{
    auto bReturn = false;

    SQLiteStatementGuard stmtGuard{ m_pGetFirstTraceCallStmt };

    if (stmtGuard.Step())
    {
        vCpuStartTimestamp = stmtGuard.Get<gtUInt64>(0);
        bReturn = true;
    }

    return bReturn;
}

/// @brief                        Retrieves a list of ProfileTraceInfo where cpuStart >= vRangeStart
///                               and cpuEnd <= vRangeEnd.
/// @param[in]  vRangeStart       The start of the range to query for profile traces.
/// @param[in]  vRangeEnd         The end of the range to query for profile traces.
/// @param[out] vProfileTraceInfo The list of ProfileTraceInfo resulted from the query.
/// @returns    status            success = the information was retrieved,
///                               failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetProfileTraceInfoInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<ProfileTraceInfo> &vProfileTraceInfo) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetCallNamesInRangeStmt };

    stmtGuard.Bind("@cpuStart", vRangeStart);
    stmtGuard.Bind("@cpuEnd", vRangeEnd);

    while (stmtGuard.Step())
    {
        ProfileTraceInfo traceInfo;
        traceInfo.m_traceId         = stmtGuard.Get<gtUInt64>(0);
        traceInfo.m_callName        = stmtGuard.Get<gtString>(1);
        traceInfo.m_cpuStart        = stmtGuard.Get<gtUInt64>(2);
        traceInfo.m_cpuEnd          = stmtGuard.Get<gtUInt64>(3);
        traceInfo.m_frameNum        = stmtGuard.Get<gtUInt64>(4);
        traceInfo.m_drawNum         = stmtGuard.Get<gtUInt64>(5);
        traceInfo.m_apiId           = stmtGuard.Get<gtUInt64>(6);
        traceInfo.m_parameterOffset = stmtGuard.Get<gtUInt64>(7);
        traceInfo.m_parameterLength = stmtGuard.Get<gtUInt64>(8);        

        vProfileTraceInfo.push_back(traceInfo);
        bReturn = success;
    }

    return bReturn;
}

status LPGPU2DatabaseAccessor::GetFrameTimeRange(const gtUInt64 vFrameNum, gtUInt64& vStartTime, gtUInt64& vEndTime) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetFrameTimeRangeStmt };

    stmtGuard.Bind("@frameNum", vFrameNum);

    while(stmtGuard.Step())
    {
        // @frameStart, @frameEnd
        vStartTime = stmtGuard.Get<gtUInt64>(0);
        vEndTime = stmtGuard.Get<gtUInt64>(1);

        bReturn = success;
    }

    return bReturn;
}

    /// @brief                  Get all the lines in the database's LPGPU2_estimatedPower table.
/// @param[out] vPwrSamples A list with EstimatedPowerSample objects representing
///                         a line in LPGPU2_estimatedPower table.
/// @returns   status       success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetEstimatedPowerSamples(gtVector<EstimatedPowerSample> &vPwrSamples) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetEstimatedPwrSamplesStmt };

    while (stmtGuard.Step())
    {
        // sampleTime, powerValue, modelId, unit, vComponentId
        EstimatedPowerSample pwrSample;
        pwrSample.m_sampleTime = stmtGuard.Get<gtUInt64>(0);
        pwrSample.m_estimatedPower = stmtGuard.Get<gtFloat32>(1);
        pwrSample.m_modelId = stmtGuard.Get<gtUInt64>(2);
        pwrSample.m_unit = stmtGuard.Get<gtString>(3);
        pwrSample.m_componentId = stmtGuard.Get<gtUInt64>(4);

        vPwrSamples.push_back(pwrSample);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                            Get a map from the table LPGPU2_estimatedPowerComponents
///                                   mapping a list of components per device.
/// param[out] vPwrComponentsByDevice A map with components per device.
/// @returns   status                 success = the information was retrieved,
///                                   failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetEstimatedPowerComponents(gtMap<gtUInt64, gtVector<EstimatedPowerComponent>> &vPwrComponentsByDevice) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetEstimatedPwrComponentsStmt };

    while (stmtGuard.Step())
    {
        // deviceId, componentId, componentName
        EstimatedPowerComponent pwrComponent;
        pwrComponent.m_deviceId = stmtGuard.Get<gtUInt64>(0);;
        pwrComponent.m_componentId = stmtGuard.Get<gtUInt64>(1);
        pwrComponent.m_componentName = stmtGuard.Get<gtString>(2);

        vPwrComponentsByDevice[pwrComponent.m_deviceId].push_back(pwrComponent);
    }

    return bReturn;
}

/// @brief                   Get all the lines in the database's LPGPU2_annotations table.
/// @param[out] vAnnotations A list with Annotation objects representing a
///                          line in the LPGPU2_annotations table.
/// @returns   status        success = the information was retrieved,
///                          failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetAnnotations(gtVector<Annotation> &vAnnotations) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetAnnotationsStmt };

    if (stmtGuard.Step())
    {
        // id, frameNum, drawNum, annotationType, cpuStart, cpuEnd, text
        Annotation annotation;
        annotation.m_annotationId = stmtGuard.Get<gtUInt64>(0);
        annotation.m_frameNum = stmtGuard.Get<gtUInt64>(1);
        annotation.m_drawNum = stmtGuard.Get<gtUInt64>(2);
        annotation.m_annotationType = stmtGuard.Get<gtUInt64>(3);
        annotation.m_cpuStartTime = stmtGuard.Get<gtUInt64>(4);
        annotation.m_cpuEndTime = stmtGuard.Get<gtUInt64>(5);
        annotation.m_text = stmtGuard.Get<gtString>(6);

        vAnnotations.push_back(annotation);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                   Get the line in  database's LPGPU2_annotations table.
///                          that matches the vAnnotationId.
/// @param[out] vAnnotation  The Annotation object that matches the vAnnotationId.
/// @returns   status        success = the information was retrieved,
///                          failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetAnnotationById(gtUInt64 vAnnotationId, Annotation &vAnnotation) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetAnnotationByIdStmt };

    stmtGuard.Bind("@id", vAnnotationId);

    if (stmtGuard.Step())
    {
        // frameNum, drawNum, annotationType, cpuStart, cpuEnd, text
        vAnnotation.m_annotationId = vAnnotationId;
        vAnnotation.m_frameNum = stmtGuard.Get<gtUInt64>(0);
        vAnnotation.m_drawNum = stmtGuard.Get<gtUInt64>(1);
        vAnnotation.m_annotationType = stmtGuard.Get<gtUInt64>(2);
        vAnnotation.m_cpuStartTime = stmtGuard.Get<gtUInt64>(3);
        vAnnotation.m_cpuEndTime = stmtGuard.Get<gtUInt64>(4);
        vAnnotation.m_text = stmtGuard.Get<gtString>(5);

        bReturn = success;
    }

    return bReturn;
}

status LPGPU2DatabaseAccessor::GetAnnotationsInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<Annotation> &vAnnotations) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetAnnotationsInRangeStmt };

    stmtGuard.Bind("@cpuStart", vRangeStart);
    stmtGuard.Bind("@cpuEnd", vRangeEnd);

    while (stmtGuard.Step())
    {
        // id, frameNum, drawNum, annotationType, cpuStart, cpuEnd, text
        Annotation annotation;
        annotation.m_annotationId = stmtGuard.Get<gtUInt64>(0);
        annotation.m_frameNum = stmtGuard.Get<gtUInt64>(1);
        annotation.m_drawNum = stmtGuard.Get<gtUInt64>(2);
        annotation.m_annotationType = stmtGuard.Get<gtUInt64>(3);
        annotation.m_cpuStartTime = stmtGuard.Get<gtUInt64>(4);
        annotation.m_cpuEndTime = stmtGuard.Get<gtUInt64>(5);
        annotation.m_text = stmtGuard.Get<gtString>(6);

        vAnnotations.push_back(annotation);
        bReturn = success;
    }

    return bReturn;
}

status LPGPU2DatabaseAccessor::GetAnnotationsInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<Annotation> &vAnnotations) const
{
    // Default to success as it is possible to have no annotations in a range
    auto bReturn = success;

    SQLiteStatementGuard stmtGuard{ m_pGetAnnotationsInFrameRangeStmt };

    stmtGuard.Bind("@frameStart", vFrameStart);
    stmtGuard.Bind("@frameEnd", vFrameEnd);

    while (stmtGuard.Step())
    {
        // id, frameNum, drawNum, annotationType, cpuStart, cpuEnd, text
        Annotation annotation;
        annotation.m_annotationId = stmtGuard.Get<gtUInt64>(0);
        annotation.m_frameNum = stmtGuard.Get<gtUInt64>(1);
        annotation.m_drawNum = stmtGuard.Get<gtUInt64>(2);
        annotation.m_annotationType = stmtGuard.Get<gtUInt64>(3);
        annotation.m_cpuStartTime = stmtGuard.Get<gtUInt64>(4);
        annotation.m_cpuEndTime = stmtGuard.Get<gtUInt64>(5);
        annotation.m_text = stmtGuard.Get<gtString>(6);

        vAnnotations.push_back(annotation);
        bReturn = success;
    }

    return bReturn;
}

status LPGPU2DatabaseAccessor::GetGlobalAnnotations(
  gtVector<Annotation> &vAnnotations) const
{
    // Default to success as it is possible to have no annotations in a range
    auto bReturn = success;

    SQLiteStatementGuard stmtGuard{ m_pGetGlobalAnnotations };

    stmtGuard.Bind("@globalIDsStartInclusive",
        static_cast<unsigned int>(kGlobalAnnotationStartID));
    stmtGuard.Bind("@globalIDsEndInclusive",
        static_cast<unsigned int>(kGlobalAnnotationEndID));

    while (stmtGuard.Step())
    {
        // id, frameNum, drawNum, annotationType, cpuStart, cpuEnd, text
        Annotation annotation;
        annotation.m_annotationId = stmtGuard.Get<gtUInt64>(0);
        annotation.m_frameNum = stmtGuard.Get<gtUInt64>(1);
        annotation.m_drawNum = stmtGuard.Get<gtUInt64>(2);
        annotation.m_annotationType = stmtGuard.Get<gtUInt64>(3);
        annotation.m_cpuStartTime = stmtGuard.Get<gtUInt64>(4);
        annotation.m_cpuEndTime = stmtGuard.Get<gtUInt64>(5);
        annotation.m_text = stmtGuard.Get<gtString>(6);

        vAnnotations.push_back(annotation);
        bReturn = success;
    }

    return bReturn;

}

status LPGPU2DatabaseAccessor::GetUserAnnotations(
  gtVector<Annotation> &vAnnotations) const
{
    // Default to success as it is possible to have no annotations in a range
    auto bReturn = success;

    SQLiteStatementGuard stmtGuard{ m_pGetUserAnnotations };

    stmtGuard.Bind("@userIDsStartInclusive",
        static_cast<unsigned int>(kUserAnnotationStartID));
    stmtGuard.Bind("@userIDsEndInclusive",
        static_cast<unsigned int>(kUserAnnotationEndID));

    while (stmtGuard.Step())
    {
        // id, frameNum, drawNum, annotationType, cpuStart, cpuEnd, text
        Annotation annotation;
        annotation.m_annotationId = stmtGuard.Get<gtUInt64>(0);
        annotation.m_frameNum = stmtGuard.Get<gtUInt64>(1);
        annotation.m_drawNum = stmtGuard.Get<gtUInt64>(2);
        annotation.m_annotationType = stmtGuard.Get<gtUInt64>(3);
        annotation.m_cpuStartTime = stmtGuard.Get<gtUInt64>(4);
        annotation.m_cpuEndTime = stmtGuard.Get<gtUInt64>(5);
        annotation.m_text = stmtGuard.Get<gtString>(6);

        vAnnotations.push_back(annotation);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                 Get all the lines in the database's LPGPU2_gpuTimers table
/// @param[out] vGpuTimers A list with GPUTimer objects representing a line
///                        in the LPGPU2_gpuTimers table
/// @returns   status      success = the information was retrieved,
///                        failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetGPUTimers(gtVector<GPUTimer> &vGpuTimers) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetGPUTimersStmt };

    while (stmtGuard.Step())
    {
        // type, frameNum, drawNum, time
        GPUTimer gpuTimer;
        gpuTimer.m_type = stmtGuard.Get<gtUInt64>(0);
        gpuTimer.m_frameNum = stmtGuard.Get<gtUInt64>(1);
        gpuTimer.m_drawNum = stmtGuard.Get<gtUInt64>(2);
        gpuTimer.m_time = stmtGuard.Get<gtUInt64>(3);

        vGpuTimers.push_back(gpuTimer);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                  Gets the list of GPUTimer objects from the database where the frameNum
///                         is between the informed range from the LPGPU2_gpuTimer table.
/// @param[in]  vRangeStart The start of the frameNum range to query.
/// @param[in]  vRangeEnd   The end of the frameNum range to query.
/// @param[out] vGpuTimers  A list of GPUTimer objects.
/// @returns    status      success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetGPUTimersInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<GPUTimer>& vGpuTimers) const
{
    bool bReturn = false;

    SQLiteStatementGuard stmtGuard{ m_pGetGPUTimersForFrameNumInRangeStmt };

    stmtGuard.Bind("@rangeStart", vRangeStart);
    stmtGuard.Bind("@rangeEnd", vRangeEnd);

    while (stmtGuard.Step())
    {
        // type, frameNum, drawNum, time
        GPUTimer gpuTimer;
        gpuTimer.m_type = stmtGuard.Get<gtUInt64>(0);
        gpuTimer.m_frameNum = stmtGuard.Get<gtUInt64>(1);
        gpuTimer.m_drawNum = stmtGuard.Get<gtUInt64>(2);
        gpuTimer.m_time = stmtGuard.Get<gtUInt64>(3);

        vGpuTimers.push_back(gpuTimer);
        bReturn = true;
    }

    return bReturn;
}

/// @brief                    Get all the lines in the database's LPGPU2_shader table.
/// @param[out] vShaderTraces A list with ShaderTrace objects representing a line
///                           in the LPGPU2_shader table
/// @returns   status         success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetShaderTraces(gtVector<ShaderTrace> &vShaderTraces) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetShaderTracesStmt };

    while (stmtGuard.Step())
    {
        // shaderId, apiId, frameNum, drawNum, shaderType, cpuTime, srcCode, asmCode, debug, binary
        ShaderTrace shaderTrace;
        shaderTrace.m_shaderId = stmtGuard.Get<gtUInt64>(0);
        shaderTrace.m_apiId = static_cast<EShaderTraceAPIId>(stmtGuard.Get<gtUInt32>(1));
        shaderTrace.m_frameNum = stmtGuard.Get<gtUInt32>(2);
        shaderTrace.m_drawNum = stmtGuard.Get<gtUInt32>(3);        
        shaderTrace.m_shaderType = stmtGuard.Get<gtUInt32>(4);
        shaderTrace.m_cpuTime = stmtGuard.Get<gtUInt32>(5);
        shaderTrace.m_srcCode = stmtGuard.Get<gtString>(6);
        shaderTrace.m_asmCode = stmtGuard.Get<gtString>(7);
        shaderTrace.m_debug = stmtGuard.Get<gtVector<gtUByte>>(8);
        shaderTrace.m_binary = stmtGuard.Get<gtVector<gtUByte>>(9);

        vShaderTraces.push_back(shaderTrace);
        bReturn = success;
    }

    return bReturn;
}

status LPGPU2DatabaseAccessor::GetShaderTracesInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<ShaderTrace> &vShaderTraces) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetShaderTracesInRangeStmt };

    stmtGuard.Bind("@cpuStart", vRangeStart);
    stmtGuard.Bind("@cpuEnd", vRangeEnd);

    while (stmtGuard.Step())
    {
        // shaderId, apiId, frameNum, drawNum, shaderType, cpuTime, srcCode, asmCode, debug, binary
        ShaderTrace shaderTrace;
        shaderTrace.m_shaderId = stmtGuard.Get<gtUInt64>(0);
        shaderTrace.m_apiId = static_cast<EShaderTraceAPIId>(stmtGuard.Get<gtUInt32>(1));
        shaderTrace.m_frameNum = stmtGuard.Get<gtUInt32>(2);
        shaderTrace.m_drawNum = stmtGuard.Get<gtUInt32>(3);
        shaderTrace.m_shaderType = stmtGuard.Get<gtUInt32>(4);
        shaderTrace.m_cpuTime = stmtGuard.Get<gtUInt32>(5);
        shaderTrace.m_srcCode = stmtGuard.Get<gtString>(6);
        shaderTrace.m_asmCode = stmtGuard.Get<gtString>(7);
        shaderTrace.m_debug = stmtGuard.Get<gtVector<gtUByte>>(8);
        shaderTrace.m_binary = stmtGuard.Get<gtVector<gtUByte>>(9);

        vShaderTraces.push_back(shaderTrace);
        bReturn = success;
    }

    return bReturn;
}

status LPGPU2DatabaseAccessor::GetShaderTracesInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<ShaderTrace> &vShaderTraces) const
{
    auto bReturn = success;

    SQLiteStatementGuard stmtGuard{ m_pGetShaderTracesInFrameRangeStmt };

    stmtGuard.Bind("@frameStart", vFrameStart);
    stmtGuard.Bind("@frameEnd", vFrameEnd);

    while (stmtGuard.Step())
    {
        // shaderId, apiId, frameNum, drawNum, shaderType, cpuTime, srcCode, asmCode, debug, binary
        ShaderTrace shaderTrace;
        shaderTrace.m_shaderId = stmtGuard.Get<gtUInt64>(0);
        shaderTrace.m_apiId = static_cast<EShaderTraceAPIId>(stmtGuard.Get<gtUInt32>(1));
        shaderTrace.m_frameNum = stmtGuard.Get<gtUInt32>(2);
        shaderTrace.m_drawNum = stmtGuard.Get<gtUInt32>(3);
        shaderTrace.m_shaderType = stmtGuard.Get<gtUInt32>(4);
        shaderTrace.m_cpuTime = stmtGuard.Get<gtUInt32>(5);
        shaderTrace.m_srcCode = stmtGuard.Get<gtString>(6);
        shaderTrace.m_asmCode = stmtGuard.Get<gtString>(7);
        shaderTrace.m_debug = stmtGuard.Get<gtVector<gtUByte>>(8);
        shaderTrace.m_binary = stmtGuard.Get<gtVector<gtUByte>>(9);

        vShaderTraces.push_back(shaderTrace);
        bReturn = success;
    }

    return bReturn;
}


/// @brief                     Gets a list of ShaderTrace objects with the specified API ID.
/// @param[in]  vApiId         The API id to look for shader traces.
/// @param[out] vShaderTraces  A list of ShaderTrace objects.
/// @returns    status         success = the information was retrieved,
///                            failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetShaderTracesByAPIId(EShaderTraceAPIId vApiId, gtVector<ShaderTrace>& vShaderTraces) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetShaderTracesByAPIIdStmt };

    stmtGuard.Bind("@apiId", static_cast<gtUInt32>(vApiId));

    while (stmtGuard.Step())
    {
        // shaderId, apiId, frameNum, drawNum, shaderType, cpuTime, srcCode, asmCode, debug, binary
        ShaderTrace shaderTrace;
        shaderTrace.m_shaderId = stmtGuard.Get<gtUInt64>(0);
        shaderTrace.m_apiId = static_cast<EShaderTraceAPIId>(stmtGuard.Get<gtUInt32>(1));
        shaderTrace.m_frameNum = stmtGuard.Get<gtUInt32>(2);
        shaderTrace.m_drawNum = stmtGuard.Get<gtUInt32>(3);
        shaderTrace.m_shaderType = stmtGuard.Get<gtUInt32>(4);
        shaderTrace.m_cpuTime = stmtGuard.Get<gtUInt32>(5);
        shaderTrace.m_srcCode = stmtGuard.Get<gtString>(6);
        shaderTrace.m_asmCode = stmtGuard.Get<gtString>(7);
        shaderTrace.m_debug = stmtGuard.Get<gtVector<gtUByte>>(8);
        shaderTrace.m_binary = stmtGuard.Get<gtVector<gtUByte>>(9);

        vShaderTraces.push_back(shaderTrace);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                   Get the shader stats from the specified shaderId.
/// @param[in]  vFrameNum    The frameNum that identifies the shader.
/// @param[in]  vDrawNum     The drawNum that identifies the shader.
/// @param[out] vShaderStats The stats for the specified shaderId.
/// @returns    status       success = the information was retrieved,
///                          failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetShaderAsmStatsByShaderId(gtUInt64 vShaderId, gtVector<ShaderAsmStat> &vShaderStats) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetShaderStatByShaderIdStmt };

    stmtGuard.Bind("@shaderId", vShaderId);

    while (stmtGuard.Step())
    {
        // shaderId, apiId, asmLine, percentage
        ShaderAsmStat shaderStat;
        shaderStat.m_shaderId = stmtGuard.Get<gtUInt64>(0);
        shaderStat.m_apiId = static_cast<EShaderTraceAPIId>(stmtGuard.Get<gtUInt32>(1));
        shaderStat.m_asmLine = stmtGuard.Get<gtUInt32>(2);
        shaderStat.m_percentage = stmtGuard.Get<gtFloat32>(3);

        vShaderStats.push_back(shaderStat);
        bReturn = success;
    }

    return bReturn;
}

/// @brief  Get the shader-to-asm mapping for the given shaderId.
/// @param[in] vShaderId    The shaderId to look for the mappings.
/// @param[out] vShaderToAsm    The vector of ShaderToAsm mappings.
/// @returns    status       success = the information was retrieved,
///                          failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetShaderToAsmByShaderId(gtUInt64 vShaderId, gtVector<ShaderToAsm> &vShaderToAsm) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetShaderToAsmByShaderIdStmt };

    stmtGuard.Bind("@shaderId", vShaderId);

    while (stmtGuard.Step())
    {
        // shaderId, apiId, shaderLine, asmStartLine, asmEndLine
        ShaderToAsm shaderToAsm;
        shaderToAsm.m_shaderId = stmtGuard.Get<gtUInt64>(0);
        shaderToAsm.m_apiId = static_cast<EShaderTraceAPIId>(stmtGuard.Get<gtUInt32>(1));
        shaderToAsm.m_shaderLine = stmtGuard.Get<gtUInt64>(2);
        shaderToAsm.m_asmStartLine = stmtGuard.Get<gtUInt64>(3);
        shaderToAsm.m_asmEndLine = stmtGuard.Get<gtUInt64>(4);

        vShaderToAsm.push_back(shaderToAsm);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                      Get all the lines in the database's LGPU2_counterWeights table.
/// @param[out] vCounterWeights A list with CounterWeight objects representing a line
///                             in the LGPU2_counterWeights table.
/// @returns   status           success = the information was retrieved,
///                             failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetCounterWeights(gtVector<CounterWeight> &vCounterWeights) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetCounterWeightsStmt };

    while (stmtGuard.Step())
    {
        // counterName, weightValue
        CounterWeight counterWeight;
        counterWeight.m_counterName = stmtGuard.Get<gtString>(0);
        counterWeight.m_weightValue = stmtGuard.Get<gtFloat32>(1);

        vCounterWeights.push_back(counterWeight);
        bReturn = success;
    }

    return bReturn;
}


/// @brief                  Get the CounterWeight of a counter looking by name.
/// @param   vCounterName   The name of the counter to look for.
/// @param   vCounterWeight The CounterWeight object for the corresponding counter name.
/// @returns status         success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetCounterWeightForCounterByName(const gtString &vCounterName, CounterWeight &vCounterWeight) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetCounterWeightForCounterByNameStmt };

    stmtGuard.Bind("@counterName", vCounterName);

    if (stmtGuard.Step())
    {
        // counterName, weightValue
        vCounterWeight.m_counterName = stmtGuard.Get<gtString>(0);
        vCounterWeight.m_weightValue = stmtGuard.Get<gtFloat32>(1);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                    Return the weighted sum for a counter with samples in the specified range
///                           using the information from the table LPGPU2_counterWeights.
/// @param[in]  vCounterName  The name of the counter.
/// @param[in]  vRangeStart   The start of range to look for samples.
/// @param[in]  vRangeEnd     The end of range to look for samples.
/// @param[out] vWeightedSum  The weighted sum the samples.
/// @returns   status         success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetWeightedSumForCounterByNameInRange(const gtString &vCounterName, gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtFloat32 &vWeightedSum) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetWeightedSumForCounterInRangeStmt };

    stmtGuard.Bind("@rangeStart", vRangeStart);
    stmtGuard.Bind("@rangeEnd", vRangeEnd);
    stmtGuard.Bind("@counterName", vCounterName);

    if (stmtGuard.Step())
    {
        vWeightedSum = stmtGuard.Get<gtFloat32>(0);
        bReturn = success;
    }

    return bReturn;
}


/// @brief                  Get a list of PPSampleData for the counter where the sample values are
///                         weighted by the corresponding counter weight.
/// @param   vCounterName   The name of the counter to look for samples.
/// @param   vSampleValues  The list of Power Profiling Sample Data for the counter name.
/// @returns status         success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetWeightedSamplesForCounterByName(const gtString &vCounterName, gtVector<PPSampleData> &vSampleValues) const
{
    CounterWeight counterWeight;
    auto bReturn = GetCounterWeightForCounterByName(vCounterName, counterWeight);

    int counterId;
    bReturn &= GetCounterIdByName(vCounterName.asASCIICharArray(), counterId);
    
    if (bReturn == success)
    {
        bReturn = failure;

        SQLiteStatementGuard stmtGuard{ m_pGetWeightedSamplesForCounterByNameStmt };

        stmtGuard.Bind("@counterId", counterId);
        stmtGuard.Bind("@counterWeight", counterWeight.m_weightValue);        

        while (stmtGuard.Step())
        {
            // counterId, quantizedTimeMs, sampleValue * counterWeight, blob
            counterId = stmtGuard.Get<int>(0);
            auto quantizedTimeMs = stmtGuard.Get<int>(1);
            auto avgSampleValue = stmtGuard.Get<double>(2);
            auto blob = stmtGuard.Get<PPSampleData::BlobVec>(3);

            vSampleValues.emplace_back(quantizedTimeMs, counterId, avgSampleValue,
                std::move(blob));
            bReturn = success;
        }        
    }

    return bReturn;
}

/// @brief                   Get all the lines in the database's LPGPU2_stackTraces table.
/// @param[out] vStackTraces A list with StackTraceInfo objects representing a line
///                          in the LPGPU2_stackTraces table.
/// @returns   status        success = the information was retrieved,
///                          failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetStackTraces(gtVector<StackTraceInfo> &vStackTraces) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetStackTracesStmt };

    while (stmtGuard.Step())
    {
        //frameNum, drawNum, baseAddr, symbolAddr, symbolName, filename
        StackTraceInfo stackTrace;
        stackTrace.m_frameNum = stmtGuard.Get<gtUInt64>(0);
        stackTrace.m_drawNum = stmtGuard.Get<gtUInt64>(1);
        stackTrace.m_baseAddr = stmtGuard.Get<gtUInt64>(2);
        stackTrace.m_symbolAddr = stmtGuard.Get<gtUInt64>(3);
        stackTrace.m_symbolName = stmtGuard.Get<gtString>(4);
        stackTrace.m_fileName = stmtGuard.Get<gtString>(5);

        vStackTraces.push_back(stackTrace);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                       Get all the lines in the database's LPGPU2_traceParameter table.
/// @param[out] vTraceParameters A list with StackTraceParameter objects representing a line
///                              in the LPGPU2_traceParameter table.
/// @returns   status            success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetStackTraceParameters(gtVector<StackTraceParameter> &vTraceParameters) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetStackTraceParamsStmt };

    while (stmtGuard.Step())
    {
        // frameNum, drawNum, threadId, typedefDescription, binaryParams, binaryReturn, error
        StackTraceParameter traceParam;
        traceParam.m_frameNum = stmtGuard.Get<gtUInt64>(0);
        traceParam.m_drawNum = stmtGuard.Get<gtUInt64>(1);
        traceParam.m_threadId = stmtGuard.Get<gtUInt64>(2);
        traceParam.m_typeDefDescription = stmtGuard.Get<gtString>(3);
        traceParam.m_binaryParams = stmtGuard.Get<gtVector<gtUByte>>(4);
        traceParam.m_paramsSize = stmtGuard.Get<gtString>(5);
        traceParam.m_binaryReturn = stmtGuard.Get<gtVector<gtUByte>>(6);
        traceParam.m_returnSize = stmtGuard.Get<gtUInt32>(7);
        traceParam.m_error = stmtGuard.Get<gtUInt32>(8);

        vTraceParameters.push_back(traceParam);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                       Get a list of StackTraceParameters for calls that match the vApiName and vCallCategory.
/// @param[in]  vCallCategory    The call category to look for in LPGPU2_callCategory.
/// @param[in]  vApiName         The apiName to look for in LPGUP2_callCategory.
/// @param[out] vTraceParameters The result StackTraceParameter vector.
/// @returns    status           success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetStackTraceParametersByCallCategoryAndAPIName(const gtString &vCallCategory, const gtString &vApiName, gtVector<StackTraceParameter> &vTraceParameters) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetStackTraceParamsForCallCategoryAndAPINameStmt };

    stmtGuard.Bind("@callCategory", vCallCategory);
    stmtGuard.Bind("@apiName", vApiName);

    while (stmtGuard.Step())
    {
        // frameNum, drawNum, threadId, typedefDescription, binaryParams, binaryReturn, error
        StackTraceParameter traceParam;
        traceParam.m_frameNum = stmtGuard.Get<gtUInt32>(0);
        traceParam.m_drawNum = stmtGuard.Get<gtUInt32>(1);
        traceParam.m_threadId = stmtGuard.Get<gtUInt32>(2);
        traceParam.m_typeDefDescription = stmtGuard.Get<gtString>(3);
        traceParam.m_binaryParams = stmtGuard.Get<gtVector<gtUByte>>(4);
        traceParam.m_paramsSize = stmtGuard.Get<gtString>(5);
        traceParam.m_binaryReturn = stmtGuard.Get<gtVector<gtUByte>>(6);
        traceParam.m_returnSize = stmtGuard.Get<gtUInt32>(7);
        traceParam.m_error = stmtGuard.Get<gtUInt32>(8);

        vTraceParameters.push_back(traceParam);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                       Get a list of StackTraceParameters for calls that match the vCallCategory.
/// @param[in]  vCallCategory    The call category to look for in LPGPU2_callCategory.
/// @param[out] vTraceParameters The result StackTraceParameter vector.
/// @returns    status           success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetStackTraceParametersByCallCategory(const gtString &vCallCategory, gtVector<StackTraceParameter> &vTraceParameters) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetStackTraceParamsForCallCategoryStmt };

    stmtGuard.Bind("@callCategory", vCallCategory);    

    while (stmtGuard.Step())
    {
        // frameNum, drawNum, threadId, typedefDescription, binaryParams, binaryReturn, error
        StackTraceParameter traceParam;
        traceParam.m_frameNum = stmtGuard.Get<gtUInt32>(0);
        traceParam.m_drawNum = stmtGuard.Get<gtUInt32>(1);
        traceParam.m_threadId = stmtGuard.Get<gtUInt32>(2);
        traceParam.m_typeDefDescription = stmtGuard.Get<gtString>(3);
        traceParam.m_binaryParams = stmtGuard.Get<gtVector<gtUByte>>(4);
        traceParam.m_paramsSize = stmtGuard.Get<gtString>(5);
        traceParam.m_binaryReturn = stmtGuard.Get<gtVector<gtUByte>>(6);
        traceParam.m_returnSize = stmtGuard.Get<gtUInt32>(7);
        traceParam.m_error = stmtGuard.Get<gtUInt32>(8);

        vTraceParameters.push_back(traceParam);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                       Get a list of StackTraceParameters for calls that match the vCallCategory.
/// @param[in]  vApiName         The apiName to look for in LPGUP2_callCategory.
/// @param[out] vTraceParameters The result StackTraceParameter vector.
/// @returns    status           success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetStackTraceParametersByAPIName(const gtString &vApiName, gtVector<StackTraceParameter> &vTraceParameters) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetStackTraceParamsForCallCategoryAndAPINameStmt };

    stmtGuard.Bind("@apiName", vApiName);

    while (stmtGuard.Step())
    {
        // frameNum, drawNum, threadId, typedefDescription, binaryParams, binaryReturn, error
        StackTraceParameter traceParam;
        traceParam.m_frameNum = stmtGuard.Get<gtUInt32>(0);
        traceParam.m_drawNum = stmtGuard.Get<gtUInt32>(1);
        traceParam.m_threadId = stmtGuard.Get<gtUInt64>(2);
        traceParam.m_typeDefDescription = stmtGuard.Get<gtString>(3);
        traceParam.m_binaryParams = stmtGuard.Get<gtVector<gtUByte>>(4);
        traceParam.m_paramsSize = stmtGuard.Get<gtString>(5);
        traceParam.m_binaryReturn = stmtGuard.Get<gtVector<gtUByte>>(6);
        traceParam.m_returnSize = stmtGuard.Get<gtUInt32>(7);
        traceParam.m_error = stmtGuard.Get<gtUInt32>(8);

        vTraceParameters.push_back(traceParam);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                  Gets the number of unique threadIds from the table
///                         LPGPU2_stackTraceParameters.
/// @param[out] vNumThreads The number of unique threadIds.
/// @returns    status      success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetNumThreadsFromTraceParameters(gtUInt64 &vNumThreads) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetNumThreadsFromTraceParamsStmt };

    if (stmtGuard.Step())
    {
        vNumThreads = stmtGuard.Get<gtUInt64>(0);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                       Get a list of StackTraceParameters for calls that match the frameNum and drawNum
/// @param[in]  vFrameNum        The frameNum to look for in LPGPU2_traceParameters.
/// @param[in]  vDrawNum         The drawNum to look for in LPGPU2_traceParameters.
/// @param[out] vTraceParameters The result StackTraceParameter vector that match the search criteria.
/// @returns    status           success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetStackTraceParametersByFrameAndDrawNumber(gtUInt64 vFrameNum, gtUInt64 vDrawNum, gtVector<StackTraceParameter> &vTraceParameters) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetStackTraceParamsForFrameAndDrawNumStmt };

    stmtGuard.Bind("@frameNum", vFrameNum);
    stmtGuard.Bind("@drawNum", vDrawNum);

    while (stmtGuard.Step())
    {
        // frameNum, drawNum, threadId, typedefDescription, binaryParams, binaryReturn, error
        StackTraceParameter traceParam;
        traceParam.m_frameNum = stmtGuard.Get<gtUInt64>(0);
        traceParam.m_drawNum = stmtGuard.Get<gtUInt64>(1);
        traceParam.m_threadId = stmtGuard.Get<gtUInt64>(2);
        traceParam.m_typeDefDescription = stmtGuard.Get<gtString>(3);
        traceParam.m_binaryParams = stmtGuard.Get<gtVector<gtUByte>>(4);
        traceParam.m_paramsSize = stmtGuard.Get<gtString>(5);
        traceParam.m_binaryReturn = stmtGuard.Get<gtVector<gtUByte>>(6);
        traceParam.m_returnSize = stmtGuard.Get<gtUInt32>(7);
        traceParam.m_error = stmtGuard.Get<gtUInt32>(8);

        vTraceParameters.push_back(traceParam);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                          Get all the lines in the database's LPGPU2_regionsOfInterest table.
/// @param[out] vRegionsOfInterest  A list with RegionOfInterest objects representing a line
///                                 in the LPGPU2_regionsOfInterest table.
/// @returns   status               success = the information was retrieved,
///                                 failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetRegionsOfInterest(gtVector<RegionOfInterest> &vRegionsOfInterest) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetRegionsOfInterestStmt };

    while (stmtGuard.Step())
    {
        // @regionId, @frameStart, @frameEnd, @degreeOfInterest
        RegionOfInterest regionOfInterest;
        regionOfInterest.m_regionId = stmtGuard.Get<gtUInt64>(0);
        regionOfInterest.m_frameStart = stmtGuard.Get<gtUInt64>(1);
        regionOfInterest.m_frameEnd = stmtGuard.Get<gtUInt64>(2);
        regionOfInterest.m_degreeOfInterest = stmtGuard.Get<gtFloat32>(3);

        vRegionsOfInterest.push_back(regionOfInterest);
        bReturn = success;
    }    
    
    return bReturn;
}

/// @brief                      Get a RegionOfInterest with the given id.
/// @param   vRegionId          The region id to look for.
/// @param   vRegionOfInterest  The RegionOfInterest with the requested id.
/// @returns status             success = the information was retrieved,
///                             failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetRegionOfInterestByRegionId(gtUInt64 vRegionId, RegionOfInterest &vRegionOfInterest) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetRegionOfInterestByIdStmt };

    stmtGuard.Bind("@regionId", vRegionId);

    if (stmtGuard.Step())
    {
        // @regionId, @frameStart, @frameEnd, @degreeOfInterest        
        vRegionOfInterest.m_regionId = stmtGuard.Get<gtUInt64>(0);
        vRegionOfInterest.m_frameStart = stmtGuard.Get<gtUInt64>(1);
        vRegionOfInterest.m_frameEnd = stmtGuard.Get<gtUInt64>(2);
        vRegionOfInterest.m_degreeOfInterest = stmtGuard.Get<gtFloat32>(3);
        
        bReturn = success;
    }

    return bReturn;
}

/// @brief                    Get the accumulated time from all the calls from the Calls
///                           summary data.
/// @param   vAccumulatedTime The accumulated time from the calls summary.
/// @returns status           success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetAccumulatedCallSummaryTime(gtUInt64 &vAccumulatedTime) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetAccumulatedTimeFromCallSummaryStmt };

    while (stmtGuard.Step())
    {
        // SUM(totalTime)
        vAccumulatedTime = stmtGuard.Get<gtUInt64>(0);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                       Get all the lines in the database's LPGPU2_callSummary table
///                              that match the informed regionId.
/// @param[in]  vRegionId        The regionId to search call summaries for.
/// @param[out] vCallSummaryList A list with CallSummary objects representing entries
///                              in the LPGPU2_regionsOfInterest table.
/// @returns   status            success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetCallSummaryForRegionById(const gtUInt64 vRegionId, gtVector<CallSummary> &vCallSummaryList) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetCallSummaryForRegionStmt };

    stmtGuard.Bind("@regionId", vRegionId);

    while (stmtGuard.Step())
    {        
        // regionId, callName, timesCalled, totalTime
        CallSummary callSummary;
        callSummary.m_regionId = stmtGuard.Get<gtUInt64>(0);
        callSummary.m_callName = stmtGuard.Get<gtString>(1);
        callSummary.m_timesCalled = stmtGuard.Get<gtUInt64>(2);
        callSummary.m_totalTime = stmtGuard.Get<gtUInt64>(3);

        vCallSummaryList.push_back(callSummary);
        bReturn = success;
    }    

    return bReturn;
}

/// @brief                        Get all the lines in the database's LPGPU2_callsPerType table
///                               that match the informed regionId.
/// @param[in]  vRegionId         The regionId to search for calls per type entries.
/// @param[out] vCallsPerType     The list of CallsPerType objects that match the informed regionId.
/// @returns    status            success = the information was retrieved,
///                               failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetCallsPerTypeForRegionById(gtUInt64 vRegionId, CallsPerType &vCallsPerType) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetCallsPerTypeForRegionStmt };

    stmtGuard.Bind("@regionId", vRegionId);

    if (stmtGuard.Step())
    {
        //regionId, drawPercentage, bindPercentage, setupPercentage, waitPercentage, errorPercentage, delimiterPercentage, otherPercentage
        vCallsPerType.m_regionId = stmtGuard.Get<gtUInt64>(0);
        vCallsPerType.m_drawPercentage = stmtGuard.Get<gtFloat32>(1);
        vCallsPerType.m_bindPercentage = stmtGuard.Get<gtFloat32>(2);
        vCallsPerType.m_setupPercentage = stmtGuard.Get<gtFloat32>(3);
        vCallsPerType.m_waitPercentage = stmtGuard.Get<gtFloat32>(4);
        vCallsPerType.m_errorPercentage = stmtGuard.Get<gtFloat32>(5);
        vCallsPerType.m_delimiterPercentage = stmtGuard.Get<gtFloat32>(7);
        vCallsPerType.m_otherPercentage = stmtGuard.Get<gtFloat32>(6);

        bReturn = success;
    }    

    return bReturn;
}

/// @brief                    Get a list of unique quantized times from the recorded samples
///                           stored in the samples table.
/// @param[out] vUniqueTimes  The list of unique times from the samples recorded.
/// @returns    status        success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetUniqueQuantizedTimesFromSamples(gtVector<gtUInt64> &vUniqueTimes) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetUniqueQuantizedTimesFromSamplesStmt };

    while (stmtGuard.Step())
    {
        vUniqueTimes.push_back(stmtGuard.Get<gtUInt64>(0));
        bReturn = success;
    }

    return bReturn;
}

/// @brief                    Get a list of unique quantized times from the recorded samples
///                           stored in the samples table but filter the results by the informed range.
/// @param[in]  vRangeStart   The start of the range to look for quantized times.
/// @param[in]  vRangeEnd     The end of the range to look for quantized times.
/// @param[out] vUniqueTimes  The list of unique times from the samples recorded.
/// @returns    status        success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetUniqueQuantizedTimesFromSamplesInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<gtUInt64> &vUniqueTimes) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetUniqueQuantizedTimesFromSamplesInRangeStmt };

    stmtGuard.Bind("@rangeStart", vRangeStart);
    stmtGuard.Bind("@rangeEnd", vRangeEnd);

    while (stmtGuard.Step())
    {
        vUniqueTimes.push_back(stmtGuard.Get<gtUInt64>(0));
        bReturn = success;
    }

    return bReturn;
}

/// @brief                       Get a list of unique quantized times from the recorded samples
///                              that match the counter category and are inside the requested range.
/// @param[in]  vCounterCategory The counter category to look for samples.
/// @param[in]  vRangeStart      The start of the range to look for quantized times.
/// @param[in]  vRangeEnd        The end of the range to look for quantized times.
/// @param[out] vUniqueTimes     The list of unique times from the samples recorded.
/// @returns    status           success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetUniqueQuantizedTimesFromSamplesByCategoryInRange(const gtString &vCounterCategory, const gtUInt64 vRangeStart, const gtUInt64 vRangeEnd, gtVector<gtUInt64> &vUniqueTimes) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetUniqueQuantizedTimesFromSamplesForCategoryInRangeStmt };

    stmtGuard.Bind("@counterCategory", vCounterCategory);
    stmtGuard.Bind("@rangeStart", vRangeStart);
    stmtGuard.Bind("@rangeEnd", vRangeEnd);    

    while (stmtGuard.Step())
    {
        vUniqueTimes.push_back(stmtGuard.Get<gtUInt64>(0));
        bReturn = success;
    }

    return bReturn;
}


/// @brief                      Return a list of CallCategory entries that match vCategory.
/// @param[in]  vCategory       The category to search entries for.
/// @param[out] vCallCategories The list of CallCategory objects that match vCategory.
/// @returns    status          success = the information was retrieved,
///                             failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetCallNamesByCategory(const gtString &vCategory, gtVector<CallCategory> &vCallCategories) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetCallNamesForCategoryStmt };

    stmtGuard.Bind("@category", vCategory);

    while (stmtGuard.Step())
    {
        // callName, category, apiName
        CallCategory callCategory;
        callCategory.m_callName = stmtGuard.Get<gtString>(0);
        callCategory.m_category = stmtGuard.Get<gtString>(1);
        callCategory.m_apiName = stmtGuard.Get<gtString>(2);

        vCallCategories.push_back(callCategory);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                            Returns a list of ProfileTraceInfo with callNames that belongs to vCategory.
/// @param[in]  vCallCategory         The category used to look for trace information.
/// @param[out] vProfileTraceInfoList The list of ProfileTraceInfo where callName is from category vCallCategory.
/// @returns    status                success = the information was retrieved,
///                                   failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetProfileTraceInfoByCallCategory(const gtString &vCallCategory, gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetProfileTraceInfoForCallCategoryStmt };

    stmtGuard.Bind("@category", vCallCategory);

    while (stmtGuard.Step())
    {
        ProfileTraceInfo traceInfo;
        traceInfo.m_traceId = stmtGuard.Get<int>(0);
        traceInfo.m_callName = stmtGuard.Get<gtString>(1);
        traceInfo.m_cpuStart = stmtGuard.Get<int>(2);
        traceInfo.m_cpuEnd = stmtGuard.Get<int>(3);
        traceInfo.m_frameNum = stmtGuard.Get<int>(4);
        traceInfo.m_drawNum = stmtGuard.Get<int>(5);
        traceInfo.m_apiId = stmtGuard.Get<int>(6);
        traceInfo.m_parameterOffset = stmtGuard.Get<int>(7);
        traceInfo.m_parameterLength = stmtGuard.Get<int>(8);

        vProfileTraceInfoList.push_back(traceInfo);
        bReturn = success;

    }

    return bReturn;
}

/// @brief                            Returns a list of ProfileTraceInfo with callNames that belongs to vApiName.
/// @param[in]  vApiName              The apiName to look for trace information.
/// @param[out] vProfileTraceInfoList The list of ProfileTraceInfo where callNAme is from API vApiName.
/// @returns    status                success = the information was retrieved,
///                                   failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetProfileTraceInfoByAPIName(const gtString &vApiName, gtVector<ProfileTraceInfo>& vProfileTraceInfoList) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetProfileTraceInfoForAPINameStmt };

    stmtGuard.Bind("@apiName", vApiName);

    while (stmtGuard.Step())
    {
        ProfileTraceInfo traceInfo;
        traceInfo.m_traceId = stmtGuard.Get<int>(0);
        traceInfo.m_callName = stmtGuard.Get<gtString>(1);
        traceInfo.m_cpuStart = stmtGuard.Get<int>(2);
        traceInfo.m_cpuEnd = stmtGuard.Get<int>(3);
        traceInfo.m_frameNum = stmtGuard.Get<int>(4);
        traceInfo.m_drawNum = stmtGuard.Get<int>(5);
        traceInfo.m_apiId = stmtGuard.Get<int>(6);
        traceInfo.m_parameterOffset = stmtGuard.Get<int>(7);
        traceInfo.m_parameterLength = stmtGuard.Get<int>(8);

        vProfileTraceInfoList.push_back(traceInfo);
        bReturn = success;

    }

    return bReturn;
}

status LPGPU2DatabaseAccessor::GetProfileTraceInfoByCallCategoryAndAPIName(const gtString &vCallCategory, const gtString &vApiName, gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetProfileTraceInfoByCallCategoryAndAPINameStmt };

    stmtGuard.Bind("@callCategory", vCallCategory);
    stmtGuard.Bind("@apiName", vApiName);

    while (stmtGuard.Step())
    {
        ProfileTraceInfo traceInfo;
        traceInfo.m_traceId = stmtGuard.Get<int>(0);
        traceInfo.m_callName = stmtGuard.Get<gtString>(1);
        traceInfo.m_cpuStart = stmtGuard.Get<int>(2);
        traceInfo.m_cpuEnd = stmtGuard.Get<int>(3);
        traceInfo.m_frameNum = stmtGuard.Get<int>(4);
        traceInfo.m_drawNum = stmtGuard.Get<int>(5);
        traceInfo.m_apiId = stmtGuard.Get<int>(6);
        traceInfo.m_parameterOffset = stmtGuard.Get<int>(7);
        traceInfo.m_parameterLength = stmtGuard.Get<int>(8);

        vProfileTraceInfoList.push_back(traceInfo);
        bReturn = success;

    }

    return bReturn;
}

/// @brief                    Get the samples for a counter with name.
/// @param[in]  vCounterName  The counter name to look for samples.
/// @param[out] vSampleValues The samples for the counter.
/// @returns    status        success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetSamplesForCounterByName(const gtString &vCounterName, gtVector<PPSampleData> &vSampleValues) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetSampleValuesForCounterWithNameStmt };

    stmtGuard.Bind("@counterName", vCounterName);

    while (stmtGuard.Step())
    {
        // counterId, quantizedTimeMs, AVG(sampledValue)        
        auto counterId = stmtGuard.Get<int>(0);
        auto quantizedTimeMs = stmtGuard.Get<int>(1);
        auto avgSampleValue = stmtGuard.Get<double>(2);
        auto blob = stmtGuard.Get<PPSampleData::BlobVec>(3);

        vSampleValues.emplace_back(quantizedTimeMs, counterId, avgSampleValue,
            blob);
        bReturn = success;
    }

    return bReturn;
}


/// @brief                          Gets the average sample values for counters that belong to the
///                                 category vCounterCategory and the quantized times lies between
///                                 vRangeStart and vRangeEnd.
/// @param[in]   vCounterCategory   The counter category used to average sample values.
/// @param[in]   vRangeStart        The start of the quantized time range.
/// @param[in]   vRangeEnd          The end of quantized time range.
/// @param[out]  vSampleValues      The list of sample values but the m_sampleValue field will be the
///                                 average of values for the corresponding counter id.
/// @returns    status              success = the information was retrieved,
///                                 failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetAverageSamplesByCounterCategoryInRange(const gtString &vCounterCategory, gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<PPSampleData> &vSampleValues) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetAverageSampleValuesForCounterCategoryStmt };

    stmtGuard.Bind("@counterCategory", vCounterCategory);
    stmtGuard.Bind("@rangeStart", vRangeStart);
    stmtGuard.Bind("@rangeEnd", vRangeEnd);

    while (stmtGuard.Step())
    {
        // counterId, quantizedTimeMs, AVG(sampledValue)        
        auto counterId = stmtGuard.Get<int>(0);
        auto quantizedTimeMs = stmtGuard.Get<int>(1);        
        auto avgSampleValue = stmtGuard.Get<double>(2);        
        auto blob = stmtGuard.Get<PPSampleData::BlobVec>(3);

        vSampleValues.emplace_back(quantizedTimeMs, counterId, avgSampleValue,
            blob);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                          Gets the maximum sample values for counters that belong to the
///                                 category vCounterCategory.
/// @param[in]   vCounterCategory   The counter category used to get the maximum of sample values.
/// @param[out]  vSampleValues      The list of sample values but the m_sampleValue field will be the
///                                 maximum of values for the corresponding counter id grouped by the
///                                 quantizedTimeMs field.
/// @returns    status              success = the information was retrieved,
///                                 failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetMaxSampleValuesByCounterCategory(const gtString &vCounterCategory, gtVector<PPSampleData> &vSampleValues) const
{
    auto bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetMaxSampleValuesForCounterCategoryStmt };

    stmtGuard.Bind("@counterCategory", vCounterCategory);

    while (stmtGuard.Step())
    {
        // counterId, quantizedTimeMs, AVG(sampledValue)
        auto counterId = stmtGuard.Get<int>(0);
        auto quantizedTimeMs = stmtGuard.Get<int>(1);
        auto maxSampleValue = stmtGuard.Get<double>(2);
        auto blob = stmtGuard.Get<PPSampleData::BlobVec>(3);

        vSampleValues.emplace_back(quantizedTimeMs, counterId, maxSampleValue,
            blob);
        bReturn = success;
    }

    return bReturn;
}

/// @brief                          Gets the maximum sample values for counters that belong to the
///                                 category vCounterCategory.
/// @param[in]   vCounterCategory   The counter category used to get the maximum of sample values.
/// @param[out]  vSampleValues      The list of sample values but the m_sampleValue field will be the
///                                 maximum of values for the corresponding counter id grouped by the
///                                 quantizedTimeMs field.
/// @returns    status              success = the information was retrieved,
///                                 failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::GetNearestSampleValueBeforeTime(gtUInt64 vTgtTime, gtInt32 vCounter, SampledValue& vrNearestSample) const
{
    status bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetNearestSampleValueBeforeTimeStmt };

    stmtGuard.Bind("@targetCounter", vCounter);
    stmtGuard.Bind("@targetTime", vTgtTime);

    while (stmtGuard.Step())
    {
        auto value = stmtGuard.Get<double>(0);
        auto nearestTime = stmtGuard.Get<gtUInt64>(1);

        vrNearestSample = SampledValue(nearestTime, value);
        
        bReturn = success;
    }

    return bReturn;
}

status LPGPU2DatabaseAccessor::GetNearestSampleValueAfterTime(gtUInt64 vTgtTime, gtInt32 vCounter, SampledValue& vrNearestSample) const
{
    status bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetNearestSampleValueAfterTimeStmt };

    stmtGuard.Bind("@targetCounter", vCounter);
    stmtGuard.Bind("@targetTime", vTgtTime);

    while (stmtGuard.Step())
    {
        auto value = stmtGuard.Get<double>(0);
        auto nearestTime = stmtGuard.Get<gtUInt64>(1);

        vrNearestSample = SampledValue(nearestTime, value);
        
        bReturn = success;
    }

    return bReturn;
}

/// @brief Retrieve all the devices from the database
/// @param devices[out] Structure which will contain the devices
/// @returns    status            success = the information was retrieved,
///                               failure = an error has occurred and result may
///                                         be inconsistent.
status LPGPU2DatabaseAccessor::GetAllDevicesAsVector(
    gtVector<Device> &devices) const
{
    status bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetAllDevicesStmt };

    while (stmtGuard.Step())
    {
        auto deviceId = stmtGuard.Get<gtUInt32>(0);
        auto deviceType = stmtGuard.Get<gtUInt32>(1);
        auto deviceName = stmtGuard.Get<gtString>(3);
        auto deviceDescription = stmtGuard.Get<gtString>(4);

        devices.push_back({deviceId, deviceType, deviceName,
            deviceDescription});

        bReturn = success;
    }

    return bReturn;
}

/// @brief Estimate the frequency/sample rate of a counter.
/// @param[in]  vCounter Counter ID to estimate sample rate of.
/// @param[out] vFreq    The estimated sample frequency.
/// @returns    status            success = the information was retrieved,
///                               failure = an error has occurred and result may
///                                         be inconsistent.
status LPGPU2DatabaseAccessor::GetEstimatedCounterFrequency(gtInt32 vCounter, double& vFreq) const
{
    status bReturn = failure;

    SQLiteStatementGuard stmtGuard{ m_pGetFirstDiffForCounter };
    stmtGuard.Bind("@counterId", vCounter);
    gtUInt64 counterTimeDiffMs;

    while (stmtGuard.Step())
    {
        counterTimeDiffMs = stmtGuard.Get<gtUInt64>(0);
    }

    vFreq = 1000.0 / static_cast<double>(counterTimeDiffMs);

    return bReturn;
}

/// @brief Insert N binary blobs which are related to N counter samples. There
///        is a 1-to-1 relationship between the number of samples and the
///        number of blobs
/// @param blobs A vector of binary payloads
/// @returns    status            success = the information was retrieved,
///                               failure = an error has occurred and result may
///                                         be inconsistent.
status LPGPU2DatabaseAccessor::InsertLPGPU2CounterSampleBlobs(
    const CounterSampleBlobsVec &blobs)
{

  auto errorCode = failure;
  for (const auto &blob: blobs)
  {
    SQLiteStatementGuard stmtGuard {m_pInsertLPGPU2CounterSampleBlobStmt};
    stmtGuard.Bind("@LPGPU2Blob", blob);
    stmtGuard.Exec();
    errorCode = CheckForErrorAndReturn(stmtGuard);

    if (errorCode == failure)
    {
      break;
    }
  }

  return errorCode;
}

/// @brief                        Write a ProfileTraceInfo into the database's LPGPU2_traces table
/// @param[in]  vProfileTraceInfo Object to the written in the database LPGPU2_traces table
/// @returns    status            success = the information was retrieved,
///                               failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAccessor::InsertProfileTraceInfo(const ProfileTraceInfo &vProfileTraceInfo)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertProfileInfoStmt };

    // traceId, callName, cpuStart, cpuEnd, frameNum, drawNum, apiId, parameterOffset, parameterLength
    stmtGuard.Bind("@traceId", vProfileTraceInfo.m_traceId);
    stmtGuard.Bind("@callName", vProfileTraceInfo.m_callName);
    stmtGuard.Bind("@cpuStart", vProfileTraceInfo.m_cpuStart);
    stmtGuard.Bind("@cpuEnd", vProfileTraceInfo.m_cpuEnd);
    stmtGuard.Bind("@frameNum", vProfileTraceInfo.m_frameNum);
    stmtGuard.Bind("@drawNum", vProfileTraceInfo.m_drawNum);
    stmtGuard.Bind("@apiId", vProfileTraceInfo.m_apiId);
    stmtGuard.Bind("@parameterOffset", vProfileTraceInfo.m_parameterOffset);
    stmtGuard.Bind("@parameterLength", vProfileTraceInfo.m_parameterLength);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                         Write an EstimatedPowerSample into the 
///                                database's LPGPU2_estimatedPower table.
/// @param[in] vEstimatedPwrSample Object to be written in the database LPGPU2_estimatedPower table.
/// @returns   status              success = the information was inserted into the database,
///                                failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertEstimatedPowerSample(const EstimatedPowerSample &vEstimatedPwrSample)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertEstimatedPowerSampleStmt };

    //sampleTime INTEGER NOT NULL, powerValue REAL, modelId INTEGER, unit TEXT, vComponentId INTEGER
    stmtGuard.Bind("@sampleTime", vEstimatedPwrSample.m_sampleTime);
    stmtGuard.Bind("@powerValue", vEstimatedPwrSample.m_estimatedPower);
    stmtGuard.Bind("@modelId", vEstimatedPwrSample.m_modelId);
    stmtGuard.Bind("@componentId", vEstimatedPwrSample.m_componentId);
    stmtGuard.Bind("@unit", vEstimatedPwrSample.m_unit);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                  Write an association between a component and device into
///                         Database's LPGPU2_estimatedPowerComponents table.
/// @param[in] vEstimatedPwrComponent Object to be written in the database 
///                                   LPGPU2_estimatedPowerComponents table.
/// @returns   status       success = the information was inserted into the database,
///                         failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertEstimatedPowerComponent(const EstimatedPowerComponent &vEstimatedPwrComponent)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertEstimatedPowerComponentStmt };

    // deviceId, componentId, componentName
    stmtGuard.Bind("@deviceId", vEstimatedPwrComponent.m_deviceId);
    stmtGuard.Bind("@componentId", vEstimatedPwrComponent.m_componentId);
    stmtGuard.Bind("@componentName", vEstimatedPwrComponent.m_componentName);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                     Write an StackTraceInfo into the 
///                            database's LPGPU2_stackTraces table.
/// @param[in] vStackTraceInfo Object to the written in the database LPGPU2_estimatedPower table.
/// @returns   status          success = the information was inserted into the database,
///                            failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertStackTrace(const StackTraceInfo &vStackTraceInfo)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertStackTraceStmt };

    // frameNum, drawNum, baseAddr, symbolAddr, symbolName, filename
    stmtGuard.Bind("@frameNum", vStackTraceInfo.m_frameNum);
    stmtGuard.Bind("@drawNum", vStackTraceInfo.m_drawNum);
    stmtGuard.Bind("@baseAddr", vStackTraceInfo.m_baseAddr);
    stmtGuard.Bind("@symbolAddr", vStackTraceInfo.m_symbolAddr);
    stmtGuard.Bind("@filename", vStackTraceInfo.m_fileName);
    stmtGuard.Bind("@symbolName", vStackTraceInfo.m_symbolName);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                      Write an StackTraceParameter into the 
///                             database's LPGPU2_traceParameter table.
/// @param[in] vStackTraceParam Object to the written in the database LPGPU2_traceParameter table.
/// @returns   status           success = the information was inserted into the database,
///                             failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertStackTraceParameter(const StackTraceParameter &vStackTraceParam)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertStackTraceParameterStmt };

    //frameNum, drawNum, threadId, typedefDescription, binaryParams, binaryReturn, error
    stmtGuard.Bind("@frameNum", vStackTraceParam.m_frameNum);
    stmtGuard.Bind("@drawNum", vStackTraceParam.m_drawNum);
    stmtGuard.Bind("@threadId", vStackTraceParam.m_threadId);
    stmtGuard.Bind("@typedefDescription", vStackTraceParam.m_typeDefDescription);
    stmtGuard.Bind("@binaryParams", vStackTraceParam.m_binaryParams);
    stmtGuard.Bind("@paramsSize", vStackTraceParam.m_paramsSize);
    stmtGuard.Bind("@binaryReturn", vStackTraceParam.m_binaryReturn);
    stmtGuard.Bind("@returnSize", vStackTraceParam.m_returnSize);
    stmtGuard.Bind("@error", vStackTraceParam.m_error);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                 Write an Annotation into the
///                        database's LPGPU2_annotations table.
/// @param[in] vAnnotation Object to the written in the database LPGPU2_annotations table.
/// @returns   status      success = the information was inserted into the database,
///                        failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertAnnotation(const Annotation &vAnnotation)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertAnnotationStmt };

    // frameNum, drawNum, annotationType, cpuStart, cpuEnd, text
    stmtGuard.Bind("@frameNum", vAnnotation.m_frameNum);
    stmtGuard.Bind("@drawNum", vAnnotation.m_drawNum);
    stmtGuard.Bind("@annotationType", vAnnotation.m_annotationType);
    stmtGuard.Bind("@cpuStart", vAnnotation.m_cpuStartTime);
    stmtGuard.Bind("@cpuEnd", vAnnotation.m_cpuEndTime);
    stmtGuard.Bind("@text", vAnnotation.m_text);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief               Write an Annotation into the 
///                      database's LPGPU2_gpuTimers table.
/// @param[in] vGpuTimer Object to the written in the database LPGPU2_gpuTimers table.
/// @returns   status    success = the information was inserted into the database,
///                      failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertGPUTimer(const GPUTimer &vGpuTimer)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertGPUTimerStmt };

    // type, frameNum, drawNum, time
    stmtGuard.Bind("@type", vGpuTimer.m_type);
    stmtGuard.Bind("@frameNum", vGpuTimer.m_frameNum);
    stmtGuard.Bind("@drawNum", vGpuTimer.m_drawNum);
    stmtGuard.Bind("@time", vGpuTimer.m_time);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                  Write an Annotation into the 
///                         database's LPGPU2_gpuTimers table.
/// @param[in] vShaderTrace Object to be written in the database 
///                         LPGPU2_gpuTimers table.
/// @returns   status       success = the information was inserted into the database,
///                         failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertShaderTrace(const ShaderTrace &vShaderTrace)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertShaderTraceStmt };

    // @shaderId, @apiId, @frameNum, @drawNum, @shaderType, @cpuTime, srcCode, @asmCode, @debug, @binary
    stmtGuard.Bind("@shaderId", vShaderTrace.m_shaderId);
    stmtGuard.Bind("@apiId", static_cast<gtUInt32>(vShaderTrace.m_apiId));
    stmtGuard.Bind("@frameNum", vShaderTrace.m_frameNum);
    stmtGuard.Bind("@drawNum", vShaderTrace.m_drawNum);
    stmtGuard.Bind("@shaderType", vShaderTrace.m_shaderType);
    stmtGuard.Bind("@cpuTime", vShaderTrace.m_cpuTime);
    stmtGuard.Bind("@srcCode", vShaderTrace.m_srcCode);
    stmtGuard.Bind("@asmCode", vShaderTrace.m_asmCode);
    stmtGuard.Bind("@debug", vShaderTrace.m_debug);
    stmtGuard.Bind("@binary", vShaderTrace.m_binary);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                    Write an Annotation into the
///                           database's LPGPU2_counterWeights table.
/// @param[in] vCounterWeight Object to the written in the database LPGPU2_gpuTimers table.
/// @returns   status         success = the information was inserted into the database,
///                           failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertCounterWeight(const CounterWeight &vCounterWeight)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertCounterWeightStmt };

    // counterName, weightValue
    stmtGuard.Bind("@counterName", vCounterWeight.m_counterName);
    stmtGuard.Bind("@weightValue", vCounterWeight.m_weightValue);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                        Inserts a region of interest in the database.
/// param[in]  vRegionOfInterest  RegionOfInterest to insert into the database.
/// @returns   status             success = the information was inserted into the database,
///                               failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertRegionOfInterest(const RegionOfInterest &vRegionOfInterest)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertRegionOfInterestStmt };

    // @regionId, @frameStart, @frameEnd, @degreeOfInterest
    stmtGuard.Bind("@regionId", vRegionOfInterest.m_regionId);
    stmtGuard.Bind("@frameStart", vRegionOfInterest.m_frameStart);
    stmtGuard.Bind("@frameEnd", vRegionOfInterest.m_frameEnd);
    stmtGuard.Bind("@degreeOfInterest", vRegionOfInterest.m_degreeOfInterest);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                   Inserts a call summary into the database.
/// @param[in] vCallSummary  CallSummary object to be inserted into the database.
/// @returns   status        success = the information was inserted into the database,
///                          failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertCallSummary(const CallSummary &vCallSummary)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertCallSummaryStmt };
    
    // regionId, callName, timesCalled, totalTime
    stmtGuard.Bind("@regionId", vCallSummary.m_regionId);
    stmtGuard.Bind("@callName", vCallSummary.m_callName);
    stmtGuard.Bind("@timesCalled", vCallSummary.m_timesCalled);
    stmtGuard.Bind("@totalTime", vCallSummary.m_totalTime);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                     Inserts a calls per type entry into the database
/// @param[out] vCallsPerType  CallsPerType object to be inserted into the database.
/// @returns   status          success = the information was inserted into the database,
///                            failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertCallsPerType(const CallsPerType &vCallsPerType)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertCallsPerTypeStmt };

    // regionId, drawPercentage, bindPercentage, setupPercentage, waitPercentage, errorPercentage, delimiterPercentage, otherPercentage
    stmtGuard.Bind("@regionId", vCallsPerType.m_regionId);
    stmtGuard.Bind("@drawPercentage", vCallsPerType.m_drawPercentage);
    stmtGuard.Bind("@bindPercentage", vCallsPerType.m_bindPercentage);
    stmtGuard.Bind("@setupPercentage", vCallsPerType.m_setupPercentage);
    stmtGuard.Bind("@waitPercentage", vCallsPerType.m_waitPercentage);
    stmtGuard.Bind("@errorPercentage", vCallsPerType.m_errorPercentage);
    stmtGuard.Bind("@delimiterPercentage", vCallsPerType.m_delimiterPercentage);
    stmtGuard.Bind("@otherPercentage", vCallsPerType.m_otherPercentage);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                  Inserts a call category entry into the database's
///                         LPGPU2_callsCategory table.
/// @param   vCallCategory  The CallCategory to insert into the database.
/// @returns status         success = the information was inserted into the database,
///                         failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertCallCategory(const CallCategory &vCallCategory)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertCallCategoryStmt };

    // callName, category, apiName
    stmtGuard.Bind("@callName", vCallCategory.m_callName);
    stmtGuard.Bind("@category", vCallCategory.m_category);
    stmtGuard.Bind("@apiName", vCallCategory.m_apiName);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                     Inserts a counter category into the database's
///                            LPGPU2_countersCategory table.
/// @param   vCounterCategory  The CounterCategory to insert into the database.
/// @returns status            success = the information was inserted into the database,
///                            failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertCounterCategory(const CounterCategory &vCounterCategory)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertCounterCategoryStmt };

    // counterId, category
    stmtGuard.Bind("@counterId", vCounterCategory.m_counterId);
    stmtGuard.Bind("@category", vCounterCategory.m_category);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                  Update the ShaderTrace srcCode field in the database.
/// @param[in] vShaderTrace The ShaderTrace that contains the new information.
/// @return                 success = The ShaderTrace was updated successfully,
///                         failure = an error has occurred and the update operation failed.
status LPGPU2DatabaseAccessor::UpdateShaderTraceSource(const ShaderTrace &vShaderTrace)
{
    SQLiteStatementGuard stmtGuard{ m_pUpdateShaderTraceSourceStmt };

    // shaderId, apiId
    stmtGuard.Bind("@shaderId", vShaderTrace.m_shaderId);
    stmtGuard.Bind("@apiId", static_cast<gtUInt32>(vShaderTrace.m_apiId));

    stmtGuard.Bind("@srcCode", vShaderTrace.m_srcCode);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                  Update the ShaderTrace srcCode field in the database.
/// @param[in] vShaderTrace The ShaderTrace that contains the new information.
/// @return                 success = The ShaderTrace was updated successfully,
///                         failure = an error has occurred and the update operation failed.
status LPGPU2DatabaseAccessor::UpdateShaderTraceFromMainShaderPacket(const ShaderTrace &vShaderTrace)
{
    SQLiteStatementGuard stmtGuard{ m_pUpdateShaderTraceSourceStmt };

    // shaderId, apiId
    stmtGuard.Bind("@shaderId", vShaderTrace.m_shaderId);
    stmtGuard.Bind("@apiId", static_cast<gtUInt32>(vShaderTrace.m_apiId));

    stmtGuard.Bind("@srcCode", vShaderTrace.m_srcCode);
#if 0 // TIZEN FIX - The following are not included in the statement m_pUpdateShaderTraceSourceStmt (UPDATE_SHADERTRACE_SOURCE_SQL)
    stmtGuard.Bind("@cpuTime", vShaderTrace.m_cpuTime);
    stmtGuard.Bind("@frameNum", vShaderTrace.m_frameNum);
    stmtGuard.Bind("@drawNum", vShaderTrace.m_drawNum);
#endif

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                  Update the ShaderTrace asmCode field in the database.
/// @param[in] vShaderTrace The ShaderTrace that contains the new information.
/// @return                 success = The ShaderTrace was updated successfully,
///                         failure = an error has occurred and the update operation failed.
status LPGPU2DatabaseAccessor::UpdateShaderTraceAsm(const ShaderTrace &vShaderTrace)
{
    SQLiteStatementGuard stmtGuard{ m_pUpdateShaderTraceAsmStmt };

    // shaderId, apiId
    stmtGuard.Bind("@shaderId", vShaderTrace.m_shaderId);
    stmtGuard.Bind("@apiId", static_cast<gtUInt32>(vShaderTrace.m_apiId));

    stmtGuard.Bind("@asmCode", vShaderTrace.m_asmCode);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                  Update the ShaderTrace binary field in the database.
/// @param[in] vShaderTrace The ShaderTrace that contains the new information.
/// @return                 success = The ShaderTrace was updated successfully,
///                         failure = an error has occurred and the update operation failed.
status LPGPU2DatabaseAccessor::UpdateShaderTraceBinary(const ShaderTrace &vShaderTrace)
{
    SQLiteStatementGuard stmtGuard{ m_pUpdateShaderTraceBinaryStmt };

    // shaderId, apiId
    stmtGuard.Bind("@shaderId", vShaderTrace.m_shaderId);
    stmtGuard.Bind("@apiId", static_cast<gtUInt32>(vShaderTrace.m_apiId));

    stmtGuard.Bind("@binary", vShaderTrace.m_binary);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                  Update the ShaderTrace debug field in the database.
/// @param[in] vShaderTrace The ShaderTrace that contains the new information.
/// @return                 success = The ShaderTrace was updated successfully,
///                         failure = an error has occurred and the update operation failed.
status LPGPU2DatabaseAccessor::UpdateShaderTraceDebug(const ShaderTrace & vShaderTrace)
{
    SQLiteStatementGuard stmtGuard{ m_pUpdateShaderTraceDebugStmt };

    // shaderId, apiId
    stmtGuard.Bind("@shaderId", vShaderTrace.m_shaderId);
    stmtGuard.Bind("@apiId", static_cast<gtUInt32>(vShaderTrace.m_apiId));

    stmtGuard.Bind("@debug", vShaderTrace.m_debug);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                  Write a ShaderAsmStat into the database.
/// @param[in] vShaderStat  ShaderAsmStat to write into the database.
/// @returns   status       success = the information was inserted into the database,
///                         failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertShaderAsmStat(const ShaderAsmStat &vShaderStat)
{
    SQLiteStatementGuard stmtGuard{ m_pInsertShaderStatStmt };

    // @shaderId, @apiId, @asmLine, @percentage
    stmtGuard.Bind("@shaderId", vShaderStat.m_shaderId);
    stmtGuard.Bind("@apiId", static_cast<gtUInt32>(vShaderStat.m_apiId));
    stmtGuard.Bind("@asmLine", vShaderStat.m_asmLine);
    stmtGuard.Bind("@percentage", vShaderStat.m_percentage);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                   Write a shader-to-asm mapping into the database.
/// @param[in] vShaderToAsm  The shader-to-asm mapping to insert into the database.
/// @returns   status        success = the information was inserted into the database,
///                          failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAccessor::InsertShaderToAsm(const ShaderToAsm &vShaderToAsm)
{    
    SQLiteStatementGuard stmtGuard{ m_pInsertShaderToAsmStmt };

    // @shaderId, @apiId, @shaderLine, @asmStartLine, @asmEndLine
    stmtGuard.Bind("@shaderId", vShaderToAsm.m_shaderId);
    stmtGuard.Bind("@apiId", static_cast<gtUInt32>(vShaderToAsm.m_apiId));
    stmtGuard.Bind("@shaderLine", vShaderToAsm.m_shaderLine);
    stmtGuard.Bind("@asmStartLine", vShaderToAsm.m_asmStartLine);
    stmtGuard.Bind("@asmEndLine", vShaderToAsm.m_asmEndLine);

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief          Deletes the Regions of Interest from the database.
/// @returns status success = the information was deleted from the database,
///                 failure = an error has occurred during the operation.
status LPGPU2DatabaseAccessor::DeleteRegionsOfInterest()
{
    SQLiteStatementGuard stmtGuard{ m_pDeleteRegionsOfInterestStmt };

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief          Deletes Annotations from the database.
/// @returns status success = the information was deleted from the database,
///                 failure = an error has occurred during the operation.
status LPGPU2DatabaseAccessor::DeleteAnnotations()
{
    SQLiteStatementGuard stmtGuard{ m_pDeleteAnnotationsStmt };

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief          Deletes Call Categories from the database.
/// @returns status success = the information was deleted from the database,
///                 failure = an error has occurred during the operation.
status LPGPU2DatabaseAccessor::DeleteCallCategories()
{
    SQLiteStatementGuard stmtGuard{ m_pDeleteCallsCategoriesStmt };

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief          Deletes Counter Categories from the database.
/// @returns status success = the information was deleted from the database,
///                 failure = an error has occurred during the operation.
status LPGPU2DatabaseAccessor::DeleteCounterCategories()
{
    SQLiteStatementGuard stmtGuard{ m_pDeleteCountersCategoriesStmt };

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief               Deletes Shader stats for the given shaderId from the database.
/// @param[in] vShaderId The shaderId that identifies the shader.
/// @param[in] vApiId    The vApiId that identifies the shader.
/// @returns   status    success = the information was deleted from the database,
///                      failure = an error has occurred during the operation.
status LPGPU2DatabaseAccessor::DeleteShaderAsmStatsForShaderId(gtUInt64 vShaderId, EShaderTraceAPIId vApiId)
{
    SQLiteStatementGuard stmtGuard{ m_pDeleteShaderStatsForShaderIdStmt };

    stmtGuard.Bind("@shaderId", vShaderId);
    stmtGuard.Bind("@apiId", static_cast<gtUInt32>(vApiId));

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

bool LPGPU2DatabaseAccessor::IsTransactionActive() const
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    return dbGuard.IsTransactionActive();
}

/// @brief           Starts a bulk insertion. This will make multiple insertions much faster.
/// @return   status success = The transaction was initiated,
///                  failure = An error has occurred and the transaction will not take effect.
/// @warning         LPGPU2DatabaseAdapter::EndTransaction() must be called for the changes
///                  to be written into the database.
/// @see             LPGPU2DatabaseAccessor::EndTransaction().
/// @see             LPGPU2DatabaseAccessor::RollbackTransaction().
status LPGPU2DatabaseAccessor::BeginTransaction()
{    
    auto bReturn = false;
    
    if (!IsTransactionActive())
    {
        SQLiteStatementGuard stmtGuard{ m_pBeginTransactionStmt };

        stmtGuard.Exec();

        bReturn = CheckForErrorAndReturn(stmtGuard);        
    }
    else
    {
        // We are inside a transaction, so return success
        bReturn = success;
    }

    return bReturn;
}

/// @brief           Commits a bulk insertion. This will make multiple insertions much faster.
/// @return   status success = The transaction was finalized and the data is not in the database,
///                  failure = An error has occurred and the transaction will not take effect,
///                  this will effectively rollback all the changes.
/// @warning         LPGPU2DatabaseAdapter::BeginTransaction() must be called before ending a
///                  transaction. In case this method fails, call
///                  LPGPU2DatabaseAccessor::RollbackTransaction to return the database to a
///                  valid previous state.
/// @see             LPGPU2DatabaseAccessor::BeginTransaction().
/// @see             LPGPU2DatabaseAccessor::RollbackTransaction().
status LPGPU2DatabaseAccessor::EndTransaction()
{
    auto bReturn = false;

    if (IsTransactionActive())
    {
        SQLiteStatementGuard stmtGuard{ m_pEndTransactionStmt };

        stmtGuard.Exec();

        bReturn = CheckForErrorAndReturn(stmtGuard);
    }
    else
    {
        // We are not inside a transaction, so return success
        bReturn = success;
    }
    
    return bReturn;
}

/// @brief           Try to rollback the database to a previous valid state.
/// @return   status success = The rollback operation was successfully and the database is in
///                            previous valid state,
///                  failure = An error has occurred and the database state is inconsistent.
/// @warning         This method should only be called in case a call to 
///                  LPGPU2DatabaseAccessor::EndTransaction() fails.
/// @see             LPGPU2DatabaseAccessor::BeginTransaction().
/// @see             LPGPU2DatabaseAccessor::EndTransaction().
status LPGPU2DatabaseAccessor::RollbackTransaction()
{
    SQLiteStatementGuard stmtGuard{ m_pRollbackTransactionStmt };

    stmtGuard.Exec();

    return CheckForErrorAndReturn(stmtGuard);
}

/// @brief                          Exports the database contents into a set of CSV files,
///                                 one per table, into the specified directory.
/// @param[in] vOutputDirectoryName Name of the directory to write the contents of database.
/// @param[in] vFileNamePrefix      Prefix to be added to each file exported.
/// @param[in] vbOverrideExistingFiles true = When exporting the database, replace any file that
///                                           already exists,
///                                    false = Fail to export in case files already exists.
/// @returns   status               success = the database was exported without errors,
///                                 failure = an error has occurred during the export process.
/// @warning                        The application must have write permissions to the directory.
/// @warning                        Files with the same name in the same directory will be overwritten.
status LPGPU2DatabaseAccessor::ExportToCSV(const gtString &vOutputDirectoryName, const gtString &vFileNamePrefix, const bool vbOverrideExistingFiles)
{
    auto bReturn = failure;

    osDirectory outputDirectory;
    outputDirectory.setDirectoryFullPathFromString(vOutputDirectoryName);

    // Create the directory if it does not exists
    if (!outputDirectory.exists())
    {
        if (!outputDirectory.create())
        {
            AppendErrorMessage(L"Cannot create output directory");
            bReturn = failure;
        }
    }
    
    // Check again 
    if(outputDirectory.exists())
    {
        // Checks for write permissions
        if (!outputDirectory.isWriteAccessible())
        {
            AppendErrorMessage(L"Output directory is not writable");
            bReturn = failure;
        }
        else
        {
            SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

            if (dbGuard.IsValid())
            {
                bReturn = success;

                // Creates a CSV file for each table in the database
                gtVector<gtString> tableNames;
                if (dbGuard.GetAllTableNames(tableNames) == success)
                {
                    for (const auto& tableName : tableNames)
                    {
                        gtString outputFileName;
                        outputFileName << vFileNamePrefix << tableName;

                        osFilePath csvFilePath;
                        csvFilePath.setFileDirectory(outputDirectory);
                        csvFilePath.setFileName(outputFileName);
                        csvFilePath.setFileExtension(L"csv");
                        
                        lpgpu2::io::CSVFileWriter csvWriter{ csvFilePath, vbOverrideExistingFiles };

                        gtVector<gtString> tableColumns;
                        if (dbGuard.GetTableColumnNames(tableName, tableColumns) == success)
                        {
                            csvWriter.AddHeader(tableName);
                            csvWriter.AddColumns(tableColumns);

                            gtVector<gtVector<gtString>> tableData;
                            if (dbGuard.GetTableData(tableName, tableData) == success)
                            {
                                for (const auto& tableRowData : tableData)
                                {
                                    // csvRow is a shared_ptr, so there is do delete it here, it is handled by CSVWriter
                                    auto csvRow = csvWriter.AddRow();
                                    if (csvRow != nullptr)
                                    {
                                        bReturn &= csvRow->SetRowData(tableRowData);
                                    }
                                    else
                                    {
                                        AppendErrorMessage(csvWriter.GetErrorMessage());
                                        bReturn = failure;
                                        break;
                                    }
                                }
                            }                            
                        }

                        // The CSVFileWriter will close itself when destroyed, but we close
                        // explicitly here in order to get any error messages generated by
                        // the writer.
                        if (csvWriter.Close() == failure)
                        {
                            AppendErrorMessage(csvWriter.GetErrorMessage());
                            bReturn = failure;
                        }
                    }
                }                
            }
            else
            {
                AppendErrorMessage(L"Database connection is not valid");
                bReturn = failure;
            }
        }
    }

    return bReturn;
}

status LPGPU2DatabaseAccessor::PrepareInsertLPGPU2CounterSampleBlob()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertLPGPU2CounterSampleBlobStmt =
      dbGuard.Prepare(INSERT_LPGPU2_COUNTER_SAMPLE_BLOB_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertProfileTraceInfoStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertProfileInfoStmt = dbGuard.Prepare(INSERT_PROFILE_TRACE_INFO_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertEstimatedPowerStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertEstimatedPowerSampleStmt = dbGuard.Prepare(INSERT_ESTIMATED_POWER_SAMPLE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertEstimatedPowerComponentStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertEstimatedPowerComponentStmt = dbGuard.Prepare(INSERT_ESTIMATED_POWER_COMPONENT_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertStackTraceStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertStackTraceStmt = dbGuard.Prepare(INSERT_STACKTRACE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertStackTraceParametersStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertStackTraceParameterStmt = dbGuard.Prepare(INSERT_STACKTRACE_PARAMS_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertAnnotationStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertAnnotationStmt = dbGuard.Prepare(INSERT_ANNOTATION_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertGPUTimerStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertGPUTimerStmt = dbGuard.Prepare(INSERT_GPUTIMER_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertShaderTraceStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertShaderTraceStmt = dbGuard.Prepare(INSERT_SHADERTRACE_SQL);    

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertShaderAsmStatStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertShaderStatStmt = dbGuard.Prepare(INSERT_SHADER_STAT_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertShaderToAsmStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertShaderToAsmStmt = dbGuard.Prepare(INSERT_SHADER_TO_ASM_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertCounterWeightStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertCounterWeightStmt = dbGuard.Prepare(INSERT_COUNTER_WEIGHT_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertRegionOfInterestStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertRegionOfInterestStmt = dbGuard.Prepare(INSERT_REGION_OF_INTEREST_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertCallSummaryStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertCallSummaryStmt = dbGuard.Prepare(INSERT_CALL_SUMMARY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertCallsPerTypeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertCallsPerTypeStmt = dbGuard.Prepare(INSERT_CALLS_PER_TYPE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertCallCategoryStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertCallCategoryStmt = dbGuard.Prepare(INSERT_CALL_CATEGORY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareInsertCounterCategoryStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pInsertCounterCategoryStmt = dbGuard.Prepare(INSERT_COUNTER_CATEGORY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareDeleteRegionsOfInterestStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pDeleteRegionsOfInterestStmt = dbGuard.Prepare(DELETE_REGIONS_OF_INTEREST_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareDeleteAnnotationsStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pDeleteAnnotationsStmt = dbGuard.Prepare(DELETE_ANNOTATIONS_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareDeleteCallsCategoryStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pDeleteCallsCategoriesStmt = dbGuard.Prepare(DELETE_CALLS_CATEGORY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareDeleteCountersCategoryStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pDeleteCountersCategoriesStmt = dbGuard.Prepare(DELETE_COUNTERS_CATEGORY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareDeleteShaderStatsForShaderIdStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pDeleteShaderStatsForShaderIdStmt = dbGuard.Prepare(DELETE_SHADER_STATS_BY_SHADER_ID_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareUpdateShaderTraceSourceStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pUpdateShaderTraceSourceStmt = dbGuard.Prepare(UPDATE_SHADERTRACE_SOURCE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareUpdateShaderTraceAsmStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pUpdateShaderTraceAsmStmt = dbGuard.Prepare(UPDATE_SHADERTRACE_ASM_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareUpdateShaderTraceBinaryStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pUpdateShaderTraceBinaryStmt = dbGuard.Prepare(UPDATE_SHADERTRACE_BINARY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareUpdateShaderTraceDebugStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pUpdateShaderTraceDebugStmt = dbGuard.Prepare(UPDATE_SHADERTRACE_DEBUG_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareBeginTransactionStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pBeginTransactionStmt = dbGuard.Prepare(BEGIN_TRANSACTION_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareEndTransactionStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pEndTransactionStmt = dbGuard.Prepare(COMMIT_TRANSACTION_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareRollbackTransactionStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pRollbackTransactionStmt = dbGuard.Prepare(ROLLBACK_TRANSACTION_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetEstimatedPowerSamplesStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetEstimatedPwrSamplesStmt = dbGuard.Prepare(SELECT_ESTIMATED_POWER_SAMPLES_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetEstimatedPowerComponentsStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetEstimatedPwrComponentsStmt = dbGuard.Prepare(SELECT_ESTIMATED_POWER_COMPONENTS_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetAnnotationsStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetAnnotationsStmt = dbGuard.Prepare(SELECT_ANNOTATIONS_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetAnnotationByIdStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetAnnotationByIdStmt = dbGuard.Prepare(SELECT_ANNOTATION_BY_ID_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetAnnotationsInRangeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetAnnotationsInRangeStmt = dbGuard.Prepare(SELECT_ANNOTATIONS_IN_RANGE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetAnnotationsInFrameRangeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetAnnotationsInFrameRangeStmt = dbGuard.Prepare(SELECT_ANNOTATIONS_IN_FRAME_RANGE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetGlobalAnnotations()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetGlobalAnnotations = dbGuard.Prepare(SELECT_GLOBAL_ANNOTATIONS_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetUserAnnotations()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetUserAnnotations = dbGuard.Prepare(SELECT_USER_ANNOTATIONS_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetGPUTimersStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetGPUTimersStmt = dbGuard.Prepare(SELECT_GPUTIMER_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetGPUTimersForFrameNumInRangeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetGPUTimersForFrameNumInRangeStmt = dbGuard.Prepare(SELECT_GPUTIMER_FOR_FRAMENUM_IN_RANGE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetStackTracesStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetStackTracesStmt = dbGuard.Prepare(SELECT_STACKTRACES_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetStackTraceParamsStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetStackTraceParamsStmt = dbGuard.Prepare(SELECT_STACKTRACE_PARAMETERS_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetStackTraceParamsForCallCategoryAndAPINameStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetStackTraceParamsForCallCategoryAndAPINameStmt = dbGuard.Prepare(SELECT_STACKTRACE_PARAMS_FOR_CALL_TYPE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetNumThreadsFromTraceParamsStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetNumThreadsFromTraceParamsStmt = dbGuard.Prepare(SELECT_NUM_THREADS_FROM_STACKTRACE_PARAMS_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetStackTraceParamsForFrameAndDrawNumStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetStackTraceParamsForFrameAndDrawNumStmt = dbGuard.Prepare(SELECT_TRACE_PARAMS_FOR_FRAME_AND_DRAW_NUM_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetStackTraceParamsForCallCategoryStatment()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetStackTraceParamsForCallCategoryStmt = dbGuard.Prepare(SELECT_STACKTRACE_PARAMS_FOR_CALL_CATEGORY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetStackTraceParamsForAPINameStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetStackTraceParamsForAPINameStmt = dbGuard.Prepare(SELECT_STACKTRACE_PARAMS_FOR_API_NAME_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetShaderTracesStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetShaderTracesStmt = dbGuard.Prepare(SELECT_SHADERTRACES_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetShaderTracesInRangeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetShaderTracesInRangeStmt = dbGuard.Prepare(SELECT_SHADERTRACES_IN_RANGE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetShaderTracesInFrameRangeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetShaderTracesInFrameRangeStmt = dbGuard.Prepare(SELECT_SHADERTRACES_IN_FRAME_RANGE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetShaderAsmStatByShaderId()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetShaderStatByShaderIdStmt = dbGuard.Prepare(SELECT_SHADER_ASM_STAT_BY_SHADER_ID_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetShaderToAsmByShaderId()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetShaderToAsmByShaderIdStmt = dbGuard.Prepare(SELECT_SHADER_TO_ASM_BY_SHADER_ID_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetShaderTracesByAPIIDStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetShaderTracesByAPIIdStmt = dbGuard.Prepare(SELECT_SHADERTRACES_BY_APIID_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetCounterWeightsStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetCounterWeightsStmt = dbGuard.Prepare(SELECT_COUNTER_WEIGHTS_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetCounterWeightForCounterWithNameStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetCounterWeightForCounterByNameStmt = dbGuard.Prepare(SELECT_COUNTER_WEIGHT_FOR_COUNTER_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetWeightedSumForCounterInRangeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetWeightedSumForCounterInRangeStmt = dbGuard.Prepare(SELECT_WEIGHTED_SUM_FOR_COUNTER_IN_RANGE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetWeightedSamplesForCounterByNameStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetWeightedSamplesForCounterByNameStmt = dbGuard.Prepare(SELECT_WEIGHTED_SAMPLES_FOR_COUNTER_WITH_NAME_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetRegionsOfInterestStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetRegionsOfInterestStmt = dbGuard.Prepare(SELECT_REGIONS_OF_INTEREST_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetRegionOfInterestByIdStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetRegionOfInterestByIdStmt = dbGuard.Prepare(SELECT_REGION_OF_INTEREST_BY_ID_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetAccumulatedTimeFromCallSummaryStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetAccumulatedTimeFromCallSummaryStmt = dbGuard.Prepare(SELECT_ACCUMULATED_TIME_FROM_CALL_SUMMARY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetCallSummaryForRegionStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetCallSummaryForRegionStmt = dbGuard.Prepare(SELECT_CALL_SUMMARY_FOR_REGION_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetCallsPerTypeForRegionStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetCallsPerTypeForRegionStmt = dbGuard.Prepare(SELECT_CALLS_PER_TYPE_FOR_REGION_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetUniqueQuantizedTimesFromSamplesStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetUniqueQuantizedTimesFromSamplesStmt = dbGuard.Prepare(SELECT_UNIQUE_QUANTIZED_TIMES_FROM_SAMPLES_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetUniqueQuantizedTimesFromSamplesInRangeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetUniqueQuantizedTimesFromSamplesInRangeStmt = dbGuard.Prepare(SELECT_UNIQUE_QUANTIZED_TIMES_FROM_SAMPLES_IN_RANGE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetUniqueQuantizedTimesFromSamplesForCategoryInRange()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetUniqueQuantizedTimesFromSamplesForCategoryInRangeStmt = dbGuard.Prepare(SELECT_UNIQUE_QUANTIZED_TIMES_FROM_SAMPLES_FOR_CATEGORY_IN_RANGE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetCallNamesForCategoryStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetCallNamesForCategoryStmt = dbGuard.Prepare(SELECT_CALLNAMES_FOR_CATEGORY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetProfileTraceInfoForCallCategoryStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetProfileTraceInfoForCallCategoryStmt = dbGuard.Prepare(SELECT_PROFILE_TRACE_FOR_CALL_CATEGORY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetProfileTraceInfoForAPINameStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetProfileTraceInfoForAPINameStmt = dbGuard.Prepare(SELECT_PROFILE_TRACE_FOR_API_NAME_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetProfileTraceInfoByCallCategoryAndAPINameStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetProfileTraceInfoByCallCategoryAndAPINameStmt = dbGuard.Prepare(SELECT_PROFILE_TRACE_FOR_CALL_TYPE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetSampleValuesForCounterWithNameStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetSampleValuesForCounterWithNameStmt = dbGuard.Prepare(SELECT_SAMPLES_FOR_COUNTER_WITH_NAME_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetAverageSampleValuesForCounterCategoryStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetAverageSampleValuesForCounterCategoryStmt = dbGuard.Prepare(SELECT_AVERAGE_VALUES_FOR_COUNTER_CATEGORY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetMaxSampleValuesForCounterCategoryStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetMaxSampleValuesForCounterCategoryStmt = dbGuard.Prepare(SELECT_MAX_VALUES_FOR_COUNTER_CATEGORY_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetAllSamples()
{
  SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

  m_pGetAllSamples = dbGuard.Prepare(SELECT_ALL_SAMPLES_SQL);

  return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetProfileTraceInfoStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetProfileTraceInfoStmt = dbGuard.Prepare(SELECT_PROFILE_TRACE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetProfileTraceInfoByCallNameStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetProfileTraceInfoByCallNameStmt = dbGuard.Prepare(SELECT_PROFILE_TRACE_BY_CALLNAME_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetProfileTraceInfoMaxCpuEndForFrameNumStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetProfileTraceInfoMaxCpuTimeForFrameNumStmt = dbGuard.Prepare(SELECT_TRACE_MAX_CPUEND_TIME_FOR_FRAMENUM_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetProfileTraceInfoForCallCategoryWithNumCallsAboveThreshold()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetTracesForCallCategoryWithNumCallsAboveThresholdStmt = dbGuard.Prepare(SELECT_TRACES_FOR_CATEGORY_WITH_NUM_CALLS_ABOVE_THRESHOLD);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetFirstTraceCallTimestampStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetFirstTraceCallStmt = dbGuard.Prepare(SELECT_FIRST_TRACE_CALL_TIMESTAMP_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetCallNamesForRangeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetCallNamesInRangeStmt = dbGuard.Prepare(SELECT_CALLNAMES_FROM_TRACE_IN_RANGE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetFrameTimeRangeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetFrameTimeRangeStmt = dbGuard.Prepare(SELECT_FRAME_TIME_RANGE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetNearestSampleValueBeforeTimeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetNearestSampleValueBeforeTimeStmt = dbGuard.Prepare(SELECT_FIRST_VALUE_BEFORE_TIME_FOR_COUNTER_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetNearestSampleValueAfterTimeStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetNearestSampleValueAfterTimeStmt = dbGuard.Prepare(SELECT_FIRST_VALUE_AFTER_TIME_FOR_COUNTER_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareGetFirstDifferenceForCounterStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetFirstDiffForCounter = dbGuard.Prepare(SELECT_FIRST_DIFFERENCE_FOR_COUNTER_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

/// @brief              Prepare the reading statements to read all the devices 
///                     from the database.
/// @returns    status  success if the operation was successful, failure
///                     otherwise
status LPGPU2DatabaseAccessor::PrepareGetAllDevicesStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseReadConnection() };

    m_pGetAllDevicesStmt = dbGuard.Prepare(SELECT_ALL_DEVICES_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

status LPGPU2DatabaseAccessor::PrepareUpdateShaderTraceFromMainShaderStatement()
{
    SQLiteDatabaseGuard dbGuard{ GetDatabaseWriteConnection() };

    m_pUpdateShaderTraceFromMainShaderStmt = dbGuard.Prepare(UPDATE_SHADERTRACEFROMMAIN_SOURCE_SQL);

    return CheckForErrorAndReturn(dbGuard);
}

/// @brief              Prepare the reading statements to read information 
///                     from the database.
/// @returns    status  success if the operation was successful, failure otherwise
status LPGPU2DatabaseAccessor::PrepareReadStatements()
{
    auto bReturn = success;

    bReturn &= PrepareGetAllSamples();
    bReturn &= PrepareGetProfileTraceInfoStatement();
    bReturn &= PrepareGetProfileTraceInfoByCallNameStatement();
    bReturn &= PrepareGetProfileTraceInfoMaxCpuEndForFrameNumStatement();
    bReturn &= PrepareGetProfileTraceInfoForCallCategoryWithNumCallsAboveThreshold();
    bReturn &= PrepareGetFirstTraceCallTimestampStatement();
    bReturn &= PrepareGetCallNamesForRangeStatement();
    bReturn &= PrepareGetFrameTimeRangeStatement();
    bReturn &= PrepareGetEstimatedPowerSamplesStatement();
    bReturn &= PrepareGetEstimatedPowerComponentsStatement();
    bReturn &= PrepareGetAnnotationsStatement();
    bReturn &= PrepareGetAnnotationByIdStatement();
    bReturn &= PrepareGetAnnotationsInRangeStatement();
    bReturn &= PrepareGetAnnotationsInFrameRangeStatement();
    bReturn &= PrepareGetGlobalAnnotations();
    bReturn &= PrepareGetUserAnnotations();
    bReturn &= PrepareGetGPUTimersStatement();
    bReturn &= PrepareGetGPUTimersForFrameNumInRangeStatement();
    bReturn &= PrepareGetStackTracesStatement();
    bReturn &= PrepareGetStackTraceParamsStatement();
    bReturn &= PrepareGetStackTraceParamsForCallCategoryAndAPINameStatement();
    bReturn &= PrepareGetNumThreadsFromTraceParamsStatement();
    bReturn &= PrepareGetStackTraceParamsForFrameAndDrawNumStatement();
    bReturn &= PrepareGetStackTraceParamsForCallCategoryStatment();
    bReturn &= PrepareGetStackTraceParamsForAPINameStatement();
    bReturn &= PrepareGetShaderTracesStatement();
    bReturn &= PrepareGetShaderTracesByAPIIDStatement();
    bReturn &= PrepareGetShaderTracesInRangeStatement();
    bReturn &= PrepareGetShaderTracesInFrameRangeStatement();
    bReturn &= PrepareGetShaderAsmStatByShaderId();
    bReturn &= PrepareGetShaderToAsmByShaderId();
    bReturn &= PrepareGetCounterWeightsStatement();
    bReturn &= PrepareGetCounterWeightForCounterWithNameStatement();
    bReturn &= PrepareGetWeightedSumForCounterInRangeStatement();
    bReturn &= PrepareGetWeightedSamplesForCounterByNameStatement();
    bReturn &= PrepareGetRegionsOfInterestStatement();
    bReturn &= PrepareGetRegionOfInterestByIdStatement();
    bReturn &= PrepareGetAccumulatedTimeFromCallSummaryStatement();
    bReturn &= PrepareGetCallSummaryForRegionStatement();
    bReturn &= PrepareGetCallsPerTypeForRegionStatement();
    bReturn &= PrepareGetUniqueQuantizedTimesFromSamplesStatement();
    bReturn &= PrepareGetUniqueQuantizedTimesFromSamplesInRangeStatement();
    bReturn &= PrepareGetUniqueQuantizedTimesFromSamplesForCategoryInRange();
    bReturn &= PrepareGetCallNamesForCategoryStatement();
    bReturn &= PrepareGetProfileTraceInfoForCallCategoryStatement();
    bReturn &= PrepareGetProfileTraceInfoForAPINameStatement();
    bReturn &= PrepareGetProfileTraceInfoByCallCategoryAndAPINameStatement();
    bReturn &= PrepareGetSampleValuesForCounterWithNameStatement();
    bReturn &= PrepareGetAverageSampleValuesForCounterCategoryStatement();
    bReturn &= PrepareGetMaxSampleValuesForCounterCategoryStatement();
    bReturn &= PrepareGetNearestSampleValueBeforeTimeStatement();
    bReturn &= PrepareGetNearestSampleValueAfterTimeStatement();
    bReturn &= PrepareGetAllDevicesStatement();
    bReturn &= PrepareGetFirstDifferenceForCounterStatement();

    try
    {
        m_readStatements.push_back(m_pGetAllSamples);
        m_readStatements.push_back(m_pGetProfileTraceInfoByCallNameStmt);
        m_readStatements.push_back(m_pGetProfileTraceInfoStmt);
        m_readStatements.push_back(m_pGetProfileTraceInfoMaxCpuTimeForFrameNumStmt);
        m_readStatements.push_back(m_pGetTracesForCallCategoryWithNumCallsAboveThresholdStmt);
        m_readStatements.push_back(m_pGetFirstTraceCallStmt);
        m_readStatements.push_back(m_pGetCallNamesInRangeStmt);
        m_readStatements.push_back(m_pGetFrameTimeRangeStmt);
        m_readStatements.push_back(m_pGetEstimatedPwrComponentsStmt);
        m_readStatements.push_back(m_pGetEstimatedPwrSamplesStmt);
        m_readStatements.push_back(m_pGetAnnotationsStmt);
        m_readStatements.push_back(m_pGetAnnotationByIdStmt);
        m_readStatements.push_back(m_pGetAnnotationsInRangeStmt);
        m_readStatements.push_back(m_pGetAnnotationsInFrameRangeStmt);
        m_readStatements.push_back(m_pGetGlobalAnnotations);
        m_readStatements.push_back(m_pGetUserAnnotations);
        m_readStatements.push_back(m_pGetGPUTimersStmt);
        m_readStatements.push_back(m_pGetGPUTimersForFrameNumInRangeStmt);
        m_readStatements.push_back(m_pGetShaderTracesStmt);
        m_readStatements.push_back(m_pGetShaderTracesByAPIIdStmt);
        m_readStatements.push_back(m_pGetStackTracesStmt);
        m_readStatements.push_back(m_pGetShaderTracesInRangeStmt);
        m_readStatements.push_back(m_pGetShaderTracesInFrameRangeStmt);
        m_readStatements.push_back(m_pGetShaderStatByShaderIdStmt);
        m_readStatements.push_back(m_pGetShaderToAsmByShaderIdStmt);
        m_readStatements.push_back(m_pGetStackTraceParamsStmt);
        m_readStatements.push_back(m_pGetStackTraceParamsForCallCategoryAndAPINameStmt);
        m_readStatements.push_back(m_pGetStackTraceParamsForCallCategoryStmt);
        m_readStatements.push_back(m_pGetStackTraceParamsForAPINameStmt);
        m_readStatements.push_back(m_pGetNumThreadsFromTraceParamsStmt);
        m_readStatements.push_back(m_pGetStackTraceParamsForFrameAndDrawNumStmt);
        m_readStatements.push_back(m_pGetCounterWeightsStmt);
        m_readStatements.push_back(m_pGetCounterWeightForCounterByNameStmt);
        m_readStatements.push_back(m_pGetWeightedSumForCounterInRangeStmt);
        m_readStatements.push_back(m_pGetWeightedSamplesForCounterByNameStmt);
        m_readStatements.push_back(m_pGetRegionsOfInterestStmt);
        m_readStatements.push_back(m_pGetRegionOfInterestByIdStmt);
        m_readStatements.push_back(m_pGetAccumulatedTimeFromCallSummaryStmt);
        m_readStatements.push_back(m_pGetCallSummaryForRegionStmt);
        m_readStatements.push_back(m_pGetCallsPerTypeForRegionStmt);
        m_readStatements.push_back(m_pGetUniqueQuantizedTimesFromSamplesStmt);
        m_readStatements.push_back(m_pGetUniqueQuantizedTimesFromSamplesInRangeStmt);
        m_readStatements.push_back(m_pGetUniqueQuantizedTimesFromSamplesForCategoryInRangeStmt);
        m_readStatements.push_back(m_pGetCallNamesForCategoryStmt);
        m_readStatements.push_back(m_pGetProfileTraceInfoForCallCategoryStmt);
        m_readStatements.push_back(m_pGetProfileTraceInfoForAPINameStmt);
        m_readStatements.push_back(m_pGetProfileTraceInfoByCallCategoryAndAPINameStmt);
        m_readStatements.push_back(m_pGetSampleValuesForCounterWithNameStmt);
        m_readStatements.push_back(m_pGetAverageSampleValuesForCounterCategoryStmt);
        m_readStatements.push_back(m_pGetMaxSampleValuesForCounterCategoryStmt);
        m_readStatements.push_back(m_pGetNearestSampleValueBeforeTimeStmt);
        m_readStatements.push_back(m_pGetNearestSampleValueAfterTimeStmt);
        m_readStatements.push_back(m_pGetAllDevicesStmt);        
    }
    catch(...)
    {
        bReturn = failure;
        AppendErrorMessage(L"Cannot store read statements due to lack of memory");
    }
    
    if(bReturn == success)
    {
        bReturn &= std::all_of(m_readStatements.begin(), m_readStatements.end(), [](sqlite3_stmt *stmt)
        {
            return stmt != nullptr;
        });
    }    
   

    return bReturn;
}

/// @brief              Prepare the writing statements to write information 
///                     into the database.
/// @returns    status  success if the operation was successful, failure otherwise.
status LPGPU2DatabaseAccessor::PrepareWriteStatements()
{
    auto bReturn = success;

    bReturn &= PrepareInsertLPGPU2CounterSampleBlob();
    bReturn &= PrepareInsertProfileTraceInfoStatement();
    bReturn &= PrepareInsertEstimatedPowerStatement();
    bReturn &= PrepareInsertEstimatedPowerComponentStatement();
    bReturn &= PrepareInsertStackTraceStatement();
    bReturn &= PrepareInsertStackTraceParametersStatement();
    bReturn &= PrepareInsertAnnotationStatement();
    bReturn &= PrepareInsertGPUTimerStatement();
    bReturn &= PrepareInsertShaderTraceStatement();
    bReturn &= PrepareInsertShaderAsmStatStatement();
    bReturn &= PrepareInsertShaderToAsmStatement();
    bReturn &= PrepareInsertCounterWeightStatement();
    bReturn &= PrepareInsertRegionOfInterestStatement();
    bReturn &= PrepareInsertCallSummaryStatement();
    bReturn &= PrepareInsertCallsPerTypeStatement();
    bReturn &= PrepareInsertCallCategoryStatement();
    bReturn &= PrepareInsertCounterCategoryStatement();    

    bReturn &= PrepareDeleteRegionsOfInterestStatement();
    bReturn &= PrepareDeleteAnnotationsStatement();
    bReturn &= PrepareDeleteCallsCategoryStatement();
    bReturn &= PrepareDeleteCountersCategoryStatement();
    bReturn &= PrepareDeleteShaderStatsForShaderIdStatement();

    bReturn &= PrepareUpdateShaderTraceSourceStatement();
    bReturn &= PrepareUpdateShaderTraceAsmStatement();
    bReturn &= PrepareUpdateShaderTraceBinaryStatement();
    bReturn &= PrepareUpdateShaderTraceDebugStatement();
    bReturn &= PrepareUpdateShaderTraceFromMainShaderStatement();

    bReturn &= PrepareBeginTransactionStatement();
    bReturn &= PrepareEndTransactionStatement();
    bReturn &= PrepareRollbackTransactionStatement();

    try
    {
        m_writeStatements.push_back(m_pInsertProfileInfoStmt);
        m_writeStatements.push_back(m_pInsertEstimatedPowerSampleStmt);
        m_writeStatements.push_back(m_pInsertEstimatedPowerComponentStmt);
        m_writeStatements.push_back(m_pInsertAnnotationStmt);
        m_writeStatements.push_back(m_pInsertGPUTimerStmt);
        m_writeStatements.push_back(m_pInsertShaderTraceStmt);
        m_writeStatements.push_back(m_pInsertShaderStatStmt);
        m_writeStatements.push_back(m_pInsertShaderToAsmStmt);
        m_writeStatements.push_back(m_pInsertStackTraceStmt);        
        m_writeStatements.push_back(m_pInsertStackTraceParameterStmt);
        m_writeStatements.push_back(m_pInsertCounterWeightStmt);
        m_writeStatements.push_back(m_pInsertRegionOfInterestStmt);
        m_writeStatements.push_back(m_pInsertCallSummaryStmt);
        m_writeStatements.push_back(m_pInsertCallsPerTypeStmt);
        m_writeStatements.push_back(m_pInsertCallCategoryStmt);
        m_writeStatements.push_back(m_pInsertCounterCategoryStmt);
        m_writeStatements.push_back(m_pInsertShaderStatStmt);

        m_writeStatements.push_back(m_pDeleteRegionsOfInterestStmt);
        m_writeStatements.push_back(m_pDeleteAnnotationsStmt);
        m_writeStatements.push_back(m_pDeleteCallsCategoriesStmt);
        m_writeStatements.push_back(m_pDeleteCountersCategoriesStmt);
        m_writeStatements.push_back(m_pDeleteShaderStatsForShaderIdStmt);

        m_writeStatements.push_back(m_pUpdateShaderTraceSourceStmt);
        m_writeStatements.push_back(m_pUpdateShaderTraceAsmStmt);
        m_writeStatements.push_back(m_pUpdateShaderTraceBinaryStmt);
        m_writeStatements.push_back(m_pUpdateShaderTraceDebugStmt);
        m_writeStatements.push_back(m_pUpdateShaderTraceFromMainShaderStmt);

        m_writeStatements.push_back(m_pBeginTransactionStmt);
        m_writeStatements.push_back(m_pEndTransactionStmt);
        m_writeStatements.push_back(m_pRollbackTransactionStmt);
    }
    catch(...)
    {
        bReturn = failure;
        AppendErrorMessage(L"Cannot store write statements due to lack of memory");
    }
    
    if (bReturn == success)
    {
        bReturn &= std::all_of(m_writeStatements.begin(), m_writeStatements.end(), [](sqlite3_stmt *stmt)
        {
            return stmt != nullptr;
        });
    }

    return bReturn;
}

/// @brief                  Appends en error message to this class state.
/// @param[in]  vMessage    Message to append to this class state.
void LPGPU2DatabaseAccessor::AppendErrorMessage(const gtString &vMessage)
{
    m_errorMsg << vMessage << L"\n";
}

/// @brief                 Checks for an error in the vDbGuard. If there is
///                        an error in the vDbGuard, appends the vMessage to
///                        this class m_ErrorMsg member, and return true or
///                        false depending on the error state.
/// @param[in]  vDbGuard   SQLiteDatabaseGuard to verify for errors.
/// @returns    status     success if the operation was successful, failure otherwise.
status LPGPU2DatabaseAccessor::CheckForErrorAndReturn(const SQLiteDatabaseGuard &vDbGuard)
{
    if (vDbGuard.HasErrors())
    {
        AppendErrorMessage(vDbGuard.GetErrorMessage());
        return failure;
    }
    else
    {
        return success;
    }
}

/// @brief                  Checks for an error in the vStmtGuard. If there is
///                         an error in the vStmtGuard, appends the vMessage to
///                         this class m_ErrorMsg member, and return true or
///                         false depending on the error state.
/// @param[in]  vStmtGuard  SQLiteStatementGuard to verify for errors.
/// @returns    status      success if the operation was successful, failure otherwise.
status LPGPU2DatabaseAccessor::CheckForErrorAndReturn(const SQLiteStatementGuard &vStmtGuard)
{
    if (vStmtGuard.IsValid())
    {
        return success;
    }
    else
    {
        AppendErrorMessage(vStmtGuard.GetErrorMessage());
        return failure;
    }
}

} // namespace db
} // namespace lpgpu2
