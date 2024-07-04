
local assert = require("assert")

function pack(...)
    return { n = select("#", ...), ... }
end

do
    local thenCalled = false
    local catchCalled = false

    local reject_func;
    local finalPromise = async.promise(function(_, reject)
        reject_func = reject
    end):continueWith(function(resolveVals)
        thenCalled = true

        return resolveVals + 20
    end):catch(function(errorVals)
        catchCalled = true

        return errorVals + 10
    end)

    reject_func(20)

    assert.isFalse(thenCalled)
    assert.isTrue(catchCalled)

    assert.isFalse(finalPromise:isErrored())
    assert.equals(finalPromise:getValue(), 30)
end

do
    local thenCalled = false
    local catchCalled = false

    local resolveFunc;
    local finalPromise = async.promise(function(resolve, _)
        resolveFunc = resolve
    end):continueWith(function(resolveVals)
        thenCalled = true

        return resolveVals + 20
    end):catch(function(errorVals)
        catchCalled = true

        return errorVals + 10
    end)

    resolveFunc(20)

    assert.isTrue(thenCalled)
    assert.isFalse(catchCalled)

    assert.isTrue(finalPromise:isResolved())
    assert.equals(finalPromise:getValue(), 40)
end
