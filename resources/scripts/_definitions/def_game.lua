--- @meta

--- @class RenderBindings
--- @field reloadShaders fun()

--- @class GameBindings
--- @field party PartyBindings
--- @field misc MiscBindings
--- @field items ItemsBindings
--- @field serialize SerializeBindings
--- @field deserialize DeserializeBindings
--- @field render RenderBindings
--- @field PartyAlignment PartyAlignmentEnum
--- @field CharacterCondition table<string, integer>
--- @field SkillType table<string, integer>
--- @field SkillMastery table<string, integer>
--- @field ClassType table<string, integer>
--- @field ItemType table<string, integer>

--- @class SkillEntry
--- @field id integer
--- @field mastery integer
--- @field level integer

--- @class CharacterInfo
--- @field name string?
--- @field class integer?
--- @field mana integer?
--- @field maxMana integer?
--- @field hp integer?
--- @field maxHp integer?
--- @field skills table<integer, SkillEntry>
--- @field condition table<integer, boolean>

--- @class PartyBindings
--- @field getGold fun(): integer
--- @field setGold fun(amount: integer)
--- @field getFood fun(): integer
--- @field setFood fun(amount: integer)
--- @field getAlignment fun(): PartyAlignment
--- @field setAlignment fun(alignment: PartyAlignment)
--- @field givePartyXp fun(amount: integer)
--- @field getPartySize fun(): integer
--- @field getActiveCharacter fun(): integer
--- @field getCharacterInfo fun(charIndex: integer, query: table): CharacterInfo
--- @field setCharacterInfo fun(charIndex: integer, info: table)
--- @field addItemToInventory fun(charIndex: integer, itemId: integer):boolean
--- @field addCustomItemToInventory fun(charIndex: integer, item: table)
--- @field playAllCharactersAwardSound fun()
--- @field playCharacterAwardSound fun(charIndex: integer)
--- @field clearCondition fun(charIndex: integer, condition: integer?)

--- @class ItemInfo
--- @field name string
--- @field level integer

--- @class ItemsBindings
--- @field getItemInfo fun(itemId: integer):ItemInfo
--- @field getRandomItem fun(filter: fun(item: table)?):integer


--- @alias PartyAlignment integer

--- @class PartyAlignmentEnum
--- @field Good PartyAlignment
--- @field Neutral PartyAlignment
--- @field Evil PartyAlignment
