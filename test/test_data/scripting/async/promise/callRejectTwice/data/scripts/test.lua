
local reject_func

async.promise(function(_, reject)
    reject_func = reject
end)

reject_func("Test")
reject_func("Test") -- Will cause an error
