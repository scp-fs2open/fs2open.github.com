local assert = require("assert")

local slot = ui.ShipWepSelect.Loadout_Ships[1]

slot.ShipClassIndex = 4
assert.equals(4, slot.ShipClassIndex)
assert.equals(0, slot.Weapons[1])
