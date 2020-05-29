
function pack(...)
    return {n = select("#", ...), ...}
end

local assert = require("assert")

local promise1Resolve
local promise1 = async.promise(function(resolve)
    promise1Resolve = resolve
end)

local promise2 = async.resolved("Test", nil, 1234)

local start_reached = false
local promise1_reached = false
local promise2_reached = false

local asyncPromise = async.run(function()
    start_reached = true

    local promise1Val = async.await(promise1)
    promise1_reached = true
    assert.equals("Promise1", promise1Val)

    local promise2Vals = pack(async.await(promise2))
    promise2_reached = true
    assert.equals(3, #promise2Vals)
    assert.equals("Test", promise2Vals[1])
    assert.equals(nil, promise2Vals[2])
    assert.equals(1234, promise2Vals[3])

    return "ResolvedReturn", 42
end)

-- Check if the returned promise behaves correctly
local thenCalled = false
local catchCalled = false;
asyncPromise:continueWith(function(stringTest, intTest)
    thenCalled = true

    assert.equals("ResolvedReturn", stringTest)
    assert.equals(42, intTest)
end)
asyncPromise:catch(function(...)
    catchCalled = true
end)

promise1Resolve("Promise1")

assert.isTrue(thenCalled)
assert.isFalse(catchCalled)

-- Check progress status. Since promises are already resolved this will resolve synchronously.
assert.isTrue(start_reached)
assert.isTrue(promise1_reached)
assert.isTrue(promise2_reached)
assert.isTrue(asyncPromise:isResolved())

local retVals = pack(asyncPromise:getValue())
assert.equals(2, #retVals)
assert.equals("ResolvedReturn", retVals[1])
assert.equals(42, retVals[2])
