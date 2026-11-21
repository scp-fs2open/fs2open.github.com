local assert = require("assert")

local slot = ui.ShipWepSelect.Loadout_Ships[1]

slot.ShipClassIndex = -1
assert.equals(3, slot.Weapons[4])
assert.equals(3, slot.Amounts[4])

slot.Amounts[4] = 1
