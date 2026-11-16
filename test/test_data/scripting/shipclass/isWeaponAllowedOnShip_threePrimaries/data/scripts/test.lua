local assert = require("assert")

local ship = tb.ShipClasses[3]

assert.equals(true, ship:isWeaponAllowedOnShip(1, 1))
assert.equals(true, ship:isWeaponAllowedOnShip(3, 4))
assert.equals(false, ship:isWeaponAllowedOnShip(4, 4))
