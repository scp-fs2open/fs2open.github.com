
local assert = require("assert")

local function_was_run = false

local asyncPromise = async.run(function()
    function_was_run = true
    return "SyncReturn"
end)

assert.isTrue(function_was_run)
assert.isTrue(asyncPromise:isResolved())
assert.equals("SyncReturn", asyncPromise:getValue())
