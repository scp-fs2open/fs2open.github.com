local module = {}

function module.__call(condition)
    assert(condition)
end

function module.equals(expected, actual)
    if (expected == actual) then
        return
    end

    error(string.format("Expected value of %q but was %q", tostring(expected), tostring(actual)))
end

function module.tablesEqual(expected, actual)
    if type(expected) ~= "table" then
        error("Expected value is not a table")
    end

    if type(actual) ~= "table" then
        error("Actual value is not a table")
    end

    for i, expectedVal in pairs(expected) do
        local actualVal = actual[i]

        if type(expectedVal) == "table" then
            module.tablesEqual(expectedVal, actualVal)
        else
            if expectedVal ~= actualVal then
                error(string.format("Mismatch at index %d. Expected value of %q but was %q", i, tostring(expected), tostring(actual)))
            end
        end
    end
end

function module.isTrue(value)
    module.equals(true, value)
end

function module.isFalse(value)
    module.equals(false, value)
end

return module
