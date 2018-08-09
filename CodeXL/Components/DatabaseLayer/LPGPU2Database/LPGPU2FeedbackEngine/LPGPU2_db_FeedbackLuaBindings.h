#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstdio>
//#include <dlfcn.h>

#ifndef uint
using uint = unsigned int;
#endif

using namespace std;
using namespace lpgpu2::db;

#include <lua.hpp>

#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

#define  LUA_TAG    "Lua says:"
#define LOGLUA(...) PrintOutput(__VA_ARGS__)


class LuaBinding
{
public:
    explicit LuaBinding(LPGPU2DatabaseAdapter &vDbAdaptor)
        : dbAdaptor(vDbAdaptor)
    {
        m_lua = luaL_newstate();

        // Make the lua symbols available globally. Needed to load other modules , eg rex_pcre
        //dlopen("liblua.so", RTLD_NOW | RTLD_GLOBAL);
        if (m_lua)
        {
            RegisterLuaBindings();
            RegisterClassBindings();

            // Defines for Annotation types.
            lua_pushnumber(m_lua, 10); lua_setglobal(m_lua, "LPGPU_SUGGESTION"); // The majority
            lua_pushnumber(m_lua, 11); lua_setglobal(m_lua, "LPGPU_INFORMATION");
            lua_pushnumber(m_lua, 12); lua_setglobal(m_lua, "LPGPU_ISSUE");
            // The following are placemarkers for possible future Annotation use:
            lua_pushnumber(m_lua, 13); lua_setglobal(m_lua, "LPGPU_ERROR");
            lua_pushnumber(m_lua, 14); lua_setglobal(m_lua, "LPGPU_GENERAL");
            lua_pushnumber(m_lua, 15); lua_setglobal(m_lua, "LPGPU_LIMITATION");

            lua_pushnumber(m_lua, 30); lua_setglobal(m_lua, "LPGPU_SUGGESTION_GLOBAL");
            lua_pushnumber(m_lua, 31); lua_setglobal(m_lua, "LPGPU_INFORMATION_GLOBAL");
            lua_pushnumber(m_lua, 32); lua_setglobal(m_lua, "LPGPU_ISSUE_GLOBAL");
            lua_pushnumber(m_lua, 33); lua_setglobal(m_lua, "LPGPU_ERROR_GLOBAL");
            lua_pushnumber(m_lua, 34); lua_setglobal(m_lua, "LPGPU_GENERAL_GLOBAL");
            lua_pushnumber(m_lua, 35); lua_setglobal(m_lua, "LPGPU_LIMITATION_GLOBAL");
        }
    }

    ~LuaBinding()
    {
        lua_close(m_lua);
        m_lua = NULL;
    }

    void SetOutputCallbackFunc(function<void(const string&)> outputCallBackFunc)
    {
        m_outputCallbackFunc = outputCallBackFunc;
    }

    template<typename... Args>
    void PrintOutput(const char *fmt, Args&&... args)
    {
        // print the result to a string, same as sprintf
        gtASCIIString outputStr;
        outputStr.appendFormattedString(fmt, std::forward<Args>(args)...);

        // Calls the output func
        if (m_outputCallbackFunc != nullptr)
        {
            m_outputCallbackFunc(outputStr.asCharArray());
        }
        else
        {
            // If no callback is defined, send the output to stdout
            printf(fmt, std::forward<Args>(args)...);
        }
    }

    void Close_Database(void)
    {
        // LOGLUA("Close_Database");
    }

    void AddTrace(std::string aString)
    {
        LOGLUA("AddTrace: %s\n", aString.c_str());
    }

    static void ReadFeedbackTable(lua_State *L, int stack_index, gtVector< AvailableFeedback > &feedback)
    {
        // Push another reference to the table on top of the stack (so we know
        // where it is, and this function can work for negative, positive and
        // pseudo indices
        lua_pushvalue(L, stack_index);

        lua_pushnil(L);

        int feedback_element = 0;
        AvailableFeedback available;
        while (lua_next(L, -2))
        {
            lua_pushvalue(L, -2);

            const char *key = lua_tostring(L, -1);

            if (lua_isstring(L, -2))
            {
                const char *value = lua_tostring(L, -2);

                if (feedback_element == 0)
                {
                    available.scriptName.fromASCIIString(value);
                }
                else if (feedback_element == 1)
                {
                    available.scriptFunction.fromASCIIString(value);
                }
                else if (feedback_element == 2)
                {
                    available.scriptCatagory.fromASCIIString(value);
                }
                else if (feedback_element == 3)
                {
                    available.inputName.fromASCIIString(value);
                }
                else if (feedback_element == 4)
                {
                    available.inputDescription.fromASCIIString(value);
                }
                else if (feedback_element == 5)
                {
                    available.inputDefault = strtol(value, 0, 0);
                }
                else if (feedback_element == 6)
                {
                    available.inputMin = strtol(value, 0, 0);
                }
                else if (feedback_element == 7)
                {
                    // Last element in the structure
                    available.inputMax = strtol(value, 0, 0);
                    feedback.push_back(available);
                }
            }
            else if (lua_isnumber(L, -2))
            {
                const int value = lua_tonumber(L, -2);
                if (feedback_element == 5)
                {
                    available.inputDefault = value;
                }
                else if (feedback_element == 6)
                {
                    available.inputMin = value;
                }
                else if (feedback_element == 7)
                {
                    // Last element in the structure
                    available.inputMax = value;
                    feedback.push_back(available);
                }
            }
            else if (lua_istable(L, -2))
            {
                // Recursive call
                ReadFeedbackTable(L, -2, feedback);
            }
            else
            {
                printf("Unknown type. key = %s \n", key);
            }

            lua_pop(L, 2);
            feedback_element++;
        }
        lua_pop(L, 1);
    }


    // This function is just for testing.
    void RunGetAvailable(std::string fileName)
    {
        gtVector< AvailableFeedback> feedback;
        gtString scriptFilename;
        scriptFilename.fromASCIIString(fileName.c_str());

        GetAvailableFeedbackOptions(scriptFilename, feedback);

        // Results:
        for (uint i = 0; i < feedback.size(); i++)
        {
            printf("\n %s, %s, %s, \n%s, %s, %d \n ", feedback[i].scriptName.asASCIICharArray(), feedback[i].scriptFunction.asASCIICharArray(), feedback[i].scriptCatagory.asASCIICharArray(), feedback[i].inputName.asASCIICharArray(), feedback[i].inputDescription.asASCIICharArray(), feedback[i].inputDefault);
        }
    }

    // Call this to see which feedback options are available for the current database.
    bool GetFeedbackScriptVersion(const gtString &scriptFilename, gtString &version)
    {
        bool success = false;
        version.makeEmpty();

        if (luaL_loadfile(m_lua, scriptFilename.asASCIICharArray()) == 0)
        {
            LOGLUA("Feedback file loaded: %s. \n", scriptFilename.asASCIICharArray());
            lua_pcall(m_lua, 0, 0, 0);

            lua_getglobal(m_lua, "getScriptVersion");
            if (!lua_isfunction(m_lua, -1))
            {
                LOGLUA("Feedback function not found: getScriptVersion. \n");
                version << L"<b>getScriptVersion()</b> was not defined.";
            }
            else
            {
                // lua_pcall(mlua, #arguments, #results, 0)
                lua_pcall(m_lua, 0, 1, 0);

                // Read the return value
                if (!lua_isnil(m_lua, -1))
                {
                    version << lua_tostring(m_lua, -1);
                    lua_pop(m_lua,1);
                }
                else
                {
                    version << L"Cannot read the script version.";
                }

                success = true;
            }
        }
        else
        {
            string error = lua_tostring(m_lua, -1);
            LOGLUA("Failed to open Feedback file %s. \n", error.c_str());

        }
        return success;
    }

    // Call this to see which feedback options are available for the current database.
    bool GetAvailableFeedbackOptions(const gtString &scriptFilename, gtVector <AvailableFeedback> &feedback)
    {
        bool success = false;
        if (luaL_loadfile(m_lua, scriptFilename.asASCIICharArray()) == 0)
        {
            LOGLUA("Feedback file loaded: %s. \n", scriptFilename.asASCIICharArray());
            lua_pcall(m_lua, 0, 0, 0);

            lua_getglobal(m_lua, "get_available_feedback");
            if (!lua_isfunction(m_lua, -1))
            {
                LOGLUA("Feedback function not found: get_available_feedback. \n");
            }
            else
            {
                // lua_pcall(mlua, #arguments, #results, 0)
                if (lua_pcall(m_lua, 0, 1, 0))
                {
                    // Display any errors returned by Lua.
                    while (lua_gettop(m_lua))
                    {
                        printf("stack = %d \n", lua_gettop(m_lua));
                        printf("message = %s \n", lua_tostring(m_lua, -1));
                        lua_pop(m_lua, 1);
                    }
                }
                else
                {
                  // Read the return table
                  ReadFeedbackTable(m_lua, -1, feedback);
                  success = true;
                }

            }
        }
        else
        {
            string error = lua_tostring(m_lua, -1);
            LOGLUA("Failed to open Feedback file %s. \n", error.c_str());

        }
        return success;
    }

    // Call this to run a feedback script.
    void RunScript(std::string fileName, std::string functionName, int num_args, int *args)
    {
        if ((num_args > 0) && (args == NULL))
        {
            LOGLUA("Error: NULL args pointer, but num_args = %d", num_args);
            return;
        }
        if (luaL_loadfile(m_lua, fileName.c_str()) == 0)
        {
            LOGLUA("Feedback file loaded: %s. \n", fileName.c_str());
            lua_pcall(m_lua, 0, 0, 0);

            lua_getglobal(m_lua, functionName.c_str());

            if (!lua_isfunction(m_lua, -1))
            {
                LOGLUA("Feedback function not found: %s. \n", functionName.c_str());
            }
            else
            {
                LOGLUA("Running Feedback function: %s with %d arguments\n", functionName.c_str(), num_args);
                for (int i = 0; i < num_args; i++)
                {
                    lua_pushnumber(m_lua, args[i]);   /* push argument */
                }

                // lua_pcall(mlua, #arguments, #results, 0)
                if (lua_pcall(m_lua, num_args, 0, 0))
                {
                    // Display any errors returned by Lua.
                    while (lua_gettop(m_lua))
                    {
                        printf("stack = %d \n", lua_gettop(m_lua));
                        printf("message = %s \n", lua_tostring(m_lua, -1));
                        lua_pop(m_lua, 1);
                    }
                }
            }
        }
        else
        {
            string error = lua_tostring(m_lua, -1);
            LOGLUA("Failed to open Feedback file %s. \n", error.c_str());
        }
    }

    // This function is just for testing.
    void lpgpu2_stackdump(lua_State *L)
    {
        int i;
        int top = lua_gettop(L);

        LOGLUA("total in stack = %d", top);

        for (i = 1; i <= top; i++)
        {
            int t = lua_type(L, i);
            switch (t)
            {
                case LUA_TNUMBER:
                    LOGLUA("%d number = %g \n", i, lua_tonumber(L, i));
                    break;

                case LUA_TBOOLEAN:
                    LOGLUA("%d bool = %g \n", i, lua_tonumber(L, i));
                    break;

                default:
                    LOGLUA("(%d) %s \n", i, lua_typename(L, t));
                    break;
            }
        }
    }


private:
    lua_State *m_lua = NULL;
    std::function<void(const std::string&)> m_outputCallbackFunc = nullptr;

    LPGPU2DatabaseAdapter &dbAdaptor;     //!< Reference to the database adapter used for interacting with the database.


                                          // TODO 
    int lpgpu2_getCompilerOutput()//self, gpuModel='Mali-T880')
    {
        gtVector<ShaderTrace> shaderTraces;

        lpgpu2_getShaders(shaderTraces);
#if 0
        /*
        ret=[]
        types=[]
        shaders=self.getShaders()
        frames=self.getColumnFromTable('frameNum', 'shader')
        draws=self.getColumnFromTable('drawNum', 'shader')
        types=self.getShaderType()
        res=''
        */
        for (int i = 0; i < shaderTraces.size(); i++)
        {
            //  shaderType=types[i][0]
            flag = '';
            if (shaderTraces[i].m_shaderType == 0) //: #OpenCL
                flag = '-k';
            else if (shaderTraces[i].m_shaderType == 35632)//: #GL Fragment
                flag = '-f';
            else if (shaderTraces[i].m_shaderType == 35633)//: #GL Vertex
                flag = '-v';
            else if (shaderTraces[i].m_shaderType == 36313)//: #GL Geometry
                flag = '-g';
            else if (shaderTraces[i].m_shaderType == 36487)//: #GL TEsselation evaluation
                flag = '-e';
            else if (shaderTraces[i].m_shaderType == 36488)//: #GL TEsselation control
                flag = '-t';
            else if (shaderTraces[i].m_shaderType == 37305)//: #GL Compute
                flag = '-C';

            code = shaderTraces[i].m_text;

            //  kernels=[]
            if (flag == '-k')
            {
                counter = 0;
                for (word in code.split())
                {
                    if (word == '__kernel' && counter == 0)
                        counter = counter + 1;
                    else if (word == 'void' && counter == 1)
                        counter = counter + 1;
                    else if (counter == 2)
                    {
                        kernels.append(word.split('(')[0]); // #To control when the bracket is in the char next to the name
                        counter = 0;
                    }
                    else
                        counter = 0;
                }
            }
            compiler = './compilers/'
                file_name = 'shader.tmp'
                shader_file = open(file_name, 'w')
                command = subprocess.run(['echo', code], stdout = shader_file);// # If I write with python, then the shell is not able to read the contents of the file until the python script ends

            output = ''
                cmd = []
                if (gpuModel[0:4] == 'Mali')
                {
                    cmd.append(compiler + 'malisc')
                        cmd.append(flag)
                        for (k in kernels)
                        {
                            cmd.append('--name')
                                cmd.append(k)
                        }

                    cmd.append(file_name)
                        cmd.append('-c')
                        cmd.append('Mali-T880') //#This is hardcoded for the Mali model in the S7. Ideally, info on the device would be held in the database
                        command = subprocess.run(cmd, stdout = subprocess.PIPE)
                        output = command.stdout
                }
                else if (gpuModel[0:6] == 'Series')
                {
                    FNULL = open(os.devnull, 'w')
                        command = subprocess.run([compiler + 'GLSLESCompiler_' + gpuModel, file_name, 'compiler_out', flag, '-profile'], stdout = FNULL)
                        command = subprocess.run(['cat', 'compiler_out.prof'], stdout = subprocess.PIPE)
                        output = command.stdout
                        subprocess.run('rm compiler_out.*', shell = True)
                }


            subprocess.run(['rm', file_name])

                shader_info = []
                lines = output.splitlines()
                start = -1
                res_p = ''
                if (gpuModel[0:4] == 'Mali')
                {
                    for (j in range(len(lines)))
                    {
                        l = lines[j].decode('ascii')
                            l_tokens = l.split()
                            if (len(l_tokens) == 1 and l_tokens[0] in kernels) or (len(l_tokens)>2 and l_tokens[1] == 'work') :
                                start = j - 1;

                            else if (start != -1 and len(l_tokens)>1 and l_tokens[0] == 'Note:')
                                for (k in range(j - 1 - start))
                                    res_p = res_p + lines[start + k].decode('ascii') + '\n'
                                    //#        ret.append(str(shaderType)+'|||'+res)
                                    res = res + '|||f' + str(frames[i][0]) + '|||d' + str(draws[i][0]) + '|||' + str(shaderType) + '|||' + res_p
                                    res_p = ''
                                    start = -1
                    }
                }
                else if (gpuModel[0:6] == 'Series')
                {
                    res = res + '|||f' + str(frames[i][0]) + '|||d' + str(draws[i][0]) + '|||' + str(shaderType) + '|||'
                        for l in lines :
                    res = res + l.decode('ascii') + '\n'
                }
            return res;
#endif
            return 0;
        }

        int lpgpu2_writeAnnotation(int annotationType, const char *message, int frame, int draw, int start, int end)
        {
            Annotation annotation;
            annotation.m_annotationType = annotationType;
            annotation.m_text.fromASCIIString(message);
            annotation.m_frameNum = frame;
            annotation.m_drawNum = draw;
            annotation.m_cpuStartTime = start;
            annotation.m_cpuEndTime = end;
            return dbAdaptor.InsertAnnotation(annotation);
        }

        int lpgpu2_getPlatform(gtString& s)
        {
            return dbAdaptor.GetLPGPU2Platform(s);
        }

        int lpgpu2_getHardware(gtString& s)
        {
            return dbAdaptor.GetLPGPU2Hardware(s);
        }

        int lpgpu2_getDCAPI(gtString& s)
        {
            return dbAdaptor.GetLPGPU2DCAPI(s);
        }

        int lpgpu2_getRAgent(gtString& s)
        {
            return dbAdaptor.GetLPGPU2RAgent(s);
        }

        int lpgpu2_getHardwareID(gtString& s)
        {
            return dbAdaptor.GetLPGPU2HardwareID(s);
        }

        int lpgpu2_getBlobSize(gtString& s)
        {
            return dbAdaptor.GetLPGPU2BlobSize(s);
        }

        int lpgpu2_getSessionInfo(AMDTProfileSessionInfo& sessionInfo)
        {
            return dbAdaptor.GetSessionInfo(sessionInfo);
        }

        // Data from the Samples Table.
        int lpgpu2_getUniqueTimesFromSamples(gtVector<gtUInt64> &vUniqueTimes)
        {
            return dbAdaptor.GetUniqueQuantizedTimesFromSamples(vUniqueTimes);
        }

        int lpgpu2_getUniqueTimesFromSamplesWithinRange(gtUInt64 rangeStart, gtUInt64 rangeEnd, gtVector<gtUInt64> &uniqueTimes)
        {
            return dbAdaptor.GetUniqueQuantizedTimesFromSamplesInRange(rangeStart, rangeEnd, uniqueTimes);
        }

        int lpgpu2_getUniqueQuantizedTimesFromSamplesByCategoryInRange(const gtString &counterCategory, gtUInt64 rangeStart, gtUInt64 rangeEnd, gtVector<gtUInt64> &uniqueTimes)
        {
            return dbAdaptor.GetUniqueQuantizedTimesFromSamplesByCategoryInRange(counterCategory, rangeStart, rangeEnd, uniqueTimes);
        }

        int lpgpu2_getUniqueFrameNumFromTraces(gtVector<gtUInt64> &uniqueFrameNumbers)
        {
            return dbAdaptor.GetUniqueProfileTraceFrameNumbers(uniqueFrameNumbers);
        }

        int lpgpu2_getAllSamples(gtVector<PPSampleData> &samples)
        {
            return dbAdaptor.GetAllSamples(samples);
        }

        //
        int lpgpu2_getMaxValuesForCategory(const gtString &counterCategory, gtVector<PPSampleData> &sampleValues)
        {
            return dbAdaptor.GetMaxSampleValuesByCounterCategory(counterCategory, sampleValues);
        }

        int lpgpu2_getNumThreads(gtUInt64 &vNumThreads)
        {
            return dbAdaptor.GetNumThreadsFromTraceParameters(vNumThreads);
        }

        int lpgpu2_getAveragedValuesForCategoryWithinRange(const gtString &counterCategory, gtUInt32 rangeStart, gtUInt32 rangeEnd, gtVector<PPSampleData> &sampleValues)
        {
            return dbAdaptor.GetAverageSamplesByCounterCategoryInRange(counterCategory, rangeStart, rangeEnd, sampleValues);
        }

        int lpgpu2_getFirstCallTimestamp(gtUInt64 &cpuStartTimestamp)
        {
            return dbAdaptor.GetFirstTraceCallTimestamp(cpuStartTimestamp);
        }

        int lpgpu2_getSamplesForCounterWithName(const gtString& counterName, gtVector<PPSampleData> &sampleValues)
        {
            return dbAdaptor.GetSamplesForCounterByName(counterName, sampleValues);
        }

        int lpgpu2_getCallNameFromTraceInRange(gtUInt32 rangeStart, gtUInt32 rangeEnd, gtVector<ProfileTraceInfo> &profileTraceInfo)
        {
            return dbAdaptor.GetProfileTraceInfoInRange(rangeStart, rangeEnd, profileTraceInfo);
        }

        int lpgpu2_getProfileTraceInfoListForCallCategory(const gtString &callCategory, gtVector<ProfileTraceInfo> &profileTraceInfoList)
        {
            return dbAdaptor.GetProfileTraceInfoByCallCategory(callCategory, profileTraceInfoList);
        }

        int lpgpu2_getSupportedCalls(gtString callType, gtVector<CallCategory> &callCategories)
        {
            return dbAdaptor.GetCallNamesByCategory(callType, callCategories);
        }

        int lpgpu2_writeCallsPerType(CallsPerType &calls)
        {
            return dbAdaptor.InsertCallsPerType(calls);
        }

        int lpgpu2_getGpuTimers(gtVector<GPUTimer> &gpuTimers)
        {
            return dbAdaptor.GetGPUTimers(gpuTimers);
        }

        int lpgpu2_getGPUTimingForRangeOfFrames(gtUInt32 rangeStart, gtUInt32 rangeEnd, gtVector<GPUTimer> &gpuTimers)
        {
            return dbAdaptor.GetGPUTimersInRange(rangeStart, rangeEnd, gpuTimers);
        }

        int lpgpu2_writeRegion(const RegionOfInterest &regionOfInterest)
        {
            return dbAdaptor.InsertRegionOfInterest(regionOfInterest);
        }

        int lpgpu2_writeCallSummary(const CallSummary &callSummary)
        {
            return dbAdaptor.InsertCallSummary(callSummary);
        }

        int lpgpu2_getProfileTraceInfoForApiAndCallType(const gtString &apiName, const gtString &callCategory, gtVector<ProfileTraceInfo> &profileTraceInfoList)
        {
            return dbAdaptor.GetProfileTraceInfoByCallCategoryAndAPIName(callCategory, apiName, profileTraceInfoList);
        }

        int lpgpu2_getStackTraceParameters(gtVector<StackTraceParameter>& traceParameters)
        {
            return dbAdaptor.GetStackTraceParameters(traceParameters);
        }

        int lpgpu2_getStackTraceParametersForApi(const gtString &apiName, gtVector<StackTraceParameter> &traceParameters)
        {
            return dbAdaptor.GetStackTraceParametersByAPIName(apiName, traceParameters);
        }

        int lpgpu2_getStackTraceParametersForCallType(const gtString &callCategory, gtVector<StackTraceParameter> &traceParameters)
        {
            return dbAdaptor.GetStackTraceParametersByCallCategory(callCategory, traceParameters);
        }

        int lpgpu2_getStackTraceParametersForApiAndCallType(const gtString &callCategory, const gtString &apiName, gtVector<StackTraceParameter> &traceParameters)
        {
            return dbAdaptor.GetStackTraceParametersByCallCategoryAndAPIName(callCategory, apiName, traceParameters);
        }

        int lpgpu2_getStackTraceParametersForFrameAndDrawNum(gtUInt32 frameNum, gtUInt32 drawNum, gtVector<StackTraceParameter> &traceParameters)
        {
            return dbAdaptor.GetStackTraceParametersByFrameAndDrawNumber(frameNum, drawNum, traceParameters);
        }

        int lpgpu2_getProfileTraceInfoForCallType(const gtString& callName, gtVector<ProfileTraceInfo>& profileTraceInfo)
        {
            return dbAdaptor.GetProfileTraceInfoByCallCategory(callName, profileTraceInfo);
        }

        // Access Shader Tables.
        int lpgpu2_getShaders(gtVector<ShaderTrace> &shaderTraces)
        {
            return dbAdaptor.GetShaderTraces(shaderTraces);
        }

        int lpgpu2_getShadersByAPIName(gtString &apiName, gtVector<ShaderTrace> &shaderTraces)
        {
            return dbAdaptor.GetShaderTracesByAPIName(apiName, shaderTraces);
        }

        int lpgpu2_getShaderAsmStatsByShaderId(gtUInt64 shaderId, gtVector<ShaderAsmStat> &shaderStats)
        {
            return dbAdaptor.GetShaderAsmStatsByShaderId(shaderId, shaderStats);
        }

        int lpgpu2_getShaderToAsmByShaderId(gtUInt64 shaderId, gtVector<lpgpu2::db::ShaderToAsm> &shaderToAsm)
        {
            return dbAdaptor.GetShaderToAsmByShaderId(shaderId, shaderToAsm);
        }

        int lpgpu2_insertShaderAsmStat(ShaderAsmStat &shaderStat)
        {
            return dbAdaptor.InsertShaderAsmStat(shaderStat);
        }

        int lpgpu2_updateShaderAsmStats(gtVector<ShaderAsmStat> &shaderStats)
        {
            return dbAdaptor.UpdateShaderAsmStats(shaderStats);
        }

        int lpgpu2_addAsmToShader(ShaderTrace shaderTrace)
        {
            return dbAdaptor.UpdateShaderTrace(EShaderMetadataType::kAsm, shaderTrace);
        }

        int lpgpu2_deleteShaderAsmStatsForShaderId(gtUInt64 shaderId, EShaderTraceAPIId apiId)
        {
            return dbAdaptor.DeleteShaderAsmStatsForShaderId(shaderId, apiId);
        }

        int lpgpu2_insertShaderToAsm(ShaderToAsm &shaderToAsm)
        {
            return dbAdaptor.InsertShaderToAsm(shaderToAsm);
        }

        int lpgpu2_updateShader(EShaderMetadataType type, const ShaderTrace &shaderTrace)
        {
            return dbAdaptor.UpdateShaderTrace(type, shaderTrace);
        }

        // Counter Weights
        int lpgpu2_getCounterWeights(gtVector<CounterWeight>& counterWeights)
        {
            return dbAdaptor.GetCounterWeights(counterWeights);
        }

        int lpgpu2_getWeightedSumForCounterInRange(gtString &counterName, gtUInt32 rangeStart, gtUInt32 rangeEnd, gtFloat32 &weightedSum)
        {
            return dbAdaptor.GetWeightedSumForCounterByNameInRange(counterName, rangeStart, rangeEnd, weightedSum);
        }

        int lpgpu2_getWeightedSumForCounterByName(const gtString &counterName, gtVector<PPSampleData> &sampleValues)
        {
            return dbAdaptor.GetWeightedSamplesForCounterByName(counterName, sampleValues);
        }

        int lpgpu2_getMaxCpuEndTimeFromTraceForFrameNum(gtUInt32 frameNum, ProfileTraceInfo &profileTraceInfo)
        {
            return dbAdaptor.GetProfileTraceInfoMaxCpuEndTimeByFrameNumber(frameNum, profileTraceInfo);
        }

        int lpgpu2_getProfileTraceInfo(gtVector<ProfileTraceInfo>& profileTraceInfoList)
        {
            return dbAdaptor.GetProfileTraceInfo(profileTraceInfoList);
        }

        int lpgpu2_writePowerEstimation(const EstimatedPowerSample &estimatedPwrSample)
        {
            return dbAdaptor.InsertEstimatedPowerSample(estimatedPwrSample);
        }

        int lpgpu2_getTracesForCallCategoryWithNumCallsAboveThreshold(gtString &counterCategory, gtUInt32 threshold, gtVector<ProfileTraceInfo> &profileTraceInfo)
        {
            return dbAdaptor.GetProfileTraceInfoByCallCategoryWithNumberOfCallsAboveThreshold(counterCategory, threshold, profileTraceInfo);
        }

        int lpgpu2_beginTransaction(void)
        {
            return dbAdaptor.BeginTransaction();
        }

        int lpgpu2_endTransaction(void)
        {
            return dbAdaptor.EndTransaction();
        }

        int lpgpu2_getProfileTraceInfoForApi(const gtString &apiName, gtVector<ProfileTraceInfo> &profileTraceInfoList)
        {
            return dbAdaptor.GetProfileTraceInfoByAPIName(apiName, profileTraceInfoList);
        }

        // The following are currently unused
        int lpgpu2_getCallsPerTypeForRegionById(gtUInt32 regionId, CallsPerType &callsPerType)
        {
            return dbAdaptor.GetCallsPerTypeForRegionById(regionId, callsPerType);
        }


        void RegisterLuaBindings(void)
        {
            // load default libs
            luaL_openlibs(m_lua);
        }

        void RegisterClassBindings(void)
        {
            // create class wrapper
            luaL_newmetatable(m_lua, "LuaBindingClassA");

            lua_pushvalue(m_lua, -1);
            lua_setfield(m_lua, -2, "__index");

            lua_pushcfunction(m_lua, Lua_Trace);
            lua_setfield(m_lua, -2, "trace");

            lua_pushcfunction(m_lua, Lua_getCompilerOutput);
            lua_setfield(m_lua, -2, "getCompilerOutput");

            lua_pushcfunction(m_lua, Lua_writeAnnotation);
            lua_setfield(m_lua, -2, "writeAnnotation");

            lua_pushcfunction(m_lua, Lua_getPlatform);
            lua_setfield(m_lua, -2, "getPlatform");

            lua_pushcfunction(m_lua, Lua_getHardware);
            lua_setfield(m_lua, -2, "getHardware");

            lua_pushcfunction(m_lua, Lua_getDCAPI);
            lua_setfield(m_lua, -2, "getDCAPI");

            lua_pushcfunction(m_lua, Lua_getRAgent);
            lua_setfield(m_lua, -2, "getRAgent");

            lua_pushcfunction(m_lua, Lua_getHardwareID);
            lua_setfield(m_lua, -2, "getHardwareID");

            lua_pushcfunction(m_lua, Lua_getBlobSize);
            lua_setfield(m_lua, -2, "getBlobSize");

            lua_pushcfunction(m_lua, Lua_getUniqueTimesFromSamples);
            lua_setfield(m_lua, -2, "getUniqueTimesFromSamples");

            lua_pushcfunction(m_lua, Lua_getUniqueFrameNumFromTraces);
            lua_setfield(m_lua, -2, "getUniqueFrameNumFromTraces");

            lua_pushcfunction(m_lua, Lua_getUniqueTimesFromSamplesInRange);
            lua_setfield(m_lua, -2, "getUniqueTimesFromSamplesInRange");

            lua_pushcfunction(m_lua, Lua_getUniqueTimesFromSamplesByCategoryInRange);
            lua_setfield(m_lua, -2, "getUniqueTimesFromSamplesByCategoryInRange");

            lua_pushcfunction(m_lua, Lua_getMaxValuesForCategory);
            lua_setfield(m_lua, -2, "getMaxValuesForCategory");

            lua_pushcfunction(m_lua, Lua_getNumThreads);
            lua_setfield(m_lua, -2, "getNumThreads");

            lua_pushcfunction(m_lua, Lua_getAveragedValuesForCategoryWithinRange);
            lua_setfield(m_lua, -2, "getAveragedValuesForCategoryWithinRange");

            lua_pushcfunction(m_lua, Lua_getFirstCallTimestamp);
            lua_setfield(m_lua, -2, "getFirstCallTimestamp");

            lua_pushcfunction(m_lua, Lua_getTimesForCounterWithName);
            lua_setfield(m_lua, -2, "getTimesForCounterWithName");

            lua_pushcfunction(m_lua, Lua_getSamplesForCounterWithName);
            lua_setfield(m_lua, -2, "getSamplesForCounterWithName");

            lua_pushcfunction(m_lua, Lua_getCallNameFromTraceInRange);
            lua_setfield(m_lua, -2, "getCallNameFromTraceInRange");

            lua_pushcfunction(m_lua, Lua_getBinaryBlobFromCounters);
            lua_setfield(m_lua, -2, "getBinaryBlobFromCounters");

            lua_pushcfunction(m_lua, Lua_getCpuStartFromTraceInRange);
            lua_setfield(m_lua, -2, "getCpuStartFromTraceInRange");

            lua_pushcfunction(m_lua, Lua_getCpuEndFromTraceInRange);
            lua_setfield(m_lua, -2, "getCpuEndFromTraceInRange");

            lua_pushcfunction(m_lua, Lua_detectFrameDelimiters);
            lua_setfield(m_lua, -2, "detectFrameDelimiters");

            lua_pushcfunction(m_lua, Lua_getSupportedCallsForCategory);
            lua_setfield(m_lua, -2, "getSupportedCallsForCategory");

            lua_pushcfunction(m_lua, Lua_writeCallsPerType);
            lua_setfield(m_lua, -2, "writeCallsPerType");

            lua_pushcfunction(m_lua, Lua_checkGpuTimersAvailable);
            lua_setfield(m_lua, -2, "checkGpuTimersAvailable");

            lua_pushcfunction(m_lua, Lua_getGPUTimingForRangeOfFrames);
            lua_setfield(m_lua, -2, "getGPUTimingForRangeOfFrames");

            lua_pushcfunction(m_lua, Lua_writeRegion);
            lua_setfield(m_lua, -2, "writeRegion");

            lua_pushcfunction(m_lua, Lua_writeCallSummary);
            lua_setfield(m_lua, -2, "writeCallSummary");

            lua_pushcfunction(m_lua, Lua_getCallNameForApi);
            lua_setfield(m_lua, -2, "getCallNameForApi");

            lua_pushcfunction(m_lua, Lua_getCpuStartForApi);
            lua_setfield(m_lua, -2, "getCpuStartForApi");

            lua_pushcfunction(m_lua, Lua_getBinaryParamsForApi);
            lua_setfield(m_lua, -2, "getBinaryParamsForApi");

            lua_pushcfunction(m_lua, Lua_getBinaryParamsFromTraceParamsForCallType);
            lua_setfield(m_lua, -2, "getBinaryParamsFromTraceParamsForCallType");

            lua_pushcfunction(m_lua, Lua_getCpuEndForApi);
            lua_setfield(m_lua, -2, "getCpuEndForApi");

            lua_pushcfunction(m_lua, Lua_getCallNameForApiAndCategory);
            lua_setfield(m_lua, -2, "getCallNameForApiAndCategory");

            lua_pushcfunction(m_lua, Lua_getCpuStartForApiAndCategory);
            lua_setfield(m_lua, -2, "getCpuStartForApiAndCategory");

            lua_pushcfunction(m_lua, Lua_getFrameNumForApiAndCategory);
            lua_setfield(m_lua, -2, "getFrameNumForApiAndCategory");

            lua_pushcfunction(m_lua, Lua_getDrawNumForApiAndCategory);
            lua_setfield(m_lua, -2, "getDrawNumForApiAndCategory");

            lua_pushcfunction(m_lua, Lua_getBinaryParamsForApiAndCategory);
            lua_setfield(m_lua, -2, "getBinaryParamsForApiAndCategory");

            lua_pushcfunction(m_lua, Lua_getCpuEndForApiAndCategory);
            lua_setfield(m_lua, -2, "getCpuEndForApiAndCategory");

            lua_pushcfunction(m_lua, Lua_getCallNameFromOpenCLMemAllocationAndSetupCalls);
            lua_setfield(m_lua, -2, "getCallNameFromOpenCLMemAllocationAndSetupCalls");

            lua_pushcfunction(m_lua, Lua_getFrameNumFromOpenCLMemAllocationAndSetupCalls);
            lua_setfield(m_lua, -2, "getFrameNumFromOpenCLMemAllocationAndSetupCalls");

            lua_pushcfunction(m_lua, Lua_getDrawNumFromOpenCLMemAllocationAndSetupCalls);
            lua_setfield(m_lua, -2, "getDrawNumFromOpenCLMemAllocationAndSetupCalls");

            lua_pushcfunction(m_lua, Lua_getBinaryParamsFromOpenCLMemAllocationAndSetupCalls);
            lua_setfield(m_lua, -2, "getBinaryParamsFromOpenCLMemAllocationAndSetupCalls");

            lua_pushcfunction(m_lua, Lua_getBinaryReturnFromOpenCLMemAllocationAndSetupCalls);
            lua_setfield(m_lua, -2, "getBinaryReturnFromOpenCLMemAllocationAndSetupCalls");

            lua_pushcfunction(m_lua, Lua_getCallNameFromOpenCLDataTransferAndNDRangeCalls);
            lua_setfield(m_lua, -2, "getCallNameFromOpenCLDataTransferAndNDRangeCalls");

            lua_pushcfunction(m_lua, Lua_getFrameNumFromOpenCLDataTransferAndNDRangeCalls);
            lua_setfield(m_lua, -2, "getFrameNumFromOpenCLDataTransferAndNDRangeCalls");

            lua_pushcfunction(m_lua, Lua_getDrawNumFromOpenCLDataTransferAndNDRangeCalls);
            lua_setfield(m_lua, -2, "getDrawNumFromOpenCLDataTransferAndNDRangeCalls");

            lua_pushcfunction(m_lua, Lua_getBinaryParamsFromOpenCLDataTransferAndNDRangeCalls);
            lua_setfield(m_lua, -2, "getBinaryParamsFromOpenCLDataTransferAndNDRangeCalls");

            // Access Shaders
            lua_pushcfunction(m_lua, Lua_getShaders);
            lua_setfield(m_lua, -2, "getShaders");

            lua_pushcfunction(m_lua, Lua_getDrawNumFromShader);
            lua_setfield(m_lua, -2, "getDrawNumFromShader");

            lua_pushcfunction(m_lua, Lua_getFrameNumFromShader);
            lua_setfield(m_lua, -2, "getFrameNumFromShader");

            lua_pushcfunction(m_lua, Lua_getShaderID);
            lua_setfield(m_lua, -2, "getShaderID");

            lua_pushcfunction(m_lua, Lua_getApiIDFromShader);
            lua_setfield(m_lua, -2, "getApiIDFromShader");

            lua_pushcfunction(m_lua, Lua_getShaderASM);
            lua_setfield(m_lua, -2, "getShaderASM");

            lua_pushcfunction(m_lua, Lua_getShaderDebug);
            lua_setfield(m_lua, -2, "getShaderDebug");

            lua_pushcfunction(m_lua, Lua_getShaderBinary);
            lua_setfield(m_lua, -2, "getShaderBinary");

            lua_pushcfunction(m_lua, Lua_getShaderToAsmExistsForShaderId);
            lua_setfield(m_lua, -2, "getShaderToAsmExistsForShaderId");

            lua_pushcfunction(m_lua, Lua_insertShaderToAsm);
            lua_setfield(m_lua, -2, "insertShaderToAsm");

            lua_pushcfunction(m_lua, Lua_deleteShaderAsmStatsForShaderId);
            lua_setfield(m_lua, -2, "deleteShaderAsmStatsForShaderId");

            lua_pushcfunction(m_lua, Lua_insertShaderAsmStat);
            lua_setfield(m_lua, -2, "insertShaderAsmStat");

            lua_pushcfunction(m_lua, Lua_updateShaderAsmStat);
            lua_setfield(m_lua, -2, "updateShaderAsmStat");
            
            lua_pushcfunction(m_lua, Lua_addAsmToShader);
            lua_setfield(m_lua, -2, "addAsmToShader");

            // Counter Weights
            lua_pushcfunction(m_lua, Lua_getCounterNamesFromCounterWeights);
            lua_setfield(m_lua, -2, "getCounterNamesFromCounterWeights");

            lua_pushcfunction(m_lua, Lua_getWeightValuesFromCounterWeights);
            lua_setfield(m_lua, -2, "getWeightValuesFromCounterWeights");

            lua_pushcfunction(m_lua, Lua_getWeightedSumForCounterInRange);
            lua_setfield(m_lua, -2, "getWeightedSumForCounterInRange");

            lua_pushcfunction(m_lua, Lua_getWeightedSumForCounterByName);
            lua_setfield(m_lua, -2, "getWeightedSumForCounterByName");

            lua_pushcfunction(m_lua, Lua_getMaxCpuEndFromTracesForFrameNum);
            lua_setfield(m_lua, -2, "getMaxCpuEndFromTracesForFrameNum");

            lua_pushcfunction(m_lua, Lua_getCallNameFromTraces);
            lua_setfield(m_lua, -2, "getCallNameFromTraces");

            lua_pushcfunction(m_lua, Lua_getCpuStartFromTraces);
            lua_setfield(m_lua, -2, "getCpuStartFromTraces");

            lua_pushcfunction(m_lua, Lua_getCpuEndFromTraces);
            lua_setfield(m_lua, -2, "getCpuEndFromTraces");

            lua_pushcfunction(m_lua, Lua_getFrameNumFromTraces);
            lua_setfield(m_lua, -2, "getFrameNumFromTraces");

            lua_pushcfunction(m_lua, Lua_getDrawNumFromTraces);
            lua_setfield(m_lua, -2, "getDrawNumFromTraces");

            lua_pushcfunction(m_lua, Lua_getCallNameFromTracesForCallType);
            lua_setfield(m_lua, -2, "getCallNameFromTracesForCallType");

            lua_pushcfunction(m_lua, Lua_getCpuStartFromTracesForCallType);
            lua_setfield(m_lua, -2, "getCpuStartFromTracesForCallType");

            lua_pushcfunction(m_lua, Lua_getCpuEndFromTracesForCallType);
            lua_setfield(m_lua, -2, "getCpuEndFromTracesForCallType");

            lua_pushcfunction(m_lua, Lua_getFrameNumFromTracesForCallType);
            lua_setfield(m_lua, -2, "getFrameNumFromTracesForCallType");

            lua_pushcfunction(m_lua, Lua_getDrawNumFromTracesForCallType);
            lua_setfield(m_lua, -2, "getDrawNumFromTracesForCallType");

            lua_pushcfunction(m_lua, Lua_getFrameNumFromTraceParams);
            lua_setfield(m_lua, -2, "getFrameNumFromTraceParams");

            lua_pushcfunction(m_lua, Lua_getDrawNumFromTraceParams);
            lua_setfield(m_lua, -2, "getDrawNumFromTraceParams");

            lua_pushcfunction(m_lua, Lua_getBinaryParamsFromTraceParams);
            lua_setfield(m_lua, -2, "getBinaryParamsFromTraceParams");

            lua_pushcfunction(m_lua, Lua_getBinaryReturnFromTraceParams);
            lua_setfield(m_lua, -2, "getBinaryReturnFromTraceParams");

            lua_pushcfunction(m_lua, Lua_getTypeDefDescriptionFromTraceParams);
            lua_setfield(m_lua, -2, "getTypeDefDescriptionFromTraceParams");

            lua_pushcfunction(m_lua, Lua_getParamsSizeFromTraceParams);
            lua_setfield(m_lua, -2, "getParamsSizeFromTraceParams");

            lua_pushcfunction(m_lua, Lua_getReturnSizeFromTraceParams);
            lua_setfield(m_lua, -2, "getReturnSizeFromTraceParams");

            lua_pushcfunction(m_lua, Lua_getErrorFromTraceParams);
            lua_setfield(m_lua, -2, "getErrorFromTraceParams");

            lua_pushcfunction(m_lua, Lua_getCallNameFromOpenGLBufferAndDrawCalls);
            lua_setfield(m_lua, -2, "getCallNameFromOpenGLBufferAndDrawCalls");

            lua_pushcfunction(m_lua, Lua_getBinaryParamsFromOpenGLBufferAndDrawCalls);
            lua_setfield(m_lua, -2, "getBinaryParamsFromOpenGLBufferAndDrawCalls");

            lua_pushcfunction(m_lua, Lua_getCallNameFromOpenGLStateAndFrameCalls);
            lua_setfield(m_lua, -2, "getCallNameFromOpenGLStateAndFrameCalls");

            lua_pushcfunction(m_lua, Lua_getFrameNumFromOpenGLStateAndFrameCalls);
            lua_setfield(m_lua, -2, "getFrameNumFromOpenGLStateAndFrameCalls");

            lua_pushcfunction(m_lua, Lua_getDrawNumFromOpenGLStateAndFrameCalls);
            lua_setfield(m_lua, -2, "getDrawNumFromOpenGLStateAndFrameCalls");

            lua_pushcfunction(m_lua, Lua_getBinaryParamsFromOpenGLStateAndFrameCalls);
            lua_setfield(m_lua, -2, "getBinaryParamsFromOpenGLStateAndFrameCalls");

            lua_pushcfunction(m_lua, Lua_getCallNameFromVulkanDescriptorAndDrawCalls);
            lua_setfield(m_lua, -2, "getCallNameFromVulkanDescriptorAndDrawCalls");

            lua_pushcfunction(m_lua, Lua_getCpuStartFromVulkanDescriptorAndDrawCalls);
            lua_setfield(m_lua, -2, "getCpuStartFromVulkanDescriptorAndDrawCalls");

            lua_pushcfunction(m_lua, Lua_getBlockingFunctions);
            lua_setfield(m_lua, -2, "getBlockingFunctions");

            lua_pushcfunction(m_lua, Lua_writePowerEstimation);
            lua_setfield(m_lua, -2, "writePowerEstimation");

            lua_pushcfunction(m_lua, Lua_getStackTraceParametersForFrameAndDrawCalls);
            lua_setfield(m_lua, -2, "getStackTraceParametersForFrameAndDrawCalls");

            lua_pushcfunction(m_lua, Lua_getFramesForCategoryWithCallsAboveThreshold);
            lua_setfield(m_lua, -2, "getFramesForCategoryWithCallsAboveThreshold");

            lua_pushcfunction(m_lua, Lua_beginTransaction);
            lua_setfield(m_lua, -2, "beginTransaction");

            lua_pushcfunction(m_lua, Lua_endTransaction);
            lua_setfield(m_lua, -2, "endTransaction");

            // create instance of object & associate with a global in the m_lua state
            LuaBinding **ud = static_cast<LuaBinding **>(lua_newuserdata(m_lua, sizeof(LuaBinding*)));
            *(ud) = this;
            luaL_setmetatable(m_lua, "LuaBindingClassA"); // set userdata metatable
            lua_setglobal(m_lua, "LPGPU2");
        }

        //------------------------------------------------------------ -------------------------------------------------------

        static int Lua_Trace(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                if (lua_isstring(L, 2))
                {
                    const char *msg = luaL_checkstring(L, 2);
                    if (msg)
                    {
                        (*ud)->AddTrace(msg);
                    }
                }
            }
            return 0;  /* number of results */
        }

        // TODO - doesn't need new accessors
        static int Lua_getCompilerOutput(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getCompilerOutput();
            }
            lua_pushnumber(L, 0);
            return 1;
        }

        static int Lua_writeAnnotation(lua_State *L)
        {
            // **** takes a variable number of parameters ****
            if (lua_gettop(L) >= 3)
            {
                int frame = 0, draw = 0;
                int startTime = 0, endTime = 0;
                int params = lua_gettop(L);
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                int type = (int) luaL_checkinteger(L, 2);
                const char* message = luaL_checkstring(L, 3);
                if (params > 3)
                    frame = luaL_checkinteger(L, 4);
                if (params > 4)
                    draw = luaL_checkinteger(L, 5);
                if (params > 5)
                    startTime = luaL_checkinteger(L, 6);
                if (params > 6)
                    endTime = luaL_checkinteger(L, 7);

                (*ud)->lpgpu2_writeAnnotation(type, message, frame, draw, startTime, endTime);
            }
            return 0;
        }

        //
        static int Lua_getPlatform(lua_State *L)
        {
            gtString s;
            if (lua_gettop(L) == 1)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getPlatform(s);
                lua_pushstring(L, s.asASCIICharArray());
                return 1;
            }
            return 0;
        }
        static int Lua_getHardware(lua_State *L)
        {
            gtString s;
            if (lua_gettop(L) == 1)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getHardware(s);
                lua_pushstring(L, s.asASCIICharArray());
                return 1;
            }
            return 0;
        }

        static int Lua_getDCAPI(lua_State *L)
        {
            gtString s;
            if (lua_gettop(L) == 1)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getDCAPI(s);
                lua_pushstring(L, s.asASCIICharArray());
                return 1;
            }
            return 0;
        }

        static int Lua_getRAgent(lua_State *L)
        {
            gtString s;
            if (lua_gettop(L) == 1)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getRAgent(s);
                lua_pushstring(L, s.asASCIICharArray());
                return 1;
            }
            return 0;
        }

        static int Lua_getHardwareID(lua_State *L)
        {
            gtString s;
            if (lua_gettop(L) == 1)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getHardwareID(s);
                lua_pushstring(L, s.asASCIICharArray());
                return 1;
            }
            return 0;
        }

        static int Lua_getBlobSize(lua_State *L)
        {
            gtString s;
            if (lua_gettop(L) == 1)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getBlobSize(s);
                lua_pushstring(L, s.asASCIICharArray());
                return 1;
            }
            return 0;
        }

        //
        static int Lua_getUniqueTimesFromSamples(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<gtUInt64> uniqueTimes;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getUniqueTimesFromSamples(uniqueTimes);
                lua_newtable(L);

                for (uint i = 0; i < uniqueTimes.size(); i++)
                {
                    lua_pushnumber(L, uniqueTimes[i]);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getUniqueTimesFromSamplesInRange(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtVector<gtUInt64> uniqueTimes;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                int startTime = (int) luaL_checkinteger(L, 2);
                int endTime = (int) luaL_checkinteger(L, 3);

                (*ud)->lpgpu2_getUniqueTimesFromSamplesWithinRange(startTime, endTime, uniqueTimes);
                lua_newtable(L);

                for (uint i = 0; i < uniqueTimes.size(); i++)
                {
                    lua_pushnumber(L, uniqueTimes[i]);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getUniqueTimesFromSamplesByCategoryInRange(lua_State *L)
        {
            if (lua_gettop(L) == 4)
            {
                gtString counterCategory;
                gtVector<gtUInt64> uniqueTimes;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char* category = luaL_checkstring(L, 2);
                counterCategory.fromASCIIString(category);
                int startTime = (int) luaL_checkinteger(L, 3);
                int endTime = (int) luaL_checkinteger(L, 4);

                (*ud)->lpgpu2_getUniqueQuantizedTimesFromSamplesByCategoryInRange(counterCategory, startTime, endTime, uniqueTimes);
                lua_newtable(L);

                for (uint i = 0; i < uniqueTimes.size(); i++)
                {
                    lua_pushnumber(L, uniqueTimes[i]);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getUniqueFrameNumFromTraces(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<gtUInt64> uniqueFrameNumbers;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getUniqueFrameNumFromTraces(uniqueFrameNumbers);
                lua_newtable(L);

                for (uint i = 0; i < uniqueFrameNumbers.size(); i++)
                {
                    lua_pushnumber(L, uniqueFrameNumbers[i]);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }


        static int Lua_getMaxValuesForCategory(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString counterCategory;
                gtVector<PPSampleData> sampleValues;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char* category = luaL_checkstring(L, 2);

                counterCategory.fromASCIIString(category);
                (*ud)->lpgpu2_getMaxValuesForCategory(counterCategory, sampleValues);

                lua_newtable(L);

                for (uint i = 0; i < sampleValues.size(); i++)
                {
                    lua_pushnumber(L, sampleValues[i].m_sampleValue);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getNumThreads(lua_State *L)
        {
            gtUInt64 vNumThreads;
            if (lua_gettop(L) == 1)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getNumThreads(vNumThreads);
                lua_pushnumber(L, (int) vNumThreads);
                return 1;
            }
            return 0;
        }

        static int Lua_getAveragedValuesForCategoryWithinRange(lua_State *L)
        {
            if (lua_gettop(L) == 4)
            {
                gtVector<PPSampleData> sampleValues;
                gtString counterCategory;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char* abstractCounter = luaL_checkstring(L, 2);
                int startTime = (int) luaL_checkinteger(L, 3);
                int endTime = (int) luaL_checkinteger(L, 4);

                counterCategory.fromASCIIString(abstractCounter);

                (*ud)->lpgpu2_getAveragedValuesForCategoryWithinRange(counterCategory, startTime, endTime, sampleValues);
                lua_newtable(L);

                for (uint i = 0; i < sampleValues.size(); i++)
                {
                    lua_pushnumber(L, sampleValues[i].m_sampleValue);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getFirstCallTimestamp(lua_State *L)
        {
            gtUInt64 cpuStartTimestamp;
            if (lua_gettop(L) == 1)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                (*ud)->lpgpu2_getFirstCallTimestamp(cpuStartTimestamp);
                lua_pushnumber(L, cpuStartTimestamp);
                return 1;
            }
            return 0;
        }

        static int Lua_getTimesForCounterWithName(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString counterName;
                gtVector<PPSampleData> sampleValues;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *counter = luaL_checkstring(L, 2);
                counterName.fromASCIIString(counter);
                (*ud)->lpgpu2_getSamplesForCounterWithName(counterName, sampleValues);
                lua_newtable(L);
                for (uint i = 0; i < sampleValues.size(); i++)
                {
                    lua_pushnumber(L, sampleValues[i].m_quantizedTime);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getSamplesForCounterWithName(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString counterName;
                gtVector<PPSampleData> sampleValues;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *counter = luaL_checkstring(L, 2);
                counterName.fromASCIIString(counter);
                (*ud)->lpgpu2_getSamplesForCounterWithName(counterName, sampleValues);
                lua_newtable(L);
                for (uint i = 0; i < sampleValues.size(); i++)
                {
                    lua_pushnumber(L, sampleValues[i].m_sampleValue);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryBlobFromCounters(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<PPSampleData> sampleValues;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getAllSamples(sampleValues);

                lua_newtable(L);
                char *params;
                for (uint i = 0; i < sampleValues.size(); i++)
                {
                    params = (char *) malloc((2 * sampleValues[i].m_blob.size() + 1) * sizeof(char));
                    uint j;
                    for (j = 0; j < sampleValues[i].m_blob.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", sampleValues[i].m_blob[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCallNameFromTraceInRange(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtVector<ProfileTraceInfo> profileTraceInfo;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                int start = (int) luaL_checkinteger(L, 2);
                int end = (int) luaL_checkinteger(L, 3);

                (*ud)->lpgpu2_getCallNameFromTraceInRange(start, end, profileTraceInfo);

                lua_newtable(L);

                for (uint i = 0; i < profileTraceInfo.size(); i++)
                {
                    lua_pushstring(L, profileTraceInfo[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCpuStartFromTraceInRange(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtVector<ProfileTraceInfo> profileTraceInfo;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                int start = (int) luaL_checkinteger(L, 2);
                int end = (int) luaL_checkinteger(L, 3);

                (*ud)->lpgpu2_getCallNameFromTraceInRange(start, end, profileTraceInfo);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfo.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfo[i].m_cpuStart);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCpuEndFromTraceInRange(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtVector<ProfileTraceInfo> profileTraceInfo;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                int start = (int) luaL_checkinteger(L, 2);
                int end = (int) luaL_checkinteger(L, 3);


                (*ud)->lpgpu2_getCallNameFromTraceInRange(start, end, profileTraceInfo);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfo.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfo[i].m_cpuEnd);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_detectFrameDelimiters(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                callCategory.fromASCIIString("FrameDelimiter");
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                (*ud)->lpgpu2_getProfileTraceInfoListForCallCategory(callCategory, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushstring(L, profileTraceInfoList[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getSupportedCallsForCategory(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString category;
                gtVector<CallCategory> callCategories;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char* abstractCounter = luaL_checkstring(L, 2);
                category.fromASCIIString(abstractCounter);

                (*ud)->lpgpu2_getSupportedCalls(category, callCategories);

                lua_newtable(L);
                for (uint i = 0; i < callCategories.size(); i++)
                {
                    lua_pushstring(L, callCategories[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_writeCallsPerType(lua_State *L)
        {
            if (lua_gettop(L) == 9)
            {
                CallsPerType calls;
                gtString delimiters;
                delimiters.fromASCIIString("Delimiters");
                gtVector<CallCategory> callCategories;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                calls.m_regionId = luaL_checkinteger(L, 2);
                calls.m_drawPercentage = luaL_checknumber(L, 3);
                calls.m_bindPercentage = luaL_checknumber(L, 4);
                calls.m_setupPercentage = luaL_checknumber(L, 5);
                calls.m_waitPercentage = luaL_checknumber(L, 6);
                calls.m_errorPercentage = luaL_checknumber(L, 7);
                calls.m_delimiterPercentage = luaL_checknumber(L, 8);
                calls.m_otherPercentage = luaL_checknumber(L, 9);

                (*ud)->lpgpu2_writeCallsPerType(calls);
            }
            return 0;
        }

        static int Lua_checkGpuTimersAvailable(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<GPUTimer> gpuTimers;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                (*ud)->lpgpu2_getGpuTimers(gpuTimers);
                lua_pushboolean(L, gpuTimers.size() ? true : false);
                return 1;
            }
            return 0;
        }

        static int Lua_getGPUTimingForRangeOfFrames(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtVector<GPUTimer> gpuTimers;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                int first_frame = (int) luaL_checkinteger(L, 2);
                int last_frame = (int) luaL_checkinteger(L, 3);

                (*ud)->lpgpu2_getGPUTimingForRangeOfFrames(first_frame, last_frame, gpuTimers);
                lua_newtable(L);

                for (uint i = 0; i < gpuTimers.size(); i++)
                {
                    lua_pushnumber(L, gpuTimers[i].m_time);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_writeRegion(lua_State *L)
        {
            if (lua_gettop(L) == 4)
            {
                RegionOfInterest regionOfInterest;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                regionOfInterest.m_regionId = (int) luaL_checkinteger(L, 2);
                regionOfInterest.m_frameStart = (int) luaL_checkinteger(L, 3);
                regionOfInterest.m_frameEnd = (int) luaL_checkinteger(L, 4);
                regionOfInterest.m_degreeOfInterest = 0.0f; // TODO
                (*ud)->lpgpu2_writeRegion(regionOfInterest);
            }
            return 0;
        }

        static int Lua_writeCallSummary(lua_State *L)
        {
            if (lua_gettop(L) == 5)
            {
                CallSummary callSummary;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callSummary.m_regionId = (int) luaL_checkinteger(L, 2);
                callSummary.m_callName.fromASCIIString(luaL_checkstring(L, 3));
                callSummary.m_timesCalled = (int) luaL_checkinteger(L, 4);
                callSummary.m_totalTime = (int) luaL_checkinteger(L, 5);
                (*ud)->lpgpu2_writeCallSummary(callSummary);
            }
            return 0;
        }

        static int Lua_getCallNameForApi(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *api = luaL_checkstring(L, 2);

                apiName.fromASCIIString(api);
                (*ud)->lpgpu2_getProfileTraceInfoForApi(apiName, profileTraceInfoList);
                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCpuStartForApi(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *api = luaL_checkstring(L, 2);
                apiName.fromASCIIString(api);

                (*ud)->lpgpu2_getProfileTraceInfoForApi(apiName, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_cpuStart);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryParamsForApi(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString apiName;
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *api = luaL_checkstring(L, 2);
                apiName.fromASCIIString(api);

                (*ud)->lpgpu2_getStackTraceParametersForApi(apiName, traceParameters);

                lua_newtable(L);
                char *params;
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryParams.size() + 1) * sizeof(char));
                    uint j;
                    for (j = 0; j < traceParameters[i].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryParamsFromTraceParamsForCallType(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString callCategory;
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *call = luaL_checkstring(L, 2);
                callCategory.fromASCIIString(call);

                (*ud)->lpgpu2_getStackTraceParametersForCallType(callCategory, traceParameters);

                lua_newtable(L);
                char *params;
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryParams.size() + 1) * sizeof(char));
                    uint j;
                    for (j = 0; j < traceParameters[i].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryReturnForCategory(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString callCategory;
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *call = luaL_checkstring(L, 2);
                callCategory.fromASCIIString(call);

                (*ud)->lpgpu2_getStackTraceParametersForCallType(callCategory, traceParameters);

                lua_newtable(L);
                char *params;
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryReturn.size() + 1) * sizeof(char));
                    uint j;
                    for (j = 0; j < traceParameters[i].m_binaryReturn.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryReturn[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCpuEndForApi(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *api = luaL_checkstring(L, 2);
                apiName.fromASCIIString(api);

                (*ud)->lpgpu2_getProfileTraceInfoForApi(apiName, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_cpuEnd);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCallNameForApiAndCategory(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *api = luaL_checkstring(L, 2);
                const char *category = luaL_checkstring(L, 3);

                apiName.fromASCIIString(api);
                callCategory.fromASCIIString(category);
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);
                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCpuStartForApiAndCategory(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *api = luaL_checkstring(L, 2);
                const char *category = luaL_checkstring(L, 3);
                apiName.fromASCIIString(api);
                callCategory.fromASCIIString(category);

                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_cpuStart);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getFrameNumForApiAndCategory(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *api = luaL_checkstring(L, 2);
                const char *category = luaL_checkstring(L, 3);
                apiName.fromASCIIString(api);
                callCategory.fromASCIIString(category);

                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_frameNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getDrawNumForApiAndCategory(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *api = luaL_checkstring(L, 2);
                const char *category = luaL_checkstring(L, 3);
                apiName.fromASCIIString(api);
                callCategory.fromASCIIString(category);

                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_drawNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryParamsForApiAndCategory(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *api = luaL_checkstring(L, 2);
                const char *category = luaL_checkstring(L, 3);
                apiName.fromASCIIString(api);
                callCategory.fromASCIIString(category);
                (*ud)->lpgpu2_getStackTraceParametersForApiAndCallType(callCategory, apiName, traceParameters);

                lua_newtable(L);
                char *params;
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryParams.size() + 1) * sizeof(char));
                    uint j;
                    for (j = 0; j < traceParameters[i].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCpuEndForApiAndCategory(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *api = luaL_checkstring(L, 2);
                const char *category = luaL_checkstring(L, 3);
                apiName.fromASCIIString(api);
                callCategory.fromASCIIString(category);
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_cpuEnd);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCallNameFromOpenCLMemAllocationAndSetupCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("MemoryAllocation");
                apiName.fromASCIIString("OpenCL");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                profileTraceInfoList.clear();
                callCategory.fromASCIIString("Setup");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);
                for (uint j = 0; j < profileTraceInfoList.size(); j++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[j].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getFrameNumFromOpenCLMemAllocationAndSetupCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("MemoryAllocation");
                apiName.fromASCIIString("OpenCL");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_frameNum);
                    lua_rawseti(L, -2, i + 1);
                }
                profileTraceInfoList.clear();
                callCategory.fromASCIIString("Setup");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);
                for (uint j = 0; j < profileTraceInfoList.size(); j++)
                {
                    lua_pushnumber(L, profileTraceInfoList[j].m_frameNum);
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getDrawNumFromOpenCLMemAllocationAndSetupCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("MemoryAllocation");
                apiName.fromASCIIString("OpenCL");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_drawNum);
                    lua_rawseti(L, -2, i + 1);
                }
                profileTraceInfoList.clear();
                callCategory.fromASCIIString("Setup");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);
                for (uint j = 0; j < profileTraceInfoList.size(); j++)
                {
                    lua_pushnumber(L, profileTraceInfoList[j].m_drawNum);
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryParamsFromOpenCLMemAllocationAndSetupCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("MemoryAllocation");
                apiName.fromASCIIString("OpenCL");
                (*ud)->lpgpu2_getStackTraceParametersForApiAndCallType(callCategory, apiName, traceParameters);

                lua_newtable(L);
                char *params;
                uint i, j;
                for (i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryParams.size() + 1) * sizeof(char));
                    for (j = 0; j < traceParameters[i].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                traceParameters.clear();
                callCategory.fromASCIIString("Setup");
                (*ud)->lpgpu2_getStackTraceParametersForApiAndCallType(callCategory, apiName, traceParameters);
                for (uint k = 0; k < traceParameters.size(); k++)
                {
                    params = (char *) malloc((2 * traceParameters[k].m_binaryParams.size() + 1) * sizeof(char));
                    for (j = 0; j < traceParameters[k].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[k].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryReturnFromOpenCLMemAllocationAndSetupCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("MemoryAllocation");
                apiName.fromASCIIString("OpenCL");
                (*ud)->lpgpu2_getStackTraceParametersForApiAndCallType(callCategory, apiName, traceParameters);

                lua_newtable(L);
                char *params;
                uint i, j;
                for (i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryReturn.size() + 1) * sizeof(char));
                    for (j = 0; j < traceParameters[i].m_binaryReturn.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryReturn[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                traceParameters.clear();
                callCategory.fromASCIIString("Setup");
                (*ud)->lpgpu2_getStackTraceParametersForApiAndCallType(callCategory, apiName, traceParameters);
                for (uint k = 0; k < traceParameters.size(); k++)
                {
                    params = (char *) malloc((2 * traceParameters[k].m_binaryReturn.size() + 1) * sizeof(char));
                    for (j = 0; j < traceParameters[k].m_binaryReturn.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[k].m_binaryReturn[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCallNameFromOpenCLDataTransferAndNDRangeCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("DataTransfer"); // clEnqueueNDRangeKernel added to this list
                apiName.fromASCIIString("OpenCL");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getFrameNumFromOpenCLDataTransferAndNDRangeCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("DataTransfer"); // clEnqueueNDRangeKernel added to this list
                apiName.fromASCIIString("OpenCL");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_frameNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getDrawNumFromOpenCLDataTransferAndNDRangeCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("DataTransfer"); // clEnqueueNDRangeKernel added to this list
                apiName.fromASCIIString("OpenCL");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_drawNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryParamsFromOpenCLDataTransferAndNDRangeCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("DataTransfer"); // clEnqueueNDRangeKernel added to this list
                apiName.fromASCIIString("OpenCL");
                (*ud)->lpgpu2_getStackTraceParametersForApiAndCallType(callCategory, apiName, traceParameters);

                lua_newtable(L);
                char *params;
                uint i, j;
                for (i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryParams.size() + 1) * sizeof(char));
                    for (j = 0; j < traceParameters[i].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        // Access Shaders
        static int Lua_getShaders(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ShaderTrace> shaderTraces;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getShaders(shaderTraces);

                lua_newtable(L);
                for (uint i = 0; i < shaderTraces.size(); i++)
                {
                    lua_pushstring(L, (const char *) shaderTraces[i].m_srcCode.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }


        static int Lua_getDrawNumFromShader(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ShaderTrace> shaderTraces;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getShaders(shaderTraces);

                lua_newtable(L);
                for (uint i = 0; i < shaderTraces.size(); i++)
                {
                    lua_pushnumber(L, shaderTraces[i].m_drawNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getFrameNumFromShader(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ShaderTrace> shaderTraces;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getShaders(shaderTraces);

                lua_newtable(L);
                for (uint i = 0; i < shaderTraces.size(); i++)
                {
                    lua_pushnumber(L, shaderTraces[i].m_frameNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getShaderID(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ShaderTrace> shaderTraces;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getShaders(shaderTraces);

                lua_newtable(L);
                for (uint i = 0; i < shaderTraces.size(); i++)
                {
                    lua_pushnumber(L, shaderTraces[i].m_shaderId);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getApiIDFromShader(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
              gtVector<ShaderTrace> shaderTraces;
              LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

              (*ud)->lpgpu2_getShaders(shaderTraces);

              lua_newtable(L);
              for (uint i = 0; i < shaderTraces.size(); i++)
              {
                lua_pushnumber(L, (uint)shaderTraces[i].m_apiId);
                lua_rawseti(L, -2, i + 1);
              }
              return 1;
            }
            return 0;
        }

        static int Lua_getShaderASM(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ShaderTrace> shaderTraces;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getShaders(shaderTraces);

                lua_newtable(L);
                for (uint i = 0; i < shaderTraces.size(); i++)
                {
                    lua_pushstring(L, (const char *) shaderTraces[i].m_asmCode.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getShaderDebug(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ShaderTrace> shaderTraces;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getShaders(shaderTraces);

                lua_newtable(L);
                for (uint i = 0; i < shaderTraces.size(); i++)
                {
                    uint j;
                    char *debug = (char *) malloc((2 * shaderTraces[i].m_debug.size() + 1) * sizeof(char));
                    for (j = 0; j < shaderTraces[i].m_debug.size(); j++)
                    {
                        sprintf(&debug[j * 2], "%02X", shaderTraces[i].m_debug[j]);
                    }
                    debug[j * 2] = '\0';
                    lua_pushstring(L, (const char *) debug);
                    free(debug);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getShaderBinary(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ShaderTrace> shaderTraces;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getShaders(shaderTraces);

                lua_newtable(L);
                for (uint i = 0; i < shaderTraces.size(); i++)
                {
                    uint j;
                    char * binary = (char *) malloc((2 * shaderTraces[i].m_binary.size() + 1) * sizeof(char));
                    for (j = 0; j < shaderTraces[i].m_binary.size(); j++)
                    {
                        sprintf(&binary[j * 2], "%02X", shaderTraces[i].m_binary[j]);
                    }
                    binary[j * 2] = '\0';
                    lua_pushstring(L, (const char *) binary);
                    free(binary);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getShaderToAsmExistsForShaderId(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtVector<lpgpu2::db::ShaderToAsm> shaderToAsm;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                gtUInt64 shaderId = (int) luaL_checkinteger(L, 2);

                (*ud)->lpgpu2_getShaderToAsmByShaderId(shaderId, shaderToAsm);

                if (shaderToAsm.size())
                {
                    lua_pushnumber(L, 1);
                }
                else
                {
                    lua_pushnumber(L, 0);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_insertShaderToAsm(lua_State *L)
        {
            if (lua_gettop(L) == 6)
            {
                ShaderToAsm shaderToAsm;

                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                shaderToAsm.m_shaderId = (int) luaL_checkinteger(L, 2);
                shaderToAsm.m_apiId = (EShaderTraceAPIId) luaL_checkinteger(L, 3);
                shaderToAsm.m_shaderLine = (int) luaL_checkinteger(L, 4);
                shaderToAsm.m_asmStartLine = (int) luaL_checkinteger(L, 5);
                shaderToAsm.m_asmEndLine = (int) luaL_checkinteger(L, 6);

                (*ud)->lpgpu2_insertShaderToAsm(shaderToAsm);

                return 1;
            }
            return 0;
        }

        static int Lua_deleteShaderAsmStatsForShaderId(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                gtUInt64 shaderId = (int) luaL_checkinteger(L, 2);
                EShaderTraceAPIId apiId = (EShaderTraceAPIId) luaL_checkinteger(L, 3);

                (*ud)->lpgpu2_deleteShaderAsmStatsForShaderId(shaderId, apiId);

                return 1;
            }
            return 0;
        }

        static int Lua_insertShaderAsmStat(lua_State *L)
        {
            if (lua_gettop(L) == 5)
            {
                ShaderAsmStat shaderStat;

                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                shaderStat.m_shaderId = (int) luaL_checkinteger(L, 2);
                shaderStat.m_apiId = (EShaderTraceAPIId) luaL_checkinteger(L, 3);
                shaderStat.m_asmLine = (int) luaL_checkinteger(L, 4);
                shaderStat.m_percentage = luaL_checknumber(L, 5);

                (*ud)->lpgpu2_insertShaderAsmStat(shaderStat);

                return 1;
            }
            return 0;
        }

        static int Lua_updateShaderAsmStat(lua_State *L)
        {
            if (lua_gettop(L) == 5)
            {
                ShaderAsmStat shaderStat;
                gtVector<ShaderAsmStat> shaderStats;

                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                shaderStat.m_shaderId = (int) luaL_checkinteger(L, 2);
                shaderStat.m_apiId = (EShaderTraceAPIId) luaL_checkinteger(L, 3);
                shaderStat.m_asmLine = (int) luaL_checkinteger(L, 4);
                shaderStat.m_percentage = luaL_checknumber(L, 5);

                shaderStats.push_back(shaderStat);
                (*ud)->lpgpu2_updateShaderAsmStats(shaderStats);

                return 1;
            }
            return 0;
        }

        static int Lua_addAsmToShader(lua_State *L)
        {
            if (lua_gettop(L) == 4)
            {
                ShaderTrace shaderTrace;

                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                shaderTrace.m_shaderId = luaL_checkinteger(L, 2);
                shaderTrace.m_apiId = (EShaderTraceAPIId)luaL_checkinteger(L, 3);
                const char *asm_code = luaL_checkstring(L, 4);
                shaderTrace.m_asmCode.fromASCIIString(asm_code);

                (*ud)->lpgpu2_addAsmToShader(shaderTrace);

                return 1;
            }
            return 0;
        }

        // Counter Weights
        static int Lua_getCounterNamesFromCounterWeights(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<CounterWeight> counterWeights;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getCounterWeights(counterWeights);

                lua_newtable(L);
                for (uint i = 0; i < counterWeights.size(); i++)
                {
                    lua_pushstring(L, (const char *) counterWeights[i].m_counterName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getWeightValuesFromCounterWeights(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<CounterWeight> counterWeights;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getCounterWeights(counterWeights);

                lua_newtable(L);
                for (uint i = 0; i < counterWeights.size(); i++)
                {
                    lua_pushnumber(L, counterWeights[i].m_weightValue);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getWeightedSumForCounterInRange(lua_State *L)
        {
            if (lua_gettop(L) == 4)
            {
                gtString counterName;
                gtFloat32 weightedSum;
                gtVector<CounterWeight> counterWeights;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *counter = luaL_checkstring(L, 2);
                counterName.fromASCIIString(counter);
                int rangeStart = (int) luaL_checkinteger(L, 3);
                int rangeEnd = (int) luaL_checkinteger(L, 4);

                (*ud)->lpgpu2_getWeightedSumForCounterInRange(counterName, rangeStart, rangeEnd, weightedSum);

                lua_pushnumber(L, weightedSum);

                return 1;
            }
            return 0;
        }

        static int Lua_getWeightedSumForCounterByName(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString counterName;
                gtVector<PPSampleData> sampleValues;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *counter = luaL_checkstring(L, 2);
                counterName.fromASCIIString(counter);

                (*ud)->lpgpu2_getWeightedSumForCounterByName(counterName, sampleValues);

                lua_newtable(L);
                for (uint i = 0; i < sampleValues.size(); i++)
                {
                    lua_pushnumber(L, sampleValues[i].m_sampleValue);
                    lua_rawseti(L, -2, i + 1);
                }

                return 1;
            }
            return 0;
        }

        static int Lua_getMaxCpuEndFromTracesForFrameNum(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                ProfileTraceInfo profileTraceInfo;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                int frameNum = (int) luaL_checkinteger(L, 2);

                (*ud)->lpgpu2_getMaxCpuEndTimeFromTraceForFrameNum(frameNum, profileTraceInfo);
                lua_pushnumber(L, (int) profileTraceInfo.m_cpuEnd);
                return 1;
            }
            return 0;
        }

        static int Lua_getCallNameFromTraces(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getProfileTraceInfo(profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCpuStartFromTraces(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getProfileTraceInfo(profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_cpuStart);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCpuEndFromTraces(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getProfileTraceInfo(profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_cpuEnd);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getFrameNumFromTraces(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getProfileTraceInfo(profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_frameNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getDrawNumFromTraces(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getProfileTraceInfo(profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_drawNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCallNameFromTracesForCallType(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString callCategory;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *category = luaL_checkstring(L, 2);
                callCategory.fromASCIIString(category);

                (*ud)->lpgpu2_getProfileTraceInfoForCallType(callCategory, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCpuStartFromTracesForCallType(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString callCategory;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *category = luaL_checkstring(L, 2);
                callCategory.fromASCIIString(category);

                (*ud)->lpgpu2_getProfileTraceInfoForCallType(callCategory, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_cpuStart);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCpuEndFromTracesForCallType(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString callCategory;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *category = luaL_checkstring(L, 2);
                callCategory.fromASCIIString(category);

                (*ud)->lpgpu2_getProfileTraceInfoForCallType(callCategory, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_cpuEnd);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getFrameNumFromTracesForCallType(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString callCategory;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *category = luaL_checkstring(L, 2);
                callCategory.fromASCIIString(category);

                (*ud)->lpgpu2_getProfileTraceInfoForCallType(callCategory, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_frameNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getDrawNumFromTracesForCallType(lua_State *L)
        {
            if (lua_gettop(L) == 2)
            {
                gtString callCategory;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *category = luaL_checkstring(L, 2);
                callCategory.fromASCIIString(category);

                (*ud)->lpgpu2_getProfileTraceInfoForCallType(callCategory, profileTraceInfoList);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_drawNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }


        static int Lua_getFrameNumFromTraceParams(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getStackTraceParameters(traceParameters);

                lua_newtable(L);
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    lua_pushnumber(L, traceParameters[i].m_frameNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getDrawNumFromTraceParams(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getStackTraceParameters(traceParameters);

                lua_newtable(L);
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    lua_pushnumber(L, traceParameters[i].m_drawNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryParamsFromTraceParams(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getStackTraceParameters(traceParameters);

                lua_newtable(L);
                char *params;
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryParams.size() + 1) * sizeof(char));
                    uint j;
                    for (j = 0; j < traceParameters[i].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);

                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryReturnFromTraceParams(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getStackTraceParameters(traceParameters);

                lua_newtable(L);
                char *params;
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryReturn.size() + 1) * sizeof(char));
                    uint j;
                    for (j = 0; j < traceParameters[i].m_binaryReturn.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryReturn[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getTypeDefDescriptionFromTraceParams(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getStackTraceParameters(traceParameters);

                lua_newtable(L);
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    lua_pushstring(L, (const char *) traceParameters[i].m_typeDefDescription.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getParamsSizeFromTraceParams(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getStackTraceParameters(traceParameters);

                lua_newtable(L);
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    lua_pushstring(L, (const char *) traceParameters[i].m_paramsSize.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getReturnSizeFromTraceParams(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getStackTraceParameters(traceParameters);

                lua_newtable(L);
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    lua_pushnumber(L, traceParameters[i].m_returnSize);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getErrorFromTraceParams(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getStackTraceParameters(traceParameters);

                lua_newtable(L);
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    lua_pushnumber(L, traceParameters[i].m_error);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCallNameFromOpenGLBufferAndDrawCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("Buffer");
                apiName.fromASCIIString("OpenGL");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                profileTraceInfoList.clear();
                callCategory.fromASCIIString("Draw");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                for (uint j = 0; j < profileTraceInfoList.size(); j++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[j].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryParamsFromOpenGLBufferAndDrawCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("Buffer");
                apiName.fromASCIIString("OpenGL");
                (*ud)->lpgpu2_getStackTraceParametersForApiAndCallType(callCategory, apiName, traceParameters);

                lua_newtable(L);
                char *params;
                uint i;
                for (i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryParams.size() + 1) * sizeof(char));
                    uint j;
                    for (j = 0; j < traceParameters[i].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                traceParameters.clear();
                callCategory.fromASCIIString("Draw");
                (*ud)->lpgpu2_getStackTraceParametersForApiAndCallType(callCategory, apiName, traceParameters);
                for (uint k = 0; k < traceParameters.size(); k++)
                {
                    params = (char *) malloc((2 * traceParameters[k].m_binaryParams.size() + 1) * sizeof(char));
                    uint j;
                    for (j = 0; j < traceParameters[k].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[k].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCallNameFromOpenGLStateAndFrameCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("State");
                apiName.fromASCIIString("OpenGL");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                profileTraceInfoList.clear();
                callCategory.fromASCIIString("Frame");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);
                for (uint j = 0; j < profileTraceInfoList.size(); j++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[j].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getFrameNumFromOpenGLStateAndFrameCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("State");
                apiName.fromASCIIString("OpenGL");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_frameNum);
                    lua_rawseti(L, -2, i + 1);
                }
                profileTraceInfoList.clear();
                callCategory.fromASCIIString("Frame");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);
                for (uint j = 0; j < profileTraceInfoList.size(); j++)
                {
                    lua_pushnumber(L, profileTraceInfoList[j].m_frameNum);
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getDrawNumFromOpenGLStateAndFrameCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("State");
                apiName.fromASCIIString("OpenGL");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_drawNum);
                    lua_rawseti(L, -2, i + 1);
                }
                profileTraceInfoList.clear();
                callCategory.fromASCIIString("Frame");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);
                for (uint j = 0; j < profileTraceInfoList.size(); j++)
                {
                    lua_pushnumber(L, profileTraceInfoList[j].m_drawNum);
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBinaryParamsFromOpenGLStateAndFrameCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("State");
                apiName.fromASCIIString("OpenGL");
                (*ud)->lpgpu2_getStackTraceParametersForApiAndCallType(callCategory, apiName, traceParameters);

                lua_newtable(L);
                char *params;
                uint i, j;
                for (i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryParams.size() + 1) * sizeof(char));
                    for (j = 0; j < traceParameters[i].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                traceParameters.clear();
                callCategory.fromASCIIString("Frame");
                (*ud)->lpgpu2_getStackTraceParametersForApiAndCallType(callCategory, apiName, traceParameters);
                for (uint k = 0; k < traceParameters.size(); k++)
                {
                    params = (char *) malloc((2 * traceParameters[k].m_binaryParams.size() + 1) * sizeof(char));
                    for (j = 0; j < traceParameters[k].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[k].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCallNameFromVulkanDescriptorAndDrawCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("Descriptor");
                apiName.fromASCIIString("Vulkan");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                profileTraceInfoList.clear();
                callCategory.fromASCIIString("Draw");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);
                for (uint j = 0; j < profileTraceInfoList.size(); j++)
                {
                    lua_pushstring(L, (const char *) profileTraceInfoList[j].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getCpuStartFromVulkanDescriptorAndDrawCalls(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString callCategory;
                gtString apiName;
                gtVector<ProfileTraceInfo> profileTraceInfoList;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                callCategory.fromASCIIString("Descriptor");
                apiName.fromASCIIString("Vulkan");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);

                lua_newtable(L);
                uint i;
                for (i = 0; i < profileTraceInfoList.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfoList[i].m_cpuStart);
                    lua_rawseti(L, -2, i + 1);
                }
                profileTraceInfoList.clear();
                callCategory.fromASCIIString("Draw");
                (*ud)->lpgpu2_getProfileTraceInfoForApiAndCallType(apiName, callCategory, profileTraceInfoList);
                for (uint j = 0; j < profileTraceInfoList.size(); j++)
                {
                    lua_pushnumber(L, profileTraceInfoList[j].m_cpuStart);
                    lua_rawseti(L, -2, i + 1);
                    i++;
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getBlockingFunctions(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                gtString block;
                block.fromASCIIString("Block");
                gtVector<CallCategory> callCategories;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_getSupportedCalls(block, callCategories);

                lua_newtable(L);
                for (uint i = 0; i < callCategories.size(); i++)
                {
                    lua_pushstring(L, callCategories[i].m_callName.asASCIICharArray());
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_writePowerEstimation(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                EstimatedPowerSample estimatedPwrSample = { 0, 0.0, 0, 0 };
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                estimatedPwrSample.m_sampleTime = (int) luaL_checkinteger(L, 2);
                estimatedPwrSample.m_estimatedPower = luaL_checknumber(L, 3);
                (*ud)->lpgpu2_writePowerEstimation(estimatedPwrSample);
            }
            return 0;
        }

        static int Lua_getStackTraceParametersForFrameAndDrawCalls(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtVector<StackTraceParameter> traceParameters;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                gtUInt32 frameNum = (int) luaL_checkinteger(L, 2);
                gtUInt32 drawNum = (int) luaL_checkinteger(L, 3);

                (*ud)->lpgpu2_getStackTraceParametersForFrameAndDrawNum(frameNum, drawNum, traceParameters);

                lua_newtable(L);
                char *params;
                for (uint i = 0; i < traceParameters.size(); i++)
                {
                    params = (char *) malloc((2 * traceParameters[i].m_binaryParams.size() + 1) * sizeof(char));
                    uint j;
                    for (j = 0; j < traceParameters[i].m_binaryParams.size(); j++)
                    {
                        sprintf(&params[j * 2], "%02X", traceParameters[i].m_binaryParams[j]);
                    }
                    params[j * 2] = '\0';
                    lua_pushstring(L, (const char *) params);
                    free(params);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;
            }
            return 0;
        }

        static int Lua_getFramesForCategoryWithCallsAboveThreshold(lua_State *L)
        {
            if (lua_gettop(L) == 3)
            {
                gtString callCategory;
                gtVector<ProfileTraceInfo> profileTraceInfo;
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));
                const char *category = luaL_checkstring(L, 2);
                int threshold = (int) luaL_checkinteger(L, 3);
                callCategory.fromASCIIString(category);

                (*ud)->lpgpu2_getTracesForCallCategoryWithNumCallsAboveThreshold(callCategory, threshold, profileTraceInfo);

                lua_newtable(L);
                for (uint i = 0; i < profileTraceInfo.size(); i++)
                {
                    lua_pushnumber(L, profileTraceInfo[i].m_frameNum);
                    lua_rawseti(L, -2, i + 1);
                }
                return 1;

            }
            return 0;
        }

        static int Lua_beginTransaction(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_beginTransaction();
            }
            return 0;
        }

        static int Lua_endTransaction(lua_State *L)
        {
            if (lua_gettop(L) == 1)
            {
                LuaBinding **ud = static_cast<LuaBinding **>(luaL_checkudata(L, 1, "LuaBindingClassA"));

                (*ud)->lpgpu2_endTransaction();
            }
            return 0;
        }

    };
