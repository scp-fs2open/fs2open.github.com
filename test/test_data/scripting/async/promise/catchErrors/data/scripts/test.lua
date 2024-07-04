
function pack(...)
    return {n = select("#", ...), ...}
end

local reject_func;

local promise = async.promise(function (_, reject)
    reject_func = reject
end)

assert(not promise:isResolved())
assert(not promise:isErrored())

local continuationCalled = false
local catchCalled = false

local contPromise = promise:continueWith(function()
    continuationCalled = true

    return 42, "Test"
end)
local errPromise = promise:catch(function(errVals)
    assert(errVals == "ErrorValue")
    catchCalled = true
    return "asdf", 1234
end)

assert(not contPromise:isResolved())
assert(not errPromise:isResolved())

-- Now kick off the chain
reject_func("ErrorValue")


assert(promise:isErrored())
assert(not promise:isResolved())
assert(promise:getErrorValue() == "ErrorValue")

assert(not continuationCalled)
assert(contPromise:isErrored())
local vals = pack(contPromise:getErrorValue())
assert(#vals == 1)
assert(vals[1] == "ErrorValue")

assert(catchCalled)
assert(not errPromise:isErrored())
assert(errPromise:isResolved())
local vals = pack(errPromise:getValue())
assert(#vals == 2)
assert(vals[1] == "asdf")
assert(vals[2] == 1234)
