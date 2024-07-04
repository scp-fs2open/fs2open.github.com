
local assert = require("assert")

return function(value)
    local serialized = ba.serializeValue(value)
    local deserialized = ba.deserializeValue(serialized)

    if type(value) == "table" then
        assert.tablesEqual(value, deserialized)
    else
        assert.equals(value, deserialized)
    end
end
