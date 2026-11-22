local assert = require("assert")

local ship = tb.ShipClasses[4]

assert.equals(false, ship:isWeaponAllowedOnShip(1))
assert.equals(false, ship:isWeaponAllowedOnShip(1, 1))
assert.equals(false, ship:isWeaponAllowedOnShip(3, 3))
