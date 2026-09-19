-- Colony Zod.

-- The evt file shows the movie on every exit from the map once Xenofex is dead, it sets the quest bit and never checks
-- it.
Game.MapEvtLines:RemoveEvent(2)
evt.map[2] = function()
    if not evt.Cmp("QBits", 105) and evt.CheckMonstersKilled{CheckType = 3, Id = 0, Count = 0} then -- "Slayed Xenofex"
        evt.Set("QBits", 105)
        evt.ShowMovie{DoubleSize = 1, ExitCurrentScreen = true, Name = "\"family reunion\" "} -- As the evt file spells it.
        evt.Add("History25", 0)
    end
end
events.LeaveMap = evt.map[2].last

-- The evt file hands over another key on every click on Roland's cage, it sets the quest bit and never checks it.
evt.hint[376] = evt.str[100]
Game.MapEvtLines:RemoveEvent(376)
evt.map[376] = function()
    evt.SpeakNPC(287) -- "Roland Ironfist"
    evt.SetSprite{SpriteId = 20, Visible = 1, Name = "dec05"}
    if not evt.Cmp("QBits", 240) then -- "Talked to Roland"
        evt.Add("Inventory", 661) -- "Colony Zod Key"
    end
    evt.Add("QBits", 240)
    evt.Add("History24", 0)
    evt.SetFacetBit{Id = 1, Bit = const.FacetBits.Untouchable, On = true}
    evt.SetFacetBit{Id = 1, Bit = const.FacetBits.Invisible, On = true}
end

-- A decoration's sprite isn't saved, so the cage would show Roland again after a reload. The faces are saved, but a
-- refill of the map brings them back.
function events.LoadMap()
    if evt.Cmp("QBits", 240) then
        evt.SetSprite{SpriteId = 20, Visible = 1, Name = "dec05"}
        evt.SetFacetBit{Id = 1, Bit = const.FacetBits.Untouchable, On = true}
        evt.SetFacetBit{Id = 1, Bit = const.FacetBits.Invisible, On = true}
    end
end
