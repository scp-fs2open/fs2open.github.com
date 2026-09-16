-- gamestate and gameevent handles have an isValid() C++ member but never had an explicit
-- ADE_FUNC(isValid, ...), so these exercise the automatically generated function.

local state = ba.GameStates["GS_STATE_MAIN_MENU"]
assert(type(state.isValid) == "function", "gamestate should expose isValid()")
assert(state:isValid() == true, "a real game state should be valid")

local badState = ba.GameStates[100000]
assert(badState:isValid() == false, "an out-of-range game state should be invalid")

local event = ba.GameEvents["GS_EVENT_MAIN_MENU"]
assert(type(event.isValid) == "function", "gameevent should expose isValid()")
assert(event:isValid() == true, "a real game event should be valid")

local badEvent = ba.GameEvents[100000]
assert(badEvent:isValid() == false, "an out-of-range game event should be invalid")
