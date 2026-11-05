---@meta porla_presets

local presets = {}

---@return PoPreset? # nil when no preset is the default
function presets.default() end

---@param id integer
---@return PoPreset? # nil when no preset has this id
---@overload fun(name: string): PoPreset?
function presets.get(id) end

---A snapshot, sorted by name.
---@return PoPreset[]
function presets.list() end

return presets
