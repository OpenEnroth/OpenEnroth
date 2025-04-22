--- @meta

--- @alias PlatformKey integer

--- @class PlatformKeyEnum
--- @field KEY_DOWN PlatformKey
--- @field KEY_UP PlatformKey

--- @class InputBindings
--- @field PlatformKey PlatformKeyEnum

--- @class MiscBindings
--- @field goToScreen fun(screenId: integer)
--- @field canClassLearn fun(classType: ClassType, skillType: SkillType)

--- @class AudioBindings
--- @field playSound fun(soundId: integer, soundPlaybackMode: integer)
--- @field playMusic fun(musicId: integer)

--- @class RendererBindings
--- @field reloadShaders fun()

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

--- @class ConfigEntry
--- @field value string
--- @field default string
--- @field name string
--- @field section string
--- @field description string
--- @field path string
--- @field reset fun()
--- @field toggle fun()

--- @class ConfigBindings
--- @field entry fun(param1: string, param2?: string): ConfigEntry
--- @field list fun(sectionName: string, filter: string): ConfigEntry[]

--- OVERLAY Bindings
--- @class Overlay
--- @field init fun()
--- @field close fun()
--- @field update fun()

--- @class OverlayBindings
--- @field addOverlay fun(name: string, view: Overlay)
--- @field removeOverlay fun(name: string)
--- @field imgui ImGuiBindings
