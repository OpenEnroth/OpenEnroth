-- Celeste.

-- Falling off Celeste lands the party at one of six spots in the Bracada Desert. The evt file has no Exit after any
-- of them, so each spot is followed by all the ones after it, and the first spot doesn't name the map.
local spots = { { 8146, 4379 }, { -2815, 1288 }, { -11883, 8667 }, { -22231, 13145 }, { -12770, 18344 }, { 9185, 18564 } }

evt.hint[451] = evt.str[100]
Game.MapEvtLines:RemoveEvent(451)
evt.map[451] = function()
    local spot = spots[Game.Rand() % 6 + 1]
    evt.MoveToMap{X = spot[1], Y = spot[2], Z = 3700, Name = "out06.odm"}
end
