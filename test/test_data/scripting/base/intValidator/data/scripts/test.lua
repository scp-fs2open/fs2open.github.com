-- shipclass is an int-index handle whose validity comes from ADE_OBJ_VALIDATOR.  Headless there are no
-- ship classes, so any index yields an invalid handle.

local sc = tb.ShipClasses[1]
assert(sc ~= nil, "an invalid handle should still be returned when the nil-instead-of-invalid option is off")
assert(type(sc.isValid) == "function", "shipclass should expose isValid()")
assert(sc:isValid() == false, "an out-of-range ship class should be invalid")
