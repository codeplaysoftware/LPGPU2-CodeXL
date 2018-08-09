
function get_available_feedback()
    print('get_available_feedback')
    return_table = {}

    -- ***** General Scripts *****
    scriptCatagory = '--General--'

    --  identify_regions -> writeRegion / writeCallSummary
    -- Requires Traces only.
    scriptName ='Calculate Shader Stats'
    scriptFunction =  'shader_counter_match'
    inputName = 'ShaderId'
    inputDescription = 'Test script'
    inputDefault = 20
    inputMin = 0
    inputMax = 0
    table.insert(return_table, {scriptName, scriptFunction, scriptCatagory, inputName, inputDescription, inputDefault, inputMin, inputMax } )

    print('#return_table = '..#return_table)

    return return_table
end