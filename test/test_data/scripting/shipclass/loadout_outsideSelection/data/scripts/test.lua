local assert = require("assert")

local sws = ui.ShipWepSelect

local slot = sws.Loadout_Ships[1]
assert.isFalse(slot:isValid())
assert.equals(nil, slot.ShipClassIndex)
slot.ShipClassIndex = 2
assert.isFalse(slot.Weapons:isValid())
assert.equals(nil, slot.Weapons[1])
slot.Weapons[1] = 1
assert.isFalse(slot.Amounts:isValid())
assert.equals(nil, slot.Amounts[1])
slot.Amounts[1] = 1

assert.equals(0, #sws.Loadout_Wings)
local wing = sws.Loadout_Wings[1]
assert.isFalse(wing:isValid())
assert.equals(nil, wing.Name)
local wingSlot = wing[1]
assert.isFalse(wingSlot:isValid())
assert.equals(nil, wingSlot.isShipLocked)
assert.equals(nil, wingSlot.isWeaponLocked)
assert.equals(nil, wingSlot.isDisabled)
assert.equals(nil, wingSlot.isFilled)
wingSlot.isFilled = true
assert.equals(nil, wingSlot.isPlayer)
assert.equals(nil, wingSlot.isPlayerAllowed)
assert.equals(nil, wingSlot.ShipClassIndex)
assert.equals(nil, wingSlot.Callsign)

assert.equals(nil, sws.Ship_Pool[1])
sws.Ship_Pool[1] = 2
assert.equals(nil, sws.Weapon_Pool[1])
sws.Weapon_Pool[1] = 2

sws.resetSelect()
sws.sendShipRequestPacket(3, 1, 1, 1, 1)
sws.sendWeaponRequestPacket(1, 2, 1, 1, 1)
