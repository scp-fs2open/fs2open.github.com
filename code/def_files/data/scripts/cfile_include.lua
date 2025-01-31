-- A version of include for lua. Originally written by Nuke

local chunck_cache = {}
--- function to load and run lua files
-- takes string filename and boolean cacheonly
-- cfile will try to find filename in all accesible locations and load it only if it hasnt done so already
-- if cacheonly flag is set, code will be cached but not executed, good for inits. however it is ignored when calling an already cached chunk.
-- returns true on success or nil on failure to find/load file
local function execute_lua_file(filename, cacheonly)
    --dont reload chunks from file if they have already been loaded. because faster == better
    if (type(chunck_cache[filename]) == "table" and type(chunck_cache[filename][1]) == "function") then
        if (not chunck_cache[filename][2] and not chunck_cache[filename][3]) then
            chunck_cache[filename][2] = true
            local val, err = pcall(chunck_cache[filename][1])
            if (err) then
                -- Add an boolean to indicate that this function has caused an error
                chunck_cache[filename][3] = true

                return false, string.format("Error while executing external lua file %q:\n%s", filename, err)
            end
            return true
        else
            return false
        end
    else
        if (cf.fileExists(filename, "", true)) then
            --open the file
            local file = cf.openFile(filename, "r", "")
            local fstring = file:read("*a") --load it all into a string
            file:close()
            if (fstring and type(fstring) == "string") then
                --use the string as a code chunk and convert it to function
                local func, error = loadstring(fstring, filename)
                if (not func) then -- Compile error
                    return false, string.format("Error while processing file %q. Errormessage:\n%s", filename, error)
                end
                chunck_cache[filename] = {}
                chunck_cache[filename][1] = func
                chunck_cache[filename][2] = false
                --maybe execute
                if (not cacheonly) then
                    chunck_cache[filename][2] = true
                    local val, err = xpcall(chunck_cache[filename][1], function(err)
                    -- Add an boolean to indicate that this function has caused an error
                        chunck_cache[filename][3] = true

                        return string.format("Error while executing external lua file %q:\n%s\n\n%s", filename, err, debug.traceback())
                    end)

                    if (err) then
                        return false, err
                    end
                end
                return true
            end
        else
            return false, string.format("Couldn't find external lua file %q!", filename)
        end
    end
end



function include(fileName)
    if (fileName == nil) then
        stackErrorf("Invalid argument for 'include'!")
    else
        if (not fileName:ends(".lua")) then
             fileName = fileName .. ".lua"
        end

        local succ, err = execute_lua_file(fileName)

        if (not succ and err) then
           error(err)
        end
    end
end
