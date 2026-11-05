---@meta porla_zip

local zip = {}

---@param buffer string
---@return table<string, string>? contents # The contents of the zip file, or nil if error
---@return PoError? err # `decode_error` (`category = "libzip"`) if decoding failed
function zip.read(buffer) end

return zip
