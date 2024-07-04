
function pack(...)
    return {n = select("#", ...), ...}
end

local assert = require("assert")

local promise1Resolve
local promise1 = async.promise(function(resolve)
    promise1Resolve = resolve
end)

local promise2 = async.errored("Test", nil, 1234)

local start_reached = false
local promise1_reached = false
local promise2_reached = false

local asyncPromise = async.run(function()
    start_reached = true

    local promise1Val = async.await(promise1)
    promise1_reached = true
    assert.equals("Promise1", promise1Val)

    async.await(promise2)
    promise2_reached = true

    return "ResolvedReturn", 42
end)

-- Check if the returned promise behaves correctly
local thenCalled = false
local catchCalled = false;
asyncPromise:continueWith(function(stringTest, intTest)
    thenCalled = true
end)
asyncPromise:catch(function(...)
    catchCalled = true

    local retVals = pack(...)
    assert.equals(3, #retVals)
    assert.equals("Test", retVals[1])
    assert.equals(nil, retVals[2])
    assert.equals(1234, retVals[3])
end)

promise1Resolve("Promise1")

assert.isFalse(thenCalled)
assert.isTrue(catchCalled)

-- Check progress status. Since promises are already resolved this will resolve synchronously.
assert.isTrue(start_reached)
assert.isTrue(promise1_reached)
assert.isFalse(promise2_reached)

assert.isFalse(asyncPromise:isResolved())
assert.isTrue(asyncPromise:isErrored())

local retVals = pack(asyncPromise:getErrorValue())
assert.equals(3, #retVals)
assert.equals("Test", retVals[1])
assert.equals(nil, retVals[2])
assert.equals(1234, retVals[3])
