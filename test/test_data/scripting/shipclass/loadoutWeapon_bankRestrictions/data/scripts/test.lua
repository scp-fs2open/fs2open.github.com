local assert = require("assert")

local slot = ui.ShipWepSelect.Loadout_Ships[1]

slot.Weapons[4] = 3
assert.equals(3, slot.Weapons[4])
assert.equals(0, slot.Amounts[4])

slot.Weapons[5] = 3
assert.equals(0, slot.Weapons[5])
assert.equals(-1, slot.Amounts[5])

slot.Weapons[1] = 2
assert.equals(0, slot.Weapons[1])
slot.Weapons[1] = 1
assert.equals(1, slot.Weapons[1])

local unrestricted = ui.ShipWepSelect.Loadout_Ships[2]
unrestricted.Weapons[1] = 1
assert.equals(0, unrestricted.Weapons[1])
assert.equals(-1, unrestricted.Amounts[1])

local outside_class_list = ui.ShipWepSelect.Loadout_Ships[3]
outside_class_list.Weapons[1] = 1
assert.equals(1, outside_class_list.Weapons[1])
outside_class_list.Weapons[1] = 2
assert.equals(0, outside_class_list.Weapons[1])
assert.equals(-1, outside_class_list.Amounts[1])
