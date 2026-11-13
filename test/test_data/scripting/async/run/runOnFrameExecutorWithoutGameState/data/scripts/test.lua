
local assert = require("assert")

local context = async.context.captureGameState()
assert.isTrue(context:determineState() == CONTEXT_VALID)

function_was_run = false

asyncPromise = async.run(function()
    function_was_run = true
    return "FrameReturn"
end, async.OnFrameExecutor)

-- The body only runs once the executor is processed, which the C++ side of this test does.
assert.isFalse(function_was_run)
assert.isFalse(asyncPromise:isResolved())
