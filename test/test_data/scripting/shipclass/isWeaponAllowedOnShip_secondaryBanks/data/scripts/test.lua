local assert = require("assert")

local restricted = tb.ShipClasses[1]

assert.equals(true, restricted:isWeaponAllowedOnShip(1))
assert.equals(true, restricted:isWeaponAllowedOnShip(1, 3))
assert.equals(false, restricted:isWeaponAllowedOnShip(1, 4))

assert.equals(true, restricted:isWeaponAllowedOnShip(2))
assert.equals(false, restricted:isWeaponAllowedOnShip(2, 3))
assert.equals(true, restricted:isWeaponAllowedOnShip(2, 4))

assert.equals(false, restricted:isWeaponAllowedOnShip(3))
assert.equals(false, restricted:isWeaponAllowedOnShip(3, 3))
assert.equals(false, restricted:isWeaponAllowedOnShip(3, 4))
assert.equals(false, restricted:isWeaponAllowedOnShip(1, 5))
