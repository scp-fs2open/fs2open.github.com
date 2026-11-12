local assert = require("assert")

local unrestricted = tb.ShipClasses[2]

assert.equals(false, unrestricted:isWeaponAllowedOnShip(1))
assert.equals(false, unrestricted:isWeaponAllowedOnShip(1, 1))
assert.equals(false, unrestricted:isWeaponAllowedOnShip(1, 3))
