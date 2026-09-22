---@meta porla_codec

local codec = {}
codec.json = {}

---@param data string
---@return any? value # Decoded value, or nil on error
---@return string? err # Parse error message, or nil on success
function codec.json.decode(data) end

return codec
