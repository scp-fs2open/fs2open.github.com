local module = {}

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

    if #expected ~= #actual then
        error(string.format("Expected table length %d did not match actual %d.", #expected, #actual))
    end

    for i, expectedVal in ipairs(expected) do
        local actualVal = actual[i]

        if expectedVal ~= actualVal then
            error(string.format("Mismatch at index %d. Expected value of %q but was %q", i, tostring(expected), tostring(actual)))
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
