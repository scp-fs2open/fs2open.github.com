
if not invocation then
    invocation = 0
end
invocation = invocation + 1

local assert = require("assert")

if invocation == 1 then
    assert.equals(2, #hv.Globals)
    local varNames = {}
    for i=1, #hv.Globals do
        table.insert(varNames, hv.Globals[i])
    end
    table.sort(varNames)

    assert.tablesEqual({ "Test", "Value" }, varNames)

    assert.equals("Hello World", hv.Test)
    assert.equals(1234, hv.Value)
elseif invocation == 2 then
    assert.equals(2, #hv.Globals)
    local varNames = {}
    for i=1, #hv.Globals do
        table.insert(varNames, hv.Globals[i])
    end
    table.sort(varNames)

    assert.tablesEqual({ "Test", "Value" }, varNames)

    assert.equals("Something else", hv.Test)
    assert.equals(1234, hv.Value)
elseif invocation == 3 then
    assert.equals(1, #hv.Globals)
    local varNames = {}
    for i=1, #hv.Globals do
        table.insert(varNames, hv.Globals[i])
    end
    table.sort(varNames)

    assert.tablesEqual({ "Value" }, varNames)

    assert.equals(nil, hv.Test)
    assert.equals(1234, hv.Value)
end
