
function pack(...)
    return {n = select("#", ...), ...}
end

local resolve_func;
local lazy_test_val = "Test"

local promise = async.promise(function (resolve)
    lazy_test_val = "NotTest"
    resolve_func = resolve
end)

assert(not promise:isResolved())

-- Check the expected behavior that the function is evaluated eagerly
assert(lazy_test_val == "NotTest")

local continuationCalled = false
local contPromise = promise:continueWith(function(testVal)
    assert(testVal == "ResolveValue")
    continuationCalled = true

    return 42, "Test"
end)

assert(not contPromise:isResolved())

-- Now kick off the chain
resolve_func("ResolveValue")

assert(continuationCalled)

assert(promise:isResolved())
assert(promise:getValue() == "ResolveValue")

assert(contPromise:isResolved())
local vals = pack(contPromise:getValue())
assert(#vals == 2)
assert(vals[1] == 42)
assert(vals[2] == "Test")

