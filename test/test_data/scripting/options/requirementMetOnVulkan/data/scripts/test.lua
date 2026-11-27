local function find(key)
	for _, option in ipairs(opt.Options) do
		if option.Key == key then
			return option
		end
	end
	error("option " .. key .. " is not registered")
end

-- the test runs as Vulkan on the stub renderer, which reports no ray tracing hardware
assert(find("Graphics.VSync").RequirementMet == true)
assert(find("Graphics.Gamma").RequirementMet == false)
