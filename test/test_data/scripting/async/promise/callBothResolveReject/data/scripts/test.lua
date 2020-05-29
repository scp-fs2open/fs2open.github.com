
local resolve_func
local reject_func

async.promise(function(resolve, reject)
    resolve_func = resolve
    reject_func = reject
end)

resolve_func("Test")
reject_func("Test") -- Will cause an error
