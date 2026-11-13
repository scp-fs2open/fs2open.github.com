local assert = require("assert")

local ship = tb.ShipClasses[1]

assert.equals(true, ship:isWeaponAllowedOnShip(2))
assert.equals(true, ship:isWeaponAllowedOnShip(3))

assert.equals(true, ship:isWeaponAllowedOnShip(1, 1))
assert.equals(false, ship:isWeaponAllowedOnShip(2, 1))
assert.equals(true, ship:isWeaponAllowedOnShip(2, 2))

assert.equals(true, ship:isWeaponAllowedOnShip(3, 3))
assert.equals(false, ship:isWeaponAllowedOnShip(4, 3))
assert.equals(false, ship:isWeaponAllowedOnShip(3, 4))
assert.equals(true, ship:isWeaponAllowedOnShip(4, 4))

assert.equals(false, ship:isWeaponAllowedOnShip(3, 5))
