local function find(key)
	for _, option in ipairs(opt.Options) do
		if option.Key == key then
			return option
		end
	end
	error("option " .. key .. " is not registered")
end

local needing = find("Graphics.VSync")
assert(needing.Requirement == "vulkan")
assert(needing.RequirementText == "Needs the Vulkan renderer")
assert(needing.RequirementMet == false)

local unjudged = find("Graphics.Gamma")
assert(unjudged.Requirement == "unknown")
assert(unjudged.RequirementMet == true)

local free = find("Input.JoystickDeadZone")
assert(free.Requirement == nil)
assert(free.RequirementText == nil)
assert(free.RequirementMet == true)
