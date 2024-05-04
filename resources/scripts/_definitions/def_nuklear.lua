---@meta

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
