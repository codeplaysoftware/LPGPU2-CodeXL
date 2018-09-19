
-- Sample script to populate the LPGPU2_shaderAsmStats and LPGPU2_shaderToAsm tables.
 
-- shader_counter_match is a special function name for 

function get_available_feedback()
    print('get_available_feedback')
    return_table = {}

    -- ***** General Scripts *****
    scriptCategory = '--General--'

    --  identify_regions -> writeRegion / writeCallSummary
    -- Requires Traces only.
    scriptName ='Calculate Shader Stats'
    scriptFunction =  'shader_counter_match'
    inputName = 'ShaderId'
    inputDescription = 'Test script'
    inputDefault = 20
    table.insert(return_table, {scriptName, scriptFunction, scriptCategory, inputName, inputDescription, inputDefault, 0, 0 } )

    print('#return_table = '..#return_table)

    return return_table
end



 
 -- helper functions to extract data from the binary blob.
 function parseU4 (b,i) n=string.byte(b,i) if n<58 then n=n-48 else n=n-55 end return n end
 function parseU8 (b,i) return parseU4 (b,i+1)+   16*parseU4 (b,i  ) end
 function parseU16(b,i) return parseU8 (b,i  )+  256*parseU8 (b,i+2) end
 function parseU32(b,i) return parseU16(b,i  )+65536*parseU16(b,i+4) end
-- Demo


function shader_counter_match(shaderid, apiid, counter)

    -- Get the shaderID, debug and apiId columns from the LPGPU2_shader table.
    shaderId = LPGPU2:getShaderID()
    debug = LPGPU2:getShaderDebug()
    apiId = LPGPU2:getApiIDFromShader()

    -- call beginTransaction and endTransaction around code that write to the database.
    -- this greatly improves the database access speed.
    LPGPU2:beginTransaction()
    print('shader_counter_match(shaderid: '..shaderid..', apiid: '..apiid..', counter: '..counter..')')
    for shader = 1, #debug do
        if (shaderid == shaderId[shader]) then
		print('shader = '..shader)
		print(debug[shader])
		index = 1
		numLineInfo = parseU32(debug[shader], index)
		print('numLineInfo = '..numLineInfo)
		index = index + 8

		for i = 1, numLineInfo  do

		    shaderLine = parseU32(debug[shader], index)
		    print(string.sub(debug[shader], index, index + 8)..'  shaderLine = '..shaderLine)
		    index = index + 8

		    asmStartLine = parseU32(debug[shader], index)
		    print('asmStartLine = '..asmStartLine)
		    index = index + 8

		    asmEndLine = parseU32(debug[shader], index)
		    print('asmEndLine = '..asmEndLine)
		    index = index + 8
        
                    -- Write into the LPGPU2_shaderToAsm table.
		    LPGPU2:insertShaderToAsm(shaderId[shader], apiId[shader], shaderLine, asmStartLine, asmEndLine)
		end

		number_asm_lines = parseU32(debug[shader], index)
		print(string.sub(debug[shader], index, index + 8)..'  number_asm_lines = '..number_asm_lines..' index = '..index)
		index = index + 8

                -- clear the table of previous entries.
                LPGPU2:deleteShaderAsmStatsForShaderId(shaderId[shader], apiId[shader])
		for asmLine = 1, number_asm_lines do
		    percentage = parseU32(debug[shader], index) / 100 
		    print(string.sub(debug[shader], index, index + 8)..'  percentage = '..percentage)
		    index = index + 8

                    -- Write into the LPGPU2_shaderAsmStats table.
		    LPGPU2:insertShaderAsmStat(shaderId[shader], apiId[shader], asmLine, percentage)
		end
        end -- if
    end
    LPGPU2:endTransaction()
end


