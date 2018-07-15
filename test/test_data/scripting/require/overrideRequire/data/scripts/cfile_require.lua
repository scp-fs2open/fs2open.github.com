

local function cfileLoader(name)
    return function()
        return "Overridden loader"
    end
end

table.insert(package.loaders, 2, cfileLoader)
