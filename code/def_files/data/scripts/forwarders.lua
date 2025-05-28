-- Basic forwarders for common Lua methods
function error(str)
    ba.error(str)
end

function warning(str)
    ba.warning(str)
end

function print(str)
    ba.print(str)
end

function println(str)
    if (str) then
        ba.print(str)
    end
    ba.print("\n")
end

function stackError(str, level)
    if (level == nil) then
        level = 2
    else
        level = level + 1
    end

    error(debug.traceback(tostring(str) .. "\n", level) .. "\n")
end

function warningf(str, ...)
    warning(string.format(str, ...))
end

function errorf(str, ...)
    error(string.format(str, ...))
end

function printf(str, ...)
    print(string.format(str, ...))
end

function stackErrorf(str, ...)
    stackError(string.format(str, ...), 2)
end

function string.starts(str, start)
    return string.sub(str, 1, string.len(start)) == start
end

function string.ends(str, e)
    return (e == '') or (string.sub(str, -string.len(e)) == e)
end


-- Global table that can hold variables that should not be able to be changed
Globals = {}
Globals.values = {}

local mt = {}

function mt.__newindex(t, index, value)
    if (index == "values") then
        stackErrorf("Cannot set value to index %q. Index is forbidden!", tostring(index))
    elseif (rawget(t.values, index) ~= nil) then
        stackErrorf("Cannot set value to index %q. Index already used!", tostring(index))
    else
        rawset(t.values, index, value)
    end
end

function mt.__index(t, index)
    if (index == "values") then
        stackErrorf("Trying to access forbidden value %q!", index)
    else
        return rawget(t.values, index)
    end
end

setmetatable(Globals, mt)


-- Initial Values
Globals.nullVec = ba.createVector(0, 0, 0)
Globals.zeroVec = Globals.nullVec
Globals.identityOrient = ba.createOrientationFromVectors(ba.createVector(0, 0, 1), ba.createVector(0, 1, 0), ba.createVector(1, 0, 0))
