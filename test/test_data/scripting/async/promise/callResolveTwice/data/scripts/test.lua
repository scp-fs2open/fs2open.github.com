
local resolve_func

async.promise(function(resolve)
    resolve_func = resolve
end)

resolve_func("Test")
resolve_func("Test") -- Will cause an error
