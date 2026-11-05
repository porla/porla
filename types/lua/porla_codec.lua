---@meta porla_codec

local codec = {}
codec.json = {}
codec.hex  = {}

---@param data string
---@return any? value # Decoded value, or nil on error
---@return PoError? err # `decode_error`, or nil on success
function codec.json.decode(data) end

---@return string? value # Decoded bytes, or nil on error
---@return PoError? err # `decode_error`, or nil on success
function codec.hex.decode(data) end

---@param data string
---@return string
function codec.hex.encode(data) end

return codec
