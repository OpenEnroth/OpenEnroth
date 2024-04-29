-- @meta
-- This meta file is here only to extract types and definitions for Lua Language Server.

---@alias PlatformKey integer

---@class PlatformKeyEnum
---@field KEY_DOWN PlatformKey
---@field KEY_UP PlatformKey

---@class InputBindings
---@field PlatformKey PlatformKeyEnum

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
--- @field getAlignment fun(): integer
--- @field setAlignment fun(alignment: integer)
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

--- @class MiscBindings
--- @field goToScreen fun(screenId: integer)
--- @field canClassLearn fun(classType: integer, skillType: integer)

--- @class AudioBindings
--- @field playSound fun(soundId: integer, soundPlaybackMode: integer)
--- @field playMusic fun(musicId: integer)

--- @class ItemInfo
--- @field name string
--- @field level integer

--- @class ItemsBindings
--- @field getItemInfo fun(itemId: integer):ItemInfo
--- @field getRandomItem fun(filter: fun(item: table)?):integer

--- @class SerializeBindings
--- @field partyAlignment fun(alignment: integer): string

--- @class DeserializeBindings
--- @field partyAlignment fun(alignment: string): integer

--- @class RenderBindings
--- @field reloadShaders fun()

--- @class GameBindings
--- @field party PartyBindings
--- @field audio AudioBindings
--- @field misc MiscBindings
--- @field items ItemsBindings
--- @field serialize SerializeBindings
--- @field deserialize DeserializeBindings
--- @field render RenderBindings
--- @field PartyAlignment table<string, integer>
--- @field CharacterCondition table<string, integer>
--- @field SkillType table<string, integer>
--- @field SkillMastery table<string, integer>
--- @field ClassType table<string, integer>
--- @field ItemType table<string, integer>

--- @class LogBindings
--- @field info fun(message:string)
--- @field trace fun(message:string)
--- @field debug fun(message:string)
--- @field warning fun(message:string)
--- @field error fun(message:string)
--- @field critical fun(message:string)

--- @class NuklearBindings

--- @class PlatformWindow
--- @field dimensions fun(): integer, integer

--- @class PlatformBindings
--- @field window PlatformWindow

--- CONFIG Bindings
---@class ConfigBindings
---@field setConfig fun(section: string, configName: string, value: any): boolean
---@field getConfig fun(section: string, configName?: string): string

--- OVERLAY Bindings
--- @class Overlay
--- @field init fun()
--- @field close fun()
--- @field update fun(ctx: NuklearContext)

--- @class OverlayBindings
--- @field addOverlay fun(name: string, view: Overlay)
--- @field removeOverlay fun(name: string)
--- @field nk Nuklear

--- NUKLEAR Bindings
---@class NuklearContext

---@class Nuklear
---@field style_push fun(ctx: NuklearContext, element: string, styleName: string, value: any)
---@field style_pop fun(ctx: NuklearContext, element: string, styleName: string)
---@field layout_row_begin fun(ctx: NuklearContext, layout: string, height: number, columns: integer)
---@field layout_row_push fun(ctx: NuklearContext, height: number)
---@field button_label fun(ctx: NuklearContext, text: string): boolean
---@field edit_string fun(ctx: NuklearContext, options: table, text: string) : string, table
---@field layout_row_end fun(ctx: NuklearContext)
---@field window_end fun(ctx: NuklearContext)
---@field window_set_bounds fun(ctx: NuklearContext, windowName: string, bounds: Rect)
---@field window_is_hidden fun(ctx: NuklearContext, windowName: string): boolean
---@field window_begin fun(ctx: NuklearContext, windowName: string, size: Rect, options: table<string>): boolean
---@field layout_row_dynamic fun(ctx: NuklearContext, height: number, columns: integer)
---@field group_scrolled_begin fun(ctx: NuklearContext, scrollPosition: nk_scroll, groupName: string, options: table<string>): boolean
---@field label_colored fun(ctx: NuklearContext, text: string, color: table)
---@field group_scrolled_end fun(ctx: NuklearContext)
---@field checkbox_label fun(ctx: NuklearContext, text: string, isChecked: boolean): boolean
---@field window_is_hovered fun(ctx: NuklearContext): boolean
---@field EditState EditStateEnum

--- @class Rect
--- @field x number
--- @field y number
--- @field w number
--- @field h number

---@alias EditState integer

---@class EditStateEnum
---@field NK_EDIT_COMMITED EditState
---@field NK_EDIT_INACTIVE EditState
---@field NK_EDIT_ACTIVE EditState

--- TODO(Gerark) Change this by providing access through a module ( or just swap nuklear to mygui with an existing lua lib integration :D )
---@class nk_scroll
---@field new fun(x: integer, y: integer): table
---@diagnostic disable-next-line: name-style-check
nk_scroll = {}
