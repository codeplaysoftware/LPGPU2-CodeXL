--
-- The MIT License
--
-- Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
-- For conditions of distribution and use, see the accompanying COPYING file.
--
--
--This is the (untested) version of the feedback engine tailored to work integrated with CodeXL

--Globals

first_pass_result={}
fp_frameIndex={}

summary_result={}
regions_result={}
worst_n_frames={}

fps_default = 0
fps_min = 0
fps_max = 0


-- This function returns the version number of this script file
-- It needs to be updated for every committed change
function getScriptVersion()
	return "1.57"
end



-- Time the last time FTime was called
function_now = 0
function_string = ""
function_progress = 0

-- returns the time since last called
function FTime()
 function_then = function_now
 function_now  = os.time()

 return function_now - function_then
end

function FLine()
 print("----------------------------------------------------------------")
end

-- Call as the first line of the function you want to time... NB - DO NOT NEST!
function FStart(name)
 FTime()

 function_string = name
 function_progress = 0

 FLine()
 print("START : " .. function_string)
 FLine()
end

-- Call to reset your progress to zero
function FSection(name)
 print("Section : "..name.."...")

 function_progress = 0
end

-- Call in the work loop of your function to report progress
function FProgress(i,n)
 if i * 100.0 / n > function_progress then
  function_progress = function_progress + 5

  print(string.format("%4d%%...",function_progress))

  if function_progress == 100 then print() end
 end
end

-- Call as the last line of the function you want to time
function FEnd()
 FLine()
 print("END : " .. function_string .. " (" .. FTime() .. " seconds)")
 FLine()
 print()
 print()
end



function annotateDispatch(stub,code,msg,frame,draw)
-- print(stub,"(",frame,",",draw,"): ",msg) -- Uncomment for debugging

-- LPGPU2:beginTransaction()
 LPGPU2:writeAnnotation(code,msg,frame,draw)
-- LPGPU2:  endTransaction()
end



function annotateInformation(msg,frame,draw) annotateDispatch("Information",LPGPU_INFORMATION,msg,frame,draw) end
function annotateSuggestion (msg,frame,draw) annotateDispatch("Suggestion" ,LPGPU_SUGGESTION ,msg,frame,draw) end
function annotateIssue      (msg,frame,draw) annotateDispatch("Issue"      ,LPGPU_ISSUE      ,msg,frame,draw) end



function DBStart(name)
 print("DB access: " .. name .. "...")
end

function DBEnd()
 print("         : complete!")
 print()
end



local   cpy_LPGPU2_getBinaryParamsFromTraceParams = nil
function    LPGPU2_getBinaryParamsFromTraceParams()
     if cpy_LPGPU2_getBinaryParamsFromTraceParams== nil then
        DBStart(  "getBinaryParamsFromTraceParams")
        cpy_LPGPU2_getBinaryParamsFromTraceParams
          = LPGPU2:getBinaryParamsFromTraceParams()
        DBEnd()
     end
 return cpy_LPGPU2_getBinaryParamsFromTraceParams
end


local   cpy_LPGPU2_getBinaryParamsFromOpenGLStateAndFrameCalls = nil
function    LPGPU2_getBinaryParamsFromOpenGLStateAndFrameCalls()
     if cpy_LPGPU2_getBinaryParamsFromOpenGLStateAndFrameCalls== nil then
        DBStart(  "getBinaryParamsFromOpenGLStateAndFrameCalls")
        cpy_LPGPU2_getBinaryParamsFromOpenGLStateAndFrameCalls
          = LPGPU2:getBinaryParamsFromOpenGLStateAndFrameCalls()
        DBEnd()
     end
 return cpy_LPGPU2_getBinaryParamsFromOpenGLStateAndFrameCalls
end


local   cpy_LPGPU2_getBinaryParamsFromOpenGLBufferAndDrawCalls= nil
function    LPGPU2_getBinaryParamsFromOpenGLBufferAndDrawCalls()
     if cpy_LPGPU2_getBinaryParamsFromOpenGLBufferAndDrawCalls== nil then
        DBStart(  "getBinaryParamsFromOpenGLBufferAndDrawCalls")
        cpy_LPGPU2_getBinaryParamsFromOpenGLBufferAndDrawCalls
          = LPGPU2:getBinaryParamsFromOpenGLBufferAndDrawCalls()
        DBEnd()
     end
 return cpy_LPGPU2_getBinaryParamsFromOpenGLBufferAndDrawCalls
end


--[[ WIP : temporarily removed due to naive implementation...
local   cpy_LPGPU2_getBinaryParamsForApiAndCategory= nil
function    LPGPU2_getBinaryParamsForApiAndCategory(api,cat)
     if cpy_LPGPU2_getBinaryParamsForApiAndCategory== nil then
        DBStart(  "getBinaryParamsForApiAndCategory")
        cpy_LPGPU2_getBinaryParamsForApiAndCategory
          = LPGPU2:getBinaryParamsForApiAndCategory(api,cat)
        DBEnd()
     end
 return cpy_LPGPU2_getBinaryParamsForApiAndCategory
end


local   cpy_LPGPU2_getBinaryParamsFromTraceParamsForCallType = nil
function    LPGPU2_getBinaryParamsFromTraceParamsForCallType(ct)
     if cpy_LPGPU2_getBinaryParamsFromTraceParamsForCallType== nil then
        DBStart(  "getBinaryParamsFromTraceParamsForCallType")
        cpy_LPGPU2_getBinaryParamsFromTraceParamsForCallType
          = LPGPU2:getBinaryParamsFromTraceParamsForCallType(ct)
        DBEnd()
     end
 return cpy_LPGPU2_getBinaryParamsFromTraceParamsForCallType
end
--]]




-- This function is used to calculate a sensible default threshold.
function get_fps()

        samples_timestamp = LPGPU2:getTimesForCounterWithName('FPS')
        samples_value = LPGPU2:getSamplesForCounterWithName('FPS')
        n_samples = #samples_timestamp

        fps_available = false
        for i=1, n_samples do
            if samples_value[i] > 0 then
                fps_available = true
            end
        end

        calls_start = LPGPU2:getCpuStartFromTraces()
        calls_end = LPGPU2:getCpuEndFromTraces()
        frame_num = LPGPU2:getFrameNumFromTraces()
        previous_frame_num = 0
        last_timestamp = LPGPU2:getFirstCallTimestamp()

        n_frames = #frame_num
        fps_table = {}
        for index = 1, n_frames do
            current_frame_num = frame_num[index]

            if current_frame_num ~= previous_frame_num then
                -- new Frame. Store the previous one.
                current_timestamp = calls_end[index]

                samples = {}
                if (fps_available) then
                    samples = LPGPU2:getAveragedValuesForCategoryWithinRange('FPS', last_timestamp, current_timestamp)
                end
                if (#samples > 0) then
                    FPS = samples[1]
                else
                    -- Time is stored in nano seconds
                    FPS = 1.0 / ((current_timestamp - last_timestamp) / 1000000000) --Approximate FPS
                end
                table.insert(fps_table, FPS)

                time_diff = current_timestamp - last_timestamp
                last_timestamp = current_timestamp
            end
            previous_frame_num = current_frame_num

        end

        if (#fps_table > 0) then
            table.sort(fps_table)

            -- Set the min / max values.
            fps_min = math.floor(fps_table[1])
            fps_max = math.ceil(fps_table[#fps_table])

            -- Set the default to catch the slowest 10% of frames.
            if (#fps_table > 1) then
                fps_default = math.floor( fps_table[math.floor(#fps_table * 0.9)] )
            else
                fps_default = fps_table[1]
            end
            if (fps_default == 0) then
                fps_default = 1
            end
        end
        print('fps_min = '..fps_min..' fps_max = '..fps_max..' fps_default = '..fps_default)
end

-- Analysis test function --
--[[
--
-- This script analyses the database and reports the frames with performance below a given threshold.
--
-- Input:
--      threshold (FPS)
--
-- Writes first_pass_results, which holds three tables:
--      resFPS -> FPS per limited frame
--      resCalls -> Function calls per limited frame (or section). Each entry of the table holds another table 
--                  in the format of the "traces" table in the database.
--      boundaries -> Start and end of the frame. Format: Timestamp Start, Timestamp end
--
--]]

--Posibly boundaries and even FPS won't be needed by other scripts. Consider eliminating


function first_pass(threshold)

        FStart("first_pass")

        resFPS={}
        resCalls={}
        boundaries={}

        samples_timestamp = LPGPU2:getTimesForCounterWithName('FPS')
        samples_value     = LPGPU2:getSamplesForCounterWithName('FPS')
        n_samples         = #samples_timestamp

        -- check the FPS counter holds +ve values.
        fps_available = false
        for i=1, n_samples do
            if samples_value[i] > 0 then
                fps_available = true
            end
        end

        calls_name  = LPGPU2:getCallNameFromTraces()
        calls_start = LPGPU2:getCpuStartFromTraces()
        calls_end   = LPGPU2:getCpuEndFromTraces()
        frame_num   = LPGPU2:getFrameNumFromTraces()
        n_calls     = #calls_name

        -- check the frame_nums do increment.
        single_frame = true

        if frame_num[#frame_num] > 0 then
            single_frame = false
        end

        total_frames=0
        current_frame_num = 0
        previous_frame_num = 0

        calls_between_frames={}
        last_timestamp=LPGPU2:getFirstCallTimestamp()

        for index = 1, n_calls do

            FProgress(index,n_calls)

            -- Get current_frame_num
            current_frame_num = frame_num[index]
            if single_frame and (index == n_calls -1 ) then
                -- print('Only one frame')
                current_frame_num = 1
            end

            if current_frame_num ~= previous_frame_num then
                -- new Frame. Store the previous one.
                current_timestamp = calls_end[index]

                if last_timestamp > current_timestamp then last_timestamp = current_timestamp end

                samples = LPGPU2:getAveragedValuesForCategoryWithinRange('FPS', last_timestamp, current_timestamp)
                if fps_available and (#samples > 0) then
                    FPS = samples[1]
                else
                    -- Time is stored in nano seconds
                    FPS = 1.0 / ((current_timestamp - last_timestamp) / 1000000000) --Approximate FPS
                end
--              print('FPS for frame '..previous_frame_num..' / index '..index..' = '..FPS..' threshold = '..threshold)
                time_diff = current_timestamp - last_timestamp
                 if FPS <= threshold then
                     -- Below threshold, store frame info
--                   print('Limited Frame ('..FPS..' FPS)'..previous_frame_num..' Call '..index)
                     table.insert(resFPS, FPS)
                     table.insert(resCalls, calls_between_frames)
                     table.insert(boundaries, {last_timestamp, current_timestamp})
                     table.insert(fp_frameIndex, previous_frame_num)
                 end

                 calls_between_frames = {} --Clear function calls since last delimiter
                 last_timestamp = current_timestamp
                 total_frames = total_frames + 1
            end

            table.insert(calls_between_frames, {calls_name[index], calls_start[index], calls_end[index]}) --Store the function call
            previous_frame_num = current_frame_num

        end -- finished

--      print('There are '..#resCalls..' limited frames of a total of '..total_frames)

        first_pass_result[1] = resFPS         -- resFPS[#limited_frames] = FPS_samples_value
        first_pass_result[2] = resCalls       -- resCalls[#limited_frames] = calls_table[0..x] = {name, start, end}
        first_pass_result[3] = boundaries     -- boundaries[#limited_frames] = {start_time, end_time} of the frame
        first_pass_result[4] = fp_frameIndex

        FEnd()
end


-- Analysis test function --
--[[
--
-- This script reports, per frame (or sections between FPS samples), the number of calls and total execution time per function.
--
-- Uses:
--      first_pass_result: A set of frames (or sections) in the format output by script First_Pass.lua
--
-- Writes summary _results, which holds three tables:
--      general_function_summary: The summary for each frame (or section). Each entry holds a table with the following format:  function_name, number_of_calls, total_execution_time
--      cpu_active_ratio: The ratio of the length of the function calls to the length of the frame, per frame
--      duration_per_frame: The total duration of the frame
--]]

function frame_summary()

        FStart("frame_summary")

         --Calls in the trace
        posFunctionName=1
        posFunctionStart=2
        posFunctionEnd=3

        --Samples
        posTimestamp=0
        posSampled=2

        num_offenders=5

        limited_frames=first_pass_result[2] -- [#limited_frames] = {name, start, end}

        general_function_summary={}
        cpu_active_ratio={}
        duration_per_frame={}

        for k,frame in ipairs(limited_frames) do --For each  frame

                FProgress(k,#limited_frames)

                total_active=0
                frame_function_summary={} --This is a dictionary indexed by function name.
                                          --Each entry holds the number of calls and total runtime for the function

                -- frame = calls_table[0..x]{calls_name[i], calls_start[i], calls_end[i]}

                for key, call in ipairs(frame) do --For each call in the frame
                        -- call = {name, start, end}

                        duration=call[posFunctionEnd]-call[posFunctionStart]

                        --Update the function summary
                        if frame_function_summary[call[posFunctionName]]==nil then
                                frame_function_summary[call[posFunctionName]] = {1, duration}
                        else
                                frame_function_summary[call[posFunctionName]][1] = frame_function_summary[ call[posFunctionName] ][1] + 1
                                frame_function_summary[call[posFunctionName]][2] = frame_function_summary[call[posFunctionName]][2] + duration
                        end
                        total_active=total_active+duration
                end
                 
                --frame_function_summary[callname][1] = number of calls
                --frame_function_summary[callname][2] = total duration

                durations={} --Table to hold the durations in a contiguous manner so the can be sorted

                dict={} --This is a dictionary indexed by duration
                        --Each entry holds the function names for the duration

                --Populate durations and  dict
                for callname,summary in pairs(frame_function_summary) do
                        -- summary = {num calls, duration}
                        num_calls = summary[1]
                        call_duraion = summary[2]
                        table.insert(durations, call_duraion)

                        if dict[call_duraion]==nil then
                                dict[call_duraion]={callname}
                        else
                                table.insert(dict[call_duraion], callname)
                        end
                end
                -- durations[i] = duration
                -- dict[duration] = table of callnames.

                table.sort(durations)

                sorted_summary={}
                last=-1
                for k2,duration in ipairs(durations) do --For each duration

                        if duration ~= last then --If it has not already been processed
                                for k3, callname in pairs(dict[duration]) do
                                        table.insert(sorted_summary, {callname, frame_function_summary[callname][1], frame_function_summary[callname][2]}) --Insert in the sorted summary
                                end
                        end
                        last=v2
                end
                -- sorted_summary[i] = { callname, number of calls, total duration }

                --Store the data into the structures that will be returned
                table.insert(general_function_summary, sorted_summary)

                --    duration =       end_time             -      start_time
                frame_duration = first_pass_result[3][k][2] - first_pass_result[3][k][1]
                table.insert(cpu_active_ratio, total_active/frame_duration)
                table.insert(duration_per_frame, first_pass_result[3][k][2]-first_pass_result[3][k][1])
        end

        summary_result[1]=general_function_summary -- unchanged [#limited_frames][#calls] { callname, number of calls, total duration }
        summary_result[2]=cpu_active_ratio         -- [#limited_frames] ratio
        summary_result[3]=duration_per_frame       -- [#limited_frames] duration

        FEnd()
end

--[[
--
-- This script identifies regions holding frames (or sections between FPS samples) with identical mixes of function calls.
-- It uses a hash-like function to identify equality. It is not an actual hash, as it
-- considers frames with different sequences of the same mix of calls equal.
--
-- Uses:
--      summary_results -> The set of frames (or regions) to analyse. It must be in the format outputted by the Frame_Summary.lua script
--
-- Writes regions_result, which contains two tables:
--      region_summaries -> The summary for each of the regions (that of the first frame belonging to each region is used). Each
--                          entry holds a table with the following format:  function_name, number_of_calls, total_execution_time
--      matching_frames -> The indices of the frames belonging to each of the regions
--
--]]


function identify_regions()

        FStart("identify_regions")

        -- summary_result[1] == general_function_summary -- [#limited_frames][#calls] { callname, number of calls, total duration }

        function_summary = summary_result[1]
        prime = 31
        calls_to_check = 20
        identified_regions={} --Non-contiguous
        for frame_key, frame in ipairs(function_summary) do --For each frame
                FProgress(frame_key,#function_summary)
                res_hash = 0
                for func_key, func in pairs(frame) do --For each function in the frame
                        func_hash = 7
                        func_tag = func[1]..'_'..func[2] --Obtain a "tag" in the format nameCall_numCalls

                        --Hash the tag
                        for j=1, #func_tag do
                                char = string.byte(func_tag, j)
                                func_hash = (func_hash*prime) + char
                        end
                        --Add to the frame hash
                        res_hash = res_hash + func_hash
                end

                --Use the frame hash to identify (and store) regions
                
                if identified_regions[res_hash] == nil then
                        identified_regions[res_hash] = {frame_key}
                else
                        table.insert(identified_regions[res_hash], frame_key)
                end
        end

        -- identified_regions[res_hash][list of frame indices]
        -- region_frame == fp_frameIndex[frame_key]

        --Convert the identified regions into a contiguous array so it can be easily looped over

        -- An identified region is a list of limited frames with the same mix of api calls.
        matching_frames={} --Contiguous, starting at 1
        new_region_summaries={}
        old_region_summaries={}

        for hash,identified_frame_indices in pairs(identified_regions) do
                table.insert(matching_frames, identified_frame_indices)
                frame_summaries = {}
                for frame = 1, #identified_frame_indices  do
                    frame_index = identified_frame_indices[frame]
                    -- limited_frames[general_function_summary][limited_frame][call_number] = {callName, timesCalled, totalTime}

                    table.insert(frame_summaries, function_summary[frame_index]) -- inset a table [#calls]{callName, timesCalled, totalTime}
                end
                table.insert(new_region_summaries, frame_summaries) -- info for all frames
                table.insert(old_region_summaries, function_summary[identified_frame_indices[1]]) -- info for the first frame only
        end

        -- matching_frames[#identified_regions][#frames] frame_index (index for table of limited frames)

        -- new_region_summaries[#identified_regions][#frames][#calls] {callName, timesCalled, totalTime}

        split_regions={}
        split_summaries={}
        region_times = {}
--[
        -- Go through the matching_frames table.
        -- matching_frames[#identified_regions][#frames] frame_index
        LPGPU2:beginTransaction()
        num_regions = 0
        for current_region_index=1, #matching_frames do
                -- these are indices, rather than the real frameNum
                region_frame_indices = matching_frames[current_region_index]

                region_frame_list = {}
                for k, frame_index in pairs(matching_frames[current_region_index]) do
                    table.insert(region_frame_list, fp_frameIndex[frame_index]) -- the real frameNum.
                end
                first_frame = region_frame_list[1] -- first frame in a region.
                last_frame = first_frame
                -- first_pass_result[3]=boundaries     -- boundaries[#limited_frames] = {start_time, end_time} of the frame
                limited_frame_index = region_frame_indices[1]
                first_frame_time = first_pass_result[3][limited_frame_index][1]
                last_frame_time = first_frame_time

                i=2
--              print('\n'..'\n'..'Write Region '..current_region_index)
                region_calls = {}
                -- add the calls from this frame to the table.
                -- new_region_summaries[#identified_regions][#frames][#calls] {callName, timesCalled, totalTime}
                for _,v in pairs (new_region_summaries[current_region_index][1]) do
                    callname = v[1]
                    if region_calls[callname]==nil then -- index by callName
                        region_calls[callname] = {v[2], v[3]}
                    else
                        region_calls[callname][1] = region_calls[callname][1] + v[2] -- times_called
                        region_calls[callname][2] = region_calls[callname][1] + v[3] -- total_time
                    end
                end

                while (i <= #region_frame_list) do -- matching_frames)

                        real_frame = region_frame_list[i] -- next frame in the list. The real frameNum

                        if real_frame ~= last_frame + 1  then -- a new region, write the previous one.

                                -- First, Calculate the times_called and total_time for each call

                                LPGPU2:writeRegion(num_regions, first_frame, last_frame) -- regionId, frameStart, frameEnd
                                table.insert(split_regions, first_frame)
                                table.insert(region_times, {first_frame_time, last_frame_time})

                                for k,v in pairs (region_calls) do    
                                    LPGPU2:writeCallSummary(num_regions, k, v[1], v[2]) -- regionId, callName, timesCalled, totalTime
                                end
                                table.insert(split_summaries, new_region_summaries[current_region_index])

                                first_frame = real_frame
                                limited_frame_index = region_frame_indices[i]
                                first_frame_time = first_pass_result[3][limited_frame_index][1]
                                num_regions = num_regions+1

                                region_calls = {}
                        end
                        -- add the calls from this frame to the table.
                        for _,v in pairs (new_region_summaries[current_region_index][i]) do
                            callname = v[1]
                            if region_calls[callname]==nil then -- index by callName
                                region_calls[callname] = {v[2], v[3]}
                            else
                                region_calls[callname][1] = region_calls[v[1]][1] + v[2] -- times_called
                                region_calls[callname][2] = region_calls[v[1]][1] + v[3] -- total_time
                            end
                         end

                        last_frame = real_frame
                        limited_frame_index = region_frame_indices[i]
                        last_frame_time = first_pass_result[3][limited_frame_index][1]
                        i = i + 1
                end

                LPGPU2:writeRegion(num_regions, first_frame, last_frame) -- regionId, firstFrame, lastFrame
                table.insert(split_regions, first_frame)
                table.insert(region_times, {first_frame_time, last_frame_time})

                for k,v in pairs (region_calls) do
                        LPGPU2:writeCallSummary(num_regions, k, v[1], v[2]) -- regionId, callName, timesCalled, totalTime
                end
                num_regions=num_regions+1
                table.insert(split_summaries, new_region_summaries[current_region_index])
        end
        LPGPU2:endTransaction()
--]]
        regions_result[1] = split_summaries   -- [#regions][#frames][#calls] {callName, timesCalled, totalTime} -- TODO #frames in "identified" region, not real region ???
        regions_result[2] = split_regions     -- [#regions]  first_frame_in_region (real fame)
        regions_result[3] = region_times      -- [#regions] {starttime, endTime}

        FEnd()
end

--[[
----
---- This function runs the mali offline compiler on the shaders contained in the database,
---- parses its ouputs and performs a budget calculation.
----
--]]

function mali_offline_compiler_analysis()
        shaders_info=nil
        if device==nil then
                shaders_info=LPGPU2:getCompilerOutput()
                device='Mali'
        else
                shaders_info=LPGPU2:getCompilerOutput(device)
        end


        --This values are hardcoded but could also be read from the database
        num_cores=12 
        freq=650*1000000
        target_fps=60
        target_resolution={1920,1080}
        counter=1

        num_shaders=0
        shaders={}
        types={}
        frame_num={}
        draw_num={}

        shortest_path={}
        longest_path={}
        percentage={}

        if device=='Imagination Series6' then
                num_cores=4
                freq=600*1000000
        end

        for s in string.gmatch(shaders_info, '[^|]+')do
                if tonumber(s)~=nil then
                        table.insert(types,s)
                        num_shaders=num_shaders+1
                elseif string.match(s, 'f%d+') then
                        table.insert(frame_num, string.sub(s,2,#s))
                elseif string.match(s, 'd%d+') then
                        table.insert(draw_num,string.sub(s,2,#s))
                else
                        table.insert(shaders,s)
                end
        end

        LPGPU2:beginTransaction()
        if string.sub(device,1,4)=='Mali' then

                for i=1, num_shaders do
                        kernel_name=''
                        spilling=false
                        bound=''
                        divergent=false

                        shortest={0,0,0}
                        longest={0,0,0}
                        shortest_b=''
                        longest_b=''

                        out_per_shader=shaders[i]
        --                print(out_per_shader)
--[
                        for l in string.gmatch(out_per_shader, '[^\n]+') do
                                l_tokens=string.gmatch(l, '%S+')
                                word_list={}
                                for word in l_tokens do
                                        table.insert(word_list, word)
                                end
                                if #word_list==1 then
                                        kernel_name=word_list[1]
                                elseif #word_list>1 and word_list[2]=='work'then
                                        spilling= word_list[#word_list-1]~='not'
                                elseif word_list[1]=='Shortest' then
                                        shortest[1]=word_list[4]
                                        shortest[2]=word_list[5]
                                        shortest[3]=word_list[6]
                                        shortest_b=word_list[7]
                                elseif word_list[1]=='Longest' then
                                        longest[1]=word_list[4]
                                        longest[2]=word_list[5]
                                        longest[3]=word_list[6]
                                        longest_b=word_list[7]
                                end
                        end
                        if longest_b~=shortest_b then
                                bound=shortest_b..'-'..longest_b
                                divergent=true

                        else
                                bound=shortest_b
                                for i=1, #shortest do
                                        if shortest[i]~=longest[i] then
                                                divergent=true
                                                bound=bound..'-'..longest_b
                                                break
                                        end
                                end
                        end

                        if kernel_name=='' then
                                kernel_name=tostring(counter)
                                counter=counter+1
                        end

                        print('Shader '..kernel_name)
                        print('\tSpilling: '..tostring(spilling))
                        print('\tDivergent: '..tostring(divergent))
                        print('\tBound: '..bound)
                        t=types[i]
                        p=0
                        if t=='35633' or t=='35632' then
                                div=target_fps
                                conf='a target FPS of '..target_fps
                                percent=''
                                if t=='v' then
                                        div=div*num_vertices
                                        conf=conf..' and '..num_vertices..' vertices.'
                                else
                                        div=div*target_resolution[1]*target_resolution[2]
                                        conf=conf..' and a '..target_resolution[1]..'x'..target_resolution[2]..' resolution.'
                                end
                                shortest_total=shortest[1]+(shortest[2]*1.5)+shortest[3]
                                if not divergent then
                                        num=((shortest_total/(num_cores*freq/div))*100)
                                        p=num
                                        percent=string.format("%.2f", num)..'% of the total GPU processing budget'
                                else
                                        if longest[1]~='N/A' and longest[2]~='N/A' and longest[3]~='N/A' then
                                                longest_total=longest[1]+(longest[2]*1.5)+longest[3]
                                                min=((shortest_total/(num_cores*freq/div))*100)
                                                max=((longest_total/(num_cores*freq/div))*100)
                                                p=(max+min)/2
                                                percent='between '..string.format("%.2f", min)..' and '..string.format("%.2f", max)..'% of the total GPU processing budget'
                                        else
                                                num=((shortest_total/(num_cores*freq/div))*100)
                                                p=num
                                                percent='at least '..string.format("%.2f", num)..'% of the total GPU processing budget'
                                        end
                                end
                                print('This shader would take '..percent..' for '..conf)
                        end

                        if spilling then
                                msg='Spilling has a negative impact on performance. Consider reducing the number of live variables, using a larger amount of work-items or vectorisation.'
                                print('\t'..msg)
                                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg)
                        end

                        if divergent then
                                msg='Divergence has a negative impact on data locality for caching. Eliminate variable length loops and/or asymmetric conditional blocks if possible to avoid it.'
                                print('\t'..msg)
                                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg)
                        end

                        table.insert(shortest_path, shortest)
                        table.insert(longest_path, longest)
                        table.insert(percentage, p)
                        print('\n')

                end

        elseif string.sub(device,1,6)=='Series' then
                for i=1, num_shaders do
                        print(shaders[i])
                        for c in string.gmatch(shaders_info, '.*cycles = (%d+)') do
                                cycles=c
                        end
                        div=target_fps
                        conf='a target FPS of '..target_fps
                        percent=''
                        if t=='v' then
                                div=div*num_vertices
                                conf=conf..' and '..num_vertices..' vertices.'
                        else
                                div=div*target_resolution[1]*target_resolution[2]
                                conf=conf..' and a '..target_resolution[1]..'x'..target_resolution[2]..' resolution.'
                        end
                        num=((cycles/(num_cores*freq/div))*100)
                        p=num
                        percent=string.format("%.2f", num)..'% of the total GPU processing budget'
                        msg='This shader would take at least '..percent..' for '..conf..'\n'
                        print(msg)
                        LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg)
                        print('This budgeting would be worth checking, as I am not completely sure of the values that I used for the HW\n')
                end
        end
        LPGPU2:endTransaction()

--]]
end



--[[
----
---- This function analyses the HW counters contained in the database and obtains the 
---- bottleneck for the application. This analysis is performed by identifying the counter
---- whose value is above a threshold for the longest.
----
----]]
--

-- *** This assumes all counters are sampled at the same time. No longer true for DCAPI V2.
-- We can (for the time being) assume that all counters for a category will be sampled together.
-- We can update this feedback to use getUniqueTimesFromSamplesForCategory()
function coarse_grain_limitation_analysis()

        FStart("coarse_grain_limitation_analysis")

        causes_to_check={}

        thresholds={}
        thresholds['CPU_LOAD']=95
        thresholds['GPU_LOAD']=95
        thresholds['CPU_TEMP']=70
        leeway=1
        hysteresis=5

        table.insert(causes_to_check, 'CPU_LOAD')
        table.insert(causes_to_check, 'GPU_LOAD')
        table.insert(causes_to_check, 'CPU_TEMP')

        detected_regions_for_cause={}

        msg='Checking causes: '
        for k,v in pairs(causes_to_check)do msg=msg..v..' ' end
        print(msg)


        samples={} --A table indexed by causes
                   --Each entry holds the max value per timestamp for the counters associated to the cause

        timestamps=LPGPU2:getUniqueTimesFromSamples()

        for key, cause in ipairs(causes_to_check) do
                max = LPGPU2:getMaxValuesForCategory(cause)
                print('cause = '..cause..' max size = '..#max..' timestamps size = '..#timestamps)
                samples[cause]={timestamps,LPGPU2:getMaxValuesForCategory(cause)}
                
                detected_regions_for_cause[cause]={}
        end
        -- samples[cause] = {table of times, table of max values} - these tables are expected to be the same size. UNSAFE

        num_samples=#samples[causes_to_check[1]]
        region_start_for_cause={}

        for key, cause in ipairs(causes_to_check) do
                region_start_for_cause[cause]=-1
        end     

        for i=1,  num_samples do --For each sample

               for key, cause in ipairs(causes_to_check)do --For each cause to check

                       if #samples[cause][2] > 0 then
                            if samples[cause][2][i]>=thresholds[cause] then --If the sample is above the threshold

                                if region_start_for_cause[cause]==-1 then --If we were not in a limited region
                                        region_start_for_cause[cause]=samples[cause][1][i] --Mark the beginning of a region
                                        thresholds[cause]=thresholds[cause]-hysteresis --Update threshold to reflect hysteresis
                                end

                            else

                                if region_start_for_cause[cause]~=-1 then --If we were in a region
                                        region={region_start_for_cause[cause], samples[cause][1][i]}
                                        table.insert(detected_regions_for_cause[cause], region) --Store region
                                        region_start_for_cause[cause]=-1 --Mark the end of the region
                                        thresholds[cause]=thresholds[cause]+hysteresis --Update threshold to not reflect hysteresis
                                end

                            end
                        end

                end

        end

        --In case the last samples are limited and there has not been an end to the region
        for key, cause in ipairs(causes_to_check)do
               if region_start_for_cause[cause]~=-1 then
                        region={region_start_for_cause[cause], samples[cause][1][num_samples-1]}
                        table.insert(detected_regions_for_cause[cause], region)
                        region_start_for_cause[cause]=-1
               end
        end



        --For each region with high temp, check if it is actually limiting performance. This is, check if cpu load is high for the region and frequency (roughly) decreasing
        actual_temp_limited_regions={}
        if detected_regions_for_cause['TEMP']~=nil then
                for key, region in ipairs(detected_regions_for_cause['TEMP']) do --For each temperature limited candidate
                        start_timestamp=region[1]
                        end_timestamp=region[2]

                        --May not properly work if a region with high temp covers several CPU high regions
                        for k, high_cpu in ipairs(detected_regions_for_cause['CPU_LOAD']) do --For each region limited by CPU
                                if (start_timestamp>=high_cpu[1] and start_timestamp<=high_cpu[2]) or 
                                   (end_timestamp>=high_cpu[1] and end_timestamp<=high_cpu[2]) or 
                                   (start_timestamp<=high_cpu[1] and start_timestamp>=high_cpu[2]) then --If there is overlapping with the temp candidate

                                        samples=LPGPU2:getAveragedValuesForCategoryWithinRange('CPU_FREQ', start_timestamp, end_timestamp) --Get avg freq for region
                                        num_samples=#samples

                                        samples_table={}

                                        for i=1, num_samples do
                                                table.insert(samples_table,  samples[i])
                                        end

                                        --Obtain first freq, last freq and median
                                        start_value=samples_table[1]
                                        last_value=samples_table[#samples_table]
                                        table.sort(samples_table)
                                        median=0
                                        if #samples_table%2~=0 then
                                                median=samples_table[math.ceil(#samples_table/2)]
                                        else
                                                median=(samples_table[#samples_table/2]+samples_table[(#samples_table/2)+1])/2
                                        end
                                        if start_value>median and last_value<start_value then --Very rough analysis of frequency trend
                                                --Temp Limited. Eliminate cpu limited
                                                table.insert(actual_temp_limited_regions, region)
                                                high_cpu[1]=high_cpu[2]
                                        else
                                                region[1]=region[2]
                                        end
                                else
                                        if end_timestamp<high_cpu[1] then
                                                region[1]=region[2] --Change the region duration so it does not add for the final decission regarding what is limitting the app
                                                                    --Consider removing it from the table for real
                                                break
                                        end
                                end


                        end
                end
        end
        detected_regions_for_cause['TEMP']=actual_temp_limited_regions --Store the actual temp limited regions

        total_duration_for_cause={}

        for key, cause in ipairs(causes_to_check) do
                total_duration_for_cause[cause]=0
        end

        --Obtain total duration for the regions limited by each cause
        for key, cause in ipairs(causes_to_check) do
                for k, region in ipairs(detected_regions_for_cause[cause]) do
                        total_duration_for_cause[cause]=total_duration_for_cause[cause]+region[2]-region[1]
                end
        end

--[
        longest='NO LIMITATION'
        duration=0
        print('\n')
        for key, cause in ipairs(causes_to_check) do
                print('Detected regions for cause '..cause..':')
                for k, region in ipairs(detected_regions_for_cause[cause]) do
                        print('\tRegion starting at timestamp '..region[1]..' and finishing at '..region[2])
                end

                if total_duration_for_cause[cause]>duration then
                        duration=total_duration_for_cause[cause]
                        longest=cause
                end

                print('Total for this limitation: '..total_duration_for_cause[cause])
                print('----------------------------------------------')
        end
--]]
        print('The main cause of limitation for the app is '..longest)
        -- string.format("found %d Calls",n_calls)

        LPGPU2:beginTransaction()
        LPGPU2:writeAnnotation(LPGPU_INFORMATION, string.format("The main cause of limitation for the app is %s",longest))
        LPGPU2:endTransaction()

        FEnd()
end

--[[
--
-- This scripts analyses the Database and
-- and tries to discover low level causes for limitation.
--
-- Supported causes:
--      SINGLE_THREAD: The application only uses one thread and multithreading
--                     should be considered.
--]]

function fine_cpu_limitation_analysis()

        FStart("fine_cpu_limitation_analysis")

        num_threads=LPGPU2:getNumThreads()
        res=''
        if num_threads==1 then
                res='the app is single threaded'

        elseif num_threads<=0 then
                print('Thread info not available')
        end

        if res ~= '' then
         LPGPU2:beginTransaction()
         LPGPU2:writeAnnotation(LPGPU_INFORMATION, string.format("The main cause of limitation is: %s",res))
         LPGPU2:endTransaction()
        end

        FEnd()
end
--[[
--
-- This scripts analyses the Database and a set of regions limited by GPU
-- and tries to discover low level causes for limitation.
--
-- Supported causes:
--      FB -> The main cause of limitation is stalls waiting for the frame
--            buffer. The stall cycles for the frame buffer are compared
--            to the total cycles.
--      TEX -> The main cause of limitation is stalls waiting for the texture
--             memory. The stall cycles for the texture memory are compared
--             to the total cycles.
--      SHADER_COMPLEXITY -> The main cause of limitation is that too much work
--                           has to be done per pixel. The number of instructons
--                           per pixel is analysed.
--
--
--]]

function fine_gpu_limitation_analysis(regions)
 FStart("fine_gpu_limitation_analysis(regions)")

        regions = regions_result[3] -- Start / End Times.

        causes_to_check={}
        threshold={}

        local cause_FB   = "frame buffer stalls"
        local cause_TEX  = "texture unit stalls"
        local cause_SHC  = "shader complexity"
        local cause_NONE = "NO_CAUSE"

        table.insert(causes_to_check, cause_FB )
        table.insert(causes_to_check, cause_TEX)
        table.insert(causes_to_check, cause_SHC)

        threshold[cause_FB ] = 10
        threshold[cause_TEX] = 10
        threshold[cause_SHC] = 10

        regions_for_cause={}

        for k, cause in pairs(causes_to_check) do
                regions_for_cause[cause]=0
        end

        regions_for_cause[cause_NONE] = 0
        for k, region in pairs(regions) do --For each region
               FProgress(i,#regions)

               start_timestamp = region[1]
               end_timestamp = region[2]
               value_for_cause = {}

               for k2, cause in pairs(causes_to_check) do --For each cause to check, get the corresponding samples and obtain their associated metric
                       value=0
                       if cause == cause_FB then

                               fb_stalls = LPGPU2:getAveragedValuesForCategoryWithinRange('FB_STALLS', start_timestamp, end_timestamp)
                               cycles = LPGPU2:getAveragedValuesForCategoryWithinRange('TOTAL_CYCLES', start_timestamp, end_timestamp)
                               num_samples = #fb_stalls
                               tot_stalls = 0
                               for i=1, num_samples do
                                       tot_stalls = tot_stalls + fb_stalls[i]
                               end
                               tot_cycles = 0
                               for i = 1, num_samples do
                                       tot_cycles = tot_cycles+cycles[i]
                               end
                               value = (tot_stalls/tot_cycles)*100

                        elseif cause == cause_TEX then

                               tex_stalls = LPGPU2:getAveragedValuesForCategoryWithinRange('TEX_STALLS', start_timestamp, end_timestamp)
                               cycles = LPGPU2:getAveragedValuesForCategoryWithinRange('TOTAL_CYCLES', start_timestamp, end_timestamp)
                               num_samples = #tex_stalls
                               tot_stalls = 0
                               for i = 0, num_samples - 1 do
                                       tot_stalls = tot_stalls + tex_stalls[i]
                               end
                               tot_cycles = 0
                               for i=0, num_samples - 1 do
                                       tot_cycles = tot_cycles + cycles[i]
                               end
                               value = (tot_stalls / tot_cycles) * 100

                        elseif cause == cause_SHC then

                               num_inst = LPGPU2:getAveragedValuesForCategoryWithinRange('NUM_INST', start_timestamp, end_timestamp)
                               num_pixels = LPGPU2:getAveragedValuesForCategoryWithinRange('NUM_PIXELS', start_timestamp, end_timestamp)
                               num_samples = #num_inst
                               tot_inst = 0
                               for i=1, num_samples do
                                       tot_inst = tot_inst + num_inst[i]
                               end
                               tot_pixels = 0
                               for i=0, num_samples - 1 do
                                       tot_pixels=tot_pixels + num_pixels[i]
                               end
                               value = tot_inst / tot_pixels
                        end

                        if value > threshold[cause] then --If metric above threshold, store it
                                value_for_cause[cause] = value
                        end
                end

                --decide cause
                max = 0
                most_significant_cause = cause_NONE

                --Choose the cause that has a value further (in relative terms) from its threshold
                for cause, num in pairs(value_for_cause) do
                        value_to_compare = num / threshold[cause]
                        if value_to_compare > max then
                                max = value_to_compare
                                most_significant_cause = cause
                        end
                end
                regions_for_cause[most_significant_cause] = regions_for_cause[most_significant_cause]+1
        end

        overall_cause = cause_NONE

        max = 0

        for cause, num_regions in pairs(regions_for_cause) do
                if num_regions > max then
                        max = num_regions
                        overall_cause = cause
                end
        end

        LPGPU2:beginTransaction()
        if overall_cause == cause_NONE then
                LPGPU2:writeAnnotation(LPGPU_INFORMATION,"The app is not limited by the GPU",0,0)
        else
                LPGPU2:writeAnnotation(LPGPU_INFORMATION,string.format("The main cause of GPU limitation is %s",overall_cause),0,0)
        end
        LPGPU2:endTransaction()

        FEnd()
end





--[[
--
-- This script classifies the functions in the summaries it is provided with
--
-- Uses:
--      regions_result: The summaries to classify. Both the format outputted by Identify_Regions.lua
--                       and Function_Summary.lua are (should be) supported
--
--]]

function function_type_summary()
        FStart("function_type_summary")

        summaries = regions_result --CHECK!!
        draw_calls = LPGPU2:getSupportedCallsForCategory('Draw')
        bind_calls = LPGPU2:getSupportedCallsForCategory('Bind')
        setup_calls = LPGPU2:getSupportedCallsForCategory('Setup')
        wait_calls = LPGPU2:getSupportedCallsForCategory('Wait')
        error_calls = LPGPU2:getSupportedCallsForCategory('Error')
        frame_delimiters = LPGPU2:getSupportedCallsForCategory('FrameDelimiter')

        available_functions_per_type={} --A list containing, for each function type, a table indexed by function name (for later efficient hash-like access)

        draw_call_dict={}
        setup_call_dict={}
        bind_call_dict={}
        wait_call_dict={}
        error_call_dict={}
        frame_delimiter_dict={}

        percentage_per_type_per_summary={}

        --Create lua tables indexed by the names of the functions of each type
        for _,v in pairs(draw_calls) do
                draw_call_dict[v] = true
        end
        table.insert(available_functions_per_type, draw_call_dict)

        for _,v in pairs(bind_calls) do
                bind_call_dict[v] = true
        end
        table.insert(available_functions_per_type,bind_call_dict)

        for _,v in pairs(setup_calls) do
                setup_call_dict[v] = true
        end
        table.insert(available_functions_per_type,setup_call_dict)

        for _,v in pairs(wait_calls) do
                wait_call_dict[v] = true
        end
        table.insert(available_functions_per_type,wait_call_dict)

        for _,v in pairs(error_calls) do
                error_call_dict[v] = true
        end
        table.insert(available_functions_per_type,error_call_dict)

        for _,v in pairs(frame_delimiters) do
                frame_delimiter_dict[v] = true
        end
        table.insert(available_functions_per_type,frame_delimiter_dict)

        total_durations={}

        -- summaries[1] == [#regions][#frames][#calls] {callName, timesCalled, totalTime} 
        for region, summary in ipairs(summaries[1]) do --For each region
                total_time=0
                time_per_type={}

                --Initialise the time per type for this summary
                for i = 1, (#available_functions_per_type + 1) do
                        table.insert(time_per_type, 0)
                end

                for k2, frame in pairs(summary) do --For each frame in the region
                    for k3, func in pairs(frame) do --For each function in the frame
                            total_time = total_time + func[3]
                            is_other_type=true
                            for i=1, #available_functions_per_type do --For each type
                                    if available_functions_per_type[i][func[1]] then --If the function is of the type
                                            time_per_type[i] = time_per_type[i] + func[3] --Update time for the type
                                            is_other_type = false
                                            break
                                    end
                            end

                            if is_other_type then
--                                  print('->'..func[1])
                                    time_per_type[#available_functions_per_type + 1] = time_per_type[#available_functions_per_type + 1] + func[3]
                            end
                    end
                end

                table.insert(total_durations, total_time)

                --Convert times into percentages of the total time for the summary
                for i = 1, #time_per_type do
                        time_per_type[i] = (time_per_type[i] / total_time) * 100
                end

                table.insert(percentage_per_type_per_summary, time_per_type)
        end


--[
        LPGPU2:beginTransaction()
        for k, v in ipairs(percentage_per_type_per_summary) do
--              print('\n')
                region = k - 1 -- index starts at zero.
--[[
                print('Function type summary for region '..region)
                print('Duration of traced calls: '..total_durations[k])
                print('-------------------------------------')
                print('Draw Functions: '..v[1]..'%')
                print('Bind Functions: '..v[2]..'%')
                print('Setup Functions: '..v[3]..'%')
                print('Wait Functions: '..v[4]..'%')
                print('Error Functions: '..v[5]..'%')
                print('Frame delimiters: '..v[6]..'%')
                print('Others: '..v[7]..'%')
--]]
                LPGPU2:writeCallsPerType(region, v[1], v[2], v[3], v[4], v[5], v[6], v[7])
        end
        LPGPU2:endTransaction()

----]]

        FEnd()
end
--[[
--
-- This script obtains the complete function call trace of one frame of each the regions it receives.
--
-- Uses:
--      regions -> The regions from which frames have to be selected
--      limited_frames -> The complete list of limited frames
--
-- Returns:
--      resFPS -> FPS per limited frame
--      resCalls -> Function calls per limited frame. Each entry of the table holds another table in the format of the "traces" table in the database.
--      boundaries -> Start and end of the frame. Format: Timestamp Start, Timestamp end
--
--]]

--This functions was used in the plotting process. Probably won't use it anymore
function get_representative_frames_from_regions(regions, limited_frames) 
        regions=regions_result
        print('Getting representative frames from the regions')
        res_FPS={}
        res_calls={}
        boundaries={}

        --Get first frame of each region
        for k,v in pairs(regions)do
                table.insert(res_FPS, limited_frames[0][v[1]])
                table.insert(res_calls, limited_frames[1][v[1]])
                table.insert(boundaries, limited_frames[2][v[1]])
        end

--        return res_FPS, res_calls, boundaries
end


-- Analysis test function --
--[[
--
-- This script obtains the the N longest running functions for each frame (or section between FPS samples) out of a given set
--
-- Inputs:
--      num_offenders -> The number of offenders to return (-1 to show all of them)
--
-- Uses:
--      first_pass_result -> The set of frames (or sections) to be analysed. Must be in the format return by the script First_Pass.lua
--
-- Returns: 
--      worst_offenders_per_frame -> A table with the num_offenders functions with the longest execution time for each frame (or section). 
--                                   Each entry of the table holds another table in the format of the "traces" table in the database.
--]]


--Probably will not be used any more. Mainly used in the plotting process. Return commented out
function identify_offenders(num_offenders) 

        limited_frame_info=first_pass_result

        print('Running offender identification')

        limited_frames=limited_frame_info[1]

        worst_offenders_per_frame={}

        for k,frame in ipairs(limited_frames) do --For each limited frame

                durations={}
                dict={} --This is a table indexed by durations. It holds the keys in "frame" for each duration
                        --Used to recover the call information after durations are sorted

                for key, call in ipairs(frame) do --For each call within the frame
                        table.insert(durations, call[posFunctionEnd]-call[posFunctionStart]) --Store its duration

                        --Add the key of the call in frame to the position of the dictionary indexed by its duration
                        if dict[call[posFunctionEnd]-call[posFunctionStart]]==nil then
                                dict[call[posFunctionEnd]-call[posFunctionStart]]={key}
                        else
                                table.insert(dict[call[posFunctionEnd]-call[posFunctionStart]], key)
                        end
                end

                table.sort(durations)

                --Decide the number of calls to selecet
                num_offenders_to_show=math.min(num_offenders, #frame)

                if num_offenders_to_show==-1 then
                        num_offenders_to_show=#frame
                end

                worst_offenders={}
                found_offenders=0

                while found_offenders<num_offenders_to_show do  --While there are calls to select
                        off=dict[durations[#durations-found_offenders]] --Recover the information of the next worst call (or group of calls) using the dictionary
                        for k,v in pairs(off) do
                                table.insert(worst_offenders,frame[v])
                                found_offenders = found_offenders + 1
                                if found_offenders >= num_offenders_to_show then
                                        break
                                end
                        end
                end

                table.insert(worst_offenders_per_frame, worst_offenders) --Store the identified offenders
        end
--[
        for k,v in ipairs(worst_offenders_per_frame) do
                if #v==0 then
                        print('No calls for limited frame')
                else
                        print('Calls for limited frame '..k)
                        print('--------------------------------------------')
                        for k2,v2 in ipairs(v) do
                                print(v2[1],v2[2], v2[3])
                        end
                end
                print('\n\n')
        end
--]]
--        return worst_offenders_per_frame
end



--[[
--
-- This function analyses the function call trace and parameters and makes suggestions
-- regarding OpenCL compile type functions.
--
--]]
function opencl_compile_functions_analysis()

        FStart('Analysing kernel and compilation calls.')

        compile_name   = LPGPU2:getCallNameForApiAndCategory('OpenCL', 'Compile')
        compile_start  = LPGPU2:getCpuStartForApiAndCategory('OpenCL', 'Compile')
        compile_frame  = LPGPU2:getFrameNumForApiAndCategory('OpenCL', 'Compile')
        compile_draw   = LPGPU2:getDrawNumForApiAndCategory('OpenCL', 'Compile')
        compile_inputs = LPGPU2:getBinaryParamsForApiAndCategory('OpenCL', 'Compile')
        num_calls = #compile_name

        kernels_per_program={}

        work_enqueues = LPGPU2:getCpuEndForApiAndCategory('OpenCL', 'Compute')

        LPGPU2:beginTransaction()
        if work_enqueues ~= nil then

                for i=1, num_calls do
                        FProgress(i,num_calls)
                        function_name = compile_name[i]
                        call_start = compile_start[i]
                        if function_name == 'clCreateProgramWithSource' then
                                msg='Using clCreateProgramWithBinary instead of clCreateProgramWithSource may improve performance.'
                                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, compile_frame[i], compile_draw[i])
                        elseif function_name == 'clCreateProgramWithBinary' then
                                --
                        elseif function_name == 'clCreateKernel' then
                                params = compile_inputs[i]
                                program = string.sub(params,1,8)
                                if kernels_per_program[program] ~= nil then
                                        kernels_per_program[program] = kernels_per_program[program]+1
                                        msg='Grouping clCreateKernel calls in a single clCreateKernelsInProgram call at the startup of the application may improve performance.'
                                        LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, compile_frame[i], compile_draw[i])
                                else
                                        kernels_per_program[program] = 1
                                end
                        end

                end
        else
                print('DB does not look like an OpenCL execution.')
        end
        LPGPU2:endTransaction()

        FEnd()
end

--[[
--
-- This function analyses the function call trace and parameters and makes suggestions
-- regarding OpenCL functions regarding to data allocation and transfer.
--
--]]

function opencl_data_functions_analysis()

        FStart('Analyzing memory related calls.')

        alignment = 16

        mem_alloc_name   = LPGPU2:getCallNameFromOpenCLMemAllocationAndSetupCalls()
        mem_alloc_frame  = LPGPU2:getFrameNumFromOpenCLMemAllocationAndSetupCalls()
        mem_alloc_draw   = LPGPU2:getDrawNumFromOpenCLMemAllocationAndSetupCalls()
        mem_alloc_inputs = LPGPU2:getBinaryParamsFromOpenCLMemAllocationAndSetupCalls()
        mem_alloc_return = LPGPU2:getBinaryReturnFromOpenCLMemAllocationAndSetupCalls()
        num_alloc_calls  = #mem_alloc_name

        data_transfer_name   = LPGPU2:getCallNameFromOpenCLDataTransferAndNDRangeCalls()
        data_transfer_frame  = LPGPU2:getFrameNumFromOpenCLDataTransferAndNDRangeCalls()
        data_transfer_draw   = LPGPU2:getDrawNumFromOpenCLDataTransferAndNDRangeCalls()
        data_transfer_params = LPGPU2:getBinaryParamsFromOpenCLDataTransferAndNDRangeCalls()
        num_transfer_calls   = #data_transfer_name

        creation_functions={'clCreateBuffer', 'clCreateImage', 'clCreatePipe', 'clSVMAlloc'}

        --This table contains mem_objects and the number of times the have been used between kernels
        mem_objects_to_map={}
        mem_objects_to_read_write={}

        LPGPU2:beginTransaction()

        FSection("1/2")
        for i=1, num_alloc_calls do
                FProgress(i,num_alloc_calls)
                function_name = mem_alloc_name[i]
                for key, creation_name in pairs(creation_functions) do
                        if function_name == creation_name then
                                mem_object = mem_alloc_return[i]
                                if function_name == 'clCreatePipe' or function_name=='clSVMAlloc' then
                                        mem_objects_to_read_write[mem_object] = 0
                                        msg='SVM and Pipe memory objects provide greater programmability but usually at the expense of performance. Consider switching to Buffers or Images'
                                        LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, mem_alloc_frame[i], mem_alloc_draw[i])
                                else
                                        inputs = mem_alloc_inputs[i]

                                        -- clCreateImage / clCreateBuffer(  cl_context context, cl_mem_flags flags, ...

                                        -- get bytes 9 & 10 (starting at one)
                                        flags=inputs:sub(9,10)

                                        --For flags: 9th and 10th hex bits
                                        --      01 -> R_W
                                        --      02 -> W
                                        --      04 -> R
                                        --      08 -> MemUse
                                        --      10 -> MemAlloc (We want this one)
                                        --      20 -> MemCopy

                                        if flags == '10' then
                                                --When possible, add size for Images
                                                mem_objects_to_map[mem_object]=0 --Must check that mem_object does not vary
                                        else
                                                mem_objects_to_read_write[mem_object] = 0
                                                msg='Consider using CL_MEM_ALLOC_HOST_PTR flag and map operations for memory objects, so copies to the GPU memory space are avoided.'
                                                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, mem_alloc_frame[i], mem_alloc_draw[i])
                                        end

                                        size = '0x'..inputs:sub(31,32)..inputs:sub(29,30)..inputs:sub(27,28)..inputs:sub(25,26)
                                        size = tonumber(size)

                                        if size % alignment ~= 0 then
                                                suggest_alignment = true
                                                msg='Your data does not seem to be adequately aligned. Data alignment on '..alignment..' byte boundaries is recommended for best performance. Consider using padding to align your data.'
                                                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, mem_alloc_frame[i], mem_alloc_draw[i])
                                        end

                             --           break
                                end
                        end
                end
        end


        candidates_for_non_blocking={'clEnqueueWriteBuffer','clEnqueueWriteBufferRect',  'clEnqueueMapBuffer', 'clEnqueueSVMMemcpy', 'clEnqueueSVMMap', 'clEnqueueReadImage', 'clEnqueueWriteImage', 'clEnqueueMapImage', 'clEnqueueReadBuffer', 'clEnqueuereadBufferRect'}

        --This tables contain mem_objects and the number of times they have been used between kernels

        FSection("2/2")
        for i=1, num_transfer_calls do
                FProgress(i,num_transfer_calls)
                function_name = data_transfer_name[i]

                if function_name == 'clEnqueueNDRangeKernel' then
                msg='Several memory transfer operations using the same object where detected between kernel enqueues. Less transfers of more data would improve performance.'
                        for b,n in pairs(mem_objects_to_map) do
                                if n>1 then
                                    LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, data_transfer_frame[i], data_transfer_draw[i])
                                end
                                mem_objects_to_map[b] = 0
                        end

                        for b,n in pairs(mem_objects_to_read_write) do --Does it make sense for a buffer to get read in written in a single frame??
                                if n > 1 then
                                    LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, data_transfer_frame[i], data_transfer_draw[i])
                                end
                                mem_objects_to_read_write[b] = 0
                        end
                else --Analyse mem_object accessed and update list
                        pos_buffer=2
                        if function_name=='clEnqueueSVMMemcpy' or function_name=="clEnqueueCopyBuffer" or function_name=="clEnqueueCopyBufferRect" or function_name=="clEnqueueCopyImage" then
                                pos_buffer = 3
                        end

                        start_buffer = ((pos_buffer-1) * 8) + 1
                        end_buffer = start_buffer + 7

                        inputs = data_transfer_params[i]

                        mem_object = inputs:sub(start_buffer, end_buffer)

                        if mem_objects_to_read_write[mem_object] ~= nil then
                                mem_objects_to_read_write[mem_object] = mem_objects_to_read_write[mem_object]+1
                        elseif mem_objects_to_map[mem_object] ~= nil then
                                mem_objects_to_map[mem_object] = mem_objects_to_map[mem_object]+1
                        end

                        for key, blocking_name in pairs(candidates_for_non_blocking) do
                                if function_name == blocking_name then
                                        if function_name == 'clEnqueueReadImage' then
                                            msg='You are using an image memory object to store GPU outputs. Buffers usually deliver better performance for this.'
                                            LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, data_transfer_frame[i], data_transfer_draw[i])
                                        elseif function_name == 'clEnqueueWriteBuffer' then
                                            msg='You are using a buffer memory object to store GPU inputs. Images usually deliver better performance for this.'
                                            LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, data_transfer_frame[i], data_transfer_draw[i])
                                        end


                                        pos_flag_blocking=3
                                        if function_name=='clEnqueueSVMMemcpy' or function_name=='clEnqueueSVMMap' then
                                                pos_flag_blocking=2
                                        end

                                        first_bit_to_check = ((pos_flag_blocking-1) * 8) + 1

                                        --For blocking, either 9th 10th or 17th and 18th hex bits, depending on the function call
                                        --      01 -> blocking
                                        --      00 -> non-blocking (We want this one)

                                        blocking = inputs:sub(first_bit_to_check, first_bit_to_check + 1)


                                        if blocking == '01' then
                                            msg='Consider switching to non-blocking operations controlled by events to be able to better overlap CPU and GPU computation.'
                                            LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, data_transfer_frame[i], data_transfer_draw[i])
                                        end


                                        if function_name=='clEnqueueWriteImage' or function_name=='clEnqueueReadImage'or function_name=='clEnqueueReadImage'or function_name=='clEnqueueWriteImage' then
                                                used_buffer=inputs:sub(9,16) --The buffer is in the 2nd param, 9th to 16th hex bits
                                                for key, buf in pairs(mem_objects_to_map[1]) do
                                                        if buf==used_buffer then
                                                            msg='You are performing Read or Write operations on a memory object that could be mapped, avoiding a potentially unnecessary copy. This copy is correct if you have taken a double buffering approach, but would have a negative impact on performance otherwise'
                                                            LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, data_transfer_frame[i], data_transfer_draw[i])
                                                                break
                                                        end
                                                end
                                        end

                                        break
                                end
                        end
                end
        end

        LPGPU2:endTransaction()

        FEnd()

end


--[[
--
-- This function parses the OpenCL kernels contained in the database and generates
-- suggestions to improve the code.
--
--]]
function opencl_kernel_analysis_original() -- Unused - for reference.

        FStart('Analysing OpenCL kernel code')

        rex=require('librex_pcre')

        kernel_info      = LPGPU2:getShaders()
        kernel_draw_num  = LPGPU2:getDrawNumFromShader()
        kernel_frame_num = LPGPU2:getFrameNumFromShader()
        num_kernel_files =# kernel_info

        vec_candidates={'char', 'uchar', 'short', 'ushort', 'int', 'uint', 'long', 'ulong', 'float', 'double', 'half'}

        types={'bool', 'size_t', 'ptrdiff_t', 'intptr_t', 'uintptr_t', 'void', 'char', 'uchar', 'short', 'ushort',
                'int', 'uint', 'long', 'ulong', 'float', 'double', 'half'}

        suggestions={}

        for i=1, num_kernel_files do
                FProgress(i,num_kernel_files)
                code = kernel_info[i]

                code = string.gsub(code, '(//[^\n]*)','') --Eliminate line comments
                code = string.gsub(code, '/%*.-%*/', '') --Eliminate multi-line comments

                print('kernel code:')
                print(code)

                kernels_in_file=rex.split(code, '(__kernel|__function)')

                for kernel in kernels_in_file do --For each kernel and function extracted from the code

                        print(kernel)
                        print('\n')

                        suggest_vector              = true
                        suggest_avoid_size_t        = false
                        suggest_avoid_casting_float = false
                        suggest_vload               = false

                        kernel_name = string.match(kernel, '%s+%w+%s+(%w+)') --Extract name from kernel header

                        if kernel_name ~= nil then
                                print('kernel name = '..kernel_name)

                                var_type = {}   --Table indexed by variable name, contains the type of the variable
                                var_depends = {}--Table indexed by variable name, contains the vars each variable depends on
                                arg_table = {}  --Contains the names of the variables that are args

                                deps_stack = {} --Holds dependencies due to { } blocks each block is separated by a ','

                                for k,t in pairs(vec_candidates) do
                                        if string.match(kernel,t..'%d ') ~= nil then
                                                suggest_vector = false
                                        end
                                end

                                if not suggest_vector then
                                        if string.match(kernel,'%svload%d+') == nil then --Check vload
                                                suggest_vload = true
                                        end
                                end

                                if string.match(kernel, '%s*size_t%s*') ~= nil then --Detect use of size_t
                                        suggest_avoid_size_t = true
                                end

                                divs_2=string.gmatch(kernel, '/%s*(%d+)') --Detect division by 2
                                for d in divs_2 do
                                        if tonumber(d)%2 == 0 then
                                                -- TODO This could be updated to point out the line number, and contents.
                                                suggest_shifts_for_divs = true
                                                break
                                        end
                                end

                                print('Get Args *******')

                                -- TODO Remove trailing spaces, they somehow break the "split".
                                args = string.match(kernel, kernel_name..'%s*%((.-)%)')


                                print('args :'..args..'.')
                                for a in rex.split(args, ',') do --Populate arg table
                                        tokens = {}
                                        for w in rex.split(a, ' ') do
                                                table.insert(tokens, w)
                                        end
                                        
                                        arg = tokens[#tokens]
                                        print('arg Before: '..arg)
                                        
                                        arg = string.match(tokens[#tokens], '([%w_]+)' )
                                        arg_type = ''

                                        check_unsigned = 2
                                        if tokens[#tokens-1] ~= '*' then
                                                arg_type = tokens[#tokens-1]
                                        else
                                                arg_type = tokens[#tokens-2]..tokens[#tokens-1]
                                                check_unsigned = 3
                                        end

                                        if #tokens >= check_unsigned and tokens[#tokens-check_unsigned] == 'unsigned'then
                                                arg_type = 'unsigned '..arg_type
                                        end

                                        table.insert(arg_table, arg)
                                        var_type[arg] = arg_type
                                        var_depends[arg] = {}
                                        print('arg: '..arg)
                                end
                                print('Got Args ******* \n')
                                
                                kernel = string.match(kernel, '.-{(.*)}')

                                print('kernel for args: ')
                                print(kernel)

                                for k,v in pairs(types) do --Populate var table

                                        for declarations in string.gmatch(kernel, '%s*('..v..'%d*%s+.-);') do
                                        -- for declarations in string.gmatch(kernel, '%s*('..v..'%d*%.*);') do
                                                print('declarations: '..declarations)

                                                -- t,var_names = string.match(declarations, '(%w*)%s+(.*)')
                                                t,var_names = string.match(declarations, '(%w*_?t?)%s+([%w_]*)')


                                                print('type: '..t)
                                                print('var_names: '..var_names..'\n')


                                                for var in rex.split(var_names, ',') do
                                                        without_initialisation = rex.split(var, '=')
                                                        var = without_initialisation()
                                                        this_t = t
                                                        if string.find(var, '%*') then
                                                                this_t = this_t..'*'
                                                                var = string.match(var, '^%s*%*%s+(.-)%s*$')
                                                        else
                                                                var = string.match(var, '^%s*(.-)%s*$')
                                                        end
                                                        var_type[var] = this_t
                                                        var_depends[var] = {}

                                                end
                                        end

                                end

                                string.gmatch(kernel, '%(%s*int%s*%)(.*);')
                                for cast in string.gmatch(kernel, '%(%s*int%s*%)(.-);') do --Check_casts
                                        cast = string.gsub(cast, '[(),%+%-%*/%%&|><=.]',' ') --Eliminate symbols
                                        cast = string.gsub(cast, '%[%d+%]',' ') --Eliminate positional access
                                        operands = string.gmatch(cast, '%S+')

                                        for op in operands do
                                                op_type=var_type[op]
                                                if op_type == 'float' or op_type=='double' or op_type == 'float*' or op_type == 'double*' then
                                                        suggest_avoid_casting_float = true
                                                end
                                        end
                                end

                                --Obtain dependencies
                                lines=string.gmatch(kernel, '(.-[;}{])[\n]?')
                                for line in lines do
                                        if string.match(line, '.*}.*') ~= nil then --Pop deps from stack if }
                                                removed=''
                                                repeat
                                                        removed = table.remove(deps_stack)
                                                until(removed == ',' or removed == nil)
                                        end

                                        conds=string.match(line,'if%s*%((.-)%)') --Is it an if?
                                        if conds == nil then

                                                conds = string.match(line,'while%s*%((.-)%)')
                                                if conds == nil then --Is it a while

                                                        conds = string.match(line,'for%s*%((.*)') --Is it a for?
                                                        if conds ~= nil then
                                                                conds = conds..lines()..lines()  --Recover all the conds as they were broken by ;
                                                        end
                                                end
                                        end

                                        if conds ~= nil then --If it is a while, for or if

                                                conds = string.gsub(conds, '[(),%+%-%*/%%&|><=%.%[%]{};\n]',' ')--Clean
                                                table.insert(deps_stack, ',')
                                                for d in string.gmatch(conds, '%S+') do
                                                        if var_type[d] ~= nil or d == 'get_local_id' or d == 'get_global_id' or tonumber(d) ~= nil then
                                                                table.insert(deps_stack, d) --Add deps to stack
                                                        end
                                                end

                                        elseif string.match(line, '[^%w_]*vstore%d+%(.-%)')~=nil then --Detect dependencies for vstore
                                                vstore_args = string.match(line, '[^%w_]*vstore%d+%((.-)%)')
                                                tokens = string.gmatch(vstore_args, '[^,]+')
                                                tokens_list = {}
                                                for t in tokens do
                                                        table.insert(tokens_list,t)
                                                end
                                                data = tokens_list[1]
                                                data = string.gsub(data, '%[.-%]', '')
                                                data = string.gsub(data, '[(),%+%-%*/%%&|><=.%[%]]', '')
                                                dest = tokens_list[3]
                                                for d in string.gmatch(data, '%S+') do
                                                        var_depends[dest][d] = true
                                                end

                                        else

                                                print('line : '..line)
                                                var,deps=string.match(line, '%s*(.-=)([^=].-)[,;]')
                                                if var ~= nil and deps ~= nil then --For assignations

                                                        print('Before. var: '..var..' deps: '..deps)

                                                        array = string.gmatch(var, '%[(.-)]') --If it is an array assignation
                                                        for a in array do
                                                                deps=deps..'+'..a --Add a dummy dependence with the index
                                                        end
                                                        var = string.gsub(var, '%[.-%]', '') --Clean
                                                        print('After Clean. var: '..var)

                                                        -- This could be '<type> <var>[blah] = ...' or '<var>[blah] = ...'

                                                        var = string.match(var, '%.*([%w_]*)%x?%.*%s*=') --Get just var written name


                                                        deps = string.gsub(deps, '[(),%+%-%*/%%&|><=.%[%]]',' ') --Eliminate symbols

                                                        print('After. var: '..var..' deps: '..deps..'\n')

                                                        for d in string.gmatch(deps, '%S+') do  --For each token of the write
                                                                if var_type[d] ~= nil or d == 'get_local_id' or d == 'get_global_id' or tonumber(d) ~= nil then

                                                                        print('var: '..var..' depends?: '..d..' var_depends[var]: '..type(var_depends[var]))

                                                                        var_depends[var][d] = true        --Add the dependence
                                                                        if var_depends[d] ~=nil then
                                                                               for v,_ in pairs(var_depends[d]) do
                                                                                    var_depends[var][v] = true --Add the dependences of the dependence
                                                                               end
                                                                        end
                                                                end
                                                        end

                                                        for _,d in ipairs(deps_stack) do --For each dependence in the stack
                                                                if var_type[d] ~= nil or d == 'get_local_id' or d == 'get_global_id' or d == 'get_global_offset' or tonumber(d)~=nil then
                                                                        var_depends[var][d] = true --Add the dependence
                                                                        if var_depends[d] ~= nil then
                                                                               for v,_ in pairs(var_depends[d]) do
                                                                                    var_depends[var][v] = true    --Add the dependences of the dependence
                                                                               end
                                                                        end
                                                                end
                                                        end
                                                end
                                        end
                                end


                                for var,deps in pairs(var_depends) do --Analyse dependencies
                                        tid_found = false
                                        num = 0
                                        for dep,_ in pairs(deps) do
                                                num = num + 1
                                                if dep == 'get_global_id' or dep == 'get_global_id' or dep == 'get_global_offset' then
                                                        tid_found = true
                                                end
                                        end
                                        if not tid_found and num > 2 then
                                                msg = kernel_name..': Variable "'..var..'" seems to hold a thread independent value that could be pre-computed by the host and passed as an argument.'
                                                table.insert(suggestions, msg)
                                        end
                                end

                                if suggest_vector then
                                        msg = kernel_name..': Consider changing your algorithm to use vector types to improve performance.'
                                        table.insert(suggestions, {msg, kernel_frame_num[i], kernel_draw_num[i]})
                                end

                                if suggest_avoid_size_t then
                                        msg = kernel_name..': Type size_t should be avoided if possible as it may result in a 64 bit type. Consider using int instead.'
                                        table.insert(suggestions, {msg, kernel_frame_num[i], kernel_draw_num[i]})
                                end

                                if suggest_avoid_casting_float then
                                        msg = kernel_name..': Casting from float or double to int is expensive and should be avoided if possible.'
                                        table.insert(suggestions, {msg, kernel_frame_num[i], kernel_draw_num[i]})
                                end

                                if suggest_vload then
                                        msg = kernel_name..': Consider using vload and vstore to manage your vector data.'
                                        table.insert(suggestions, {msg, kernel_frame_num[i], kernel_draw_num[i]})
                                end

                                if suggest_shifts_for_divs then
                                        msg = kernel_name..': Divisions are expensive. Performance may improve if divisions by powers of two are replaced with bit shifts (Care should be taken for the rounding of negative numbers).'
                                        table.insert(suggestions, {msg, kernel_frame_num[i], kernel_draw_num[i]})
                                end
                        end
                end
        end


        LPGPU2:beginTransaction()
        for k, s in pairs(suggestions) do
                print(s)
                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, s[1], s[2], s[3]) -- (message, frame, draw)
        end
        LPGPU2:endTransaction()

        FEnd()

end

--[[
--
-- This function parses the OpenCL kernels contained in the database and generates
-- suggestions to improve the code.
--
--]]
function opencl_kernel_analysis()

        FStart('Analysing OpenCL kernel code')

        rex=require('librex_pcre')

        kernel_info      = LPGPU2:getShaders()
        kernel_draw_num  = LPGPU2:getDrawNumFromShader()
        kernel_frame_num = LPGPU2:getFrameNumFromShader()
        num_kernel_files = #kernel_info

        vec_candidates={'char', 'uchar', 'short', 'ushort', 'int', 'uint', 'long', 'ulong', 'float', 'double', 'half'}

        types={'bool', 'size_t', 'ptrdiff_t', 'intptr_t', 'uintptr_t', 'void', 'char', 'uchar', 'short', 'ushort',
                'int', 'uint', 'long', 'ulong', 'float', 'double', 'half'}

        suggestions={}

        for i=1, num_kernel_files do
                FProgress(i,num_kernel_files)

                code = kernel_info[i]

                code = string.gsub(code, '(//[^\n]*)','') --Eliminate line comments
                code = string.gsub(code, '/%*.-%*/', '') --Eliminate multi-line comments
                -- TODO - pre-processor. Remove #if 0, replace #define etc

            --    print('kernel code:')
            --    print(code)

                kernels_in_file=rex.split(code, '(__kernel|__function)')

                for kernel in kernels_in_file do --For each kernel and function extracted from the code

                    --    print(kernel)

                        suggest_vector = true
                        suggest_avoid_size_t = false
                        suggest_vload = false
                        suggest_shifts_for_divs = false
                        suggest_shifts_for_double = false -- TODO
                        suggest_thread_sage = false -- TODO clSetKernelArg

                        kernel_name = string.match(kernel, '%s+%w+%s+(%w+)') --Extract name from kernel header

                        if kernel_name ~= nil then
                                print('kernel name = '..kernel_name)

                                for k,t in pairs(vec_candidates) do
                                        if string.match(kernel,t..'%d ') ~= nil then
                                                suggest_vector = false
                                        end
                                end

                                if not suggest_vector then
                                        if string.match(kernel,'%svload%d+') == nil then --Check vload
                                                suggest_vload = true
                                        end
                                end

                                if string.match(kernel, '%s*size_t%s*') ~= nil then --Detect use of size_t
                                        suggest_avoid_size_t = true
                                end

                                divs_2=string.gmatch(kernel, '/%s*(%d+)') --Detect division by 2

                                for d in divs_2 do
                                
                                        if tonumber(d)%2 == 0 then
                                                -- TODO - Power of 2, NOT just a multiple
                                                -- TODO This could be updated to point out the line number, and contents.
                                                suggest_shifts_for_divs = true
                                                break
                                        end
                                end

                                if suggest_vector then
                                        msg = kernel_name..': Consider changing your algorithm to use vector types to improve performance.'
                                        table.insert(suggestions, {msg, kernel_frame_num[i], kernel_draw_num[i]})
                                end

                                if suggest_avoid_size_t then
                                        msg = kernel_name..': Type size_t should be avoided if possible as it may result in a 64 bit type. Consider using int instead.'
                                        table.insert(suggestions, {msg, kernel_frame_num[i], kernel_draw_num[i]})
                                end

                                if suggest_vload then
                                        msg = kernel_name..': Consider using vload and vstore to manage your vector data.'
                                        table.insert(suggestions, {msg, kernel_frame_num[i], kernel_draw_num[i]})
                                end

                                if suggest_shifts_for_divs then
                                        msg = kernel_name..': Divisions are expensive. Performance may improve if divisions by powers of two are replaced with bit shifts (Care should be taken for the rounding of negative numbers).'
                                        table.insert(suggestions, {msg, kernel_frame_num[i], kernel_draw_num[i]})
                                end
                        end
                end
        end


        LPGPU2:beginTransaction()
        for k, s in pairs(suggestions) do
                print(s[1])
                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, s[1], s[2], s[3]) -- (message, frame, draw)
        end
        LPGPU2:endTransaction()

        FEnd()
end

--[[
--
-- This function parses the OpenCL kernels contained in the database and generates
-- suggestions to improve the code.
--
--]]
function opencl_kernel_analysis_two()

        FStart('Analysing OpenCL kernel code')

        rex=require('librex_pcre')

        kernel_info      = LPGPU2:getShaders()
        kernel_draw_num  = LPGPU2:getDrawNumFromShader()
        kernel_frame_num = LPGPU2:getFrameNumFromShader()
        num_kernel_files = #kernel_info

        vec_candidates={'char', 'uchar', 'short', 'ushort', 'int', 'uint', 'long', 'ulong', 'float', 'double', 'half'}

        types={'bool', 'size_t', 'ptrdiff_t', 'intptr_t', 'uintptr_t', 'void', 'char', 'uchar', 'short', 'ushort',
                'int', 'uint', 'long', 'ulong', 'float', 'double', 'half'}

        suggestions={}

        for i=1, num_kernel_files do
                FProgress(i,num_kernel_files)

                code = kernel_info[i]

                code = string.gsub(code, '(//[^\n]*)','') --Eliminate line comments
                code = string.gsub(code, '/%*.-%*/', '') --Eliminate multi-line comments

             --   print('kernel code:')
             --   print(code)

                kernels_in_file=rex.split(code, '(__kernel|__function)')

                for kernel in kernels_in_file do --For each kernel and function extracted from the code

                   --     print(kernel)
                   --     print('\n')

                        suggest_avoid_casting_float = false

                        kernel_name = string.match(kernel, '%s+%w+%s+(%w+)') --Extract name from kernel header

                        if kernel_name ~= nil then
                                print('kernel name = '..kernel_name)

                                var_type = {}   --Table indexed by variable name, contains the type of the variable
                                var_depends = {}--Table indexed by variable name, contains the vars each variable depends on
                                arg_table = {}  --Contains the names of the variables that are args

                                deps_stack = {} --Holds dependencies due to { } blocks each block is separated by a ','

                            --    print('Get Args *******')

                                -- TODO Remove trailing spaces, they somehow break the "split".
                                args = string.match(kernel, kernel_name..'%s*%((.-)%)')


                            --    print('args :'..args..'.')
                                for a in rex.split(args, ',') do --Populate arg table
                                        tokens = {}
                                        for w in rex.split(a, ' ') do
                                                table.insert(tokens, w)
                                        end
                                        
                                        arg = tokens[#tokens]
                                  --      print('arg Before: '..arg)
                                        
                                        arg = string.match(tokens[#tokens], '([%w_]+)' )
                                        arg_type = ''

                                        check_unsigned = 2
                                        if tokens[#tokens-1] ~= '*' then
                                                arg_type = tokens[#tokens-1]
                                        else
                                                arg_type = tokens[#tokens-2]..tokens[#tokens-1]
                                                check_unsigned = 3
                                        end

                                        if #tokens >= check_unsigned and tokens[#tokens-check_unsigned] == 'unsigned'then
                                                arg_type = 'unsigned '..arg_type
                                        end

                                        table.insert(arg_table, arg)
                                        var_type[arg] = arg_type
                                        var_depends[arg] = {}
                                  --      print('arg: '..arg)
                                end
                            --    print('Got Args ******* \n')
                                
                                kernel = string.match(kernel, '.-{(.*)}')

                              --  print('kernel for args: ')
                              --  print(kernel)

                                for k,v in pairs(types) do --Populate var table

                                        for declarations in string.gmatch(kernel, '%s*('..v..'%d*%s+.-);') do
                                        -- for declarations in string.gmatch(kernel, '%s*('..v..'%d*%.*);') do
                                            --    print('declarations: '..declarations)

                                                -- t,var_names = string.match(declarations, '(%w*)%s+(.*)')
                                                t,var_names = string.match(declarations, '(%w*_?t?)%s+([%w_]*)')


                                           -    print('type: '..t)
                                           -    print('var_names: '..var_names..'\n')


                                                for var in rex.split(var_names, ',') do
                                                        without_initialisation = rex.split(var, '=')
                                                        var = without_initialisation()
                                                        this_t = t
                                                        if string.find(var, '%*') then
                                                                this_t = this_t..'*'
                                                                var = string.match(var, '^%s*%*%s+(.-)%s*$')
                                                        else
                                                                var = string.match(var, '^%s*(.-)%s*$')
                                                        end
                                                        var_type[var] = this_t
                                                        var_depends[var] = {}

                                                end
                                        end

                                end

                                string.gmatch(kernel, '%(%s*int%s*%)(.*);')
                                for cast in string.gmatch(kernel, '%(%s*int%s*%)(.-);') do --Check_casts
                                        cast = string.gsub(cast, '[(),%+%-%*/%%&|><=.]',' ') --Eliminate symbols
                                        cast = string.gsub(cast, '%[%d+%]',' ') --Eliminate positional access
                                        operands = string.gmatch(cast, '%S+')

                                        for op in operands do
                                                op_type=var_type[op]
                                                if op_type == 'float' or op_type=='double' or op_type == 'float*' or op_type == 'double*' then
                                                        suggest_avoid_casting_float = true
                                                end
                                        end
                                end

                                --Obtain dependencies
                                lines=string.gmatch(kernel, '(.-[;}{])[\n]?')
                                for line in lines do
                                        if string.match(line, '.*}.*') ~= nil then --Pop deps from stack if }
                                                removed=''
                                                repeat
                                                        removed = table.remove(deps_stack)
                                                until(removed == ',' or removed == nil)
                                        end

                                        conds=string.match(line,'if%s*%((.-)%)') --Is it an if?
                                        if conds == nil then

                                                conds = string.match(line,'while%s*%((.-)%)')
                                                if conds == nil then --Is it a while

                                                        conds = string.match(line,'for%s*%((.*)') --Is it a for?
                                                        if conds ~= nil then
                                                                conds = conds..lines()..lines()  --Recover all the conds as they were broken by ;
                                                        end
                                                end
                                        end

                                        if conds ~= nil then --If it is a while, for or if

                                                conds = string.gsub(conds, '[(),%+%-%*/%%&|><=%.%[%]{};\n]',' ')--Clean
                                                table.insert(deps_stack, ',')
                                                for d in string.gmatch(conds, '%S+') do
                                                        if var_type[d] ~= nil or d == 'get_local_id' or d == 'get_global_id' or tonumber(d) ~= nil then
                                                                table.insert(deps_stack, d) --Add deps to stack
                                                        end
                                                end

                                        elseif string.match(line, '[^%w_]*vstore%d+%(.-%)')~=nil then --Detect dependencies for vstore
                                                vstore_args = string.match(line, '[^%w_]*vstore%d+%((.-)%)')
                                                tokens = string.gmatch(vstore_args, '[^,]+')
                                                tokens_list = {}
                                                for t in tokens do
                                                        table.insert(tokens_list,t)
                                                end
                                                data = tokens_list[1]
                                                data = string.gsub(data, '%[.-%]', '')
                                                data = string.gsub(data, '[(),%+%-%*/%%&|><=.%[%]]', '')
                                                dest = tokens_list[3]
                                                for d in string.gmatch(data, '%S+') do
                                                        var_depends[dest][d] = true
                                                end

                                        else

                                           --     print('line : '..line)
                                                var,deps=string.match(line, '%s*(.-=)([^=].-)[,;]')
                                                if var ~= nil and deps ~= nil then --For assignations

                                               --        print('Before. var: '..var..' deps: '..deps)

                                                        array = string.gmatch(var, '%[(.-)]') --If it is an array assignation
                                                        for a in array do
                                                                deps=deps..'+'..a --Add a dummy dependence with the index
                                                        end
                                                        var = string.gsub(var, '%[.-%]', '') --Clean
                                                  --      print('After Clean. var: '..var)

                                                        -- This could be '<type> <var>[blah] = ...' or '<var>[blah] = ...'

                                                        var = string.match(var, '%.*([%w_]*)%x?%.*%s*=') --Get just var written name


                                                        deps = string.gsub(deps, '[(),%+%-%*/%%&|><=.%[%]]',' ') --Eliminate symbols

                                                  --      print('After. var: '..var..' deps: '..deps..'\n')

                                                        for d in string.gmatch(deps, '%S+') do  --For each token of the write
                                                                if var_type[d] ~= nil or d == 'get_local_id' or d == 'get_global_id' or tonumber(d) ~= nil then

                                                                     --   print('var: '..var..' depends?: '..d..' var_depends[var]: '..type(var_depends[var]))

                                                                        var_depends[var][d] = true        --Add the dependence
                                                                        if var_depends[d] ~=nil then
                                                                               for v,_ in pairs(var_depends[d]) do
                                                                                    var_depends[var][v] = true --Add the dependences of the dependence
                                                                               end
                                                                        end
                                                                end
                                                        end

                                                        for _,d in ipairs(deps_stack) do --For each dependence in the stack
                                                                if var_type[d] ~= nil or d == 'get_local_id' or d == 'get_global_id' or d == 'get_global_offset' or tonumber(d)~=nil then
                                                                        var_depends[var][d] = true --Add the dependence
                                                                        if var_depends[d] ~= nil then
                                                                               for v,_ in pairs(var_depends[d]) do
                                                                                    var_depends[var][v] = true    --Add the dependences of the dependence
                                                                               end
                                                                        end
                                                                end
                                                        end
                                                end
                                        end
                                end


                                for var,deps in pairs(var_depends) do --Analyse dependencies
                                        tid_found = false
                                        num = 0
                                        for dep,_ in pairs(deps) do
                                                num = num + 1
                                                if dep == 'get_global_id' or dep == 'get_global_id' or dep == 'get_global_offset' then
                                                        tid_found = true
                                                end
                                        end
                                        if not tid_found and num > 2 then
                                                msg = kernel_name..': Variable "'..var..'" seems to hold a thread independent value that could be pre-computed by the host and passed as an argument.'
                                                table.insert(suggestions, msg)
                                        end
                                end

                                if suggest_avoid_casting_float then
                                        msg = kernel_name..': Casting from float or double to int is expensive and should be avoided if possible.'
                                        table.insert(suggestions, {msg, kernel_frame_num[i], kernel_draw_num[i]})
                                end
                        end
                end
        end


        LPGPU2:beginTransaction()
        for k, s in pairs(suggestions) do
                print(s)
                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, s[1], s[2], s[3]) -- (message, frame, draw)
        end
        LPGPU2:endTransaction()

        FEnd()
end

--[[
--
-- This function analyses the function call trace and parameters and makes suggestions
-- regarding OpenCL functions related to the launch of work.
--
--]]
function opencl_work_functions_analysis()

        FStart("OpenCL Work Functions")

        work_calls  = LPGPU2:getCallNameForApiAndCategory('OpenCL', 'Compute')
        work_frame  = LPGPU2:getFrameNumForApiAndCategory('OpenCL', 'Compute')
        work_draw   = LPGPU2:getDrawNumForApiAndCategory('OpenCL', 'Compute')
        work_params = LPGPU2:getBinaryParamsForApiAndCategory('OpenCL', 'Compute')
        num_calls   = #work_calls

        suggest_local_work_size = '0'
        suggest_callbacks = false

        LPGPU2:beginTransaction()
        for i=0, num_calls-1 do
                FProgress(i,num_calls)
                function_name=work_calls[i]
                if function_name=='clEnqueueNDRangeKernel' then
                        parameters=work_params[i]

                        --make sense of the blob!
                        --
                        --num_dimensions is in hex bit 18
                        --local_work_size is in hex bits 41-48
                        local_w=parameters:sub(41,48)
                        if local_w == '00000000' then --If it is null (Default local_work_size selected by driver)
                                num_dims = parameters:sub(18,18)
                                if num_dims == '1' then
                                        suggest_local_work_size = '128'
                                elseif num_dims == '2' then
                                        suggest_local_work_size = '16x8'
                                else
                                        suggest_local_work_size = '8x8x4'
                                end
                                msg='You are using the default local_work_size, which is chosen by the driver and may not be optimal. A value of '..suggest_local_work_size..' is usually reasonable, however profiling would be advisable to identify the optimal size (and shape).'
                                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg, work_frame[i], work_draw[i])
                        end

                        suggest_binary = true
                end
        end
        LPGPU2:endTransaction()

        FEnd()
end

function opengl_buffer_functions_analysis()
        FStart("opengl_buffer_functions_analysis")

        alignment = 8

        buffer_calls  = LPGPU2:getCallNameFromOpenGLBufferAndDrawCalls()
        buffer_params = LPGPU2_getBinaryParamsFromOpenGLBufferAndDrawCalls()
        num_buffer_calls = #buffer_calls

        suggest_subdata_to_update = false
        suggest_group_updates = false
        suggest_alignment = false

        --this table is indexed by buffer id and contains the size of the buffer
        found_buffers_size={}
        found_buffers_updates_between_draws={}

        --This table is indexed by target. Each entry contains, a buffer identifier and its size
        current_buffers={}

        LPGPU2:beginTransaction()

        for i=1, num_buffer_calls do
                FProgress(i,num_buffer_calls)

                function_name = buffer_calls[i]
                inputs = buffer_params[i]
                buffer_to_update = nil
                new_allocation = false
                size = 0
                if function_name == 'glBindBuffer' then
                        target = inputs:sub(1,8)
                        buffer = inputs:sub(9,16)
                        current_buffers[target] = buffer

                elseif function_name == 'glBufferData' then
                        target = inputs:sub(1,8)
                        size_hex = '0x'..inputs:sub(15,16)..inputs:sub(13,14)..inputs:sub(11,12)..inputs:sub(9,10)
                        size = tonumber(size_hex)
                        buffer_to_update = current_buffers[target]
                        new_allocation = true

                elseif function_name == 'glNamedBufferData'then
                        buffer=inputs:sub(1,8)
                        size_hex = '0x'..inputs:sub(15,16)..inputs:sub(13,14)..inputs:sub(11,12)..inputs:sub(9,10)
                        size = tonumber(size_hex)
                        buffer_to_update = buffer
                        new_allocation = true

                elseif function_name == 'glBufferSubData' then
                        target = inputs:sub(1,8)
                        size_hex = '0x'..inputs:sub(23,24)..inputs:sub(21,22)..inputs:sub(19,20)..inputs:sub(17,18)
                        size = tonumber(size_hex)
                        buffer_to_update = current_buffers[target]

                elseif function_name == 'glNamedBufferSubData' then
                        buffer = inputs:sub(1,8)
                        size_hex = '0x'..inputs:sub(23,24)..inputs:sub(21,22)..inputs:sub(19,20)..inputs:sub(17,18)
                        size = tonumber(size_hex)
                        buffer_to_update = buffer

                else --Draw call
                        for target, buf in pairs(current_buffers) do
                                found_buffers_updates_between_draws[buf] = 0
                        end
                end

                if new_allocation then --glBufferData. New allocation
                        found_buffers_updates_between_draws[buffer_to_update]=0
                        if size == found_buffers_size[buffer_to_update] then --Same size as the previous allocation
                                suggest_subdata_to_update = true
                                msg='You are using glBufferData/glNamedBufferData to update the contents of a buffer. This function allocates new memory each time it is called. To reuse memory consider using glBufferSubData.'
                                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg)
                        end
                        --print('New allocation: Buffer '..buffer_to_update..' with size '..size)
                        found_buffers_size[buffer_to_update] = size

                        if size % alignment ~= 0 then
                                suggest_alignment = true
                                msg='Your data does not seem to be adequately aligned. Data alignment on '..alignment..' byte boundaries is recommended for best performance. Consider using padding to align your data.'
                                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg)
                        end

                elseif buffer_to_update ~= nil then --glBufferSubData. Proper update
                        found_buffers_updates_between_draws[buffer_to_update] = found_buffers_updates_between_draws[buffer_to_update] + 1
                        if found_buffers_updates_between_draws[buffer_to_update] > 1 then
                                suggest_batch_updates = true
                                msg='You are performing several updates to the same buffer using glBufferSubData/glNamedBufferSubData. Consider batching them into a single call to reduce API calls and avoid orphaning.'
                                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg)
                        end
                end
        end
        LPGPU2:endTransaction()

        FEnd()
end

--[[
--
-- This function analyses the function call trace and parameters and makes suggestions
-- regarding the OpenGL state. The development of this function stopped, but it should
-- correctly identify inadequate ordering of draws for transparent and opaque objects.
--
--]]
function opengl_state_tracker()

        FStart("opengl_state_tracker")

        posName=1
        posStart=2
        posEnd=3

        posFrameNum=2
        posDrawNum=3

        posInputs=4
        posReturn=5

        alignment=8
        state_name   = LPGPU2:getCallNameFromOpenGLStateAndFrameCalls()
        state_frame  = LPGPU2:getFrameNumFromOpenGLStateAndFrameCalls()
        state_draw   = LPGPU2:getDrawNumFromOpenGLStateAndFrameCalls()
        state_inputs = LPGPU2_getBinaryParamsFromOpenGLStateAndFrameCalls()

        num_state_calls = #state_name

        state_calls={}
        state_params={}

        FSection("1/2")
        for i=1, num_state_calls do
                FProgress(i,num_state_calls)

                table.insert(state_calls, {state_name[i], state_frame[i], state_draw[i]})
                table.insert(state_params, state_inputs[i])
        end

        any_transparent_draw=false

        --this table is indexed by the parameters of the functions that change states and just identifies the state changes that have taken place since the last draw call
        state_changes_between_frames={}

        --this table is indexed by function name or target if it is a bind function, or what has been enabled if it is glEnable, and stores the last params for the function
        current_state = {}
        current_state['glDepthMask'] = '01000000'

        --contains pairs of frameNum, drawNum for the state changes that have been found as potentially redundant
        redundant_state_changes = {}
        suggest_transparent_last = false

        FSection("2/2")
        for i=1, num_state_calls do
                FProgress(i,num_state_calls)

                function_name = state_calls[i][posName]
                inputs = state_params[i]

                if function_name:sub(1,6) == 'glDraw' then
                        if current_state['glDepthMask'] == '00000000' then
                                any_transparent_draw = true
                        elseif any_transparent_draw then
                                suggest_transparent_last = true
                        end
                elseif function_name == 'eglSwapBuffers' or function_name=='glFinish'then
                        for params, v in pairs(state_changes_between_frames) do
                                state_changes_between_frames[params] = nil
                                any_transparent_draw = false
                        end
                else --bind, enable or depthmask
                        --is there any case in which the exact same set of parameters are used in two different functions within the same frame???
                        if state_changes_between_frames[function_name..inputs] ~= nil then --and #inputs>8 then --Certain functions only take true/false
                                table.insert(redundant_state_changes, {state_calls[i][posFrameNum], state_calls[i][posDrawNum], 'Reorder calls to reuse state.'})
                                --print(state_calls[i][posFrameNum], state_calls[i][posDrawNum], function_name, inputs)
                        else
                                state_changes_between_frames[function_name..inputs] = true

                                tag=inputs:sub(1,8)
                                value=''
                                if function_name:sub(1,6) == 'glBind' then
                                        value = inputs:sub(9,#inputs)
                                elseif function_name == 'glEnable' then
                                        value='TRUE'
                                elseif function_name == 'glDisable' then
                                        value = 'FALSE'
                                else --function_name=='glDepthMask'then
                                        tag = function_name
                                        value = inputs
                                end

                                if current_state[tag] == value then
                                        table.insert(redundant_state_changes, {state_calls[i][posFrameNum], state_calls[i][posDrawNum], 'There was an attempt to set state that is already set.'})
                                else
                                        current_state[tag] = value
                                end
                        end
                end
        end


        LPGPU2:beginTransaction()

        if #redundant_state_changes > 0 then
                msg='Some state changes are potentially redundant. Refactor your code so all the work on a state is performed at once and state is not set to its current value.'

                for k,v in pairs(redundant_state_changes) do
                        frame = v[1]
                        draw = v[2]
                        type = v[3]
                        LPGPU2:writeAnnotation(LPGPU_SUGGESTION, string.format("%s %s", msg, type), frame, draw)
                end
        end

        if suggest_transparent_last then
                LPGPU2:writeAnnotation(LPGPU_ISSUE,'Draw calls for transparent elements should be made after those for opaque ones.')
        end

        LPGPU2:endTransaction()

        FEnd()
end


--[[
--
-- This function analyses the function call trace and parameters and makes suggestions
-- regarding OpenGL textures.
--
--]]
function opengl_texture_analysis()

        FStart("opengl_texture_analysis")

        posStart=2
        posEnd=3

        posReturn=5

        alignment=8

        texture_calls=LPGPU2:getCallNameForApiAndCategory('OpenGL', 'Texture')
        texture_params=LPGPU2:getBinaryParamsForApiAndCategory('OpenGL', 'Texture')
        num_texture_calls=#texture_calls

        suggest_compress=false

        compressed_formats_ranges={}
        table.insert(compressed_formats_ranges, {tonumber('0x86A3'), tonumber('0x86A3')})
        table.insert(compressed_formats_ranges, {tonumber('0x84E9'), tonumber('0x84EE')})
        table.insert(compressed_formats_ranges, {tonumber('0x8C48'), tonumber('0x8C4B')})
        table.insert(compressed_formats_ranges, {tonumber('0x8225'), tonumber('0x8226')})
        table.insert(compressed_formats_ranges, {tonumber('0x8DBB'), tonumber('0x8DBE')})
        table.insert(compressed_formats_ranges, {tonumber('0x8E8C'), tonumber('0x8E8F')})
        table.insert(compressed_formats_ranges, {tonumber('0x9270'), tonumber('0x9279')})
        table.insert(compressed_formats_ranges, {tonumber('0x93B0'), tonumber('0x93BD')})
        table.insert(compressed_formats_ranges, {tonumber('0x93D0'), tonumber('0x93DD')})
        table.insert(compressed_formats_ranges, {tonumber('0x86B0'), tonumber('0x86B1')})
        table.insert(compressed_formats_ranges, {tonumber('0x8C70'), tonumber('0x8C73')})
        table.insert(compressed_formats_ranges, {tonumber('0x83F0'), tonumber('0x83F3')})
        table.insert(compressed_formats_ranges, {tonumber('0x8C48'), tonumber('0x8C4F')})

        --this table is indexed by buffer id and contains the size of the buffer

        for i=1, num_texture_calls do
                FProgress(i,num_texture_calls)
                function_name=texture_calls[i]
                inputs = texture_params[i]
                if function_name:sub(1,10) == 'glTexImage' then
                        format=tonumber('0x'..inputs:sub(23,24)..inputs:sub(21,22)..inputs.sub(19,20)..inputs.sub(17,18))
                        compressed = false
                        for k,v in pairs(compressed_formats_ranges) do
                                if format >= v[1] and format <= v[2] then
                                        compressed=true
                                        break
                                end
                        end

                        if not compressed then
                                suggest_compress = true
                        end
                end
        end


        LPGPU2:beginTransaction()
        if suggest_compress then
                msg='You are not using compressed textures. Compression can substantially improve performance.'
                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, msg)
        end
        LPGPU2:endTransaction()

        FEnd()

end

--[[
--
-- This function analyses the function call trace and parameters and makes suggestions
-- regarding functions related to Vulkan descriptors
--
--]]
function vulkan_descriptor_functions_analysis()

        FStart("vulkan_descriptor_functions_analysis")

        posName=1
        posStart=2
        posEnd=3

        descriptor_name = LPGPU2:getCallNameFromVulkanDescriptorAndDrawCalls()
        descriptor_start = LPGPU2:getCpuStartFromVulkanDescriptorAndDrawCalls()
        num_calls = #descriptor_name

        descriptor_calls={}

        FSection("1/2")
        for i=1, num_calls do
                FProgress(i,num_calls)
                table.insert(descriptor_calls, {descriptor_name[i], descriptor_start[i]})
        end

        suggest_creation_at_startup = false
        suggest_one_per_draw = false
        suggest_update = false

        if num_calls > 0 then

                drawn=false
                descriptors_bound=0

                FSection("2/2")
                for i=1, num_calls do
                        FProgress(i,num_calls)
                        function_name = descriptor_calls[i][posName]
                        call_start = descriptor_calls[i][posStart]
                        if function_name == 'vkAllocateDescriptorSets' and drawn then
                                suggest_creation_at_startup = true
                        elseif function_name == 'vkCmdBindDescriptorSets' then
                                descriptors_bound = descriptors_bound+1
                        elseif function_name == 'vkResetDescriptorPool' then
                                suggest_update = true
                        elseif function_name:sub(1,5) == 'vkCmd' then --Draw
                                drawn = true
                                if descriptors_bound>1 then
                                        suggest_one_per_draw = true
                                end
                                descriptors_bound=0
                        end
                end

        else
                print('DB does not look like a Vulkan execution')
        end

        messages={}
        if suggest_creation_at_startup then
                msg='Consider moving descriptor set creation to the startup of the app. Descriptor set creation is costly, so it should be performed as early as possible and descriptor sets reused.'
                table.insert(messages, msg)
        end

        if suggest_one_per_draw then
                msg='For optimal performance, only one descriptor should be bound per draw call.'
                table.insert(messages, msg)
        end

        if suggest_update then
                msg='Consider updating existing descriptor sets instead of resetting the descriptor pool and allocating new ones.'
                table.insert(messages, msg)
        end

        LPGPU2:beginTransaction()
        for _,v in pairs(messages) do
                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, v)
        end
        LPGPU2:endTransaction()

        FEnd()
end

--[[
--
-- This function analyses the function call trace and parameters and makes suggestions
-- regarding miscellaneous Vulkan functions.
--
--]]
function vulkan_others()

        FStart("vulkan_others")

        posName  = 1
        posStart = 2
        posEnd   = 3

        other_name  = LPGPU2:getCallNameForApiAndCategory('Vulkan', 'Other')
        other_start = LPGPU2:getCpuStartForApiAndCategory('Vulkan', 'Other')
        num_other   = #other_name

        other_calls={}

        for i=1, num_other do
                table.insert(other_calls, {other_name[i], other_start[i]})
        end

        suggest_avoid_secondary_commands = false
        suggest_reduce_allocs = false
        suggest_avoid_resolve = false

        num_allocs = 0

        for i=1, num_other do
                FProgress(i,num_other)

                function_name = other_calls[i][posName]
                call_start = other_calls[i][posStart]

                if function_name == 'vkCmdExecuteCommands' then
                        suggest_avoid_secondary_commands = true
                elseif function_name == 'vkAllocateMemory' then
                        num_allocs = num_allocs + 1
                        if num_allocs > 5 then
                                suggest_reduce_allocs = true
                        end
                elseif function_name == 'vkCmdResolveImage' then
                        suggest_avoid_resolve = true
                end

        end

        messages={}
        if suggest_avoid_secondary_commands then
                msg='Secondary command buffers should be avoided. Some drivers use memcpy to handle them, which incurs a significant overhead. They also make it harder for the engine to optimise draw calls and minimise state changes.'
                table.insert(messages, msg)
        end

        if suggest_reduce_allocs then
                msg='vkAllocateMemory is expensive and should not be used often. Allocate bigger chunks of memory with vkAllocateMemory and then sub-allocate memory yourself with vkBindImageMemory'
                table.insert(messages, msg)
        end

        if suggest_avoid_resolve then
                msg='Mali GPUs support resolving multisampled framebuffers on-tile. Use pResolveAttachments in a subpass to automatically resolve multisampled color buffer to single-sampled color buffer'
                table.insert(messages, msg)
        end
        LPGPU2:beginTransaction()
        for _,v in pairs(messages) do
                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, v)
        end
        LPGPU2:endTransaction()

        FEnd()
end

--[[
--
-- This function analyses the function call trace and parameters and makes suggestions
-- regarding functions related to pipelines.
--
--]]
function vulkan_pipeline_functions_analysis()

        FStart("vulkan_pipeline_functions_analysis")

        posName  = 1
        posStart = 2
        posEnd   = 3

        pipeline_name = LPGPU2:getCallNameForApiAndCategory('Vulkan', 'Pipeline')
        pipeline_start = LPGPU2:getCpuStartForApiAndCategory('Vulkan', 'Pipeline')
        pipeline_params_info = LPGPU2:getBinaryParamsForApiAndCategory('Vulkan', 'Pipeline')
        num_calls = #pipeline_name

        pipeline_calls={}
        pipeline_params={}

        FSection("1/2")
        for i=1, num_calls do
                FProgress(i,num_calls)
                table.insert(pipeline_calls, {pipeline_name[i], pipeline_start[i]})
                if pipeline_params_info ~= {} then
                        --The Vulkan Database I had has no parameters table
                --        table.insert(pipeline_params, pipeline_params_info[i][0])
                end
        end


        draws = LPGPU2:getCpuEndForApiAndCategory('Vulkan', 'Draw')
        if draws[1] ~= nil then
                suggest_pipeline_cache = false
                suggest_single_pipeline_cache = false
                suggest_write_cache = true
                suggest_pipelines_at_startup = false
                suggest_avoid_derivatives = false

                time_first_draw=draws[1]

                num_pipeline_caches = 0
                FSection("2/2")
                for i=1, num_calls do
                        FProgress(i,num_calls)
                        function_name = pipeline_calls[i][posName]
                        call_start = pipeline_calls[i][posStart]
                        if function_name == 'vkCreatePipelineCache' then
                                num_pipeline_caches = num_pipeline_caches + 1
                        elseif function_name == 'vkGetPipelineCache' then
                                suggest_write_cache = false
                        elseif function_name == 'vkCreateComputePipelines' or function_name == 'vkCreateGraphicsPipelines' then
                                if pipeline_params[1] ~= nil and pipeline_params[1] > 0 then
                                        inputs = pipeline_params[i]

                                        last = inputs:len()

                                        --Assuming sizes of 32b
                                        flags=inputs:sub(17,19)
                                        if flags == '04' or flags == '05' or flags == '06' then
                                                index = inputs:sub(last - 7, last)
                                                handle = inputs:sub(last - 15, last - 8)
                                                if index ~= 'FFFFFFFF' or handle ~= '00000000' then
                                                        suggest_avoid_derivatives = true
                                                end
                                        end
                                end

                                if call_start > time_first_draw then
                                        suggest_pipelines_at_startup = true
                                end
                        end
                end

                if num_pipeline_caches == 0 then
                        suggest_pipeline_cache = true
                elseif num_pipeline_caches>1 then
                        suggest_single_pipeline_cache = true
                end
        else
                print('DB does not look like a Vulkan execution')
        end

        messages={}
        if suggest_pipeline_cache then
                msg='Reusing state can improve performance; consider using a pipeline cache.'
                table.insert(messages, msg)
        end

        if suggest_single_pipeline_cache then
                msg='It is advisable to use a single pipeline cache so the driver can reuse state from all previously created pipelines.'
                table.insert(messages, msg)
        end

        if suggest_write_cache then
                msg='A pipeline cache can be used between runs; consider persisting to a file.'
                table.insert(messages, msg)
        end

        if suggest_pipelines_at_startup then
                msg='Creating pipelines at draw time can degrade performance. Consider moving pipeline creation to the startup of the application.'
                table.insert(messages, msg)
        end

        if suggest_avoid_derivatives then
                msg='Pipeline derivatives usually are less efficient than a single pipeline on mobile devices due to the way their GPUs load state into memory. Some drivers ignore derivative pipelines.'
                table.insert(messages, msg)
        end

        LPGPU2:beginTransaction()
        for _,v in pairs(messages) do
                LPGPU2:writeAnnotation(LPGPU_SUGGESTION, v)
        end
        LPGPU2:endTransaction()

        FEnd()
end

-- TODO. This expects eglSwapBuffers or glFinish to be frame delimiters.
        -- GL Only
function why_late(target_FPS)

        FStart("why_late")

       local terminator = {}

       terminator["eglSwapBuffers"   ] = true
       terminator["glClear"          ] = true
       terminator["glFlush"          ] = true
       terminator["glFinish"         ] = true
       terminator["vkQueuePresentKHR"] = true
       terminator["clFlush"          ] = true
       terminator["clFinish"         ] = true
       terminator["clWaitForEvents"  ] = true

       -- first_pass_result[1]=resFPS         -- resFPS[#limited_frames] = FPS_samples_value
       -- first_pass_result[2]=resCalls       -- resCalls[#limited_frames] = calls_table[0..x] = {name, start, end}
       -- first_pass_result[3]=boundaries     -- boundaries[#limited_frames] = {start_time, end_time} of the frame
       -- first_pass_result[4]=frameIndex     -- frameIndex[#limited_frames] = frameIndex

        frame_calls=first_pass_result[2]
        boundaries=first_pass_result[3]


        expected_frame_length=(1000000000 / target_FPS) -- 1 second in nanoseconds divided by FPS.

        first_correction = LPGPU2:getFirstCallTimestamp()

        blockingCandidates = LPGPU2:getBlockingFunctions()

        cause_per_frame={}

        for i=1, #frame_calls do --This loop only works properly for limited frames defined by eglSwapBuffers and not by counter samples
                cause='UNKNOWN'

                calls_in_frame = frame_calls[i]

                frame_start = boundaries[i][1]

                expected_frame_end = frame_start + expected_frame_length

                actual_frame_end = boundaries[i][2]

--              print('expected_frame_end = '..expected_frame_end..', actual_frame_end = '..actual_frame_end)
                start_swap = 0

                total_api_time = 0
                time_blocking = 0
                time_to_first_draw = 0

                for _,call in ipairs(calls_in_frame) do
                        blocking = false
                        name = call[1]
                        if terminator[name] ~= nil then
                                start_swap = call[2] - first_correction
                                swap_duration = call[3]-call[2]
                        end

                        duration=call[3]-call[2]
                        total_api_time = total_api_time + duration
                        for _,item in ipairs(blockingCandidates) do
                                if item == name then
                                        blocking = true
                                        break
                                end
                        end
                        if blocking and terminator[name] ~= nil then
                                time_blocking = time_blocking+duration
                        elseif string.sub(name,1,7) == 'glDraw' and time_to_first_draw == 0 then
                                time_to_first_draw = call[2]-first_correction
                        end

                end

                if actual_frame_end > expected_frame_end then

                        duration = boundaries[i][2] - boundaries[i][1]

                        non_api_duration = actual_frame_end - frame_start-total_api_time

                        start_swap_api_only = start_swap - non_api_duration

                        end_swap_api_only = actual_frame_end - non_api_duration

                        non_blocking_duration = actual_frame_end-frame_start-time_blocking
                        start_swap_non_blocking = start_swap-time_blocking
                        end_swap_non_blocking = actual_frame_end-time_blocking

                        if (start_swap >= expected_frame_end and start_swap_api_only < expected_frame_end) or end_swap_api_only <= expected_frame_end then
                                --Too much non-API work on the host
                                cause='general CPU processing'
                        elseif (start_swap >= expected_frame_end and start_swap_non_blocking<expected_frame_end) or end_swap_non_blocking <= expected_frame_end then
                                --Blocking calls are the cause of the problem
                                cause='blocking calls'
                        elseif start_swap<expected_frame_end then
                                --Too much GL or incorrect use
                                cause='too many API calls'
                        else
                                --Too much work for the GPU
                                cause='GPU work'
                        end

                        table.insert(cause_per_frame, cause)
                end
        end

        LPGPU2:beginTransaction()
        for k,v in pairs(cause_per_frame) do
            frame = fp_frameIndex[k]
            LPGPU2:writeAnnotation(LPGPU_INFORMATION, string.format("The limiting factor for this frame is %s", v), frame)
        end
        LPGPU2:endTransaction()

        FEnd()

        return cause_per_frame

end
--Adapt to use the regions tables


function greatest_contribution_to_power()

        region_frames = regions_result[1]

        greatest_contributor_per_region = {}

        counters_for_model = LPGPU2:getCounterNamesFromCounterWeights()
        weights_for_model = LPGPU2:getWeightValuesFromCounterWeights()
        num_counters_for_model = #counters_for_model

        c = 0
        start = 0
        if counters_for_model[start] == 'Constant' then
                c = weights_for_model[start]
                start = 1
        end

        for _,region in ipairs(region_frames) do
                start_frame = region[1]
                end_frame = region[#region]

                timestamp_start = 0
                if start_frame > 1 then
                        timestamp_start = LPGPU2:getMaxCpuEndFromTracesForFrameNum(start_frame-1)
                end
                timestamp_end = LPGPU2:getMaxCpuEndFromTracesForFrameNum(end_frame)

                sample_timestamps = LPGPU2:getUniqueTimesFromSamplesInRange(timestamp_start, timestamp_end)

                contribution_per_counter={}
                for i=start, num_counters_for_model-1 do
                        if counters_for_model[i] ~= 'Constant' then
                                contribution_per_counter[counters_for_model[i]] = 0
                        else
                                c = weights_for_model[i]
                        end
                end

                max_contribution = c
                max_contributor='Constant'

                for i=start, num_counters_for_model-1 do
                        if counters_for_model[i] ~= 'Constant' then
                                -- TODO -- Think it's Ok.
                                contribution=LPGPU2:getWeightedSumForCounterInRange(counters_for_model[i], timestamp_start, timestamp_end)
                                if contribution[1] ~= nil and contribution[1] >= max_contribution then
                                        max_contribution = contribution[1]
                                        max_contributor = counters_for_model[i]
                                end
                        end
                end
                table.insert(greatest_contributor_per_region, max_contributor)

        end

        LPGPU2:beginTransaction()
        for k,v in ipairs(greatest_contributor_per_region) do
                LPGPU2:writeAnnotation(LPGPU_INFORMATION, string.format("The greatest contributor to energy consumption for region %d is %s",k, v))
        end
        LPGPU2:endTransaction()
end

function obtain_energy_estimations()

        counters_for_model = LPGPU2:getCounterNamesFromCounterWeights()
        weights_for_model = LPGPU2:getWeightValuesFromCounterWeights()
        num_counters_for_model = #counters_for_model

        c=0
        start=0
        if counters_for_model[start] == 'Constant' then
                c = weights_for_model[start]
                start = 1
        end
        timestamps = LPGPU2:getUniqueTimesFromSamples()

        num_samples_per_counter = #timestamps

        model_contributions_per_timestamp = {}
        for i = 1, num_samples_per_counter do
                model_contributions_per_timestamp[timestamps[i]]={}
        end

        for i = start, num_counters_for_model do
                if counters_for_model[i] == 'Constant' then
                        c = weights_for_model[i]
                else
                    -- TODO. Ok I think.
                        contributions=LPGPU2:getWeightedSumForCounterByName(counters_for_model[i])
                        -- contributions=LPGPU2:getColumnFromTable('sampledValue*'..weights_for_model[i], 'samples', 'WHERE counterId IN (SELECT counterId from counters WHERE counterName="'..counters_for_model[i]..'")')
                        for j=1, num_samples_per_counter do
                                table.insert(model_contributions_per_timestamp[timestamps[j]], {counters_for_model[i], contributions[j]})
                        end

                end
        end

        LPGPU2:beginTransaction()
        for t, contributions in pairs(model_contributions_per_timestamp) do
                estimation = c
                for _, c in pairs(contributions) do
                     estimation=estimation+tonumber(c[2])
                end
                LPGPU2:writePowerEstimation(t, estimation)
        end
        LPGPU2:endTransaction()

end

function shaders_in_use()
        FStart("shaders_in_use")

        shaders_per_frame_ret={} --In ith position a list of frameNum,drawNum pairs which represent the shaders in the shader table used in frame i

        shader_frame = LPGPU2:getFrameNumFromShader()
        shader_draw  = LPGPU2:getDrawNumFromShader()
        num_shaders  = #shader_frame

        --Indexed by shader GL id, contains the shader frameId and drawId pair
        shader_table={}

        for i=1, num_shaders do
                shader_id = string.sub(LPGPU2:getStackTraceParametersForFrameAndDrawCalls(shader_frame[i], shader_draw[i])[1],1,8)
                shader_table[shader_id] = {shader_frame[i], shader_draw[i]}
        end

        names  = LPGPU2:getCallNameFromTracesForCallType('Shader')
        frames = LPGPU2:getFrameNumFromTracesForCallType('Shader')
        params = LPGPU2:getBinaryParamsFromTraceParamsForCallType('Shader')

        num_calls=#names

        shaders_for_program={}

        FSection("1/2")
        shaders_per_frame={}
        for i=1, num_calls do
                FProgress(i,num_calls)
                call_name   = names[i]
                call_params = params[i]
                call_frame  = frames[i]
                if call_name=='glAttachShader' then
                        program = string.sub(call_params,1,8)
                        shader  = string.sub(call_params,9,16)
                        if shaders_for_program[program] == nil then
                                shaders_for_program[program]={shader}
                        else
                                table.insert(shaders_for_program[program], shader)
                        end
                elseif call_name == 'glDetachShader'then
                        program = string.sub(call_params,1,8)
                        shader  = string.sub(call_params,9,16)

                        for k,v in pairs(shaders_for_program[program]) do
                                if v == shader then
                                        table.remove(shaders_for_program[program],k)
                                        break
                                end
                        end

                elseif call_name == 'glUseProgram' then
                        program = call_params
                        if shaders_per_frame[call_frame] == nil then
                                shaders_per_frame[call_frame] = {}
                        end
                        if shaders_for_program[program] ~= nil then
                            for _,s in pairs(shaders_for_program[program]) do
                                table.insert(shaders_per_frame[call_frame], s)
                            end
                        end
                end

        end

        frames=LPGPU2:getUniqueFrameNumFromTraces()

        current_shaders = {}
        FSection("2/2")
        for i=1, #frames do
                FProgress(i,#frames)
                if shaders_per_frame[i] ~= nil then
                        current_shaders = shaders_per_frame[i]
                end
                shaders_per_frame_ret[i] = {}

                for _,s in pairs(current_shaders) do
                        table.insert(shaders_per_frame_ret[i], shader_table[s])
                end
        end
        LPGPU2:beginTransaction()
        for k,v in ipairs(shaders_per_frame_ret) do
                for frame,v2 in pairs(v) do
                     LPGPU2:writeAnnotation(LPGPU_INFORMATION, string.format("Shaders in Use: %d, %d", v2[1], v2[2]), frame)
                end
        end
        LPGPU2:endTransaction()

        FEnd()

        return shaders_per_frame_ret
end


function shaders_divergent()
 FStart("Divergent Shaders")

 local shaders = LPGPU2:getShaders()

 local count_if   = 0
 local count_else = 0

 local i = 0

 for _,shader in pairs(shaders) do
  FProgress(i,#shaders)

  i = i + 1

  for word in string.gmatch(shader, "%a+") do
   if word == "if"   then count_if   = count_if   + 1 end
   if word == "else" then count_else = count_else + 1 end
  end
 end

 if count_if ~= 0 or count_else ~= 0 then
  LPGPU2:beginTransaction()
  LPGPU2:writeAnnotation(LPGPU_SUGGESTION,"Branch statements such as 'if' and 'else' have been detected in one or more shaders suggesting they are divergent. Consider using an algorithm that avoids divergent code paths.",0,0)
  LPGPU2:endTransaction()
 end

 FEnd()
end


function shaders_precision()
 FStart("Shader Precision")

 local shaders = LPGPU2:getShaders()

 local count_prec = 0;

 local i = 0

 for _,shader in pairs(shaders) do
  FProgress(i,#shaders)

  i = i + 1

  for word in string.gmatch(shader, "%a+") do
   if word == "highp" or word == "mediump" then count_prec = count_prec + 1 end
  end
 end

 if count_prec ~= 0 then
  LPGPU2:beginTransaction()
  LPGPU2:writeAnnotation(LPGPU_SUGGESTION,"'highp' and/or 'mediump' precision directives detected in one or more shaders. Consider using lower precision to improve performance.",0,0)
  LPGPU2:endTransaction()
 end

 FEnd()
end


function number_of_draws(max_draw_calls)

        frame_ids = LPGPU2:getFramesForCategoryWithCallsAboveThreshold('Draw', max_draw_calls)
        num_frames = #frame_ids

        LPGPU2:beginTransaction()
        for i=1, num_frames do
            LPGPU2:writeAnnotation(LPGPU_ISSUE, 'This frame has too many draw calls', frame_ids[i])
        end
        LPGPU2:endTransaction()

end

-- **
function toplevel_identify_regions(threshold) -- Done
    first_pass(threshold) --> first_pass_results
    frame_summary() --> summary _results
    identify_regions() --> regions_result
end

-- **
function toplevel_function_type_summary(threshold) -- Done
    toplevel_identify_regions(threshold)
    function_type_summary(threshold) -- uses regions_result
end

-- **
function toplevel_fine_gpu_limitation_analysis(threshold)
    toplevel_identify_regions(threshold)
    fine_gpu_limitation_analysis() -- uses regions_result
end

-- **
function toplevel_why_late(threshold)
    first_pass(threshold)
    why_late(threshold)
end

function toplevel_greatest_contribution_to_power(threshold)
    toplevel_identify_regions(threshold)
    greatest_contribution_to_power() -- uses regions_result
end


-- This function is called when the script is loaded. It returns a table listing all the available feedback scripts
-- depending on what data is available in the database.

function get_available_feedback()
    print('get_available_feedback')
    return_table = {}

    -- Check what's available in the Database

    -- *** Traces ***
    tracesAvailable = false
    callNames = LPGPU2:getCallNameFromTraces()
    if #callNames > 0 then
       tracesAvailable = true
    end

    -- *** Timers ***
    gpuTimersAvailable = LPGPU2:checkGpuTimersAvailable() 

    -- Now go through all the various feedback scripts.

    -- ***** General Scripts *****
    scriptCategory = '--General--'

    if tracesAvailable then
        -- Calculate a sensible default threshold
        get_fps()
        --  identify_regions -> writeRegion / writeCallSummary
        -- Requires Traces only.
        scriptName ='Identify Regions'
        scriptFunction =  'toplevel_identify_regions'
        inputName = 'Threshold(FPS)'
        inputDescription = 'Regions will contain frames with framerate below the threshold'
        inputDefault = fps_default
        inputMin = fps_min
        inputMax = fps_max
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )

        -- function_type_summary -> writeCallsPerType
        -- Requires identify_regions only

        scriptName ='Function Summary by Region'
        scriptFunction =  'toplevel_function_type_summary'
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    --  coarse_grain_limitation_analysis -> writeAnnotation. Requires CPU_LOAD samples (at least). Makes assumptions, needs work.
    if cpuLoadAvailable then
        scriptName ='Coarse Grain Limitation Analysis'
        scriptFunction =  'coarse_grain_limitation_analysis'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    -- gpu2cpu_ratio -> . *** *** No output to Database *** ***
    if tracesAvailable then
        scriptName ='GPU to CPU Ratio'
        scriptFunction =  'gpu_2_cpu_ratio'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    -- number_of_draws(max_draw_calls)
    if gpuTimersAvailable then
        scriptName ='Draw Calls Limited'
        scriptFunction =  'number_of_draws'
        inputName = 'Threshold(Draw Calls)'
        inputDescription = 'Find frames with more draw calls than the threshold'
        inputDefault = fps_default
        inputMin = fps_min
        inputMax = fps_max
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

        scriptName ='Device Information'
        scriptFunction =  'call_device_info'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )


    -- ***** CPU Scripts *****
    scriptCategory = '--CPU--'

    params = LPGPU2_getBinaryParamsFromTraceParams()
    -- fine_cpu_limitation_analysis -> writeAnnotation. No requirements. Only checks number of threads
    if (#params > 0) then
        scriptName ='CPU Limitations'
        scriptFunction =  'fine_cpu_limitation_analysis'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
    table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    -- ***** GPU Scripts *****
    scriptCategory = '--GPU--'

    -- fine_gpu_limitation_analysis -> writeAnnotation.  Probably needs a tweak.
    fbstalls = LPGPU2:getMaxValuesForCategory('FB_STALLS')
    cycles = LPGPU2:getMaxValuesForCategory('TOTAL_CYCLES')
    texstalls = LPGPU2:getMaxValuesForCategory('TEX_STALLS')
    num = LPGPU2:getMaxValuesForCategory('NUM_INST')
    pixels = LPGPU2:getMaxValuesForCategory('NUM_PIXELS')
    if ((#fbstalls > 0) and (#cycles > 0)) or ((#texstalls > 0) and (#cycles > 0)) or ((#num > 0) and (#pixels > 0)) then
        scriptName ='Fine GPU Limitation'
        scriptFunction =  'toplevel_fine_gpu_limitation_analysis'
        inputName = 'Threshold(FPS)'
        inputDescription = 'Regions will contain frames with framerate below the threshold'
        inputDefault = fps_default
        inputMin = fps_min
        inputMax = fps_max
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end


    print('OpenGL Scripts')
    -- ***** OpenGL Scripts *****
    scriptCategory = '--OpenGL--'

    -- toplevel_why_late(threshold) *** No output to Database *** Fixed
    calls = LPGPU2:getCallNameForApi('OpenGL')
    blockingCandidates=LPGPU2:getBlockingFunctions()
    if #blockingCandidates > 0 and #calls > 0 then
        scriptName ='Why Late?'
        scriptFunction =  'toplevel_why_late'
        inputName = 'Threshold(Draw Calls)'
        inputDescription = 'Looks for Reasons why a frame might be late'
        inputDefault = fps_default
        inputMin = fps_min
        inputMax = fps_max
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    -- opengl_buffer_functions_analysis() --> writeAnnotation
    buffer_calls=LPGPU2:getCallNameFromOpenGLBufferAndDrawCalls()
    buffer_params=LPGPU2_getBinaryParamsFromOpenGLBufferAndDrawCalls()
    print('opengl_buffer_functions_analysis '..#buffer_calls..', '..#buffer_params)
    if (#buffer_calls > 0) and (#buffer_params > 0) then
        scriptName ='OpenGL Buffer Functions'
        scriptFunction =  'opengl_buffer_functions_analysis'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    -- opengl_state_tracker() *** No output to Database *** Fixed
    state_draw=LPGPU2:getDrawNumFromOpenGLStateAndFrameCalls()
    state_inputs=LPGPU2_getBinaryParamsFromOpenGLStateAndFrameCalls()
    print('opengl_state_tracker '..#state_draw..', '..#state_inputs)
    if (#state_draw > 0) and (#state_inputs > 0) then
        scriptName ='OpenGL State Tracker'
        scriptFunction =  'opengl_state_tracker'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    -- opengl_texture_analysis() --> writeAnnotation
    texture_calls=LPGPU2:getCallNameForApiAndCategory('OpenGL', 'Texture')
    texture_params=LPGPU2:getBinaryParamsForApiAndCategory('OpenGL', 'Texture')
    print('opengl_texture_analysis '..#texture_calls..', '..#texture_params)
    if (#texture_calls > 0) and (#texture_params > 0) then
        scriptName ='OpenGL Texture Functions'
        scriptFunction =  'opengl_texture_analysis'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    -- call_sequence_analysis() --> writeAnnotation
    sequence_calls =LPGPU2:getCallNameFromOpenGLBufferAndDrawCalls()
    sequence_params=LPGPU2_getBinaryParamsFromOpenGLBufferAndDrawCalls()
    print('call_sequence_analysis '..#sequence_calls..', '..#sequence_params)
    if (#sequence_calls > 0) then -- and (#sequence_params > 0) then
        scriptName ='OpenGL Object Lifetime Analysis'
        scriptFunction =  'object_lifetime_analysis_GL'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )

        scriptName ='OpenGL Call Sequence Analysis'
        scriptFunction =  'call_sequence_analysis_GL'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end


    -- ***** Vulkan Scripts *****
    scriptCategory = '--Vulkan--'

    --vulkan_descriptor_functions_analysis() --> writeAnnotation
    descriptor_name = LPGPU2:getCallNameFromVulkanDescriptorAndDrawCalls()
    if #descriptor_name > 0 then
        scriptName ='Vulkan Descriptor Functions'
        scriptFunction =  'vulkan_descriptor_functions_analysis'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    --vulkan_pipeline_functions_analysis() --> writeAnnotation
    pipeline_name=LPGPU2:getCallNameForApiAndCategory('Vulkan', 'Pipeline')
     if #pipeline_name > 0 then
        scriptName ='Vulkan Pipeline Functions'
        scriptFunction =  'vulkan_pipeline_functions_analysis'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    --vulkan_others() --> writeAnnotation
    other_name=LPGPU2:getCallNameForApiAndCategory('Vulkan', 'Other')
    if #other_name > 0 then
        scriptName ='Vulkan Other Functions'
        scriptFunction =  'vulkan_others'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    --call_sequence_analysis() --> writeAnnotation
    names=LPGPU2:getCallNameForApi('Vulkan')
     if #names > 0 then
        scriptName ='Vulkan Object Lifetime Analysis'
        scriptFunction =  'object_lifetime_analysis_VK'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )

        scriptName ='Vulkan Call Sequence Analysis'
        scriptFunction =  'call_sequence_analysis_VK'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end


    -- ***** OpenCL Scripts *****
    scriptCategory = '--OpenCL--'

    --opencl_compile_functions_analysis --> writeAnnotation
    names = LPGPU2:getCallNameForApiAndCategory('OpenCL', 'Compile')
    compile_inputs=LPGPU2:getBinaryParamsForApiAndCategory('OpenCL', 'Compile')
    if (#names > 0) and (#compile_inputs > 0) then
        scriptName ='OpenCL Compile Functions'
        scriptFunction =  'opencl_compile_functions_analysis'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end


    --opencl_data_functions_analysis  -> writeAnnotation.
    params1 = LPGPU2:getBinaryParamsFromOpenCLDataTransferAndNDRangeCalls()
    params2 = LPGPU2:getBinaryParamsFromOpenCLMemAllocationAndSetupCalls()
    if (#params1 > 0) and (#params2 > 0) then
        scriptName ='OpenCL Data Functions'
        scriptFunction =  'opencl_data_functions_analysis'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    --opencl_work_functions_analysis() --> writeAnnotation
    work_params=LPGPU2:getBinaryParamsForApiAndCategory('OpenCL', 'Compute')
    if #work_params > 0 then
        scriptName ='OpenCL Work Functions'
        scriptFunction =  'opencl_work_functions_analysis'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    --opencl_kernel_analysis() --> writeAnnotation
    kernel_info=LPGPU2:getShaders()
    calls =  LPGPU2:getCallNameForApi('OpenCL')
    if (#kernel_info > 0) and (#calls > 0) then
        scriptName ='OpenCL Kernel Analysis'
        scriptFunction =  'opencl_kernel_analysis'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
        scriptName ='OpenCL Further Kernel Analysis'
        scriptFunction =  'opencl_kernel_analysis_two'
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end


    -- ***** Power Scripts *****
    scriptCategory = '--Power--'

    -- greatest_contribution_to_power *** No output to Database *** Fixed
    counters_for_model=LPGPU2:getCounterNamesFromCounterWeights()
    if #counters_for_model > 0 and tracesAvailable then
        scriptName ='Greatest Contribution to Power'
        scriptFunction =  'toplevel_greatest_contribution_to_power'
        inputName = 'Threshold(FPS)'
        inputDescription = 'Regions will contain frames with framerate below the threshold'
        inputDefault = fps_default
        inputMin = fps_min
        inputMax = fps_max
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    -- obtain_energy_estimations --> writePowerEstimation
    if #counters_for_model > 0 then
        scriptName ='Energy Estimations'
        scriptFunction =  'obtain_energy_estimations'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end


    -- ***** Shader Scripts *****
    scriptCategory = '--Shader--'

    -- shaders_in_use *** No output to Database ***
    params= LPGPU2:getBinaryParamsFromTraceParamsForCallType('Shader')

    if #params > 0 then
        scriptName ='Shaders in Use'
        scriptFunction =  'shaders_in_use'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end

    -- shaders_in_use *** No output to Database ***
    local shaders= LPGPU2:getShaders()

    if #shaders > 0 then
        scriptName ='Divergent Shaders'
        scriptFunction =  'shaders_divergent'
        inputName = ''
        inputDescription = ''
        inputDefault = 0
        inputMin = 0
        inputMax = 0
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )

        scriptName ='Shader Precision'
        scriptFunction =  'shaders_precision'
        table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, inputMin, inputMax } )
    end


    print('#return_table = '..#return_table)

    return return_table
end





function call_device_info() -- Insert Device Information into the annotations table
 FStart("call_device_info")

 LPGPU2:beginTransaction()

 LPGPU2:writeAnnotation(
  LPGPU_INFORMATION,

     'Device data\n'
  .. '-----------'
  .. '\nPlatform   :' .. LPGPU2:getPlatform  ()
  .. '\nHardware   :' .. LPGPU2:getHardware  ()
  .. '\nDCAPI      :' .. LPGPU2:getDCAPI     ()
  .. '\nRAgent     :' .. LPGPU2:getRAgent    ()
  .. '\nHardwareID :' .. LPGPU2:getHardwareID()
  .. '\nBlobSize   :' .. LPGPU2:getBlobSize  ()

  ,0,0 -- frame,draw
 )

 LPGPU2:endTransaction()

 FEnd()
end



 -- Vulkan return codes...
  -- success codes...
  VK_SUCCESS     = 0
  VK_NOT_READY   = 1
  VK_TIMEOUT     = 2
  VK_EVENT_SET   = 3
  VK_EVENT_RESET = 4
  VK_INCOMPLETE  = 5

  -- error codes
  VK_ERROR_OUT_OF_HOST_MEMORY    =  -1
  VK_ERROR_OUT_OF_DEVICE_MEMORY  =  -2
  VK_ERROR_INITIALIZATION_FAILED =  -3
  VK_ERROR_DEVICE_LOST           =  -4
  VK_ERROR_MEMORY_MAP_FAILED     =  -5
  VK_ERROR_LAYER_NOT_PRESENT     =  -6
  VK_ERROR_EXTENSION_NOT_PRESENT =  -7
  VK_ERROR_FEATURE_NOT_PRESENT   =  -8
  VK_ERROR_INCOMPATIBLE_DRIVER   =  -9
  VK_ERROR_TOO_MANY_OBJECTS      = -10
  VK_ERROR_FORMAT_NOT_SUPPORTED  = -11
  VK_ERROR_FRAGMENTED_POOL       = -12

  local VkResultNames = {}

  VkResultNames[VK_SUCCESS    ] = "VK_SUCCESS"
  VkResultNames[VK_NOT_READY  ] = "VK_NOT_READY"
  VkResultNames[VK_TIMEOUT    ] = "VK_TIMEOUT"
  VkResultNames[VK_EVENT_SET  ] = "VK_EVENT_SET"
  VkResultNames[VK_EVENT_RESET] = "VK_EVENT_RESET"
  VkResultNames[VK_INCOMPLETE ] = "VK_INCOMPLETE"

  VkResultNames[VK_ERROR_OUT_OF_HOST_MEMORY   ] = "VK_ERROR_OUT_OF_HOST_MEMORY"
  VkResultNames[VK_ERROR_OUT_OF_DEVICE_MEMORY ] = "VK_ERROR_OUT_OF_DEVICE_MEMORY"
  VkResultNames[VK_ERROR_INITIALIZATION_FAILED] = "VK_ERROR_INITIALIZATION_FAILED"
  VkResultNames[VK_ERROR_DEVICE_LOST          ] = "VK_ERROR_DEVICE_LOST"
  VkResultNames[VK_ERROR_MEMORY_MAP_FAILED    ] = "VK_ERROR_MEMORY_MAP_FAILED"
  VkResultNames[VK_ERROR_LAYER_NOT_PRESENT    ] = "VK_ERROR_LAYER_NOT_PRESENT"
  VkResultNames[VK_ERROR_EXTENSION_NOT_PRESENT] = "VK_ERROR_EXTENSION_NOT_PRESENT"
  VkResultNames[VK_ERROR_FEATURE_NOT_PRESENT  ] = "VK_ERROR_FEATURE_NOT_PRESENT"
  VkResultNames[VK_ERROR_INCOMPATIBLE_DRIVER  ] = "VK_ERROR_INCOMPATIBLE_DRIVER"
  VkResultNames[VK_ERROR_TOO_MANY_OBJECTS     ] = "VK_ERROR_TOO_MANY_OBJECTS"
  VkResultNames[VK_ERROR_FORMAT_NOT_SUPPORTED ] = "VK_ERROR_FORMAT_NOT_SUPPORTED"
  VkResultNames[VK_ERROR_FRAGMENTED_POOL      ] = "VK_ERROR_FRAGMENTED_POOL"



 -- Vulkan Bitmasks...

 -- Valid bitmask checking helper function (assumes 0 is valid)
 function check_VkBitmask(name,pname,val,frame,draw,mask)
  if val ~=0 and val & mask == 0 then
   annotateIssue("Spurious bits are set in '" .. pname .. "' = " .. string.format("0x%X",val) .. " parameter in call to " .. name,frame,draw)
  end
 end


 -- VkAccessFlagBits...
 local     VK_ACCESS_INDIRECT_COMMAND_READ_BIT                  = 0x00000001
 local     VK_ACCESS_INDEX_READ_BIT                             = 0x00000002
 local     VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT                  = 0x00000004
 local     VK_ACCESS_UNIFORM_READ_BIT                           = 0x00000008
 local     VK_ACCESS_INPUT_ATTACHMENT_READ_BIT                  = 0x00000010
 local     VK_ACCESS_SHADER_READ_BIT                            = 0x00000020
 local     VK_ACCESS_SHADER_WRITE_BIT                           = 0x00000040
 local     VK_ACCESS_COLOR_ATTACHMENT_READ_BIT                  = 0x00000080
 local     VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT                 = 0x00000100
 local     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT          = 0x00000200
 local     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT         = 0x00000400
 local     VK_ACCESS_TRANSFER_READ_BIT                          = 0x00000800
 local     VK_ACCESS_TRANSFER_WRITE_BIT                         = 0x00001000
 local     VK_ACCESS_HOST_READ_BIT                              = 0x00002000
 local     VK_ACCESS_HOST_WRITE_BIT                             = 0x00004000
 local     VK_ACCESS_MEMORY_READ_BIT                            = 0x00008000
 local     VK_ACCESS_MEMORY_WRITE_BIT                           = 0x00010000
 local     VK_ACCESS_COMMAND_PROCESS_READ_BIT_NVX               = 0x00020000
 local     VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX              = 0x00040000
 local     VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT  = 0x00080000

 local ALL_VkAccessFlagBits = -- useful for checking valid masks
           VK_ACCESS_INDIRECT_COMMAND_READ_BIT
         | VK_ACCESS_INDEX_READ_BIT
         | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
         | VK_ACCESS_UNIFORM_READ_BIT
         | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT
         | VK_ACCESS_SHADER_READ_BIT
         | VK_ACCESS_SHADER_WRITE_BIT
         | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
         | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
         | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
         | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
         | VK_ACCESS_TRANSFER_READ_BIT
         | VK_ACCESS_TRANSFER_WRITE_BIT
         | VK_ACCESS_HOST_READ_BIT
         | VK_ACCESS_HOST_WRITE_BIT
         | VK_ACCESS_MEMORY_READ_BIT
         | VK_ACCESS_MEMORY_WRITE_BIT
         | VK_ACCESS_COMMAND_PROCESS_READ_BIT_NVX
         | VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX
         | VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT

 function check_VkAccessFlagBits(name,pname,val,frame,draw)
          check_VkBitmask       (name,pname,val,frame,draw,ALL_VkAccessFlagBits)
 end


 -- VkCommandBufferResetFlagBits...
 local     VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT = 1

 local ALL_VkCommandBufferResetFlagBits =
           VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT

 function check_VkCommandBufferResetFlagBits(name,pname,val,frame,draw)
          check_VkBitmask                   (name,pname,val,frame,draw,ALL_VkCommandBufferResetFlagBits)
 end


 -- VkCommandPoolResetFlagBits...
 local     VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT = 1

 local ALL_VkCommandPoolResetFlagBits =
           VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT

 function check_VkCommandPoolResetFlagBits(name,pname,val,frame,draw)
          check_VkBitmask                 (name,pname,val,frame,draw,ALL_VkCommandPoolResetFlagBits)
 end


 -- VkImageCreateFlagBits...
 local     VK_IMAGE_CREATE_SPARSE_BINDING_BIT                        = 0x00000001
 local     VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT                      = 0x00000002
 local     VK_IMAGE_CREATE_SPARSE_ALIASED_BIT                        = 0x00000004
 local     VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT                        = 0x00000008
 local     VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT                       = 0x00000010
 local     VK_IMAGE_CREATE_ALIAS_BIT                                 = 0x00000400
 local     VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT           = 0x00000040
 local     VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT                   = 0x00000020
 local     VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT           = 0x00000080
 local     VK_IMAGE_CREATE_EXTENDED_USAGE_BIT                        = 0x00000100
 local     VK_IMAGE_CREATE_PROTECTED_BIT                             = 0x00000800
 local     VK_IMAGE_CREATE_DISJOINT_BIT                              = 0x00000200
 local     VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT = 0x00001000
 local     VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR       = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT
 local     VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR               = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT
 local     VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT_KHR       = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT
 local     VK_IMAGE_CREATE_EXTENDED_USAGE_BIT_KHR                    = VK_IMAGE_CREATE_EXTENDED_USAGE_BIT
 local     VK_IMAGE_CREATE_DISJOINT_BIT_KHR                          = VK_IMAGE_CREATE_DISJOINT_BIT
 local     VK_IMAGE_CREATE_ALIAS_BIT_KHR                             = VK_IMAGE_CREATE_ALIAS_BIT

 local ALL_VkImageCreateFlagBits = -- useful for checking valid masks
           VK_IMAGE_CREATE_SPARSE_BINDING_BIT
         | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT
         | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT
         | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT
         | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
         | VK_IMAGE_CREATE_ALIAS_BIT
         | VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT
         | VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT
         | VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT
         | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT
         | VK_IMAGE_CREATE_PROTECTED_BIT
         | VK_IMAGE_CREATE_DISJOINT_BIT
         | VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT
         | VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR
         | VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR
         | VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT_KHR
         | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT_KHR
         | VK_IMAGE_CREATE_DISJOINT_BIT_KHR
         | VK_IMAGE_CREATE_ALIAS_BIT_KHR

 function check_VkImageCreateFlagBits(name,pname,val,frame,draw)
          check_VkBitmask            (name,pname,val,frame,draw,ALL_VkImageCreateFlagBits)
 end


 -- VkImageUsageFlagBits...
 local     VK_IMAGE_USAGE_TRANSFER_SRC_BIT                      = 0x00000001
 local     VK_IMAGE_USAGE_TRANSFER_DST_BIT                      = 0x00000002
 local     VK_IMAGE_USAGE_SAMPLED_BIT                           = 0x00000004
 local     VK_IMAGE_USAGE_STORAGE_BIT                           = 0x00000008
 local     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT                  = 0x00000010
 local     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT          = 0x00000020
 local     VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT              = 0x00000040
 local     VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT                  = 0x00000080

 local ALL_VkImageUsageFlagBits = -- useful for checking valid masks
           VK_IMAGE_USAGE_TRANSFER_SRC_BIT
         | VK_IMAGE_USAGE_TRANSFER_DST_BIT
         | VK_IMAGE_USAGE_SAMPLED_BIT
         | VK_IMAGE_USAGE_STORAGE_BIT
         | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
         | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
         | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
         | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT

 function check_VkImageUsageFlagBits(name,pname,val,frame,draw)
          check_VkBitmask           (name,pname,val,frame,draw,ALL_VkImageUsageFlagBits)
 end


 -- VkPipelineStageFlagBits...
 local     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT                    = 0x00000001
 local     VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT                  = 0x00000002
 local     VK_PIPELINE_STAGE_VERTEX_INPUT_BIT                   = 0x00000004
 local     VK_PIPELINE_STAGE_VERTEX_SHADER_BIT                  = 0x00000008
 local     VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT    = 0x00000010
 local     VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT = 0x00000020
 local     VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT                = 0x00000040
 local     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT                = 0x00000080
 local     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT           = 0x00000100
 local     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT            = 0x00000200
 local     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT        = 0x00000400
 local     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT                 = 0x00000800
 local     VK_PIPELINE_STAGE_TRANSFER_BIT                       = 0x00001000
 local     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT                 = 0x00002000
 local     VK_PIPELINE_STAGE_HOST_BIT                           = 0x00004000
 local     VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT                   = 0x00008000
 local     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT                   = 0x00010000
 local     VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX            = 0x00020000

 local ALL_VkPipelineStageFlagBits = -- useful for checking valid masks
           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
         | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
         | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
         | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
         | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
         | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
         | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
         | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
         | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
         | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
         | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
         | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
         | VK_PIPELINE_STAGE_TRANSFER_BIT
         | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
         | VK_PIPELINE_STAGE_HOST_BIT
         | VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT
         | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
         | VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX

 function check_VkPipelineStageFlagBits(name,pname,val,frame,draw)
          check_VkBitmask              (name,pname,val,frame,draw,ALL_VkPipelineStageFlagBits)
 end


 -- VkQueryResultFlagBits...
 local VK_QUERY_RESULT_64_BIT                = 0x00000001
 local VK_QUERY_RESULT_WAIT_BIT              = 0x00000002
 local VK_QUERY_RESULT_WITH_AVAILABILITY_BIT = 0x00000004
 local VK_QUERY_RESULT_PARTIAL_BIT           = 0x00000008



 -- VkSampleCountFlagBits...
 local     VK_SAMPLE_COUNT_1_BIT  = 0x00000001
 local     VK_SAMPLE_COUNT_2_BIT  = 0x00000002
 local     VK_SAMPLE_COUNT_4_BIT  = 0x00000004
 local     VK_SAMPLE_COUNT_8_BIT  = 0x00000008
 local     VK_SAMPLE_COUNT_16_BIT = 0x00000010
 local     VK_SAMPLE_COUNT_32_BIT = 0x00000020
 local     VK_SAMPLE_COUNT_64_BIT = 0x00000040

 local ALL_VkSampleCountFlagBits = -- useful for checking valid masks
           VK_SAMPLE_COUNT_2_BIT
         | VK_SAMPLE_COUNT_4_BIT
         | VK_SAMPLE_COUNT_8_BIT
         | VK_SAMPLE_COUNT_16_BIT
         | VK_SAMPLE_COUNT_32_BIT
         | VK_SAMPLE_COUNT_64_BIT

 function check_VkSampleCountFlagBits(name,pname,val,frame,draw)
          check_VkBitmask            (name,pname,val,frame,draw,ALL_VkSampleCountFlagBits)
 end


 -- VkShaderStageFlagBits...
 local     VK_SHADER_STAGE_VERTEX_BIT                  = 0x00000001
 local     VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT    = 0x00000002
 local     VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004
 local     VK_SHADER_STAGE_GEOMETRY_BIT                = 0x00000008
 local     VK_SHADER_STAGE_FRAGMENT_BIT                = 0x00000010
 local     VK_SHADER_STAGE_COMPUTE_BIT                 = 0x00000020
 local     VK_SHADER_STAGE_ALL_GRAPHICS                = 0x0000001F
 local     VK_SHADER_STAGE_ALL                         = 0x7FFFFFFF

 local ALL_VkShaderStageFlagBits = -- useful for checking valid masks
           VK_SHADER_STAGE_VERTEX_BIT
         | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
         | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
         | VK_SHADER_STAGE_GEOMETRY_BIT
         | VK_SHADER_STAGE_FRAGMENT_BIT
         | VK_SHADER_STAGE_COMPUTE_BIT
         | VK_SHADER_STAGE_ALL_GRAPHICS
         | VK_SHADER_STAGE_ALL

 function check_VkShaderStageFlagBits(name,pname,val,frame,draw)
          check_VkBitmask            (name,pname,val,frame,draw,ALL_VkShaderStageFlagBits)
 end


 -- VkStencilFaceFlagBits...
 local     VK_STENCIL_FACE_FRONT_BIT = 0x00000001
 local     VK_STENCIL_FACE_BACK_BIT  = 0x00000002
 local     VK_STENCIL_FRONT_AND_BACK = 0x00000003

 local ALL_VkStencilFaceFlagBits = -- useful for checking valid masks (included for completeness)
           VK_STENCIL_FACE_FRONT_BIT
         | VK_STENCIL_FACE_BACK_BIT
         | VK_STENCIL_FRONT_AND_BACK

 function check_VkStencilFaceFlagBits(name,pname,val,frame,draw)
          check_VkBitmask            (name,pname,val,frame,draw,ALL_VkStencilFaceFlagBits)
 end


 -- VkQueryControlFlagBits...
 local     VK_QUERY_CONTROL_PRECISE_BIT = 1

 local ALL_VkQueryControlFlagBits =
           VK_QUERY_CONTROL_PRECISE_BIT

 function check_VkQueryControlFlagBits(name,pname,val,frame,draw)
          check_VkBitmask             (name,pname,val,frame,draw,ALL_VkQueryControlFlagBits)
 end

 -- End Vulkan Bitmasks



 -- Vulkan enums...

 -- VkFormat...
 --
 -- This enum has very many options not explicitly needed at present
 --
   function check_VkFormat(name,pname,val,frame,draw)
    -- These hardwired constants are taken from the enum definition for VkFormat

    if not ((val >=0 and val <= 184) or (val >= 1000156000 and val <= 1000156033) or (val >= 1000154000 and val <= 1000156007)) then
     annotateIssue("Invalid VkFormat for " .. pname .. " in call to " .. name,frame,draw)
    end
   end

 -- VkImageLayout...
 local VK_IMAGE_LAYOUT_UNDEFINED                        = 0
 local VK_IMAGE_LAYOUT_GENERAL                          = 1
 local VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL         = 2
 local VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3
 local VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL  = 4
 local VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL         = 5
 local VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL             = 6
 local VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL             = 7
 local VK_IMAGE_LAYOUT_PREINITIALIZED                   = 8
 local VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL = 1000117000
 local VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL = 1000117001
 local VK_IMAGE_LAYOUT_PRESENT_SRC_KHR                            = 1000001002
 local VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR                         = 1000111000
 local VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
 local VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL


 -- VkImageType
 local VK_IMAGE_TYPE_1D = 0
 local VK_IMAGE_TYPE_2D = 1
 local VK_IMAGE_TYPE_3D = 2

 function check_VkImageType(name,pname,val,frame,draw)
  if val ~= VK_IMAGE_TYPE_1D and
     val ~= VK_IMAGE_TYPE_2D and
     val ~= VK_IMAGE_TYPE_3D then
   annotateIssue("Invalid VkImageType for " .. pname .. " in call to " .. name,frame,draw)
  end
 end


 -- VkImageTiling
 local VK_IMAGE_TILING_OPTIMAL = 0
 local VK_IMAGE_TILING_LINEAR  = 1

 function check_VkImageTiling(name,pname,val,frame,draw)
  if val ~= VK_IMAGE_TILING_OPTIMAL and
     val ~= VK_IMAGE_TILING_LINEAR then
   annotateIssue("Invalid VkImageTiling for " .. pname .. " in call to " .. name,frame,draw)
  end
 end

 -- End Vulkan enums



 local rawFrameNum   = {}
 local rawDrawNum    = {}
 local rawCallName   = {}
 local rawCpuStart   = {}
 local rawCpuEnd     = {}
 local rawHexParams  = {}
 local rawHexReturn  = {}
 local rawTypes      = {}
 local rawParamsSize = {}
 local rawReturnSize = {}

 local rawGPUTemp    = {}
 local rawSamplesTim = {}

 rawFrameNum   = LPGPU2:getFrameNumFromTraces()
 rawDrawNum    = LPGPU2:getDrawNumFromTraces()
 rawCallName   = LPGPU2:getCallNameFromTraces()
 rawCpuStart   = LPGPU2:getCpuStartFromTraces()
 rawCpuEnd     = LPGPU2:getCpuEndFromTraces()

 local rawFrameNum2   = LPGPU2:getFrameNumFromTraceParams()
 local rawDrawNum2    = LPGPU2:getDrawNumFromTraceParams()
 local rawHexParams2  = LPGPU2_getBinaryParamsFromTraceParams()
 local rawHexReturn2  = LPGPU2:getBinaryReturnFromTraceParams()
 local rawTypes2      = LPGPU2:getTypeDefDescriptionFromTraceParams()
 local rawParamsSize2 = LPGPU2:getParamsSizeFromTraceParams()
 local rawReturnSize2 = LPGPU2:getReturnSizeFromTraceParams()

 local i=1

 for j=1,#rawFrameNum2 do
  while i < #rawFrameNum and (rawFrameNum[i] ~= rawFrameNum2[j] or rawDrawNum[i] ~= rawDrawNum2[j]) do
   rawHexParams [i] = ""
   rawHexReturn [i] = ""
   rawTypes     [i] = "void"
   rawParamsSize[i] = "0," -- Traces Params Sizes have a trailing comma in DB . I.e. "4,4,8,2,4,"
   rawReturnSize[i] = "0"

   i=i+1
  end

  rawHexParams [i] = rawHexParams2 [j]
  rawHexReturn [i] = rawHexReturn2 [j]
  rawTypes     [i] = rawTypes2     [j]
  rawParamsSize[i] = rawParamsSize2[j]
  rawReturnSize[i] = rawReturnSize2[j]

  i=i+1
 end

 rawFrameNum2   = nil
 rawDrawNum2    = nil
 rawHexParams2  = nil
 rawHexReturn2  = nil
 rawTypes2      = nil
 rawParamsSize2 = nil
 rawReturnSize2 = nil

 rawGPUTemp    = LPGPU2:getSamplesForCounterWithName("GPU Temperature")
 rawSamplesTim = LPGPU2:getTimesForCounterWithName  ("GPU Temperature")



 local apiExistsGLES = 0 < #LPGPU2:getCallNameForApi('OpenGL')
 local apiExistsCL   = 0 < #LPGPU2:getCallNameForApi('OpenCL')
 local apiExistsVK   = 0 < #LPGPU2:getCallNameForApi('Vulkan')



 local calls   = {} -- all the calls   -- numbered 1 to #calls

 local frames  = {} -- all the frames  -- numbered 0 to #frames
                                 -- i.e. there are 1 + #frames elements ... yeah, I know; nuts!
                                 -- btw... wherever draw numbers occur, they also are zero-based

 local regions = {} -- all the regions -- numbered 1 to #regions



 local hasParams = false



 local hw2cores = {} -- mapping hardware ID's to number of cores until binding is available

 hw2cores["samsung - SM-G930F(samsungexynos8890)"] = 8
 hw2cores["samsung - SM-G930V(qcom)"             ] = 4



 local hardware = {} -- This variable will capture overridable hardware specific parameters based on hardware ID.
                     -- Defaults and descriptions follow...

       hardware.MaxRenderPassPipelineBarriers = 20 -- If a render pass issues this many or more pipeline barrier commands, a warning is emitted



 -- NB: #tab is not always the number of elements in table 'tab'
 -- This was discovered with tables using 0 as an index. In that case,
 -- #tab is the number of elements counting from index 1.
 --
 -- If in doubt, don't use #tab ... or ... start your indexing from 1.
 --
 -- Good luck!



 -- The glName2Enum and glEnum2Name tables were generated by a python script that parsed
 -- LuaBindings.h in the replay app execept for 6 extra elements in glEnum2Name to take
 -- into account the bitfield used for "glClear". This is so we can diagnose the 'name'
 -- GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT" from the enum which
 -- is an or'd combination of the three values. Similarly for the three possible combinations
 -- of two or'd values.

 local glName2Enum = {}
 local glEnum2Name = {}


 function init_glName2Enum()

 glName2Enum[                              "GL_FALSE" ]  = 0
 glName2Enum[                             "GL_POINTS" ]  = 0
 glName2Enum[                               "GL_ZERO" ]  = 0
 glName2Enum[                           "GL_NO_ERROR" ]  = 0
 glName2Enum[                               "GL_NONE" ]  = 0
 glName2Enum[                    "GL_GLES_PROTOTYPES" ]  = 1
 glName2Enum[                     "GL_ES_VERSION_2_0" ]  = 1
 glName2Enum[                               "GL_TRUE" ]  = 1
 glName2Enum[                              "GL_LINES" ]  = 1
 glName2Enum[                                "GL_ONE" ]  = 1
 glName2Enum[                          "GL_LINE_LOOP" ]  = 2
 glName2Enum[                         "GL_LINE_STRIP" ]  = 3
 glName2Enum[                          "GL_TRIANGLES" ]  = 4
 glName2Enum[                     "GL_TRIANGLE_STRIP" ]  = 5
 glName2Enum[                       "GL_TRIANGLE_FAN" ]  = 6
 glName2Enum[                            "GL_REPLACE" ]  = 7681
 glName2Enum[                     "GL_FRONT_AND_BACK" ]  = 1032
 glName2Enum[                     "GL_BLEND_EQUATION" ]  = 32777
 glName2Enum[                 "GL_BLEND_EQUATION_RGB" ]  = 32777
 glName2Enum[                      "GL_FUNC_SUBTRACT" ]  = 32778
 glName2Enum[              "GL_FUNC_REVERSE_SUBTRACT" ]  = 32779
 glName2Enum[                              "GL_FIXED" ]  = 5132
 glName2Enum[                            "GL_VERSION" ]  = 7938
 glName2Enum[                 "GL_TEXTURE_MAG_FILTER" ]  = 10240
 glName2Enum[                        "GL_SCISSOR_BOX" ]  = 3088
 glName2Enum[                       "GL_SCISSOR_TEST" ]  = 3089
 glName2Enum[       "GL_STENCIL_BACK_PASS_DEPTH_PASS" ]  = 34819
 glName2Enum[                       "GL_RENDERBUFFER" ]  = 36161
 glName2Enum[                              "GL_NEVER" ]  = 512
 glName2Enum[                                "GL_INT" ]  = 5124
 glName2Enum[                           "GL_INT_VEC2" ]  = 35667
 glName2Enum[      "GL_INVALID_FRAMEBUFFER_OPERATION" ]  = 1286
 glName2Enum[                          "GL_SRC_COLOR" ]  = 768
 glName2Enum[                        "GL_BLEND_COLOR" ]  = 32773
 glName2Enum[                  "GL_COLOR_CLEAR_VALUE" ]  = 3106
 glName2Enum[                    "GL_COLOR_WRITEMASK" ]  = 3107
 glName2Enum[                     "GL_CONSTANT_COLOR" ]  = 32769
 glName2Enum[                              "GL_FLOAT" ]  = 5126
 glName2Enum[              "GL_CURRENT_VERTEX_ATTRIB" ]  = 34342
 glName2Enum[                         "GL_FLOAT_MAT4" ]  = 35676
 glName2Enum[            "GL_RENDERBUFFER_ALPHA_SIZE" ]  = 36179
 glName2Enum[                                "GL_RGB" ]  = 6407
 glName2Enum[                       "GL_INVALID_ENUM" ]  = 1280
 glName2Enum[                   "GL_COLOR_BUFFER_BIT" ]  = 16384
 glName2Enum[                          "GL_DECR_WRAP" ]  = 34056
 glName2Enum[             "GL_UNSIGNED_SHORT_4_4_4_4" ]  = 32819
 glName2Enum[                               "GL_LESS" ]  = 513
 glName2Enum[                "GL_POLYGON_OFFSET_FILL" ]  = 32823
 glName2Enum[              "GL_POLYGON_OFFSET_FACTOR" ]  = 32824
 glName2Enum[               "GL_BLEND_EQUATION_ALPHA" ]  = 34877
 glName2Enum[           "GL_ONE_MINUS_CONSTANT_COLOR" ]  = 32770
 glName2Enum[        "GL_VERTEX_ATTRIB_ARRAY_POINTER" ]  = 34373
 glName2Enum[                  "GL_STENCIL_BACK_FUNC" ]  = 34816
 glName2Enum[            "GL_RENDERBUFFER_DEPTH_SIZE" ]  = 36180
 glName2Enum[                   "GL_DEPTH_ATTACHMENT" ]  = 36096
 glName2Enum[                      "GL_INVALID_VALUE" ]  = 1281
 glName2Enum[                                 "GL_CW" ]  = 2304
 glName2Enum[           "GL_VERTEX_ATTRIB_ARRAY_SIZE" ]  = 34339
 glName2Enum[                              "GL_EQUAL" ]  = 514
 glName2Enum["GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL" ]  = 36050
 glName2Enum[                              "GL_RGBA4" ]  = 32854
 glName2Enum[                            "GL_RGB5_A1" ]  = 32855
 glName2Enum[                     "GL_CONSTANT_ALPHA" ]  = 32771
 glName2Enum[                  "GL_STENCIL_BACK_FAIL" ]  = 34817
 glName2Enum[          "GL_RENDERBUFFER_STENCIL_SIZE" ]  = 36181
 glName2Enum[                 "GL_TEXTURE_BINDING_2D" ]  = 32873
 glName2Enum[                  "GL_INVALID_OPERATION" ]  = 1282
 glName2Enum[                                "GL_CCW" ]  = 2305
 glName2Enum[           "GL_ALIASED_POINT_SIZE_RANGE" ]  = 33901
 glName2Enum[           "GL_ALIASED_LINE_WIDTH_RANGE" ]  = 33902
 glName2Enum[         "GL_VERTEX_ATTRIB_ARRAY_STRIDE" ]  = 34340
 glName2Enum[                             "GL_LEQUAL" ]  = 515
 glName2Enum[            "GL_MAX_TEXTURE_IMAGE_UNITS" ]  = 34930
 glName2Enum[                               "GL_BOOL" ]  = 35670
 glName2Enum[                 "GL_MAX_VERTEX_ATTRIBS" ]  = 34921
 glName2Enum[     "GL_VERTEX_ATTRIB_ARRAY_NORMALIZED" ]  = 34922
 glName2Enum[           "GL_ONE_MINUS_CONSTANT_ALPHA" ]  = 32772
 glName2Enum[                           "GL_TEXTURE0" ]  = 33984
 glName2Enum[       "GL_STENCIL_BACK_PASS_DEPTH_FAIL" ]  = 34818
 glName2Enum[           "GL_VERTEX_ATTRIB_ARRAY_TYPE" ]  = 34341
 glName2Enum[                          "GL_DONT_CARE" ]  = 4352
 glName2Enum[                       "GL_ARRAY_BUFFER" ]  = 34962
 glName2Enum[               "GL_ELEMENT_ARRAY_BUFFER" ]  = 34963
 glName2Enum[               "GL_ARRAY_BUFFER_BINDING" ]  = 34964
 glName2Enum[       "GL_ELEMENT_ARRAY_BUFFER_BINDING" ]  = 34965
 glName2Enum[                    "GL_DEPTH_COMPONENT" ]  = 6402
 glName2Enum[                               "GL_DECR" ]  = 7683
 glName2Enum[                           "GL_TEXTURE4" ]  = 33988
 glName2Enum[                              "GL_FRONT" ]  = 1028
 glName2Enum[ "GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING" ]  = 34975
 glName2Enum[                    "GL_SAMPLE_COVERAGE" ]  = 32928
 glName2Enum[                    "GL_MIRRORED_REPEAT" ]  = 33648
 glName2Enum[     "GL_NUM_COMPRESSED_TEXTURE_FORMATS" ]  = 34466
 glName2Enum[                   "GL_STENCIL_BACK_REF" ]  = 36003
 glName2Enum[            "GL_STENCIL_BACK_VALUE_MASK" ]  = 36004
 glName2Enum[             "GL_STENCIL_BACK_WRITEMASK" ]  = 36005
 glName2Enum[                               "GL_BYTE" ]  = 5120
 glName2Enum[               "GL_RENDERBUFFER_BINDING" ]  = 36007
 glName2Enum[                     "GL_SAMPLE_BUFFERS" ]  = 32936
 glName2Enum[                            "GL_SAMPLES" ]  = 32937
 glName2Enum[              "GL_SAMPLE_COVERAGE_VALUE" ]  = 32938
 glName2Enum[             "GL_SAMPLE_COVERAGE_INVERT" ]  = 32939
 glName2Enum[                           "GL_NOTEQUAL" ]  = 517
 glName2Enum[                  "GL_COLOR_ATTACHMENT0" ]  = 36064
 glName2Enum[                             "GL_LINEAR" ]  = 9729
 glName2Enum[                          "GL_BOOL_VEC3" ]  = 35672
 glName2Enum[                           "GL_TEXTURE8" ]  = 33992
 glName2Enum[                "GL_ONE_MINUS_DST_ALPHA" ]  = 773
 glName2Enum[                           "GL_TEXTURE9" ]  = 33993
 glName2Enum[            "GL_FRAMEBUFFER_UNSUPPORTED" ]  = 36061
 glName2Enum[               "GL_POLYGON_OFFSET_UNITS" ]  = 10752
 glName2Enum[                           "GL_FUNC_ADD" ]  = 32774
 glName2Enum[                               "GL_BACK" ]  = 1029
 glName2Enum[                          "GL_TEXTURE10" ]  = 33994
 glName2Enum[                 "GL_STENCIL_BUFFER_BIT" ]  = 1024
 glName2Enum[                           "GL_TEXTURE1" ]  = 33985
 glName2Enum[                           "GL_TEXTURE2" ]  = 33986
 glName2Enum[                           "GL_TEXTURE3" ]  = 33987
 glName2Enum[                      "GL_OUT_OF_MEMORY" ]  = 1285
 glName2Enum[                           "GL_TEXTURE5" ]  = 33989
 glName2Enum[                           "GL_TEXTURE6" ]  = 33990
 glName2Enum[                            "GL_GREATER" ]  = 516
 glName2Enum[                      "GL_BLEND_DST_RGB" ]  = 32968
 glName2Enum[                      "GL_BLEND_SRC_RGB" ]  = 32969
 glName2Enum[                    "GL_BLEND_DST_ALPHA" ]  = 32970
 glName2Enum[                    "GL_BLEND_SRC_ALPHA" ]  = 32971
 glName2Enum[                          "GL_TEXTURE12" ]  = 33996
 glName2Enum[                          "GL_TEXTURE13" ]  = 33997
 glName2Enum[                          "GL_TEXTURE14" ]  = 33998
 glName2Enum[                          "GL_TEXTURE15" ]  = 33999
 glName2Enum[                          "GL_TEXTURE16" ]  = 34000
 glName2Enum[                          "GL_TEXTURE17" ]  = 34001
 glName2Enum[                          "GL_DST_COLOR" ]  = 774
 glName2Enum[                          "GL_TEXTURE19" ]  = 34003
 glName2Enum[                          "GL_TEXTURE20" ]  = 34004
 glName2Enum[                          "GL_TEXTURE21" ]  = 34005
 glName2Enum[                          "GL_TEXTURE22" ]  = 34006
 glName2Enum[                          "GL_TEXTURE23" ]  = 34007
 glName2Enum[                          "GL_TEXTURE24" ]  = 34008
 glName2Enum[                          "GL_TEXTURE25" ]  = 34009
 glName2Enum[                          "GL_TEXTURE26" ]  = 34010
 glName2Enum[                          "GL_TEXTURE27" ]  = 34011
 glName2Enum[                          "GL_TEXTURE28" ]  = 34012
 glName2Enum[                          "GL_TEXTURE29" ]  = 34013
 glName2Enum[                          "GL_TEXTURE30" ]  = 34014
 glName2Enum[                          "GL_TEXTURE31" ]  = 34015
 glName2Enum[                        "GL_STREAM_DRAW" ]  = 35040
 glName2Enum[ "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE" ]  = 36048
 glName2Enum[                              "GL_SHORT" ]  = 5122
 glName2Enum[                        "GL_STATIC_DRAW" ]  = 35044
 glName2Enum[ "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME" ]  = 36049
 glName2Enum[                             "GL_ALWAYS" ]  = 519
 glName2Enum[                          "GL_TEXTURE18" ]  = 34002
 glName2Enum[                "GL_ONE_MINUS_DST_COLOR" ]  = 775
 glName2Enum["GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE" ]  = 36051
 glName2Enum[                   "GL_UNPACK_ALIGNMENT" ]  = 3317
 glName2Enum[                               "GL_KEEP" ]  = 7680
 glName2Enum[                            "GL_TEXTURE" ]  = 5890
 glName2Enum[                          "GL_BOOL_VEC2" ]  = 35671
 glName2Enum[                             "GL_VENDOR" ]  = 7936
 glName2Enum[                 "GL_TEXTURE_MIN_FILTER" ]  = 10241
 glName2Enum[                   "GL_DEPTH_BUFFER_BIT" ]  = 256
 glName2Enum[                "GL_ONE_MINUS_SRC_COLOR" ]  = 769
 glName2Enum[                          "GL_SRC_ALPHA" ]  = 770
 glName2Enum[                "GL_ONE_MINUS_SRC_ALPHA" ]  = 771
 glName2Enum[                          "GL_DST_ALPHA" ]  = 772
 glName2Enum[                     "GL_PACK_ALIGNMENT" ]  = 3333
 glName2Enum[                     "GL_UNSIGNED_SHORT" ]  = 5123
 glName2Enum[                          "GL_INCR_WRAP" ]  = 34055
 glName2Enum[                 "GL_SRC_ALPHA_SATURATE" ]  = 776
 glName2Enum[                          "GL_LUMINANCE" ]  = 6409
 glName2Enum[                             "GL_INVERT" ]  = 5386
 glName2Enum["GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" ]  = 36055
 glName2Enum[               "GL_FRAMEBUFFER_COMPLETE" ]  = 36053
 glName2Enum[                   "GL_TEXTURE_CUBE_MAP" ]  = 34067
 glName2Enum[           "GL_TEXTURE_BINDING_CUBE_MAP" ]  = 34068
 glName2Enum[        "GL_TEXTURE_CUBE_MAP_POSITIVE_X" ]  = 34069
 glName2Enum[        "GL_TEXTURE_CUBE_MAP_NEGATIVE_X" ]  = 34070
 glName2Enum[        "GL_TEXTURE_CUBE_MAP_POSITIVE_Y" ]  = 34071
 glName2Enum[        "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y" ]  = 34072
 glName2Enum[               "GL_LINEAR_MIPMAP_LINEAR" ]  = 9987
 glName2Enum[        "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z" ]  = 34074
 glName2Enum[                           "GL_RENDERER" ]  = 7937
 glName2Enum[                 "GL_STENCIL_ATTACHMENT" ]  = 36128
 glName2Enum[                         "GL_LINE_WIDTH" ]  = 2849
 glName2Enum[                      "GL_CLAMP_TO_EDGE" ]  = 33071
 glName2Enum[                    "GL_FRAGMENT_SHADER" ]  = 35632
 glName2Enum[                      "GL_VERTEX_SHADER" ]  = 35633
 glName2Enum[                               "GL_INCR" ]  = 7682
 glName2Enum[                   "GL_MAX_TEXTURE_SIZE" ]  = 3379
 glName2Enum[              "GL_RENDERBUFFER_RED_SIZE" ]  = 36176
 glName2Enum[                            "GL_NEAREST" ]  = 9728
 glName2Enum[             "GL_UNSIGNED_SHORT_5_5_5_1" ]  = 32820
 glName2Enum[                  "GL_MAX_VIEWPORT_DIMS" ]  = 3386
 glName2Enum[                       "GL_UNSIGNED_INT" ]  = 5125
 glName2Enum[             "GL_NEAREST_MIPMAP_NEAREST" ]  = 9984
 glName2Enum[                        "GL_FRAMEBUFFER" ]  = 36160
 glName2Enum[                     "GL_ACTIVE_TEXTURE" ]  = 34016
 glName2Enum[                 "GL_RENDERBUFFER_WIDTH" ]  = 36162
 glName2Enum[                "GL_RENDERBUFFER_HEIGHT" ]  = 36163
 glName2Enum[                          "GL_CULL_FACE" ]  = 2884
 glName2Enum[                     "GL_CULL_FACE_MODE" ]  = 2885
 glName2Enum[                         "GL_FRONT_FACE" ]  = 2886
 glName2Enum[                     "GL_STENCIL_INDEX8" ]  = 36168
 glName2Enum[     "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS" ]  = 35660
 glName2Enum[   "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS" ]  = 35661
 glName2Enum[                        "GL_SHADER_TYPE" ]  = 35663
 glName2Enum[                      "GL_SUBPIXEL_BITS" ]  = 3408
 glName2Enum[                         "GL_FLOAT_VEC3" ]  = 35665
 glName2Enum[                           "GL_RED_BITS" ]  = 3410
 glName2Enum[                         "GL_GREEN_BITS" ]  = 3411
 glName2Enum[                          "GL_BLUE_BITS" ]  = 3412
 glName2Enum[                         "GL_ALPHA_BITS" ]  = 3413
 glName2Enum[                         "GL_DEPTH_BITS" ]  = 3414
 glName2Enum[                       "GL_STENCIL_BITS" ]  = 3415
 glName2Enum[                         "GL_EXTENSIONS" ]  = 7939
 glName2Enum[                          "GL_BOOL_VEC4" ]  = 35673
 glName2Enum[                         "GL_FLOAT_MAT2" ]  = 35674
 glName2Enum[                         "GL_FLOAT_MAT3" ]  = 35675
 glName2Enum[              "GL_LINEAR_MIPMAP_NEAREST" ]  = 9985
 glName2Enum[              "GL_MAX_RENDERBUFFER_SIZE" ]  = 34024
 glName2Enum[                         "GL_SAMPLER_2D" ]  = 35678
 glName2Enum[                       "GL_SAMPLER_CUBE" ]  = 35680
 glName2Enum[                            "GL_FASTEST" ]  = 4353
 glName2Enum[                             "GL_RGB565" ]  = 36194
 glName2Enum[               "GL_UNSIGNED_SHORT_5_6_5" ]  = 33635
 glName2Enum[                        "GL_BUFFER_SIZE" ]  = 34660
 glName2Enum[                       "GL_BUFFER_USAGE" ]  = 34661
 glName2Enum[  "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS" ]  = 36057
 glName2Enum[             "GL_RENDERBUFFER_BLUE_SIZE" ]  = 36178
 glName2Enum[                             "GL_REPEAT" ]  = 10497
 glName2Enum[                       "GL_STENCIL_FUNC" ]  = 2962
 glName2Enum[                        "GL_DEPTH_RANGE" ]  = 2928
 glName2Enum[                         "GL_DEPTH_TEST" ]  = 2929
 glName2Enum[                    "GL_DEPTH_WRITEMASK" ]  = 2930
 glName2Enum[                  "GL_DEPTH_CLEAR_VALUE" ]  = 2931
 glName2Enum[                         "GL_DEPTH_FUNC" ]  = 2932
 glName2Enum[          "GL_MAX_CUBE_MAP_TEXTURE_SIZE" ]  = 34076
 glName2Enum[              "GL_NEAREST_MIPMAP_LINEAR" ]  = 9986
 glName2Enum[                       "GL_DYNAMIC_DRAW" ]  = 35048
 glName2Enum[                           "GL_TEXTURE7" ]  = 33991
 glName2Enum[                      "GL_DELETE_STATUS" ]  = 35712
 glName2Enum[                     "GL_COMPILE_STATUS" ]  = 35713
 glName2Enum[                        "GL_LINK_STATUS" ]  = 35714
 glName2Enum[                    "GL_VALIDATE_STATUS" ]  = 35715
 glName2Enum[                    "GL_INFO_LOG_LENGTH" ]  = 35716
 glName2Enum[                   "GL_ATTACHED_SHADERS" ]  = 35717
 glName2Enum[                    "GL_ACTIVE_UNIFORMS" ]  = 35718
 glName2Enum[          "GL_ACTIVE_UNIFORM_MAX_LENGTH" ]  = 35719
 glName2Enum[               "GL_SHADER_SOURCE_LENGTH" ]  = 35720
 glName2Enum[                  "GL_ACTIVE_ATTRIBUTES" ]  = 35721
 glName2Enum[        "GL_ACTIVE_ATTRIBUTE_MAX_LENGTH" ]  = 35722
 glName2Enum[           "GL_SHADING_LANGUAGE_VERSION" ]  = 35724
 glName2Enum[                    "GL_CURRENT_PROGRAM" ]  = 35725
 glName2Enum[                       "GL_STENCIL_TEST" ]  = 2960
 glName2Enum[                "GL_STENCIL_CLEAR_VALUE" ]  = 2961
 glName2Enum[               "GL_GENERATE_MIPMAP_HINT" ]  = 33170
 glName2Enum[                 "GL_STENCIL_VALUE_MASK" ]  = 2963
 glName2Enum[                       "GL_STENCIL_FAIL" ]  = 2964
 glName2Enum[            "GL_STENCIL_PASS_DEPTH_FAIL" ]  = 2965
 glName2Enum[            "GL_STENCIL_PASS_DEPTH_PASS" ]  = 2966
 glName2Enum[                        "GL_STENCIL_REF" ]  = 2967
 glName2Enum[                  "GL_STENCIL_WRITEMASK" ]  = 2968
 glName2Enum[       "GL_RENDERBUFFER_INTERNAL_FORMAT" ]  = 36164
 glName2Enum[     "GL_IMPLEMENTATION_COLOR_READ_TYPE" ]  = 35738
 glName2Enum[   "GL_IMPLEMENTATION_COLOR_READ_FORMAT" ]  = 35739
 glName2Enum[                     "GL_TEXTURE_WRAP_T" ]  = 10243
 glName2Enum[                           "GL_VIEWPORT" ]  = 2978
 glName2Enum[  "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" ]  = 36054
 glName2Enum[                  "GL_DEPTH_COMPONENT16" ]  = 33189
 glName2Enum[        "GL_VERTEX_ATTRIB_ARRAY_ENABLED" ]  = 34338
 glName2Enum[        "GL_TEXTURE_CUBE_MAP_POSITIVE_Z" ]  = 34073
 glName2Enum[                            "GL_LOW_INT" ]  = 36339
 glName2Enum[           "GL_SAMPLE_ALPHA_TO_COVERAGE" ]  = 32926
 glName2Enum[                               "GL_RGBA" ]  = 6408
 glName2Enum[                         "GL_FLOAT_VEC2" ]  = 35664
 glName2Enum[                             "GL_DITHER" ]  = 3024
 glName2Enum[         "GL_COMPRESSED_TEXTURE_FORMATS" ]  = 34467
 glName2Enum[                              "GL_ALPHA" ]  = 6406
 glName2Enum[          "GL_NUM_SHADER_BINARY_FORMATS" ]  = 36345
 glName2Enum[                      "GL_UNSIGNED_BYTE" ]  = 5121
 glName2Enum[                    "GL_SHADER_COMPILER" ]  = 36346
 glName2Enum[                         "GL_TEXTURE_2D" ]  = 3553
 glName2Enum[                              "GL_BLEND" ]  = 3042
 glName2Enum[                "GL_FRAMEBUFFER_BINDING" ]  = 36006
 glName2Enum[            "GL_RENDERBUFFER_GREEN_SIZE" ]  = 36177
 glName2Enum[                     "GL_TEXTURE_WRAP_S" ]  = 10242
 glName2Enum[                         "GL_FLOAT_VEC4" ]  = 35666
 glName2Enum[                          "GL_LOW_FLOAT" ]  = 36336
 glName2Enum[                       "GL_MEDIUM_FLOAT" ]  = 36337
 glName2Enum[                         "GL_HIGH_FLOAT" ]  = 36338
 glName2Enum[                             "GL_GEQUAL" ]  = 518
 glName2Enum[                         "GL_MEDIUM_INT" ]  = 36340
 glName2Enum[                           "GL_HIGH_INT" ]  = 36341
 glName2Enum[                          "GL_TEXTURE11" ]  = 33995
 glName2Enum[                             "GL_NICEST" ]  = 4354
 glName2Enum[              "GL_SHADER_BINARY_FORMATS" ]  = 36344
 glName2Enum[                           "GL_INT_VEC3" ]  = 35668
 glName2Enum[                    "GL_LUMINANCE_ALPHA" ]  = 6410
 glName2Enum[         "GL_MAX_VERTEX_UNIFORM_VECTORS" ]  = 36347
 glName2Enum[                "GL_MAX_VARYING_VECTORS" ]  = 36348
 glName2Enum[       "GL_MAX_FRAGMENT_UNIFORM_VECTORS" ]  = 36349
 glName2Enum[                           "GL_INT_VEC4" ]  = 35669

 end -- init_glName2Enum



 function init_glEnum2Name()

  for name,enum in pairs(glName2Enum) do
   if name ~= "GL_COLOR_BUFFER_BIT" and name ~= "GL_DEPTH_BUFFER_BIT" and name ~= "GL_STENCIL_BUFFER_BIT" then
    if glEnum2Name[enum] == nil then
     glEnum2Name[enum] = name
    else
     glEnum2Name[enum] = glEnum2Name[enum] .. '/' .. name
    end
   end
  end


  glEnum2Name[
    glName2Enum["GL_COLOR_BUFFER_BIT"] + glName2Enum["GL_DEPTH_BUFFER_BIT"] + glName2Enum["GL_STENCIL_BUFFER_BIT"]
  ] = ""     .. "GL_COLOR_BUFFER_BIT | "          .. "GL_DEPTH_BUFFER_BIT | "          .. "GL_STENCIL_BUFFER_BIT"

  glEnum2Name[
    glName2Enum["GL_COLOR_BUFFER_BIT"] + glName2Enum["GL_DEPTH_BUFFER_BIT"]
  ] = ""     .. "GL_COLOR_BUFFER_BIT | "          .. "GL_DEPTH_BUFFER_BIT"

  glEnum2Name[
    glName2Enum["GL_COLOR_BUFFER_BIT"]                                      + glName2Enum["GL_STENCIL_BUFFER_BIT"]
  ] = ""     .. "GL_COLOR_BUFFER_BIT | "                                               .. "GL_STENCIL_BUFFER_BIT"

  glEnum2Name[
                                         glName2Enum["GL_DEPTH_BUFFER_BIT"] + glName2Enum["GL_STENCIL_BUFFER_BIT"]
  ] = ""                                          .. "GL_DEPTH_BUFFER_BIT | "          .. "GL_STENCIL_BUFFER_BIT"

 end -- function init_glEnum2Name



 glSetupCalls = {
                "glGetGraphicsResetStatus",
                "glGetIntegerv",
                "glGetInteger64v",
                "glDeteleSync",
                "glFenceSync",
                "glGetSynciv",
                "glIsSync",
                "glGenQueries",
                "glDeleteQueries",
                "glBeginQuery",
                "glBeginQueryIndexed",
                "glEndQuery",
                "glEndQueryIndexed",
                "glIsQuery",
                "glGetQueryiv",
                "glGetQueryIndexediv",
                "glGetQueryObjectuiv",
                "glGetQueryObjecti64v",
                "glGetQueryObjectui64v",
                "glQueryCounter",
                "glGenBuffers",
                "glCreateBuffers",
                "glDeleteBuffers",
                "glIsBuffer",
                "glGetBufferSubData",
                "glGetNamedBufferSubData",
                "glGetBufferParameteriv",
                "glGetBufferParameteri64v",
                "glGetNamedBufferParameteriv",
                "glGetNamedBufferParameteri64v",
                "glGetBufferPointerv",
                "glGetNamedBufferPointerv",
                "glCreateShader",
                "glShaderSource",
                "glCompileShader",
                "glReleaseShaderCompiler",
                "glDeleteShader",
                "glIsShader",
                "glShaderBinary",
                "glCreateProgram",
                "glAttachShader",
                "glDetachShader",
                "glLinkProgram",
--              "glUseProgram",
                "glCreateShaderProgramv",
                "glProgramParameteri",
                "glDeleteProgram",
                "glIsProgram",
                "glGetProgramInterfaceiv",
                "glGetProgramResourceIndex",
                "glGetProgramResourceName",
                "glGetProgramResourceiv",
                "glGetProgramResourceLocation",
                "glGetProgramResourceLocationIndex",
                "glGenProgramPipelines",
                "glDeleteProgramPipelines",
                "glIsProgramPipeline",
                "glBindProgramPipeline",
                "glCreateProgramPipelines",
                "glUseProramStages",
                "glActiveShaderProgram",
                "glGetProgramBinary",
                "glProgramBinary",
                "glGetUniformLocation",
                "glGetActiveUniformName",
                "glGetUniformIndices",
                "glgetActiveUniform",
                "glgetActiveUniformsiv",
                "glGetActiveUniformBlockIndex",
                "glGetActiveUniformBlockName",
                "glGetActiveUniformBlockiv",
                "glGetActiveAtomicCounterBufferiv",
--[[
                "glUniform1i",
                "glUniform2i",
                "glUniform3i",
                "glUniform4i",
                "glUniform1f",
                "glUniform2f",
                "glUniform3f",
                "glUniform4f",
                "glUniform1d",
                "glUniform2d",
                "glUniform3d",
                "glUniform4d",
                "glUniform1ui",
                "glUniform2ui",
                "glUniform3ui",
                "glUniform4ui",
                "glUniform1iv",
                "glUniform2iv",
                "glUniform3iv",
                "glUniform4iv",
                "glUniform1fv",
                "glUniform2fv",
                "glUniform3fv",
                "glUniform4fv",
                "glUniform1dv",
                "glUniform2dv",
                "glUniform3dv",
                "glUniform4dv",
                "glUniform1uiv",
                "glUniform2uiv",
                "glUniform3uiv",
                "glUniform4uiv",
                "glUniformMatrix2fv",
                "glUniformMatrix3fv",
                "glUniformMatrix4fv",
                "glUniformMatrix2dv",
                "glUniformMatrix3dv",
                "glUniformMatrix4dv",
                "glUniformMatrix2x3fv",
                "glUniformMatrix3x2fv",
                "glUniformMatrix2x4fv",
                "glUniformMatrix4x2fv",
                "glUniformMatrix3x4fv",
                "glUniformMatrix4x3fv",
                "glUniformMatrix2x3dv",
                "glUniformMatrix3x2dv",
                "glUniformMatrix2x4dv",
                "glUniformMatrix4x2dv",
                "glUniformMatrix3x4dv",
                "glUniformMatrix4x3dv",
--]]
                "glProgramUniform1i",
                "glProgramUniform2i",
                "glProgramUniform3i",
                "glProgramUniform4i",
                "glProgramUniform1f",
                "glProgramUniform2f",
                "glProgramUniform3f",
                "glProgramUniform4f",
                "glProgramUniform1d",
                "glProgramUniform2d",
                "glProgramUniform3d",
                "glProgramUniform4d",
                "glProgramUniform1ui",
                "glProgramUniform2ui",
                "glProgramUniform3ui",
                "glProgramUniform4ui",
                "glProgramUniform1iv",
                "glProgramUniform2iv",
                "glProgramUniform3iv",
                "glProgramUniform4iv",
                "glProgramUniform1fv",
                "glProgramUniform2fv",
                "glProgramUniform3fv",
                "glProgramUniform4fv",
                "glProgramUniform1dv",
                "glProgramUniform2dv",
                "glProgramUniform3dv",
                "glProgramUniform4dv",
                "glProgramUniform1uiv",
                "glProgramUniform2uiv",
                "glProgramUniform3uiv",
                "glProgramUniform4uiv",
                "glProgramUniformMatrix2fv",
                "glProgramUniformMatrix3fv",
                "glProgramUniformMatrix4fv",
                "glProgramUniformMatrix2dv",
                "glProgramUniformMatrix3dv",
                "glProgramUniformMatrix4dv",
                "glProgramUniformMatrix2x3fv",
                "glProgramUniformMatrix3x2fv",
                "glProgramUniformMatrix2x4fv",
                "glProgramUniformMatrix4x2fv",
                "glProgramUniformMatrix3x4fv",
                "glProgramUniformMatrix4x3fv",
                "glProgramUniformMatrix2x3dv",
                "glProgramUniformMatrix3x2dv",
                "glProgramUniformMatrix2x4dv",
                "glProgramUniformMatrix4x2dv",
                "glProgramUniformMatrix3x4dv",
                "glProgramUniformMatrix4x3dv",
                "glUniformBlockBinding",
                "glShaderStorageBlockBinding",
                "glGetSubroutineUniformLocation",
                "glGetSubroutineIndex",
                "glGetActiveSubroutineName",
                "glGetActiveSubroutineUniformName",
                "glGetActiveSubroutineUniformiv",
                "glUniformSubroutinesuiv",
                "glMemoryBarrier",
                "glMemoryBarrierByRegion",
                "glGetShaderiv",
                "glGetProgramiv",
                "glGetProgramPipelineiv",
                "glGetAttachedShaders",
                "glGetShaderInfoLog",
                "glGetProgramInfoLog",
                "glGetProgramPipelineInfoLog",
                "glGetShaderSource",
                "glGetShaderPrecisionFormat",
                "glGetUniformfv",
                "glGetUniformiv",
                "glGetUniformdv",
                "glGetUniformuiv",
                "glGetnUniformfv",
                "glGetnUniformiv",
                "glGetnUniformdv",
                "glGetnUniformuiv",
                "glGetUniformSubroutineuiv",
                "glGetProgramStageuiv",
--              "glActiveTexture",
                "glGenTextures",
                "glCreateTextures",
                "glDeleteTextures",
                "glIsTexture",
                "glGenSamplers",
                "glCreateSamplers",
                "glSamplerParameteri",
                "glSamplerParameterf",
                "glSamplerParameteriv",
                "glSamplerParameterfv",
                "glSamplerParameterIiv",
                "glSamplerParameterIuiv",
                "glDeleteSamplers",
                "glIsSampler",
                "glGetSamplerParameteri",
                "glGetSamplerParameterf",
                "glGetSamplerParameteriv",
                "glGetSamplerParameterfv",
                "glGetSamplerParameterIiv",
                "glGetSamplerParameterIuiv",
                "glPixelStorei",
                "glPixelStoref",
                "glTexImage3D",             --? NOT SURE THESE ARE SETUP FUNCTIONS (TEX)
                "glTexImage2D",
                "glTexImage1D",
                "glCopyTexImage2D",
                "glCopyTexImage1D",
                "glTexSubImage3D",          --?
                "glTexSubImage2D",
                "glTexSubImage1D",
                "glCopyTexSubImage3D",
                "glCopyTexSubImage2D",
                "glCopyTexSubImage1D",
                "glTextureSubImage3D",      --?
                "glTextureSubImage2D",
                "glTextureSubImage1D",
                "glCopyTextureSubImage3D",      --?
                "glCopyTextureSubImage2D",
                "glCopyTextureSubImage1D",
                "glCompressedTexImage3D",             --? NOT SURE THESE ARE SETUP FUNCTIONS (TEX)
                "glCompressedTexImage2D",
                "glCompressedTexImage1D",
                "glCompressedTexSubImage3D",          --?
                "glCompressedTexSubImage2D",
                "glCompressedTexSubImage1D",
                "glCompressedTextureSubImage3D",      --?
                "glCompressedTextureSubImage2D",
                "glCompressedTextureSubImage1D",
                "glTexImage3DMultisample",
                "glTexImage2DMultisample",
                "glTexBufferRange",
                "glTextureBufferRange",
                "glTexBuffer",
                "glTextureBuffer",
                "glTexParameteri",
                "glTexParameterf",
                "glTexParameteriv",
                "glTexParameterfv",
                "glTexParameterliv",
                "glTexParameterluiv",
                "glTextureParameterf",
                "glTextureParameteri",
                "glTextureParameterfv",
                "glTextureParameteriv",
                "glTextureParameterliv",
                "glTextureParameterluiv",
                "glGetTexParameteriv",
                "glGetTexParameterfv",
                "glGetTexParameterliv",
                "glGetTexParameterluiv",
                "glGetTextureParameterfv",
                "glGetTextureParameteriv",
                "glGetTextureParameterliv",
                "glGetTextureParameterluiv",
                "glGetTexLevelParameteriv",
                "glGetTexLevelParameterfv",
                "glGetTexLevelParameterliv",
                "glGetTexLevelParameterluiv",
                "glGetTexImage",
                "glGetTextureImage",
                "glGetTexImage",
                "glGetTextureSubImage",
                "glGetCompressedTexImage",
                "glGetCompressedTextureImage",
                "glGetnCompressedTexImage",
                "glGetCompressedTextureSubImage",
--              "glEnable",
--              "glDisable",
                "glIsEnabled",
                "glGenerateMipmap",         --?
                "glGenerateTextureMipmap",  --?
                "glTextureView",
                "glTexStorage1D",
                "glTexStorage2D",
                "glTexStorage3D",
                "glTextureStorage1D",
                "glTextureStorage2D",
                "glTextureStorage3D",
                "glTexStorage2DMultisample",
                "glTexStorage2DMultisample",
                "glTextureStorage3DMultisample",
                "glTextureStorage3DMultisample",
                "glInvalidateTexSubImage",
                "glInvalidateTexImage",
                "glClearTexSubImage",
                "glClearTexImage",
                "glCreateFramebuffers",
                "glGenFramebuffers",
                "glDeleteFramebuffers",
                "glIsFrameBuffer",
                "glFramebufferParameteri",
                "glNamedFramebufferParameteri",
                "glGetFramebufferParameteriv",
                "glGetNamedFramebufferParameteriv",
                "glGetFramebufferAttachmentParameteriv",
                "glGetNamedFramebufferAttachmentParameteriv",
                "glGenRenderbuffers",
                "glCreateRenderbuffers",
                "glDeleteRenderbuffers",
                "glIsRenderbuffer",
                "glRenderbufferStorageMultisample",
                "glNamedRenderbufferStorageMultisample",
                "glRenderbufferStorage",
                "glNamedRenderbufferStorage",
                "glGetRenderbufferParameteriv",
                "glGetNamedRenderbufferParameteriv",
                "glFramebufferRenderbuffer",
                "glNamedFramebufferRenderbuffer",
                "glFrameBufferTexture",
                "glNamedFramebufferTexture",
                "glFrameBufferTexture1D",
                "glFrameBufferTexture2D",
                "glFrameBufferTexture3D",
                "glFrameBufferTextureLayer",
                "glNamedFrameBufferTextureLayer",
                "glTextureBarrier",
                "glCheckFramebufferStatus",
                "glCheckNamedFramebufferStatus",
                "glPatchParameteri",
                "glVertexAttrib1s",
                "glVertexAttrib2s",
                "glVertexAttrib3s",
                "glVertexAttrib4s",
                "glVertexAttrib1f",
                "glVertexAttrib2f",
                "glVertexAttrib3f",
                "glVertexAttrib4f",
                "glVertexAttrib1d",
                "glVertexAttrib2d",
                "glVertexAttrib3d",
                "glVertexAttrib4d",
                "glVertexAttrib1sv",
                "glVertexAttrib2sv",
                "glVertexAttrib3sv",
                "glVertexAttrib4sv",
                "glVertexAttrib1fv",
                "glVertexAttrib2fv",
                "glVertexAttrib3fv",
                "glVertexAttrib4fv",
                "glVertexAttrib1dv",
                "glVertexAttrib2dv",
                "glVertexAttrib3dv",
                "glVertexAttrib4dv",
                "glVertexAttrib4bv",
                "glVertexAttrib4sv",
                "glVertexAttrib4iv",
                "glVertexAttrib4fv",
                "glVertexAttrib4dv",
                "glVertexAttrib4ubv",
                "glVertexAttrib4usv",
                "glVertexAttrib4uiv",
                "glVertexAttrib4b",
                "glVertexattribNub",
                "glVertexAttrib4Nbv",
                "glVertexAttrib4Nsv",
                "glVertexAttrib4Niv",
                "glVertexAttrib4Nfv",
                "glVertexAttrib4Ndv",
                "glVertexAttrib4Nubv",
                "glVertexAttrib4Nusv",
                "glVertexAttrib4Nuiv",
                "glVertexAttribI1i",
                "glVertexAttribI2i",
                "glVertexAttribI3i",
                "glVertexAttribI4i",
                "glVertexAttribI1ui",
                "glVertexAttribI2ui",
                "glVertexAttribI3ui",
                "glVertexAttribI4ui",
                "glVertexAttribI1iv",
                "glVertexAttribI2iv",
                "glVertexAttribI3iv",
                "glVertexAttribI4iv",
                "glVertexAttribI1uiv",
                "glVertexAttribI2uiv",
                "glVertexAttribI3uiv",
                "glVertexAttribI4uiv",
                "glVertexAttribI4bv",
                "glVertexAttribI4sv",
                "glVertexAttribI4ubv",
                "glVertexAttribI4usv",
                "glVertexAttribL1d",
                "glVertexAttribL2d",
                "glVertexAttribL3d",
                "glVertexAttribL4d",
                "glVertexAttribL1dv",
                "glVertexAttribL2dv",
                "glVertexAttribL3dv",
                "glVertexAttribL4dv",
                "glVertexAttribP1ui",
                "glVertexAttribP2ui",
                "glVertexAttribP3ui",
                "glVertexAttribP4ui",
                "glVertexAttribP1uiv",
                "glVertexAttribP2uiv",
                "glVertexAttribP3uiv",
                "glVertexAttribP4uiv",
                "glGenVertexArrays",
                "glDeleteVertexArrays",
                "glCreateVertexArrays",
                "glIsVertexArray",
                "glVertexArrayElementBuffer",
                "glVertexAttribFormat",
                "glVertexAttribIFormat",
                "glVertexAttribLFormat",
                "glVertexArrayAttribFormat",
                "glVertexArrayAttribIFormat",
                "glVertexArrayAttribLFormat",
                "glVertexArrayVertexBuffer",
                "glVertexArrayVertexBuffers",
                "glVertexAttribBinding",
                "glVertexArrayAttribBinding",
--[[
                "glVertexAttribPointer",
                "glVertexAttribIPointer",
                "glVertexAttribLPointer",
                "glEnableVertexAttribArray",
                "glEnableVertexArrayAttrib",
                "glDisableVertexAttribArray",
                "glDisableVertexArrayAttrib",
--]]
                "glVertexBindingDivisor",
                "glVertexArrayBindingDivisor",
                "glVertexAttribDivisor",
                "glPrimitiveRestartIndex",
                "glGetVertexArrayiv",
                "glGetArrayIndexdiv",
                "glGetVertexArrayIndexed64iv",
                "glGetVertexAttribdv",
                "glGetVertexAttribfv",
                "glGetVertexAttribiv",
                "glGetVertexAttribIiv",
                "glGetVertexAttribIuiv",
                "glGetVertexAttribLdv",
                "glGetVertexAttribPointerv",
                "glBeginConditionalRender",
                "glEndConditionalRender",
                "glGetActiveAttrib",
                "glGetAttribLocation",
                "glTransformFeedbackVaryings",
                "glGetTransformFeedbackVarying",
                "glValidateProgram",
                "glValidateProgramPipeline",
                "glPatchParameterfv",
                "glGenTransformFeedbacks",
                "glDeleteTransformFeedbacks",
                "glIsTransformFeedback",
                "glCreateTransformFeedbacks",
                "glBeginTransformFeedback",
                "glEndTransfromFeedback",
                "glPauseTransformFeedback",
                "glResumeTransformFeedback",
                "glTransformBufferFeedbackBufferRange",
                "glTransformFeedbackBufferBase",
                "glProvokingVertex",
                "glClipControl",
                "glDepthRangeArrayv",
                "glDepthRangeIndexed",
                "glDepthRange",
                "glDepthRangef",
                "glViewportArrayv",
                "glViewportIndexedf",
                "glViewportIndexedfv",
                "glViewport",
                "glGetMultisamplefv",
                "glMinSampleShading",
                "glPointSize",
                "glPointParameteri",
                "glPointParameterf",
                "glPointParameteriv",
                "glPointParameterfv",
                "glLineWidth",
                "glFrontFace",
                "glCullFace",
                "glPolygonMode",
                "glPolygonOffset",
                "glGetFragDataLocation",
                "glGetFragDataIndex",
                "glEnablei",
                "glDisablei",
                "glIsEnabledi",
                "glScissorArrayv",
                "glScissorIndexed",
                "glScissorIndexedv",
                "glScissor",
                "glSampleCoverage",
                "glSampleMaski",
                "glStencilFunc",
                "glStencilFuncSeparate",
                "glStencilOp",
                "glStencilOpSeparate",
                "glDepthFunc",
                "glBeginQuery",
                "glEndQuery",
                "glBlendEquation",
                "glBlendEquationSeparate",
                "glBlendEquationi",
                "glBlendEquationSeparatei",
                "glBlendFunc",
                "glBlendFuncSeparate",
                "glBlendFunci",
                "glBlendFuncSeparatei",
                "glBlendColor",
                "glLogicOp",
                "glHint",
                "glNamedFramebufferDrawBuffer",
                "glColorMask",
                "glColorMaski",
                "glDepthMask",
                "glStencilMask",
                "glStencilMaskSeparate",
                "glInvalidateSubFramebuffer",
                "glInvalidateNamedFramebuffersubdata",
                "glInvalidateFramebuffer",
                "glInvalidateNamedFramebufferData",
                "glClampColor",
                "glBlitFramebuffer",
                "glBlitNamedFramebuffer",
                "glGetBooleanv",
                "glGetFloatv",
                "glGetDoublev",
                "glGetDoublei_v",
                "glGetBooleani_v",
                "glgetIntegeri_v",
                "glGetFloati_v",
                "glGetDoublei_v",
                "glgetInteger64i_v",
                "glGetPointerv",
                "glGetString",
                "glGetStringi",
                "glGetInternalformativ",
                "glGetInternalformati64v",
                "glGetTransformFeedbackiv",
                "glGetTransformFeedbacki_v",
                "glGetTransformFeedbacki64_v",
                "glInvalidateFramebuffer",
                "glGetFixedv"
 }



 function type_to_bytes(t,s)
  if t=="void"      then return 0 end
  if t=="GLboolean" then return 1 end

  return s
 end



 function convert_u32_to_i32(r)
  if r > 0x7fffffff then
   local b,s = 1,0

   for i=1,32 do if (r and b) == 0 then s = s + b end b = 2 * b end

   r = -s-1
  end

  return r
 end



 function string_split(s,delim) r = {} for m in (s..delim):gmatch("(.-)"..delim) do table.insert(r,m) end return r end



 function table_contains(tab,elem) for _,e in ipairs(tab) do if e == elem then return true end end return false end

 function table_min     (tab)      if #tab == 0 then return 0.0/0.0 end local r = tab[#tab] for _,e in ipairs(tab) do if e < r then r = e end end return r end
 function table_max     (tab)      if #tab == 0 then return 0.0/0.0 end local r = tab[#tab] for _,e in ipairs(tab) do if e > r then r = e end end return r end
 function table_sum     (tab)                                           local r = 0.0       for _,e in ipairs(tab) do               r = r + e end return r end

 function table_mean    (tab) return table_sum(tab)/#tab end

 function table_std     (tab) local s,s2 = 0.0,0.0 for _,e in ipairs(tab) do s=s+e; s2=s2+e*e end local n = #tab return math.sqrt((s2-s*s/n)/n) end

 function table_at      (tab,i) return tab.value[tab.ranks[i]] end



 function parse_params(hex,sizes)
  function ascii2int(c)
   if     c>=65 and c<65+26 then c=c-65+10
   elseif c>=97 and c<97+26 then c=c-97+10
   elseif c>=48 and c<48+10 then c=c-48
   end

   return c
  end

  function stringdex2int(p,i)
   return ascii2int(p:byte(i))
  end

  function convert(p)
   local r = 0

   for i = 0,#p/2-1 do
    r = 256 * r + 16 * stringdex2int(p,#p-2*i-1)
                +      stringdex2int(p,#p-2*i  )
   end

   return r
  end

  local s = string_split(sizes,',')

  local r = {}
  local t = {}

  local n = 1

  for i = 1,#s do
   if s[i]:len() > 0 then
    p = hex:sub(n,n+2*tonumber(s[i])-1)

    n = n + 2*s[i]

    r[i] = convert(p)
    t[i] = "<?>"
   end
  end

  return t,r
 end




--[[
-- Don;t know if this is working, yet!!!!
 function asFloat(n)
  local s = math.floor(n / (2^31))
  local e = math.floor(BitAND(n,0x7f800000) / (2^23))
  local m = BitAND(n,0x007fffff)

  local r = (m*1.0)*(2^e)

  if s ~= 0 then r = -r end

  return r
 end


 function parseFloat(b) return asFloat(parseU32(b,1)) end
--]]



 -- Customise this function to characterise how regions are defined
 function same_region(i,j)
  if i < 0 or j < 0 or i > #frames or j > #frames then return false end

  return frames[i].CallList == frames[j].CallList
 end



 function display_calls()
  print("---------------")
  print("---  Calls  ---")
  print("---------------")

  local thing = ""

  for _,call in ipairs(calls) do
   thing = string.format("%30s"   ,call.CallName  )
        .. string.format(" %6d"   ,call.FrameNum  )
        .. string.format(" %6d"   ,call.DrawNum   )
        .. string.format(" %12g"  ,call.CpuStart  )
        .. string.format(" %12g"  ,call.CpuEnd    )
        .. string.format(" %12g"  ,call.CallTime  )
        .. string.format(" %12g"  ,call.Delay     )
        .. string.format(" %8s"   ,call.HexReturn )
        .. string.format(" %42s"  ,call.HexParams )

   if hasParams then
    thing = thing .. string.format(" %12s [",call.TypesTab[1])

    for i = 2,#call.TypesTab do
     c = string.format("%s 0x%X",call.TypesTab[i],call.ValueTab[i])
 
     if i == 2 then
      thing = thing .. " "  .. c
     else
      thing = thing .. ", " .. c
     end
    end

    thing = thing .. " ]"
   end

   print(thing)
  end

  print("...end calls")
 end



 function display_frames()
  print("----------------")
  print("---  Frames  ---")
  print("----------------")

  for i = 0,#frames do
   print(frames[i].NumCalls,"\t",frames[i].RegionNum)
  end
 end



 function display_regions_stats(ireg,field,name)
  local n      = regions[ireg].NumCalls-1

  local final_index = field.ranks[n          ]
  local final_value = field.value[final_index]

  local cap_index = math.min(10-1,n)

  local thing = string.format("\t Rank    Draw    %s",name)

  print("\t-------------------------------------------")
  print(thing)
  print("\t-------------------------------------------")

  for k = 0,cap_index do
   local index = field.ranks[k]
   local value = field.value[index]

   local thing =
       string.format(     "\t%5d",k+1  )
    .. string.format(       "%8d",index)
    .. string.format("    %12.3f",value)
    .. string.format(    "    %s",frames[regions[ireg].frames[1]].calls.data[index].CallName)

--  frames[FrameNum].calls.data[DrawNum].CallName = calls[CallNum].CallName


   if (value ~= final_value) then
    print(thing) -- "\t",k+1,"\t",index,"\t",value)
   end
  end

  print()
 end



 function display_regions()
  print("-----------------")
  print("---  Regions  ---")
  print("-----------------")

  for i = 1,#regions do
   print()
   print("Region ",i)
   print()
   print("\tFrames          :",regions[i].NumFrames)
   print("\tCalls per frame :",regions[i].NumCalls )
   print()
   print("\tFrame Statistics")
   print()
   print("\t\tDuration (ms)")
   print("\t\t\tMin  :",regions[i].Duration .frame.min )
   print("\t\t\tMax  :",regions[i].Duration .frame.max )
   print("\t\t\tMean :",regions[i].Duration .frame.mean)
   print("\t\t\tSTD  :",regions[i].Duration .frame.std )
   print()
   print("\t\tLoop Delay (ms) - [ Time between frames ]")
   print("\t\t\tMin  :",regions[i].LoopDelay.frame.min )
   print("\t\t\tMax  :",regions[i].LoopDelay.frame.max )
   print("\t\t\tMean :",regions[i].LoopDelay.frame.mean)
   print("\t\t\tSTD  :",regions[i].LoopDelay.frame.std )
   print()
   print("\t\tTotal API Call Time (ms)")
   print("\t\t\tMin  :",regions[i].CallTime .frame.min )
   print("\t\t\tMax  :",regions[i].CallTime .frame.max )
   print("\t\t\tMean :",regions[i].CallTime .frame.mean)
   print("\t\t\tSTD  :",regions[i].CallTime .frame.std )
   print()
   print("\t\tTotal Delay (ms) - [ Non API Call Time ]")
   print("\t\t\tMin  :",regions[i].Delay    .frame.min )
   print("\t\t\tMax  :",regions[i].Delay    .frame.max )
   print("\t\t\tMean :",regions[i].Delay    .frame.mean)
   print("\t\t\tSTD  :",regions[i].Delay    .frame.std )
   print()

   display_regions_stats(i,regions[i].CallTime.calls.mean,"Mean CallTime (ms)")
   display_regions_stats(i,regions[i].Delay   .calls.mean,"Mean Delay (ms)")
   display_regions_stats(i,regions[i].Delay   .calls.max ,"Max Delay (ms)")
  end
 end



 function check_GL_glClear()
  FStart("check_GL_glClear")

  local ncalls  = 0
  local nframes = 0
  local frame   =-1 -- previous frame
  local draw

  local cmd = ""

  local i=1
  for _,call in ipairs(calls) do
   FProgress(i,#calls)
   if cmd == "glClear" and call.CallName == cmd then
    ncalls = ncalls + 1
    if frame ~= call.FrameNum then
     nframes = nframes + 1
     frame = call.FrameNum
     draw  = call. DrawNum
    end
   end

   cmd = call.CallName
   i=i+1
  end

  if ncalls ~= 0 then
   local msg =
       "----------------------------------------\n"
    .. "CONSECUTIVE CALLS TO 'glClear' DETECTED!\n"
    .. "----------------------------------------\n"
    .. "\n"
    .. "	'glClear()' is a moderately expensive call; it addresses every pixel in the buffer.\n"
    .. "\n"
    .. "	You made " .. string.format("%d",ncalls) .. " unnecessary calls to glClear() in " .. string.format("%d",nframes) .. " frames.\n"
    .. "\n"
    .. "	Consecutive calls to glClear() should always be folded into a single call\n"
    .. "	using the appropriately combined bitmask. For example, this 'C' code:\n"
    .. "\n"
    .. "	    glClear(GL_COLOR_BUFFER_BIT);\n"
    .. "	    glClear(GL_COLOR_BUFFER_BIT);   // Uneccessary duplicate call!\n"
    .. "	    glClear(GL_DEPTH_BUFFER_BIT);   // Inefficient separate call!\n"
    .. "	    glClear(GL_STENCIL_BUFFER_BIT); // Another inefficient call!\n"
    .. "\n"
    .. "	should be replaced by:\n"
    .. "\n"
    .. "	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Cool!\n"

   annotateSuggestion(msg,frame,draw)
  end

  FEnd()
 end



 function check_GL_texture_binding()
  if not hasParams then return end

  FStart("check_GL_texture_binding")

  local unit = {} -- track which texture is bound to which texture unit
  local act  = 0
  local tex  = 0

  local FrameNum = 0
  local  DrawNum = 0

  local detected = false

  local count = {}

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   if call.CallName == "glBindTexture" then
    tex = call.ValueTab[3]

    if act ~= 0 and tex ~= 0 and unit[act] == tex then
     detected =true

     if count[frames[i].RegionNum] == nil then count[frames[i].RegionNum] = 0 end

     count[frames[i].RegionNum] = count[frames[i].RegionNum] + 1

     FrameNum = call.FrameNum
      DrawNum = call. DrawNum
    end

    unit[act] = tex
   elseif call.CallName == "glActiveTexture" then
    act = call.ValueTab[2]
   end

  end end

  if detected then
   local msg =
       "-----------------------------\n"
    .. "UNNECCESSARY TEXTURE BINDING!\n"
    .. "-----------------------------\n"
    .. "\n"
    .. "	Textures are being bound to texture units they are already bound to.\n"
    .. "	These calls are unnecessary, and can be omitted.\n"
    .. "\n"
    .. "	Superfluous calls to glBindTexture were detected in...\n"
    .. "\n"
    .. "	Region   : Calls   : Per frame (avg.)\n"
    .. "	-------------------------------------\n"

   for i=1,#regions do
    if count[i] ~= nil then
     msg = msg
      .. "\t"   .. string.format("%6d"  ,i)
      .. "   :" .. string.format("%6d"  ,count[i])
      .. "   :" .. string.format("%8.1f",count[i]*1.0/regions[i].NumFrames)
      .. "\n"
    end
   end

   if DrawNum ~= 0 then DrawNum = DrawNum - 1 end

   annotateSuggestion(msg,FrameNum,DrawNum)
  end

  FEnd()
 end



 function check_GL_active_texture()
  FStart("check_GL_active_texture")

  local prev     = false
  local detected = false
  local FrameNum = 0
  local  DrawNum = 0

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   if call.CallName == "glActiveTexture" then
    if prev then
     detected = true
     FrameNum = call.FrameNum
      DrawNum = call. DrawNum
    else
     prev = true
    end
   else
    prev = false
   end

  end end

  if detected then
   local msg =
       "-------------------------------\n"
    .. "UNNECESSARY ACTIVE TEXTURE CALL\n"
    .. "-------------------------------\n"
    .. "Consecutive calls to glActiveTexture simply overwrite the state without using it.\n"
    .. "Remove this call.\n"

   if DrawNum ~= 0 then DrawNum = DrawNum - 1 end

   annotateSuggestion(msg,FrameNum,DrawNum)
  end

  FEnd()
 end



-- check for redundant calls that does not need params for detection
 function check_GL_redundancy()
  FStart("check_GL_redundancy")

-- vars for redundant glViewport calls
  local viewport = false

-- vars for redundant glScissor calls
  local scissor = false

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

-- checking for redundant glViewport calls
   if call.CallName == "glViewport" then
    if viewport then
     annotateSuggestion("This call to glViewport is redundant. You can safely remove it",call.FrameNum,call.DrawNum-1)
    else
     viewport = true;
    end
   else
    viewport = false
   end

-- checking for redundant glScissor calls
   if call.CallName == "glScissor" then
    if scissor then
     annotateSuggestion("This call to glScissor is redundant. You can safely remove it",call.FrameNum,call.DrawNum-1)
    else
     scissor = true;
    end
   else
    scissor = false
   end

  end end

  FEnd()
 end



-- check for redundant calls that *do* need params for detection
 function check_GL_redundancy_param()
  if not hasParams then return end

  FStart("check_GL_redundancy_param")

-- vars for redundant glEnable / glDisable
  local enable_arg    = -1

-- vars for redundant glEnablei / glDisablei
  local enablei_arg   = -1
  local enablei_index = -1

-- vars for redundanct glScissor after glViewport
  local viewport_x    = -1
  local viewport_y    = -1
  local viewport_w    = -1
  local viewport_h    = -1

-- vars for redundanct glBlendFunc
  local blendfunc     = false


  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

-- checking for redundant glEnable / glDisable
   if call.CallName == "glEnable" or call.CallName == "glDisable" then
    if enable_arg == call.ValueTab[2] then
     annotateSuggestion("This call to " .. call.CallName .. " is redundant. You can safely remove it",call.FrameNum,call.DrawNum-1)
    else
     enable_arg = call.ValueTab[2]
    end
   else
    enable_arg = -1
   end

-- checking for redundant glEnablei / glDisablei
   if call.CallName == "glEnablei" or call.CallName == "glDisablei" then
    if enablei_arg == call.ValueTab[2] and enablei_index == call.ValueTab[3] then
     annotateSuggestion("This call to " .. call.CallName .. " is redundant. You can safely remove it",call.FrameNum,call.DrawNum-1)
    else
     enablei_arg   = call.ValueTab[2]
     enablei_index = call.ValueTab[3]
    end
   else
    enablei_arg  = -1
    enable_index = -1
   end

-- checking for redundant glScissor after glViewport
   if call.CallName == "glViewport" then
    viewport_x = call.ValueTab[2]
    viewport_y = call.ValueTab[3]
    viewport_w = call.ValueTab[4]
    viewport_h = call.ValueTab[5]
   elseif call.CallName == "glScissor" then
    if viewport_x == call.ValueTab[2] and viewport_y == call.ValueTab[3] and viewport_w == call.ValueTab[4] and viewport_h == call.ValueTab[5] then
     annotateSuggestion("Calling glScissor after glViewport with the same arguments is redundant",call.FrameNum,call.DrawNum)
    end
   end

-- checking for redundant glBlendFunc
   if call.CallName == "glBlendFunc" then
    if blendfunc then
     annotateSuggestion("Consecutive calls to glBlendFunc. The first call is redundant. You can safely remove it.",call.FrameNum,call.DrawNum)
    else
     blendfunc=true
    end
   end

  end end

  FEnd()
 end



-- Check for glDelete* bad handle and for unmatched glCreate* and using an uninitialised Program or Shader handle
 function check_GL_prog_shad_life()
  if not hasParams then return end

  FStart("check_GL_prog_shad_life")

  local calls_program = {
    "glIsProgram"
   ,"glUseProgram"
   ,"glLinkProgram"
   ,"glAttachShader"
   ,"glGetActiveAttrib"
   ,"glGetActiveUniform"
   ,"glGetAttachedShaders"
   ,"glGetAttribLocation"
   ,"glGetProgram"
   ,"glGetProgramInfoLog"
   ,"glGetUniformfv"
   ,"glGetUniformiv"
   ,"glGetUniformuiv"
   ,"glGetUniformdv"
   ,"glGetnUniformfv"
   ,"glGetnUniformiv"
   ,"glGetnUniformuiv"
   ,"glGetnUniformdv"
   ,"glGetUniformLocation"
   ,"glGetTransformFeedbackVaryings"
   ,"glTransformFeedbackVaryings"
  }

  local calls_shader_gen = {
    "glGetShader"
   ,"glGetShaderInfoLog"
   ,"glGetShaderSource"
   ,"glIsShader"
   ,"glCompileShader"
   ,"glShaderSource"
  }

  local calls_shader_prg = {
    "glAttachShader"
   ,"glDetachShader"
  }

-- The following strings make legitimate gl cammands when suffixed to "glCreate" and "glDelete". glCreateShaderProgramv will be treated as a special case
  local stubs = {}

  stubs["Program"]= 1
  stubs["Shader" ]= 2

  local count = {{},{}}

  function alive(stub,id)
   if stubs[stub] == nil then
    return false
   elseif count[stubs[stub]] == nil then
    return false
   elseif count[stubs[stub]].ids == nil then
    return false
   elseif count[stubs[stub]].ids[id] == nil then
    return false
   else
    return true
   end
  end

  function check(name,value,stub,id,frame,draw,msg)
   if name == value then
    if not alive(stub,id) then
     annotateIssue(msg,frame,draw)
    end
   end
  end

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   local val2 = call.ValueTab[2]
   local val3 = call.ValueTab[3]

   -- pretend that "glCreateShaderProgramv" is "glCreateShader" for Create/Delete accounting
   if name == "glCreateShaderProgramv" then name = "glCreateShader" end

   if name:find("glCreate") ~= nil then
    name = name:gsub("glCreate","") -- strip prefix

    local i = stubs[name]

    if i ~= nil then
     if count[i].ids == nil then count[i].ids = {} end

     local id = call.ValueTab[1]

     if count[i].ids[id] == nil then
      count[i].ids[id]       = {}
      count[i].ids[id].count = 1
      count[i].ids[id].name  = name
      count[i].ids[id].frame = frame
      count[i].ids[id].draw  = draw
      count[i].ids[id].len   = call.ValueTab[1]
     else
      count[i].ids[id].count = count[i].ids[id].count + 1
     end
    end
   end

   if name:find("glDelete") ~= nil then
    name = name:gsub("glDelete","") -- strip prefix

    local i = stubs[name]

    if i ~= nil then
     if count[i].ids == nil then count[i].ids = {} end

     local id = call.ValueTab[2]

     if count[i].ids[id] == nil then
      annotateIssue("Bad argument passed to glDelete" .. name,frame,draw)
     else
      count[i].ids[id].count = count[i].ids[id].count - 1

      if count[i].ids[id].len ~= val2 then
       annotateIssue("Objects created with one length but deleted with another",frame,draw)
      end

      -- this next line prevents the feedback engine from missing the case where a user 'deletes' a resource which doesn't exist
      -- and then 'fortuitously' creating a resource which returns the same handle erroneously deleted, and then never deleting
      -- the correctly allocated handle
      --
      -- I.e...
      --
      -- GLuint id = 1; glDeleteProgram(1); // Wrong! Program '1' was never created.
      --        id =    glCreateProgram( ); // Unlucky if id == 1 after this call.
      --
      -- Simple reference counting will not pick this up because it has the correct zero sum of creats and deletes
      --
      if count[i].ids[id].count == 0 then count[i].ids[id] = nil end
     end
    end
   end

   -- This section is for checking if the lifetimes we are tracking are being used correctly

   for _,v in pairs(calls_program   ) do check(name,v,"Program",val2,frame,draw,"No such (GL) program") end
   for _,v in pairs(calls_shader_gen) do check(name,v,"Shader" ,val2,frame,draw,"No such shader"      ) end
   for _,v in pairs(calls_shader_prg) do check(name,v,"Shader" ,val3,frame,draw,"No such shader"      ) end

  end end

  for i=1,#count do
   if count[i].ids ~= nil then
    for id,c in pairs(count[i].ids) do
     annotateIssue("A call to glCreate" .. c.name .. " was not matched by a call to glDelete" .. c.name,c.frame,c.draw)
    end
   end
  end

  FEnd()
 end



 function check_GL_wait_sync()
  if not hasParams then return end

  FStart("check_GL_wait_sync")

  local busy = false
  local wait = "?" -- previous call name

  for i=0,#frames do

   FProgress(i,#frames)

   for j=0,#frames[i].calls.data do

   local call  = frames[i].calls.data[j]

   local param = call.ValueTab -- param[1] is the return value
                               -- param[2] is the zeroth function parameter, etc.

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   if name == "glClientWaitSync" or name == "glWaitSync" then
    if prev == name then
     busy = true
    else
     prev = name
    end
   end

   end
  end

  if busy then
   annotateSuggestion("Consecutive calls to glClientWaitSync / glWaitSync. Is this a 'busy' wait? Is the timeout too short?",frame,draw)
  end

  FEnd()
 end



 function check_VK_map_unmap()
  FStart("check_VK_map_unmap")

  local total = 0

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   if name == "vkMapMemory" then
    total = total + 1
   end

   if name == "vkUnmapMemory" then
    total = total - 1

    if total < 0 then
     annotateIssue("You have unmapped memory not previously mapped",frame,draw)
    end
   end
  end end

  if total > 0 then
   annotateIssue("You have unmapped memory not previously mapped",0,0)
  end

  FEnd()
 end



 function check_VK_allocate_free()
  FStart("check_VK_allocate_free")

  local stubs = {}

  stubs["Memory"        ]=1
  stubs["DescriptorSets"]=2
  stubs["CommandBuffers"]=3

  local istubs = {} for k,v in pairs(stubs) do istubs[v]=k end -- istubs is the inverse map of stubs

  local count = {0,0,0}
  local frees = {0,0,0}

  local total = 0

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   if name:find("vkAllocate") then
    name = name:gsub("vkAllocate","")

    local k = stubs[name]

    if k ~= nil then
     total    = total    + 1
     count[k] = count[k] + 1

     if call.CallName == "vkAllocateDescriptorSet" and frees[k] ~= 0 then
      annotateSuggestion("Consider resuing descriptor sets",frame,draw)
     end
    end
   end

   if name:find("vkFree") then
    name = name:gsub("vkFree","")

    local k = stubs[name]

    if k ~= nil then
     total    = total    - 1
     count[k] = count[k] - 1
     frees[k] = frees[k] + 1

     if count[k] < 0 then
      annotateIssue("You have called vkFree" .. name .. " more times than vlAllocate" .. name,frame,draw)
     end
    end
   end

  end end

  for k=1,#count do
   if count[k]>0 then
    annotateIssue("You have called vkAllocate" .. istubs[k] .. " more times than vkFree" .. istubs[k],0,0)
   end
  end

  FEnd()
 end



 function check_VK_used_after_destroy() -- annotate if a handle is used after it is destroyed (works mid-trace!)
  FStart("check_VK_used_after_destroy")

  local stubs = {}

  stubs["Buffer"                  ]= 1
--stubs["BufferView"              ]= 2 -- Used only in structs
  stubs["CommandPool"             ]= 3
  stubs["DescriptorPool"          ]= 4
--stubs["DescriptorSetLayout"     ]= 5 -- Used only in structs
  stubs["DescriptorUpdateTemplate"]= 6
  stubs["Device"                  ]= 7
  stubs["DeviceMemory"            ]= 8
  stubs["Event"                   ]= 9
  stubs["Fence"                   ]=10
--stubs["FrameBuffer"             ]=11 -- Used only in structs
  stubs["Image"                   ]=12
--stubs["ImageView"               ]=13 -- Used only in structs
  stubs["Instance"                ]=14
  stubs["Pipeline"                ]=15
  stubs["PipelineCache"           ]=16
  stubs["PipelineLayout"          ]=17
  stubs["QueryPool"               ]=18
  stubs["RenderPass"              ]=19
--stubs["Sampler"                 ]=20 -- Used only in structs
--stubs["SamplerYcbcrConversion"  ]=21 -- Used only in structs
--stubs["Semaphore"               ]=22 -- Used only in structs
--stubs["ShaderModule"            ]=23 -- Used only in structs

  local cmd = {}

  cmd[stubs["Buffer"]] = {
   {"vkBindMemoryBuffer"           ,3},
   {"vkBindIndexBuffer"            ,3},
   {"vkCmdCopyBuffer"              ,3},
   {"vkCmdCopyBuffer"              ,4},
   {"vkCmdCopyBufferToImage"       ,3},
   {"vkCmdCopyImageToBuffer"       ,5},
   {"vkCmdCopyQueryPoolResults"    ,6},
   {"vkCmdDispatchIndirect"        ,3},
   {"vkCmdDrawIndexedIndirect"     ,3},
   {"vkCmdDrawIndirect"            ,3},
   {"vkCmdFillBuffer"              ,3},
   {"vkCmdUpdateBuffer"            ,3},
   {"vkGetBufferMemoryRequirements",3}
  }

  cmd[stubs["CommandPool"]] = {
   {"vkFreeCommandBuffers",3},
   {"vkResetCommandPool"  ,3},
   {"vkTrimCommandPool"   ,3},
   {"vkTrimCommandPoolKHR",3}
  }

  cmd[stubs["DescriptorPool"]] = {
   {"vkFreeDescriptorSets" ,3},
   {"vkResetDescriptorPool",3}
  }

  cmd[stubs["DescriptorUpdateTemplate"]] = {
   {"vkUpdateDescriptorSetWithTemplate",4}
  }

  cmd[stubs["Device"]] = {
   {"vkAllocateCommandBuffers"           ,2},
   {"vkAllocateDescriptorSets"           ,2},
   {"vkAllocateMemory"                   ,2},
   {"vkBindBufferMemory"                 ,2},
   {"vkBindBufferMemory2"                ,2},
   {"vkBindImageMemory"                  ,2},
   {"vkBindImageMemory2"                 ,2},
   {"vkCreateBuffer"                     ,2},
   {"vkCreateBufferView"                 ,2},
   {"vkCreateCommandPool"                ,2},
   {"vkCreateComputePipelines"           ,2},
   {"vkCreateDescriptorPool"             ,2},
   {"vkCreateDescriptorSetLayout"        ,2},
   {"vkCreateDescriptorUpdateTemplate"   ,2},
   {"vkCreateEvent"                      ,2},
   {"vkCreateFence"                      ,2},
   {"vkCreateFramebuffer"                ,2},
   {"vkCreateGraphicsPipeline"           ,2},
   {"vkCreateImage"                      ,2},
   {"vkCreateImageView"                  ,2},
   {"vkCreatePipelineCache"              ,2},
   {"vkCreatePipelineLayout"             ,2},
   {"vkCreateQueryPool"                  ,2},
   {"vkCreateRenderPass"                 ,2},
   {"vkCreateSampler"                    ,2},
   {"vkCreateSamplerYcbcrConversion"     ,2},
   {"vkCreateSemaphore"                  ,2},
   {"vkCreateShaderModule"               ,2},
   {"vkDestroyBuffer"                    ,2},
   {"vkDestroyBufferView"                ,2},
   {"vkDestroyCommandPool"               ,2},
   {"vkDestroyDescriptorPool"            ,2},
   {"vkDestroyDescriptorSetLayout"       ,2},
   {"vkDestroyDescriptorUpdateTemplate"  ,2},
   {"vkDestroyDevice"                    ,2},
   {"vkDestroyEvent"                     ,2},
   {"vkDestroyFence"                     ,2},
   {"vkDestroyFramebuffer"               ,2},
   {"vkDestroyImage"                     ,2},
   {"vkDestroyImageView"                 ,2},
   {"vkDestroyPipeline"                  ,2},
   {"vkDestroyPipelineCache"             ,2},
   {"vkDestroyPipelineLayout"            ,2},
   {"vkDestroyQueryPool"                 ,2},
   {"vkDestroyRenderPass"                ,2},
   {"vkDestroySampler"                   ,2},
   {"vkDestroySamplerYcbcrConversion"    ,2},
   {"vkDestroySemaphore"                 ,2},
   {"vkDestroyShaderModule"              ,2},
   {"vkDeviceWaitIdle"                   ,2},
   {"vkFlushMappedMemoryRanges"          ,2},
   {"vkFreeCommandBuffers"               ,2},
   {"vkFreeDescriptorSets"               ,2},
   {"vkFreeMemory"                       ,2},
   {"vkGetBufferMemoryRequirements"      ,2},
   {"vkGetBufferMemoryRequirements2"     ,2},
   {"vkGetDescriptorSetLayoutSupport"    ,2},
   {"vkGetDeviceGroupPeerMemoryFeatures" ,2},
   {"vkGetDeviceMemoryCommitment"        ,2},
   {"vkGetDeviceProcAddr"                ,2},
   {"vkGetDeviceQueue"                   ,2},
   {"vkGetDeviceQueue2"                  ,2},
   {"vkGetEventStatus"                   ,2},
   {"vkGetFenceStatus"                   ,2},
   {"vkGetImageMemoryRequirements"       ,2},
   {"vkGetImageMemoryRequirements2"      ,2},
   {"vkGetImageSparseMemoryRequirements" ,2},
   {"vkGetImageSparseMemoryRequirements2",2},
   {"vkGetImageSubresourceLayout"        ,2},
   {"vkGetPipelineCacheData"             ,2},
   {"vkGetQueryPoolResults"              ,2},
   {"vkGetRenderAreaGranularity"         ,2},
   {"vkInvalidateMappedMemoryRanges"     ,2},
   {"vkMapMemory"                        ,2},
   {"vkMergePipelineCaches"              ,2},
   {"vkResetCommandPool"                 ,2},
   {"vkResetDescriptorPool"              ,2},
   {"vkResetEvent"                       ,2},
   {"vkResetFences"                      ,2},
   {"vkSetEvent"                         ,2},
   {"vkTrimCommandPool"                  ,2},
   {"vkUnmapMemory"                      ,2},
   {"vkUpdateDescriptorSetWithTemplate"  ,2},
   {"vkUpdateDescriptorSets"             ,2},
   {"vkWaitForFences"                    ,2},
  }

  cmd[stubs["DeviceMemory"]] = {
   {"vkAllocateMemory"           ,2},
   {"vkBindBufferMemory"         ,4},
   {"vkBindImageMemory"          ,4},
   {"vkGetMemoryDeviceCommitment",3},
   {"vkMapMemory"                ,3},
   {"vkUnmapMemory"              ,3}
  }

  cmd[stubs["Event"]] = {
   {"vkCmdResetEvent" ,3},
   {"vkCmdSetEvent"   ,3},
   {"vkGetEventStatus",3},
   {"vkResetEvent"    ,3},
   {"vkSetEvent"      ,3}
  }

  cmd[stubs["Fence"]] = {
   {"vkGetFenceStatus" ,3},
   {"vkQueueBindSparse",5},
   {"vkQueueSubmit"    ,5}
  }

  cmd[stubs["Image"]] = {
   {"vkBindImageMemory"                 ,3},
   {"vkCmdBlitImage"                    ,3},
   {"vkCmdBlitImage"                    ,5},
   {"vkCmdClearColorImage"              ,3},
   {"vkCmdClearDepthStencilImage"       ,3},
   {"vkCmdCopyBufferToImage"            ,4},
   {"vkCmdCopyImage"                    ,3},
   {"vkCmdCopyImageToBuffer"            ,3},
   {"vkCmdResolveImage"                 ,3},
   {"vkCmdResolveImage"                 ,5},
   {"vkGetImageMemoryRequirements"      ,3},
   {"vkGetImageSparseMemoryRequirements",3},
   {"vkGetImageSubresourceLayout"       ,3}
  }

  cmd[stubs["Instance"]] = {
   {"vkEnumeratePhysicalDeviceGroups",2},
   {"vkEnumeratePhysicalDevices"     ,2},
  }

  cmd[stubs["Pipeline"]] = {
   {"vkCmdBindPipeline",4}
  }

  cmd[stubs["PipelineCache"]] = {
   {"vkCreateComputePipelines" ,3},
   {"vkCreateGraphicsPipelines",3},
   {"vkGetPipelineCacheData"   ,3},
   {"vkMergePipelineCaches"    ,3}
  }

  cmd[stubs["PipelineLayout"]] = {
   {"vkCmdDescriptorSets",4},
   {"vkCmdPushConstants" ,3}
  }

  cmd[stubs["QueryPool"]] = {
   {"vkCmdBeginQuery"          ,3},
   {"vkCmdCopyQueryPoolResults",3},
   {"vkCmdEndQuery"            ,3},
   {"vkCmdRestQueryPool"       ,3},
   {"vkCmdWriteTimestamp"      ,4},
   {"vkGetQueryPoolResults"    ,3}
  }

  cmd[stubs["RenderPass"]] = {
   {"vkGetRenderAreaGranularity",3}
  }

  local istubs    = {}
  local destroyed = {}

  for name,index in pairs(stubs) do
   istubs   [index] = name -- istubs is the inverse map of stubs
   destroyed[index] = {}   -- destroy contains lists of handles that have been destroyed
  end

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   if name:find("vkDestroy") then
    name = name:gsub("vkDestroy","")

    local k = stubs[name]

    if k ~= nil then
     table.insert(destroyed[k],call.ValueTab[2])
    end

   else

    for k,v in pairs(cmd) do
     for k1,v1 in pairs(v) do
      if v1[1] == name then
       local param  = v1[2]
       local handle = call.ValueTab[param]

       for kd,vd in pairs(destroyed[k]) do
        if handle == vd then
         annotateIssue("You have used a Vk" .. istubs[k] .. " opaque handle in a call to " .. name .. " after it had been destroyed with vkDestroy" .. istubs[k],frame,draw)
        end
       end
      end
     end
    end

   end
  end end

  FEnd()
 end



 function check_VK_count_resolves()
  FStart("check_VK_count_resolves")

  local count = 0

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   if name == "vkCmdResolveImage" then count = count + 1 end

  end end

  if count ~= 0 then
   annotateInformation("There were " .. string.format("%f",(count*1.0/#frames)) .. " resolves per frame, on average. (" .. string.format("%d",count) .. " resolves in " .. string.format("%d",#frames) .. " frames)",0,0)
  end

  FEnd()
 end



 function check_VK_create_destroy() -- does not use parameters
  FStart("check_VK_create_destroy")

  local stubs = {}

  stubs["Instance"              ]= 1
  stubs["Device"                ]= 2
  stubs["Fence"                 ]= 3
  stubs["Semaphore"             ]= 4
  stubs["Event"                 ]= 5
  stubs["QueryPool"             ]= 6
  stubs["Buffer"                ]= 7
  stubs["BufferView"            ]= 8
  stubs["Image"                 ]= 9
  stubs["ImageView"             ]=10
  stubs["ShaderModule"          ]=11
  stubs["PipelineCache"         ]=12
--stubs["Pipeline"              ]=?? -- A parameter passed to create functions specifies the number of pipelines created
  stubs["PipelineLayout"        ]=13
  stubs["Sampler"               ]=14
  stubs["DescriptorSetLayout"   ]=15
  stubs["DescriptorPool"        ]=16
  stubs["Framebuffer"           ]=17
  stubs["RenderPass"            ]=18
  stubs["CommandPool"           ]=19
  stubs["SurfaceKHR"            ]=20
--stubs["SwapchainKHR"          ]=?? -- Waters muddied by vkCreateSharedSwapChainsKHR
  stubs["DebugReportCallbackEXT"]=21

  local istubs = {} for k,v in pairs(stubs) do istubs[v]=k end -- istubs is the inverse map of stubs

  local count = {0,0,0,0,0,
                 0,0,0,0,0,
                 0,0,0,0,0,
                 0,0,0,0,0,
                 0} -- 21 count objects to match stubs[]

  local total = 0 -- tracks the vkCreate/vkDestroy functions at large

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   if name ~= "vkCreateGraphicsPipeines" and
      name ~= "vkCreateComputePipelines"
   then
    if name:find("vkCreate") and name:find("SurfaceKHR") then name = "vkCreateSurfaceKHR" end


    if name:find("vkCreate") then
     name = name:gsub("vkCreate","")

     local k = stubs[name]

     if k ~= nil then
      total = total + 1

      count[k] = count[k] + 1
     end
    end


    if name:find("vkDestroy") then
     name = name:gsub("vkDestroy","")

     local k = stubs[name]

     if k ~= nil then
      total = total - 1

      if total < 0 then -- it is a handy check to count the total number of objects crated and destroyed
--     print("ERROR: you have vkDestroy'd an object never created")
      end

      count[k] = count[k] - 1

      if count[k] < 0 then
       annotateIssue("You have called vkDestroy" .. name .. " more times than vkCreate" .. name,frame,draw)
      end
     end
    end

   end

  end end

  if total > 0 then -- variable 'total' is a sanity check for debugging
-- print("ERROR: you have destroyed more than you have created")
  else
-- print("Congratulations: you have destroyed everything you have created")
  end

  for k=1,#count do
   if count[k] > 0 then
    annotateIssue("You have called vkCreate" .. istubs[k] .. " more times than the destroy function",0,0)
   end
  end

  FEnd()
 end



 function check_VK_empty_renderpass()
  FStart("check_VK_empty_renderpass")

  local prev = ""

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name = call.CallName

   if name == "vkCmdEndRenderPass" and prev == "vkCmdBeginRenderPass" then
    annotateSuggestion("A vkCmdEndRenderPass was detected immediately after a vkCmdBeginRenderPass. Empty render passes are an unnecessary performance overhead.",call.FrameNum,call.DrawNum)
   end

   prev = name
  end end

  FEnd()
 end



 function check_VK_image_layout()
-- this function checks whenever a VkImageLayout is passed as a top level parameter to a Vulkan API call,
-- but it does not check when a VkImageLayout is a data member of a structure parameter
  if not hasParams then return end

  FStart("check_VK_image_layout")

  local call = {}

  function check(i)
   if call.ValueTab[i] == VK_IMAGE_LAYOUT_GENERAL then
    annotateSuggestion("VK_IMAGE_LAYOUT_GENERAL is suboptimal",call.FrameNum,call.DrawNum)
   end
  end

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   call = frames[i].calls.data[j]

   local  name = call.CallName

   if     name == "vkCmdBlitImage"              then check(4) check(6)
   elseif name == "vkCmdClearColorImage"        then check(4)
   elseif name == "vkCmdClearDepthStencilImage" then check(4)
   elseif name == "vkCmdCopyBufferToImage"      then check(5)
   elseif name == "vkCmdCopyImage"              then check(4) check(6)
   elseif name == "vkCmdCopyImageToBuffer"      then check(4)
   elseif name == "vkCmdResolveImage"           then check(4) check(6)
   end
  end end

  FEnd()
 end



 function check_VK_renderpass()
  -- Checks for pipeline barriers within renderpasses (Can you use a subpass dependency instead?)

  if not hasParams then return end

  FStart("check_VK_renderpass")

  local inside = false

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   if name == "vkCmdBeginRenderPass" then
    inside = true
   elseif name == "vkCmdEndRenderPass" then
    inside = false
   elseif name == "vkCmdPipelineBarrier" then
    if inside then
     annotateSuggestion("You are using pipeline barriers inside render passes. Could you use a subpass dependency instead?",frame,draw)
    end
   elseif name:find("vkCmdDraw") ~= nil then
    if not inside then
     annotateIssue(name .. " must not be called outside a render pass",frame,draw)
    end
   elseif name == "vkCmdWaitEvents" then
    if inside then
     annotateIssue(name .. " must not be called inside a render pass",frame,draw)
    end
   end

  end end

  FEnd()
 end



 function check_VK_command_buffers()
  -- Checks for vkCmd* function outside vk[Begin|End]CommandBuffer

  if not hasParams then return end

  FStart("check_VK_command_buffers")

  local inside = false

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   if name == "vkBeginCommandBuffer" then
    inside = true
   elseif name == "vkEndCommandBuffer" then
    inside = false
   elseif name:find("vkCmd") ~= nil then
    if not inside then
     annotateIssue(name .. " must not be called outside a command buffer",frame,draw)
    end
   end

  end end

  FEnd()
 end



 function check_VK_bad_arrays()
  -- Checks for nullptr buffers that are supposed to be finite length

  if not hasParams then return end

  FStart("check_VK_bad_arrays")

  local arrays = {}

  arrays["vkBindBufferMemory2"           ] = {{1,2}}
  arrays["vkBindBufferMemory2KHR"        ] = {{1,2}}
  arrays["vkBindImageMemory2"            ] = {{1,2}}
  arrays["vkBindImageMemory2KHR"         ] = {{1,2}}
  arrays["vkCmdBindDescriptorSets"       ] = {{4,5},{6,7}}
  arrays["vkCmdBindVertexBuffers"        ] = {{2,3},{2,4}}
  arrays["vkCmdBlitImage"                ] = {{5,6}}
  arrays["vkCmdClearAttachments"         ] = {{1,2},{3,4}}
  arrays["vkCmdClearColorImage"          ] = {{4,5}}
  arrays["vkCmdClearDepthStencilImage"   ] = {{4,5}}
  arrays["vkCmdCopyBuffer"               ] = {{3,4}}
  arrays["vkCmdCopyBufferToImage"        ] = {{4,5}}
  arrays["vkCmdCopyImage"                ] = {{5,6}}
  arrays["vkCmdCopyImageToBuffer"        ] = {{4,5}}
  arrays["vkCmdExecuteCommands"          ] = {{1,2}}
  arrays["vkCmdPipelineBarrier"          ] = {{4,5},{6,7},{8,9}}
  arrays["vkCmdPushConstants"            ] = {{4,5}}
  arrays["vkCmdResolveImage"             ] = {{5,6}}
  arrays["vkCmdSetViewport"              ] = {{2,3}}
  arrays["vkCmdUpdateBuffer"             ] = {{3,4}}
  arrays["vkCmdWaitEvents"               ] = {{1,2},{5,6},{7,8},{9,10}}
  arrays["vkCreateComputePipelines"      ] = {{2,3},{2,5}}
  arrays["vkCreateGraphicsPipelines"     ] = {{2,3},{2,5}}
  arrays["vkFlushMappedMemoryRanges"     ] = {{1,2}}
  arrays["vkFreeCommandBuffers"          ] = {{2,3}}
  arrays["vkFreeDescriptorSets"          ] = {{2,3}}
  arrays["vkGetQueryPoolResults"         ] = {{4,5}}
  arrays["vkInvalidateMappedMemoryRanges"] = {{1,2}}
  arrays["vkMergePipelineCaches"         ] = {{2,3}}
  arrays["vkQueueBindSparse"             ] = {{1,2}}
  arrays["vkQueueSubmit"                 ] = {{1,2}}
  arrays["vkResetFences"                 ] = {{1,2}}
  arrays["vkUpdateDescriptorSets"        ] = {{1,2},{3,4}}
  arrays["vkWaitForFences"               ] = {{1,2}}

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   local a = arrays[name]

   if a ~= nil then
    for i=1,#a do
     local count_index = a[i][1]+2 -- because: +1 for 1-based to 0-based, and +1 for return value treated as an extra parameter
     local  data_index = a[i][2]+2 -- ditto

     local num = call.ValueTab[count_index] -- this is the count of the 'C' array
     local dat = call.ValueTab[ data_index] -- this is the buffer pointer

     if num ~= 0 and dat == 0 then
      annotateIssue("Finite count parameter, but null pointer to array passed in call to " .. name,frame,draw)
     end
    end
   end
  end end

  FEnd()
 end



 function check_VK_bad_pointers()
  -- Checks for null pointers to structures that are never supposed to be null
  -- excludes arrays because they are treated in check_VK_bad_arrays

  if not hasParams then return end

  FStart("check_VK_bad_Pointers")

  local arrays = {}

  arrays["vkAllocateCommandBuffers"                          ] = {1,2}
  arrays["vkAllocateDescriptorSets"                          ] = {1,2}
  arrays["vkAllocateMemory"                                  ] = {1,3}
  arrays["vkBeginCommandBuffer"                              ] = {1}
  arrays["vkCmdBeginRenderPass"                              ] = {1}
  arrays["vkCmdClearColorImage"                              ] = {3,5}
  arrays["vkCmdClearDepthStencilImage"                       ] = {3}
  arrays["vkCmdSetBlendConstants"                            ] = {1}
  arrays["vkCreateBuffer"                                    ] = {1,3}
  arrays["vkCreateBufferView"                                ] = {1,3}
  arrays["vkCreateCommandPool"                               ] = {1,3}
  arrays["vkCreateDescriptorPool"                            ] = {1,3}
  arrays["vkCreateDescriptorSetLayout"                       ] = {1,3}
  arrays["vkCreateDescriptorUpdateTemplate"                  ] = {1,3}
  arrays["vkCreateDescriptorUpdateTemplateKHR"               ] = {1,3}
  arrays["vkCreateDevice"                                    ] = {1,3}
  arrays["vkCreateEvent"                                     ] = {1,3}
  arrays["vkCreateFence"                                     ] = {1,3}
  arrays["vkCreateFramebuffer"                               ] = {1,3}
  arrays["vkCreateImage"                                     ] = {1,3}
  arrays["vkCreateImageView"                                 ] = {1,3}
  arrays["vkCreateInstance"                                  ] = {0,2}
  arrays["vkCreatePipelineCache"                             ] = {1,3}
  arrays["vkCreatePipelineLayout"                            ] = {1,3}
  arrays["vkCreateQueryPool"                                 ] = {1,3}
  arrays["vkCreateRenderPass"                                ] = {1,3}
  arrays["vkCreateSampler"                                   ] = {1,3}
  arrays["vkCreateSamplerYcbcrConversion"                    ] = {1,3}
  arrays["vkCreateSamplerYcbcrConversionKHR"                 ] = {1,3}
  arrays["vkCreateSemaphore"                                 ] = {1,3}
  arrays["vkCreateShaderModule"                              ] = {1,3}
  arrays["vkEnumerateInstanceExtensionProperties"            ] = {1,2}
  arrays["vkEnumerateDeviceLayerProperties"                  ] = {1}
  arrays["vkEnumerateInstanceLayerProperties"                ] = {0}
  arrays["vkEnumerateInstanceVersion"                        ] = {0}
  arrays["vkEnumeratePhysicalDeviceGroups"                   ] = {1}
  arrays["vkEnumeratePhysicalDeviceGroupsKHR"                ] = {1}
  arrays["vkEnumeratePhysicalDevices"                        ] = {1}
  arrays["vkGetBufferMemoryRequirements"                     ] = {2}
  arrays["vkGetBufferMemoryRequirements2"                    ] = {1,2}
  arrays["vkGetBufferMemoryRequirements2KHR"                 ] = {1,2}
  arrays["vkGetDescriptorSetLayoutSupport"                   ] = {1,2}
  arrays["vkGetDescriptorSetLayoutSupportKHR"                ] = {1,2}
  arrays["vkGetDeviceGroupPeerMemoryFeatures"                ] = {4}
  arrays["vkGetDeviceMemoryCommitment"                       ] = {2}
  arrays["vkGetDeviceProcAddr"                               ] = {1}
  arrays["vkGetDeviceQueue"                                  ] = {3}
  arrays["vkGetDeviceQueue2"                                 ] = {1,2}
  arrays["vkGetImageMemoryRequirements"                      ] = {2}
  arrays["vkGetImageMemoryRequirements2"                     ] = {1,2}
  arrays["vkGetImageMemoryRequirements2KHR"                  ] = {1,2}
  arrays["vkGetImageSparseMemoryRequirements"                ] = {2,3}
  arrays["vkGetImageSparseMemoryRequirements2"               ] = {1,2,3}
  arrays["vkGetImageSparseMemoryRequirements2KHR"            ] = {1,2,3}
  arrays["vkGetImageSubresourceLayout"                       ] = {2,3}
  arrays["vkGetInstanceProcAddr"                             ] = {1}
  arrays["vkGetPhysicalDeviceExternalBufferProperties"       ] = {1,2}
  arrays["vkGetPhysicalDeviceExternalBufferPropertiesKHR"    ] = {1,2}
  arrays["vkGetPhysicalDeviceExternalFenceProperties"        ] = {1,2}
  arrays["vkGetPhysicalDeviceExternalFencePropertiesKHR"     ] = {1,2}
  arrays["vkGetPhysicalDeviceExternalSemaphoreProperties"    ] = {1,2}
  arrays["vkGetPhysicalDeviceExternalSemaphorePropertiesKHR" ] = {1,2}
  arrays["vkGetPhysicalDeviceFeatures"                       ] = {1}
  arrays["vkGetPhysicalDeviceFeatures2"                      ] = {1}
  arrays["vkGetPhysicalDeviceFeatures2KHR"                   ] = {1}
  arrays["vkGetPhysicalDeviceFormatProperties"               ] = {2}
  arrays["vkGetPhysicalDeviceFormatProperties2"              ] = {2}
  arrays["vkGetPhysicalDeviceFormatProperties2KHR"           ] = {2}
  arrays["vkGetPhysicalDeviceImageFormatProperties"          ] = {6}
  arrays["vkGetPhysicalDeviceImageFormatProperties2"         ] = {1,2}
  arrays["vkGetPhysicalDeviceImageFormatProperties2KHR"      ] = {1,2}
  arrays["vkGetPhysicalDeviceMemoryProperties"               ] = {1}
  arrays["vkGetPhysicalDeviceMemoryProperties2"              ] = {1}
  arrays["vkGetPhysicalDeviceMemoryProperties2KHR"           ] = {1}
  arrays["vkGetPhysicalDeviceProperties"                     ] = {1}
  arrays["vkGetPhysicalDeviceProperties2"                    ] = {1}
  arrays["vkGetPhysicalDeviceProperties2KHR"                 ] = {1}
  arrays["vkGetPhysicalDeviceQueueFamilyProperties"          ] = {1}
  arrays["vkGetPhysicalDeviceQueueFamilyProperties2"         ] = {1,2}
  arrays["vkGetPhysicalDeviceQueueFamilyProperties2KHR"      ] = {1,2}
  arrays["vkGetPhysicalDeviceSparseImageFormatProperties"    ] = {6}
  arrays["vkGetPhysicalDeviceSparseImageFormatProperties2"   ] = {1,2}
  arrays["vkGetPhysicalDeviceSparseImageFormatProperties2KHR"] = {1,2}
  arrays["vkGetRenderAreaGranularity"                        ] = {2}
  arrays["vkMapMemory"                                       ] = {5}
  arrays["vkUpdateDescriptorSetWithTemplate"                 ] = {3}
  arrays["vkUpdateDescriptorSetWithTemplateKHR"              ] = {3}

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   local a = arrays[name]

   if a ~= nil then
    for i=1,#a do
     local index = a[i]+2 -- +2 because: +1 for 0-based to 1-based, and +1 for return value treated as a parameter

     local buf = call.ValueTab[index] -- this is the pointer parameter value

     if buf == 0 then
      annotateIssue("Pointer parameter " .. string.format("%d",index-1) .. "not allowed to be null in call to " .. name,frame,draw)
     end
    end
   end
  end end

  FEnd()
 end



 function check_VK_result()
  -- Checks the VKResult returned from API functions and reports interesting values

  if not hasParams then return end

  FStart("check_VK_result")

  local result = {} -- Vulkan functions return a VkResult

  result["vkAllocateCommandBuffers"                    ] = true
  result["vkAllocateDescriptorSets"                    ] = true
  result["vkAllocateMemory"                            ] = true
  result["vkBeginCommandBuffer"                        ] = true
  result["vkBindBufferMenory"                          ] = true
  result["vkBindBufferMenory2"                         ] = true
  result["vkBindBufferMenory2KHR"                      ] = true
  result["vkBindImageMemory"                           ] = true
  result["vkBindImageMemory2"                          ] = true
  result["vkBindImageMemory2KHR"                       ] = true
  result["vkCreateBuffer"                              ] = true
  result["vkCreateBufferView"                          ] = true
  result["vkCreateCommandPool"                         ] = true
  result["vkCreateComputePipelines"                    ] = true
  result["vkCreateDescriptorPool"                      ] = true
  result["vkCreateDescriptorSetLayout"                 ] = true
  result["vkCreateDescriptorUpdateTemplate"            ] = true
  result["vkCreateDevice"                              ] = true
  result["vkCreateEvent"                               ] = true
  result["vkCreateFence"                               ] = true
  result["vkCreateFramebuffer"                         ] = true
  result["vkCreateGraphicsPipeline"                    ] = true
  result["vkCreateImage"                               ] = true
  result["vkCreateImageView"                           ] = true
  result["vkCreateInstance"                            ] = true
  result["vkCreatePipelineCache"                       ] = true
  result["vkCreatePipelineLayout"                      ] = true
  result["vkCreateQueryPool"                           ] = true
  result["vkCreateRenderPass"                          ] = true
  result["vkCreateSampler"                             ] = true
  result["vkCreateSamplerYcbcrConversion"              ] = true
  result["vkCreateSemaphore"                           ] = true
  result["vkCreateShaderModule"                        ] = true
  result["vkDeviceWaitIdle"                            ] = true
  result["vkEndCommandBuffer"                          ] = true
  result["vkEnumerateDeviceExtensionProperties"        ] = true
  result["vkEnumerateDeviceLayerProperties"            ] = true
  result["vkEnumerateInstanceVersion"                  ] = true
  result["vkEnumeratePhysicalDevices"                  ] = true
  result["vkFlushMappedMemoryRanges"                   ] = true
  result["vkFreeDescriptorSets"                        ] = true
  result["vkGetEventStatus"                            ] = true
  result["vkGetFenceStatus"                            ] = true
  result["vkGetPhysicalDeviceImageFormatProperties"    ] = true
  result["vkGetPhysicalDeviceImageFormatProperties2"   ] = true
  result["vkGetPhysicalDeviceImageFormatProperties2KHR"] = true
  result["vkGetPipelineCacheData"                      ] = true
  result["vkGetQueryPoolResults"                       ] = true
  result["vkInvalidateMappedMemoryRanges"              ] = true
  result["vkMapMemory"                                 ] = true
  result["vkMergePipelineCaches"                       ] = true
  result["vkQueueSubmit"                               ] = true
  result["vkQueueWaitIdle"                             ] = true
  result["vkResetCommandBuffer"                        ] = true
  result["vkResetCommandPool"                          ] = true
  result["vkResetDescriptorPool"                       ] = true
  result["vkResetEvent"                                ] = true
  result["vkResetFences"                               ] = true
  result["vkSetEvent"                                  ] = true
  result["vkWaitForFences"                             ] = true

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call  = frames[i].calls.data[j]

   local param = call.ValueTab -- param[1] is the return value
                               -- param[2] is the zeroth function parameter, etc.

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   if result[name] ~= nil and param[1] < 0 then
    annotateIssue("VkResult returned error code, " .. VkResultNames[param[1]] .. " in call to " .. name,frame,draw)
   end
  end end

  FEnd()
 end



 function check_VK_fence_wait()
  if not hasParams then return end

  FStart("check_VK_fence_wait")

  local busy = false
  local prev = "?" -- previous call name

  for i=0,#frames do

   FProgress(i,#frames)

   for j=0,#frames[i].calls.data do

   local call  = frames[i].calls.data[j]

   local param = call.ValueTab -- param[1] is the return value
                               -- param[2] is the zeroth function parameter, etc.

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   if name == "vkWaitFences" then
    if prev == name then
     busy = true
    else
     prev = name
    end
   end

   end
  end

  if busy then
   annotateSuggestion("Consecutive calls to vkWaitFence. Is this a busy wait? Is the timeout too short?",frame,draw)
  end

  FEnd()
 end



 function check_VK_Valid_Usage()
  -- Implements the "Valid Usage" sections from the Vulkan specification document:
  --
  --  excludes bad array   checks : See check_VK_bad_arrays  ()
  --  excludes bad pointer checks : See check_VK_bad_pointers()
  --  excludes VkResult    checks : See check_VK_result      ()
  --
  -- If a SUGGESTION type annotation is emitted in what follows, the issue is not strictly "valid usage".
  -- The problem may be efficiency-related, for example, such as attempting to draw zero primitives with
  -- one of the vkCmdDraw* calls, This is actively supported by the spec but is not a useful call from an
  -- application point of view.
  --
  -- Because such issues were detected during a sweep of the spec, it is convenient to capture them here in one pass.

  if not hasParams then return end

  FStart("check_VK_Valid_Usage")

   -- internal function section...

   function check_image_layout(name,pname,layout,frame,draw,gen,special)
    -- this convenience function captures a number of common "layout must be one of" type checks of VkImageLayout parameters
    -- in Vulkan API functions
    --
    -- If you want it to allow VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, use gen = true
    --
    -- Special is some particular value you want to allow

    if (layout ~= VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR or not gen) and
        layout ~= VK_IMAGE_LAYOUT_GENERAL and
        layout ~= special then

     annotateIssue(pname .. " parameter is invalid : " .. string.format("%d",layout) .. " in call to " .. name,frame,draw)
    end
   end

   function check_image_layout_src(name,pname,layout,frame,draw,gen)
            check_image_layout    (name,pname,layout,frame,draw,gen,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
   end

   function check_image_layout_dst(name,pname,layout,frame,draw,gen)
            check_image_layout    (name,pname,layout,frame,draw,gen,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
   end


   function check_param_non_zero(name,pname,param,frame,draw)
    -- this convenience function captures a common pattern requiring a parameter to be strictly greater than 0

    if param == 0 then
     annotateIssue(pname .. " is not allowed to be 0 in call to " .. name,frame,draw)
    end
   end


   function check_param_non_null(name,pname,param,frame,draw)
    -- this convenience function for pointer parameters emits and issue level ennotation if the parameter is 0, i.e. null

    if param == 0 then
     annotateIssue(pname .. " must not be null in call to " .. name,frame,draw)
    end
   end


   function check_param_equals  (name,pname,param,frame,draw,target)
    -- this convenience function is for checking when a parameter must be equal to a certain value, 'target'

    if param ~= target then
     annotateIssue(pname .. " ( = " .. string.format("%d",param) .. " ) should be equal to " .. string.format("%d",target) .. " in call to " .. name,frame,draw)
    end
   end


   function check_multiple_of_4(name,pname,val,frame,draw,msg)
    if val & 3 ~= 0 then
     annotateIssue(pname .. " parameter must be a multiple of 4 in call to " .. name .. msg,frame,draw)
    end
   end


   function check_multiple_of_8(name,pname,val,frame,draw,msg)
    if val & 7 ~= 0 then
     annotateIssue(pname .. " parameter must be a multiple of 8 in call to " .. name .. msg,frame,draw)
    end
   end

   -- END internal function section


  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call  = frames[i].calls.data[j]

   local param = call.ValueTab -- param[1] is the return value not checked here
                               -- param[2] is the zeroth function parameter, etc.

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum


   -- This if / elseif block covers the entire Vulkan spec one command at a time, unless it is expedient to group them...
   if name == "vkBindBufferMemory2"    or
      name == "vkBindBufferMemory2KHR" or
      name == "vkBindImageMemory2"     or
      name == "vkBindImageMemory2KHR" then

    check_param_non_zero(name,"bindInfoCount",param[3],frame,draw)


   --=============================
   elseif name == "vkCmdBeginQuery" then

    local flags = param[5]

    if flags ~= VK_QUERY_CONTROL_PRECISE_BIT then -- At present there is only this one bit
     annotateIssue("Invalid flags passed to " .. name,frame,draw)
    end


   --=============================
   elseif name == "vkCmdBlitImage" then
    check_image_layout_src(name,"srcImageLayout" ,param[4],frame,draw,true)
    check_image_layout_dst(name,"dstImageLayout" ,param[6],frame,draw,true)
    check_param_non_zero  (name,"regionCount"    ,param[7],frame,draw)


   --=============================
   elseif name == "vkCmdClearAttachments" then
    check_param_non_zero  (name,"attachmentCount",param[3],frame,draw)
    check_param_non_zero  (name,"rectCount"      ,param[5],frame,draw)


   --=============================
   elseif name == "vkCmdClearColorImage" then
    check_image_layout_dst(name,"imageLayout"    ,param[4],frame,draw,true)
    check_param_non_zero  (name,"rangeCount"     ,param[6],frame,draw)


   --=============================
   elseif name == "vkCmdClearDepthStencilImage" then
    check_image_layout_dst(name,"imageLayout"    ,param[4],frame,draw,false)
    check_param_non_zero  (name,"rangeCount"     ,param[6],frame,draw)


   --=============================
   elseif name == "vkCmdCopyBuffer" then
    check_param_non_zero  (name,"regionCount"    ,param[5],frame,draw)


   --=============================
   elseif name == "vkCmdCopyBufferToImage" then
    check_image_layout_dst(name,"imageLayout"    ,param[5],frame,draw,true)
    check_param_non_zero  (name,"regionCount"    ,param[6],frame,draw)


   --=============================
   elseif name == "vkCmdCopyQueryPoolResults" then
    check_image_layout_dst(name,"imageLayout"    ,param[5],frame,draw,true)

    local dstOffset = param[7]
    local stride    = param[8]
    local flags     = param[9]

    if flags & VK_QUERY_RESULT_64_BIT then
     check_multiple_of_8(name,"dstOffset",dstOffset,frame,draw," if VK_QUERY_RESULT_64_BIT is set in the flags parameter")
     check_multiple_of_8(name,"stride"   ,stride   ,frame,draw," if VK_QUERY_RESULT_64_BIT is set in the flags parameter")
    else
     check_multiple_of_4(name,"dstOffset",dstOffset,frame,draw," if VK_QUERY_RESULT_64_BIT is unset in the flags parameter")
     check_multiple_of_4(name,"stride"   ,stride   ,frame,draw," if VK_QUERY_RESULT_64_BIT is unset in the flags parameter")
    end


   --=============================
   elseif name == "vkCmdDispatchIndirect" then
    local offset = param[4]

    check_multiple_of_4(name,"offset",offset,frame,draw,"")


   --=============================
   elseif name == "vkCmdDraw" then
    if param[3] * param[5] == 0 then -- if vertices * instances == 0
     annotateSuggestion(name .. ": There is nothing to draw, because vertexCount * instanceCount == 0",frame,draw)
    end


   --=============================
   elseif name == "vkCmdDrawIndexed" then
    if param[3] * param[4] == 0 then -- if indices * instances == 0
     annotateSuggestion(name .. ": There is nothing to draw, because indexCount * instanceCount == 0",frame,draw)
    end


   --=============================
   elseif name == "vkCmdDrawIndexedIndirect" or
          name == "vkCmdDrawIndirect" then
    local offset    = param[3]
    local drawCount = param[4]
    local stride    = param[5]

    check_multiple_of_4(name,"offset",offset,frame,draw,"")

    if drawCount == 0 then
     annotateSuggestion("Nothing to draw in call to " .. name .. " (drawCount parameter is 0)",frame,draw)
    elseif drawCount > 1 then
     check_multiple_of_4(name,"stride",stride,frame,draw," because the drawCount parameter is greater than 1")
    end


   --=============================
   elseif name == "vkCmdExecuteCommands" then
    check_param_non_zero(name,"commandBufferCount",param[3],frame,draw)


   --=============================
   elseif name == "vkCmdFillBuffer" then
    local size = param[5]

    check_param_non_zero(name,"size",size,frame,draw)
    check_multiple_of_4 (name,"size",size,frame,draw,"")


   --=============================
   elseif name == "vkCmdPipelineBarrier" then
    check_param_non_zero(name,"srcStageMask",param[3],frame,draw)
    check_param_non_zero(name,"dstStageMask",param[4],frame,draw)


   --=============================
   elseif name == "vkCmdPushConstants" then
    local stageFlags = param[4]
    local offset     = param[5]
    local size       = param[6]

    check_param_non_zero(name,"stageFlags",stageFlags,frame,draw)

    if stageFlags ~=0 and stageFlags & ALL_VkShaderStageFlagBits == 0 then
     annotateIssue("stageFlags parameter has spurious bits set in call to " .. name,frame,draw)
    end

    check_multiple_of_4(name,"offset",offset,frame,draw,"")
    check_multiple_of_4(name,"size"  ,size  ,frame,draw,"")


   --=============================
   elseif name == "vkCmdResetEvent" then
    local stageFlags = param[4]

    check_param_non_zero(name,"stageFlags",stageFlags,frame,draw)

    if stageFlags ~=0 then
     if stageFlags & ALL_VkShaderStageFlagBits == 0 then
      annotateIssue("stageFlags parameter has spurious bits set in call to " .. name,frame,draw)
     end

     if stageFlags & ALL_VkShaderStageFlagBits ~= 0 then
      annotateIssue("stageFlags must not have VK_SHADER_STAGE_HOST_BIT set in call to " .. name,frame,draw)
     end
    end


   --=============================
   elseif name == "vkCmdResetQueryPool" then
    if param[5] == 0 then -- queryCount parameter
     annotateSuggestion("No queries were reset because queryCount parameter is 0 in call to " .. name,frame,draw)
    end


   --=============================
   elseif name == "vkCmdResolveImage" then
    check_image_layout_src(name,"srcImageLayout",param[4],frame,draw,true)
    check_image_layout_dst(name,"dstImageLayout",param[6],frame,draw,true)

    check_param_non_zero  (name,"regionCount"   ,param[7],frame,draw)


   --=============================
   elseif name == "vkCmdSetBlendConstants" then
    check_param_not_null  (name,"blendConstants",param[3],frame,draw)


   --=============================
   elseif name == "vkCmdSetDepthBounds" then

    --[[ pending toFloat() function either from lua module or explicit

    local minDepthBounds = toFloat(param[3])
    local maxDepthBounds = toFloat(param[4])

    if maxDepthBounds <= minDepthBounds then
     annotateSuggestion("maxDepthBouds <= minDepthBounds in call to " .. name,frame,draw)
    end

    --]]


   --=============================
   elseif name == "vkCmdSetDeviceMask" or
          name == "vkCmdSetDeviceMaskKHR" then
    check_param_non_zero  (name,"deviceMask"    ,param[2],frame,draw)


   --=============================
   elseif name == "vkCmdSetEvent" then
    if param[3] & VK_PIPELINE_STAGE_HOST_BIT ~= 0 then
     annotateIssue("stageMask must not have the VK_PIPELINE_STAGE_HOST_BIT set in call to " .. name,frame,draw)
    end


   --=============================
   elseif name == "vkCmdSetScissor" then
    check_param_non_zero(name,"scissorCount",param[4],frame,draw)


   --=============================
   elseif name == "vkCmdSetStencilCompareMask" or
          name == "vkCmdSetStencilReference"   or
          name == "vkCmdSetStencilWriteMask" then
    local faceMask = param[3]

    check_param_non_zero(name,"faceMask" ,fasceMask,frame,draw)

    if faceMask ~=0 and faceMask & VK_STENCIL_FRONT_AND_BACK ==0 then
     annotateIssue("faceMask parameter has spurious bits set in call to " .. name,frame,draw)
    end


   --=============================
   elseif name == "vkCmdSetViewport" then
    local firstViewport = param[3]
    local viewportCount = param[4]

    check_param_non_zero(name,"viewportCount",viewportCount,frame,draw)

    check_param_non_zero(name,"sum of firstViewport and viewportCount",firstViewport + viewportCount,frame,draw)


   --=============================
   elseif name == "vkCmdUpdateBuffer" then
    local dstOffset = param[4]
    local dataSize  = param[5]

    check_multiple_of_4 (name,"dstOffset",dstOffset,frame,draw,"")
    check_multiple_of_4 (name,"dataSize" ,dataSize ,frame,draw,"")
    check_param_non_zero(name,"dataSize" ,dataSize ,frame,draw)

    if dataSize > 65536 then
     annotateIssue("dataSize parameter must be less than or equal to 65536 in call to " .. name,frame,draw)
    end


   --=============================
   elseif name == "vkCmdWaitEvents" then
    local               eventCount = param[ 3]
    local       memoryBarrierCount = param[ 7]
    local bufferMemoryBarrierCount = param[ 9]
    local  imageMemoryBarrierCount = param[11]

    local srcStageMask = param[5]
    local dstStageMask = param[6]

    if     memoryBarrierCount == 0 and
     bufferMemoryBarrierCount == 0 and
      imageMemoryBarrierCount == 0 then
     annotateIssue("No barriers of any kind are being waited upon in call to " .. name,frame,draw)
    end


   --=============================
   elseif name == "vkCreateComputePipelines" or
          name == "vkCreateGraphicsPipelines" then
    check_param_non_zero(name, "createInfoCount",param[4],frame,draw)


   --=============================
   elseif name == "vkFlushMappedMemoryRanges" then
    check_param_non_zero(name,"memoryRangeCount",param[3],frame,draw)


   --=============================
   elseif name == "vkFreeCommandBuffers" then
    check_param_non_zero(name,"commandBufferCount",param[4],frame,draw)


   --=============================
   elseif name == "vkFreeDescriptorSets" then
    check_param_non_zero(name,"descriptorSetCount",param[4],frame,draw)


   --=============================
   elseif name == "vkGetDeviceGroupPeerMemoryFeatures" then
    if param[4] == param[5] then
     annotateIssue("localDeviceIndex must not equal remoteDeviceIndex in call to " .. name,frame,draw)
    end


   --=============================
   elseif name == "vkGetPhysicalDeviceImageFormatProperties" then
    local format = param[3]
    local type   = param[4]
    local tiling = param[5]
    local usage  = param[6]
    local flags  = param[7]

    check_VkFormat             (name,"format",format,frame,draw)
    check_VkImageType          (name,"type"  ,type  ,frame,draw)
    check_VkImageTiling        (name,"tiling",tiling,frame,draw)
    check_VkImageUsageFlagBits (name,"usage" ,usage ,frame,draw)
    check_VkImageCreateFlagBits(name,"flags" ,flags ,frame,draw)

    check_param_non_zero       (name,"usage" ,usage ,frame,draw)


   --=============================
   elseif name == "vkGetPhysicalDeviceSparseImageFormatProperties" then
    local format  = param[3]
    local type    = param[4]
    local samples = param[5]
    local usage   = param[6]
    local tiling  = param[7]

    check_VkFormat             (name,"format",format,frame,draw)
    check_VkImageType          (name,"type"  ,type  ,frame,draw)
    check_VkSampleCountFlagBits(name,"sample",sample,frame,draw)
    check_VkImageUsageFlagBits (name,"usage" ,usage ,frame,draw)
    check_VkImageTiling        (name,"tiling",tiling,frame,draw)

    check_param_non_zero       (name,"usage" ,usage ,frame,draw)


   --=============================
   elseif name == "vkGetQueryPoolResults" then
    local dataSize = param[6]
    local pData    = param[7]
    local stride   = param[8]
    local flags    = param[9]

    check_param_non_zero(name,"dataSize",dataSize,frame,draw)

    if flags & VK_QUERY_RESULT_64_BIT then
     check_multiple_of_8(name,"pData" ,pData ,frame,draw," if VK_QUERY_RESULT_64_BIT is set in the flags parameter")
     check_multiple_of_8(name,"stride",stride,frame,draw," if VK_QUERY_RESULT_64_BIT is set in the flags parameter")
    else
     check_multiple_of_4(name,"pData" ,pData ,frame,draw," if VK_QUERY_RESULT_64_BIT is unset in the flags parameter")
     check_multiple_of_4(name,"stride",stride,frame,draw," if VK_QUERY_RESULT_64_BIT is unset in the flags parameter")
    end


   --=============================
   elseif name == "vkMapMemory" then
    local size  = param[5]
    local flags = param[6]

    check_param_non_zero(name,"size" ,size ,frame,draw)
    check_param_equals  (name,"flags",flags,frame,draw,0) -- flags is reserved for future use and *must* be 0 in the meantime


   --=============================
   elseif name == "vkResetCommandBuffer" then
    check_VkCommandBufferResetFlagBits(name,"flags",param[3],frame,draw)


   --=============================
   elseif name == "vkResetCommandPool" then
    check_VkCommandPoolResetFlagBits(name,"flags",param[4],frame,draw)


   --=============================
   elseif name == "vkResetDescriptorPool" then
    check_param_equals(name,"flags",param[4],frame,draw,0) -- flags is reserved for future use and *must* be 0 in the meantime


   --=============================
   elseif name == "vkResetFences" then
    check_param_non_zero(name,"fenceCount",param[3],frame,draw,0)


   --=============================
   elseif name == "vkTrimCommandPool" or
          name == "vkTrimCommandPoolKHR" then
    check_param_equals(name,"flags",param[4],frame,draw,0) -- flags is reserved for future use and *must* be 0 in the meantime


   --=============================
   elseif name == "vkUpdateDescriptorSets" then
    local descriptorWriteCount = param[3]
    local descriptorCopyCount  = param[5]

    if descriptorWriteCount == 0 and descriptorCopyCount == 0 then
     annotateIssue("No write- or copy- descriptor commands are requested by this call to " .. name,frame,draw)
    end


   --=============================
   elseif name == "vkWaitForFences" then
    check_param_non_zero(name,"fenceCount",param[3],frame,draw)

   end

  end end

  FEnd()

 end



 function check_VK_Pipeline_Barriers()
  --[[ Check vkCmdPipelineBarrier arguments

        Annotate if...

        1) top of pipe is used as a source
        2) bottom of pipe is used as a destination
        3) no actual barriers have been sent


        Do not annotate...

        1) if counts are positive but array pointers are null, because that is taken care of in an explicit vulkan array checking script (coming soon!)
  --]]

  if not hasParams then return end

  FStart("check_VK_Pipeline_Barriers")

  local count = 0 -- render pass pipeline barrier count

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name  = call.CallName
   local frame = call.FrameNum
   local draw  = call.DrawNum

   if name == "vkCmdBeginRenderPass" then

    count = 0

   elseif name == "vkCmdEndRenderPass" then

    if count >= hardware.MaxRenderPassPipelineBarriers then
     annotateSuggestion(
      "This render pass has issued " .. string.format("%d",count) .. " pipeline barriers. Are they all necessary or can work be rescheduled to obviate the need for them?"
      ,frame,draw)
    end

   elseif name == "vkCmdPipelineBarrier" then

    count = count + 1

    local src = call.ValueTab[3]
    local dst = call.ValueTab[4]

    if src == VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT then
     annotateSuggestion(
      "In this call to vkCmdPipelineBarrier, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT is included in the 'source' pipeline stage bitmask, " ..
      "which is effectively meaningless because all stages that do anything come after it."
      ,frame,draw)
    end

    if dst == VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT then
     annotateSuggestion(
      "In this call to vkCmdPipelineBarrier, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT is included in the 'destination' pipeline stage bitmask. " ..
      "which is effectively meaningless because all stages that do anything come before it."
      ,frame,draw)
    end

    local memBar = call.ValueTab[ 6] -- number of memory barriers
    local bufBar = call.ValueTab[ 8] -- number of buffer barriers
    local imgBar = call.ValueTab[10] -- number of image  barriers

    if memBar == 0 and bufBar == 0 and imgBar == 0 then
     annotateSuggestion(
      "This call to vkCmdPipelineBarrier has no effect because no memory-, buffer- or image-barriers were attached."
      ,frame,draw
     )
    end
   end
  end end

  FEnd()

 end



 -- perfdoc inspired check for many small indexed drawcalls
 function check_VK_cmd_draw_indexed()
  FStart("check_VK_cmd_draw_indexed")

  local smallIndexedDrawcallIndices = 10 -- how many indices make a small call
  local maxSmallIndexedDrawcalls    = 10 -- how many small calls trigger annotation

  local count = 0

  for i=0,#frames do
   FProgress(i,#frames)

   for j=0,#frames[i].calls.data do
    local call  = frames[i].calls.data[j]
    local name  = call.CallName
    local frame = call.FrameNum
    local draw  = call.DrawNum

    if name == "vkBeginCommandBuffer" then
     count = 0

    elseif name == "vkEndCommandBuffer" then
     if count >= maxSmallIndexedDrawcalls then
      annotateSuggestion("This command buffer contains " .. string.format("%d",count) .. " small indexed draw calls, which may be causing pipeline bubbles. Try batching draw calls or instancing, where appropriate.",frame,draw)
     end

    elseif name == "vkCmdDrawIndexed" then
     local    indexCount = call.ValueTab[3]
     local instanceCount = call.ValueTab[4]

     if indexCount * instanceCount < smallIndexedDrawcallIndices then
      count = count + 1
     end

    end
   end
  end

  FEnd()
 end



 function check_GL_gen_delete()
  FStart("check_GL_gen_delete")

  local stubs = {}

  stubs["Buffers"           ]=1
  stubs["Framebuffers"      ]=2
  stubs["ProgramPipelines"  ]=3
  stubs["Queries"           ]=4
  stubs["Renderbuffers"     ]=5
  stubs["Samplers"          ]=6
  stubs["Textures"          ]=7
  stubs["TransformFeedbacks"]=8
  stubs["VertexArrays"      ]=9

  local count = {{},{},{},{},{},{},{},{},{},{},{}}

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do

   local call = frames[i].calls.data[j]

   local name = call.CallName

   if name ~= "glGenerateMipmap" and name ~= "glGenerateTextureMipmap" then

    if name:find("glGen") ~= nil or name:find("glCreate") ~= nil then
     name = name:gsub("glGen"   ,"") -- strip prefix
     name = name:gsub("glCreate","") -- strip prefix

     local k = stubs[name]

     if k ~= nil then
      if count[k].ids == nil then count[k].ids = {} end

      local id = call.ValueTab[3] -- this is actually a GLuint*

      if count[k].ids[id] == nil then
       count[k].ids[id]       = {}
       count[k].ids[id].count = 1
       count[k].ids[id].name  = name
       count[k].ids[id].frame = call.FrameNum
       count[k].ids[id].draw  = call.DrawNum
       count[k].ids[id].len   = call.ValueTab[2]
      else
       count[k].ids[id].count = count[k].ids[id].count + 1
      end
     end
    end

    if name:find("glDelete") ~= nil then
     name = name:gsub("glDelete","") -- strip prefix

     local i = stubs[name]

     if k ~= nil then
      if count[k].ids == nil then count[k].ids = {} end

      local id = call.ValueTab[3] -- this is actually a GLuint*

      if count[k].ids[id] == nil then
       annotateIssue("Bad pointer passed to glDelete" .. name .. ". The data block pointed to was not initialised with glGen" .. name,call.FrameNum,call.DrawNum)

      elseif count[k].ids[id].len ~= call.ValueTab[2] then
       annotateIssue("Bad argument count passed to glDelete" .. name .. ". Does not match the glGen" .. name .. " for that data pointer",call.FrameNum,call.DrawNum)
      
      else

       count[k].ids[id].count = count[k].ids[id].count - 1

       if count[k].ids[id].len ~= call.ValueTab[2] then
        annotateIssue("Objects created with one length but deleted with another",frame,draw)
       end

       -- this next line prevents the feedback engine from missing the case where a user 'deletes' a resource which doesn't exist
       -- and then 'fortuitously' creating a resource which returns the same handle erroneously deleted, and then never deleting
       -- the correctly allocated handle
       --
       -- I.e...
       --
       -- GLuint id = 1; glDeleteProgram(1); // Wrong! Program '1' was never created.
       --        id =    glCreateProgram( ); // Unlucky if id == 1 after this call.
       --
       -- Simple reference counting will not pick this up because it has the correct zero sum of creats and deletes
       --
       if count[k].ids[id].count == 0 then count[k].ids[id] = nil end
      end
     end
    end

   end -- glGenerateMipmap

  end end

  for i=1,#count do
   if count[i].ids ~= nil then
    for id,c in pairs(count[i].ids) do
     annotateIssue("A call to glGen" .. c.name .. " / glCreate" .. c.name .. " was not matched by a call to glDelete" .. c.name,c.frame,c.draw)
    end
   end
  end

  FEnd()

 end



 function check_GL_mipmaps()
  FStart("check_GL_mipmaps")

  local detected = false;

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do
   local call = frames[i].calls.data[j]

   if call.CallName == "glGenerateMipmap" or call.CallName == "glGenerateTextureMipmap" or (call.CallName:sub(1,2) ~= "gl" and call.CallName:sub(1,3) ~= "egl") then
    detected = true
   end
  end end

  if not detected then
   annotateSuggestion("Mipmaps are not being used. Mipmaps can increase rendering speed as well as reducing aliasing artefacts.",0,0)
  end

  FEnd()
 end



 function check_GL_uniforms()
  FStart("check_GL_uniforms")

  if not hasParams then return end

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do
   local call = frames[i].calls.data[j]
   local msg  = ""

   local r = call.ValueTab[1]

   if call.CallName == "glGetUniformLocation" then
    if r == glName2Enum["GL_INVALID_VALUE"] or r == glName2Enum["GL_INVALID_OPERATION"] then
     msg = "The program ID passed glGetUniformLocation is invalid. This could be because that ID does not correspond to an OpenGL shader program, but it could also be because the shader program has not been linked (with glLinkProgram)."
    elseif r == -1 then
     msg = "The name passed to glGetUniformLocation does not exist in the shader program"
    end

    if msg ~= "" then annotateIssue(msg,call.FrameNum,call.DrawNum) end

   elseif call.CallName:sub(1,9) == "glUniform" then
    if r == -1 or r == glName2Enum["GL_INVALID_VALUE"] or r == glName2Enum["GL_INVALID_OPERATION"] then
     msg = "The uniform ID passed to " .. call.CallName .. " is not valid"
    end

    if msg ~= "" then annotateIssue(msg,call.FrameNum,call.DrawNum) end
   end

  end end

  FEnd()
 end



 function check_GL_legacy()
  FStart("check_GL_legacy")

  local calls = {}

  calls["glBegin"              ] = true
  calls["glEnd"                ] = true
  calls["glScalef"             ] = true
  calls["glScaled"             ] = true
  calls["glRotatef"            ] = true
  calls["glRotated"            ] = true
  calls["glTranslatef"         ] = true
  calls["glTranslated"         ] = true
  calls["glMatrixMode"         ] = true
  calls["glPushMatrix"         ] = true
  calls["glPopMatrix"          ] = true
  calls["glMultMatrix"         ] = true
  calls["glLoadMatrix"         ] = true
  calls["glLoadTransposeMatrix"] = true
  calls["glMultTransposeMatrix"] = true

  local legacy = false

  for i=0,#frames do

   FProgress(i,#frames)

  for j=0,#frames[i].calls.data do
   local call = frames[i].calls.data[j]

   if calls[call.CallName] == true then
    legacy = true
   end

  end end

  if legacy then
   annotateInformation("Calls to legacy functions such as glBegin and glEnd were detected",0,0)
  end

  FEnd()
 end



 function check_setup_calls()
  FStart("check_setup_calls")

  local nsetup = 0
  local msetup = 0
  local iframe = -1
  local detected = false
  local names = {}

  local i=0 -- worth doing, this function can take a while compared to others, so shows up in large traces
  for _1,call in ipairs(calls) do
   i=i+1

   FProgress(i,#calls)

   if iframe ~= call.FrameNum then
    msetup = math.max(msetup,nsetup)
    if detected then nsetup = nsetup + 1 else nsetup = 0 end
    iframe = call.FrameNum
    detected = false
   end

   for _2,name in ipairs(glSetupCalls) do
    if name == call.CallName then
     detected = false
     for _3,found in ipairs(names) do
      if found == name then detected = true end
     end
     if not detected then table.insert(names,name) end
     detected = true
    end
   end
  end

  if msetup > 10 then
   local msg =
       "--------------------------------\n"
    .. "SUSPICIOUS SETUP CALLS DETECTED!\n"
    .. "--------------------------------\n"
    .. "\n"
    .. "	You made typical setup calls in " .. string.format("%d",msetup) .. " consecutive frames\n"
    .. "\n"
    .. "	A setup call such as glCreateShader(), for example, would typically be made\n"
    .. "	during the initialisation section of a program only, and not generally be\n"
    .. "	called every frame.\n"
    .. "\n"
    .. "	Are you doing work every frame that only needs to be done once?\n"
    .. "	Are you creating and compiling shaders unnecessarily, for example?\n"
    .. "\n"
    .. "	You made calls to a combination of the following typical setup functions:-\n"
    .. "\n"

   for _,name in ipairs(names) do
    msg = msg .. "		" .. name .. "\n"
   end

   annotateSuggestion(msg,0,0)
  end

  FEnd()
 end



 function is_setup_frame(k)
  for i=0,frames[k].NumCalls-1 do
   for _,call in ipairs(glSetupCalls) do
    if frames[k].calls.data[i].CallName == call then return true end
   end
  end

  return false
 end



 function check_setup_regions()
  FStart("check_setup_regions")

  local detected = false

  for i=1,#regions do

   FProgress(i,#regions)

   if regions[i].isSetup then detected = true end
  end

  if detected then
   annotateInformation("Setup regions have been detected. A setup region is a collection of frames (often a single frame) that contains setup type commands such as shader compilation or pipeline creation commands",0,0)
  end

  FEnd()
 end



 function check_downgrade()
  local fraction = 2.0 / 100.0 -- percentage threshold. If everything is less than 'fraction' of the duration, consider downgrading

  for i=1,#regions do
   local worst_CallTime = table_at(regions[i].CallTime.calls.mean,1)
   local worst_Delay    = table_at(regions[i].Delay   .calls.mean,1)

   local worst_Period   = math.max(worst_CallTime,worst_Delay)

   local worst_fraction = worst_Period/regions[i].Duration.frame.mean

   -- if the worst call or duration is less than 'fraction' of the duration of the frame
   -- then there is no opportunity to optimise anything
   if worst_fraction < fraction and not regions[i].isSetup then
    local msg =
        "-----------------\n"
     .. "DOWNGRADE REGION!\n"
     .. "-----------------\n"
     .. "\n"
     .. "	Region " .. string.format("%d",i) .. "\n"
     .. "\n"
     .. "	The longest individual call and longest individual delay between the calls in region " .. string.format("%d",i) .. "\n"
     .. "	each makes up less than " .. string.format("%0.1f%%",100*worst_fraction) .. " of the frame duration, on average. Therefore, there is\n"
     .. "	little value in optimising any particular call or delay between calls.\n"
     .. "\n"
     .. "	Instead, focus on downgrading the application by reducing content,\n"
     .. "	thereby invoking fewer calls.\n"

     annotateSuggestion(msg,0,0)
   end
  end
 end



 function check_GL_errors()
  FStart("check_GL_errors")

  local e = LPGPU2:getErrorFromTraceParams()

  if #e ~= #calls then return end

  local i = 1
  for _,call in ipairs(calls) do
   FProgress(i,#calls)

   if e[i] ~= 0 and e[i] ~= 12288 then -- EGL_SUCCESS == 12288

    local s = glEnum2Name[e[i]];

    if s == nil then
     annotateInformation("Unrecognised error state, " .. string.format("%d",e[i]) .. " was set after a call to " .. call.CallName,call.FrameNum,call.DrawNum)
    else
     annotateInformation("After a call to " .. call.CallName .. " the error state was set to " .. s,call.FrameNum,call.DrawNum)
    end

   end

   i=i+1
  end

  FEnd()
 end



 function core_count_estimate()
  local hw = LPGPU2:getHardware()

  local nc = hw2cores[hw]

  if nc ~= nil then
   print('hardware: '       ,hw)
   print('number of cores: ',nc)

   return nc
  end

  function exists(name) return #LPGPU2:getSamplesForCounterWithName(name) ~= 0 end

  nc = 1

  if exists("CPU Core 7 Load")      then nc = 8 end
  if exists("CPU Core 7 Frequency") then nc = 8 end

  if exists("CPU Core 5 Load")      then nc = 6 end
  if exists("CPU Core 5 Frequency") then nc = 6 end

  if exists("CPU Core 3 Load")      then nc = 4 end
  if exists("CPU Core 3 Frequency") then nc = 4 end

  if exists("CPU Core 1 Load")      then nc = 2 end
  if exists("CPU Core 1 Frequency") then nc = 2 end

  print('estimated number of cores: ',nc)

  return nc
 end



 function thread_affinity_stats()
  FStart("thread_affinity_stats")

  local affinity = LPGPU2:getSamplesForCounterWithName("CPU Affinity")

  if #affinity == 0 then return end -- no data to analyse

  local   num_cores    = core_count_estimate()

  local count_cores    = {} for a = 0,num_cores-1 do count_cores[a] = 0 end

  local  prev_core     = -1 -- previously logged core
  local  prev_cluster  = -1 -- previously logged cluster

  local count_clusters = {0,0} -- two clusters: 1 & 2
  local inter_cluster  =  0    -- count of migrations occuring between clusters

  for i = 1,#affinity do
   local a = affinity[i]

   if count_cores[a] == nil then count_cores[a] = 0 end

   count_cores[a] = count_cores[a] + 1

   if a < num_cores/2 then
    -- cluster 1
    count_clusters[1] = count_clusters[1] + 1

    if prev_cluster == 2 then inter_cluster = inter_cluster + 1 end
   else
    -- cluster 2
    count_clusters[2] = count_clusters[2] + 1

    if prev_cluster == 1 then inter_cluster = inter_cluster + 1 end
   end

   -- update previous core
   prev_core = a

   -- update previous cluster
   if a < num_cores/2 then prev_cluster = 1 else prev_cluster = 2 end

  end

  local msg = "CPU Residency Statistics\n\n"

  for i = 0,num_cores-1 do
   msg = msg .. string.format("core %d\n",i)
             .. string.format(" sample count = %d\n"   ,count_cores[i])
             .. string.format(" residency %%    %f\n\n",count_cores[i] * 100.0 / #affinity)
  end

  for i = 1,2 do
   msg = msg .. string.format("cluster %d\n",i)
             .. string.format(" sample count = %d\n"   ,count_clusters[i])
             .. string.format(" residency %%    %f\n\n",count_clusters[i] * 100.0 / #affinity)
  end

  msg = msg .. string.format("Cluster migration %%: %f\n",inter_cluster * 100.0 / #affinity)

  annotateInformation(msg,0,0)


  local thread_migration_threshold_ratio = 0.25 -- If more than 25% of thread migrations are between clusters, emit a suggestion.

  if inter_cluster * 1.0 / #affinity > thread_migration_threshold_ratio then
   msg = "Excessive thread migrations between clusters\n\n" .. string.format("%f%%",inter_cluster * 100.0 / #affinity)
      .. " of all detected thread migrations occur between clusters. Setting the CPU affinity mask to the CPU's in a single cluster may increase performance and/or reduce power."
      .. " For more information on working with CPU affinity masks, consult the LPGPU2 API Reference Manual and User Guide."

   annotateSuggestion(msg,0,0)
  end

  FEnd()
 end



function init_analysis()

 LPGPU2:beginTransaction()
 annotateInformation("Script version : " .. getScriptVersion(),0,0)
 LPGPU2:endTransaction()

 init_glName2Enum()
 init_glEnum2Name()

 if #rawHexParams == #rawDrawNum then
  hasParams = true
 else
  hasParams = false
 end

 local end_of_last = -1

 local CallNum = 0

 FStart("Populating frames")

 for icall = 1,#rawFrameNum do -- loop over calls

  FProgress(icall,#rawFrameNum)

  CallNum = CallNum + 1

  local CallName = rawCallName [icall]
  local CpuStart = rawCpuStart [icall]
  local CpuEnd   = rawCpuEnd   [icall]
  local FrameNum = rawFrameNum [icall]
  local DrawNum  = rawDrawNum  [icall]

  local Delay    = 0                 -- Delay before the API call is invoked
  local CallTime = CpuEnd - CpuStart -- Duration of the API call

  if end_of_last<0 then Delay = 0 else Delay = CpuStart - end_of_last end

  local TypesTab  = nil
  local ValueTab  = nil
  local HexParams = nil
  local HexReturn = nil

  if hasParams then
   HexParams = rawHexParams[icall]
   HexReturn = rawHexReturn[icall]

   local Types      = rawTypes     [icall]
   local ParamsSize = rawParamsSize[icall]
   local ReturnSize = rawReturnSize[icall]

   if (HexParams == nil) then HexParams = "" end
   if (HexReturn == nil) then HexReturn = "" end
   if (Types     == nil) then Types     = "" end

   local s = ReturnSize

   if ParamsSize ~= "" then s = s .. ',' .. ParamsSize end

   TypesTab,ValueTab = parse_params(HexReturn .. HexParams,s)
  end

  if (calls[CallNum] == nil) then
   calls[CallNum]           = {}
   calls[CallNum].CallName  = CallName
   calls[CallNum].CpuStart  = CpuStart
   calls[CallNum].CpuEnd    = CpuEnd
   calls[CallNum].FrameNum  = FrameNum
   calls[CallNum].DrawNum   = DrawNum
   calls[CallNum].CallTime  = CallTime
   calls[CallNum].Delay     = Delay

   if hasParams then
    calls[CallNum].TypesTab  = TypesTab
    calls[CallNum].ValueTab  = ValueTab
    calls[CallNum].HexReturn = HexReturn
    calls[CallNum].HexParams = HexParams
   end
  end


  if (frames[FrameNum] == nil) then
   frames[FrameNum]             = {}
   frames[FrameNum].NumCalls    = 0
   frames[FrameNum].CallTime    = 0.0   -- Sum of the CallTime of each call in the frame
   frames[FrameNum].Delay       = 0     -- total amount of call delay
   frames[FrameNum].LoopDelay   = Delay -- Time since the end of the last api call of the previous frame
   frames[FrameNum].CpuStart    = CpuStart - Delay
   frames[FrameNum].CallList    = CallName
   frames[FrameNum].RegionNum   =-1     -- region index
   frames[FrameNum].RegionFrame = 0     -- index of frame within region

   frames[FrameNum].calls       = {}
   frames[FrameNum].calls.data  = {}
  else
   frames[FrameNum].CallList = frames[FrameNum].CallList .. "," .. CallName
  end

  frames[FrameNum].NumCalls = frames[FrameNum].NumCalls + 1
  frames[FrameNum].CallTime = frames[FrameNum].CallTime + CallTime
  frames[FrameNum].Delay    = frames[FrameNum].Delay    + Delay
  frames[FrameNum].CpuEnd   = CpuEnd

  frames[FrameNum].calls.data[DrawNum]          = {}
  frames[FrameNum].calls.data[DrawNum].CallName = calls[CallNum].CallName
  frames[FrameNum].calls.data[DrawNum].CpuStart = calls[CallNum].CpuStart
  frames[FrameNum].calls.data[DrawNum].CpuEnd   = calls[CallNum].CpuEnd
  frames[FrameNum].calls.data[DrawNum].FrameNum = calls[CallNum].FrameNum
  frames[FrameNum].calls.data[DrawNum].DrawNum  = calls[CallNum].DrawNum
  frames[FrameNum].calls.data[DrawNum].CallTime = calls[CallNum].CallTime
  frames[FrameNum].calls.data[DrawNum].Delay    = calls[CallNum].Delay

  if hasParams then
   frames[FrameNum].calls.data[DrawNum].TypesTab = calls[CallNum].TypesTab
   frames[FrameNum].calls.data[DrawNum].ValueTab = calls[CallNum].ValueTab
  end

  end_of_last = CpuEnd
 end

 FEnd()



 -- Calculate Frame durations
 for i = 0,#frames do
  frames[i].Duration = frames[i].CpuEnd - frames[i].CpuStart
 end



 -- Identify regions
 for i = 0,#frames do
  if not same_region(i-1,i) then
   local n = 1 + #regions

   regions[n]           = {}
   regions[n].frames    = {} -- list of the frame numbers in this region
   regions[n].NumCalls  = 0  -- number of calls in each frame of this region
   regions[n].NumFrames = 0  -- number of frames in this region
  end

  frames[i].RegionNum = #regions
 end



 FStart("Populating regions")

 -- Populate regions
 for i = 0,#frames do

  FProgress(i,#frames)

  local j = frames[i].RegionNum

  regions[j].NumFrames = regions[j].NumFrames + 1

  if regions[j].NumCalls == 0 then
   regions[j].NumCalls = frames[i].NumCalls

   regions[j].CallTime                    = {}
   regions[j].CallTime .frame             = {} -- Various frame total call time statistics
   regions[j].CallTime .frame.data        = {} -- The samples from which to gather the stats

   regions[j].Delay                       = {}
   regions[j].Delay    .frame             = {} -- Various frame total delay time statistics
   regions[j].Delay    .frame.data        = {}

   regions[j].Duration                    = {}
   regions[j].Duration .frame             = {} -- Various frame duration statistics
   regions[j].Duration .frame.data        = {}

   regions[j].LoopDelay                   = {}
   regions[j].LoopDelay.frame             = {} -- Various frame first call delay statistics
   regions[j].LoopDelay.frame.data        = {}

   regions[j].CallTime .calls             = {} -- Length of call statistics
   regions[j].CallTime .calls.data        = {}

   regions[j].CallTime .calls.min         = {}
   regions[j].CallTime .calls.min .value  = {} -- Minimum call time of each draw number call
   regions[j].CallTime .calls.min .ranks  = {}

   regions[j].CallTime .calls.max         = {}
   regions[j].CallTime .calls.max .value  = {} -- Maximum call time of each draw number call
   regions[j].CallTime .calls.max .ranks  = {}

   regions[j].CallTime .calls.mean        = {}
   regions[j].CallTime .calls.mean.value  = {}
   regions[j].CallTime .calls.mean.ranks  = {}

   regions[j].CallTime .calls.std         = {}
   regions[j].CallTime .calls.std .value  = {}
   regions[j].CallTime .calls.std .ranks  = {}

   regions[j].Delay    .calls             = {}
   regions[j].Delay    .calls.data        = {}

   regions[j].Delay    .calls.min         = {}
   regions[j].Delay    .calls.min .value  = {} -- Maximum Delay of each draw number call
   regions[j].Delay    .calls.min .ranks  = {}

   regions[j].Delay    .calls.max         = {}
   regions[j].Delay    .calls.max .value  = {}
   regions[j].Delay    .calls.max .ranks  = {}

   regions[j].Delay    .calls.mean        = {}
   regions[j].Delay    .calls.mean.value  = {}
   regions[j].Delay    .calls.mean.ranks  = {}

   regions[j].Delay    .calls.std         = {}
   regions[j].Delay    .calls.std .value  = {}
   regions[j].Delay    .calls.std .ranks  = {}

   for k = 0,regions[j].NumCalls-1 do
    regions[j].CallTime.calls.min .value[k] = 0.0/0.0
    regions[j].CallTime.calls.max .value[k] = 0.0/0.0
    regions[j].CallTime.calls.mean.value[k] = 0.0/0.0
    regions[j].CallTime.calls.std .value[k] = 0.0/0.0

    regions[j].CallTime.calls.data[k]       = {}

    regions[j].Delay   .calls.min .value[k] = 0.0/0.0
    regions[j].Delay   .calls.max .value[k] = 0.0/0.0
    regions[j].Delay   .calls.mean.value[k] = 0.0/0.0
    regions[j].Delay   .calls.std .value[k] = 0.0/0.0

    regions[j].Delay   .calls.data[k]       = {}
   end
  end

  table.insert (regions[j].CallTime .frame.data   ,frames[i]              .CallTime )
  table.insert (regions[j].Delay    .frame.data   ,frames[i]              .Delay    )
  table.insert (regions[j].Duration .frame.data   ,frames[i]              .Duration )
  table.insert (regions[j].LoopDelay.frame.data   ,frames[i]              .LoopDelay)

  for k = 0,regions[j].NumCalls-1 do
   if frames [i].calls.data[k] ~= nil then
    table.insert(regions[j].CallTime .calls.data[k],frames[i].calls.data[k].CallTime )
    table.insert(regions[j].Delay    .calls.data[k],frames[i].calls.data[k].Delay    )
   end
  end

  frames[i].RegionNum   = j
  frames[i].RegionFrame = regions[j].NumFrames

  table.insert(regions[j].frames,i)

  -- Region identifier no longer needed
  frames[i].CallList = nil
 end

 FEnd()



 FStart("Regions 2nd pass")

 -- Second pass over the regions to produce call statistics and frame statistics
 for i = 1,#regions do
  FProgress(i,#regions)

  regions[i].CallTime .frame.min  = table_min (regions[i].CallTime .frame.data)
  regions[i].CallTime .frame.max  = table_max (regions[i].CallTime .frame.data)
  regions[i].CallTime .frame.mean = table_mean(regions[i].CallTime .frame.data)
  regions[i].CallTime .frame.std  = table_std (regions[i].CallTime .frame.data)

  regions[i].Delay    .frame.min  = table_min (regions[i].Delay    .frame.data)
  regions[i].Delay    .frame.max  = table_max (regions[i].Delay    .frame.data)
  regions[i].Delay    .frame.mean = table_mean(regions[i].Delay    .frame.data)
  regions[i].Delay    .frame.std  = table_std (regions[i].Delay    .frame.data)

  regions[i].Duration .frame.min  = table_min (regions[i].Duration .frame.data)
  regions[i].Duration .frame.max  = table_max (regions[i].Duration .frame.data)
  regions[i].Duration .frame.mean = table_mean(regions[i].Duration .frame.data)
  regions[i].Duration .frame.std  = table_std (regions[i].Duration .frame.data)

  regions[i].LoopDelay.frame.min  = table_min (regions[i].LoopDelay.frame.data)
  regions[i].LoopDelay.frame.max  = table_max (regions[i].LoopDelay.frame.data)
  regions[i].LoopDelay.frame.mean = table_mean(regions[i].LoopDelay.frame.data)
  regions[i].LoopDelay.frame.std  = table_std (regions[i].LoopDelay.frame.data)

  for j = 0, regions[i].NumCalls-1 do
   local t = 0.0

   regions[i].CallTime.calls.min .value[j] = table_min (regions[i].CallTime.calls.data[j])
   regions[i].CallTime.calls.max .value[j] = table_max (regions[i].CallTime.calls.data[j])
   regions[i].CallTime.calls.mean.value[j] = table_mean(regions[i].CallTime.calls.data[j])
   regions[i].CallTime.calls.std .value[j] = table_std (regions[i].CallTime.calls.data[j])

   regions[i].Delay   .calls.min .value[j] = table_min (regions[i].Delay   .calls.data[j])
   regions[i].Delay   .calls.max .value[j] = table_max (regions[i].Delay   .calls.data[j])
   regions[i].Delay   .calls.mean.value[j] = table_mean(regions[i].Delay   .calls.data[j])
   regions[i].Delay   .calls.std .value[j] = table_std (regions[i].Delay   .calls.data[j])
  end
 end

 FEnd()



 FStart("Regions final pass")

 -- Fourth pass over the regions to calculate higher level attributes
 for i = 1,#regions do
  FProgress(i,#regions)

  if regions[i].NumFrames == 1 and is_setup_frame(regions[i].frames[1]) then
   regions[i].isSetup = true
  else
   regions[i].isSetup = false
  end
 end

 FEnd()

 LPGPU2:beginTransaction()
end



function fina_analysis()
 LPGPU2:endTransaction()
end



function object_lifetime_analysis_GL()
 init_analysis()

 check_GL_prog_shad_life()
 check_GL_gen_delete    ()

 fina_analysis()
end



function call_sequence_analysis_GL()
 init_analysis()

 -- display_calls  ()
 -- display_frames ()
 -- display_regions()

 check_GL_glClear         ()
 check_GL_texture_binding ()
 check_GL_active_texture  ()
 check_GL_redundancy      () -- this is for redundancy checking that does not need params
 check_GL_redundancy_param()
 check_GL_wait_sync       ()
 check_GL_mipmaps         ()
 check_GL_uniforms        ()
 check_GL_legacy          ()
 check_GL_errors          ()

 check_setup_calls        ()
 check_setup_regions      ()

 fina_analysis()
end



function object_lifetime_analysis_VK()
 init_analysis()

 check_VK_create_destroy () -- Match vkCreate*   / vkDestroy*
 check_VK_allocate_free  () -- Match vkAllocate* / vkFree*
 check_VK_map_unmap      () -- Match vkMapMemory / vkUnmapMemory
 check_VK_command_buffers() -- check vkCmd* functions
 check_VK_renderpass     () -- checking functions which are and are not allowed inside render passes

 fina_analysis()
end



function call_sequence_analysis_VK()
 init_analysis()

 -- display_calls  ()

 check_VK_empty_renderpass  ()
 check_VK_image_layout      ()
 check_VK_Valid_Usage       ()
 check_VK_Pipeline_Barriers ()
 check_VK_bad_arrays        () -- arrays of finite length should have non null ptr's
 check_VK_bad_pointers      () -- pointers that are never allowed to be null
 check_VK_cmd_draw_indexed  () -- check for many small indexed draw calls (a la perfdoc)
 check_VK_used_after_destroy() -- check for many small indexed draw calls (a la perfdoc)
 check_VK_fence_wait        () -- check for consecutive 'busy' wait calls
 check_VK_count_resolves    () -- calculate the average number of resolves per frame
 check_VK_result            () -- check return status of API functions

 fina_analysis()
end



function thread_affinity_analysis()
 init_analysis()

 thread_affinity_stats() -- API agnostic counters analysis

 fina_analysis()
end



function gpu_2_cpu_ratio()
 init_analysis()

 FStart("gpu_2_cpu_ratio")

 local StartTime = frames[0].calls.data[0].CpuStart
 local   EndTime = StartTime
 local  CallTime = StartTime -- This is to get the type right, see adjustment after the loop

 for i=0,#frames do

  FProgress(i,#frames)

 for j=0,#frames[i].calls.data do
  local call = frames[i].calls.data[j]

  CallTime = CallTime + call.CallTime
   EndTime =            call.CpuEnd
 end end

 CallTime = CallTime - StartTime -- Remove the initial value

 local total = EndTime - StartTime
 local gpu   = CallTime * 1.0 / total
 local cpu   = 1.0 - gpu

 annotateInformation(string.format("GPU time = %f4.1%%, CPU time = %f4.1%%",100 * gpu,100 * cpu),0,0)

 fina_analysis()

 FEnd()
end
