local assert = require("assert")

local ship = tb.ShipClasses[2]

assert.equals(true, ship:isWeaponAllowedOnShip(3, 2))
assert.equals(false, ship:isWeaponAllowedOnShip(4, 2))
assert.equals(true, ship:isWeaponAllowedOnShip(4, 4))
assert.equals(true, ship:isWeaponAllowedOnShip(3, 3))
assert.equals(false, ship:isWeaponAllowedOnShip(3, 4))

assert.equals(true, ship:isWeaponAllowedOnShip(1, 1))
assert.equals(false, ship:isWeaponAllowedOnShip(2, 1))
assert.equals(false, ship:isWeaponAllowedOnShip(2))
