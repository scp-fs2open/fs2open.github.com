
local serializeTest = require("serializeTest")

serializeTest({})

local array = {}
table.insert(array, 1)
table.insert(array, "asdf")
table.insert(array, false)
serializeTest(array)

serializeTest({
    test = 42,
    __hello = "Hello World"
})

serializeTest({
    test = { "asdf", false, 42, true },
    __hello = "Hello World",
    _nestedTable = {
        test = 42,
        __hello = "Hello World"
    }
})
