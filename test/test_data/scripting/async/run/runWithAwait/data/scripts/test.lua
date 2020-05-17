
function pack(...)
    return {n = select("#", ...), ...}
end

local assert = require("assert")

local promise1_cb;
local promise1 = async.promise(function(resolve)
    promise1_cb = resolve;
end)

local promise2_cb;
local promise2 = async.promise(function(resolve)
    promise2_cb = resolve;
end)

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

    return "AsyncReturn", 42
end)

-- Check progress status
assert.isTrue(start_reached)
assert.isFalse(promise1_reached)
assert.isFalse(promise2_reached)
assert.isFalse(asyncPromise:isResolved())

-- Resolve the first promise
promise1_cb("Promise1")

-- Check progress status
assert.isTrue(start_reached)
assert.isTrue(promise1_reached)
assert.isFalse(promise2_reached)
assert.isFalse(asyncPromise:isResolved())

-- Resolve the second promise
promise2_cb("Test", nil, 1234)

-- Check progress status
assert.isTrue(start_reached)
assert.isTrue(promise1_reached)
assert.isTrue(promise2_reached)
assert.isTrue(asyncPromise:isResolved())

local retVals = pack(asyncPromise:getValue())
assert.equals(2, #retVals)
assert.equals("AsyncReturn", retVals[1])
assert.equals(42, retVals[2])
