local module = {}

function module.equals(expected, actual)
    if (expected == actual) then
        return
    end

    error(string.format("Expected value of %q but was %q", tostring(expected), tostring(actual)))
end

function module.isTrue(value)
    module.equals(true, value)
end

function module.isFalse(value)
    module.equals(false, value)
end

return module
